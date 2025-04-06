#!/bin/bash
# tests/edge_cases/disconnect_test.sh

PORT=${1:-6667}
PASSWORD=${2:-"testpassword"}

# Démarrer le serveur
if [[ "$0" == /* ]]; then
    SCRIPT_DIR=$(dirname "$0")
else
    SCRIPT_DIR=$(dirname "$(pwd)/$0")
fi
ROOT_DIR=$(cd "$SCRIPT_DIR/../.." && pwd)

# Fonction de nettoyage
cleanup() {
    echo "Nettoyage des ressources..."
    [[ -n "$SERVER_PID" ]] && kill $SERVER_PID 2>/dev/null || true
    [[ -n "$CLIENT1_PID" ]] && kill $CLIENT1_PID 2>/dev/null || true
    [[ -n "$CLIENT2_PID" ]] && kill $CLIENT2_PID 2>/dev/null || true
    rm -f /tmp/irc_test_disc_*_$$
}
trap cleanup EXIT INT TERM

cd "$ROOT_DIR"
echo "Démarrage du serveur..."
./ircserv $PORT $PASSWORD > /dev/null 2>&1 &
SERVER_PID=$!
sleep 2

echo "Test de déconnexion imprévue..."

# Préparation des fichiers d'entrée pour les clients
echo "PASS $PASSWORD" > /tmp/irc_test_disc_client1_$$
echo "NICK disconnecter" >> /tmp/irc_test_disc_client1_$$
echo "USER disc disc disc :Disconnecting User" >> /tmp/irc_test_disc_client1_$$
echo "JOIN #testdisconnect" >> /tmp/irc_test_disc_client1_$$

echo "PASS $PASSWORD" > /tmp/irc_test_disc_client2_$$
echo "NICK witness" >> /tmp/irc_test_disc_client2_$$
echo "USER witness witness witness :Witness User" >> /tmp/irc_test_disc_client2_$$
echo "JOIN #testdisconnect" >> /tmp/irc_test_disc_client2_$$

# Lancer le client qui va se déconnecter brutalement
nc localhost $PORT < /tmp/irc_test_disc_client1_$$ > /dev/null &
CLIENT1_PID=$!

# Lancer le client témoin qui restera connecté
(nc localhost $PORT < /tmp/irc_test_disc_client2_$$ > /tmp/irc_test_disc_output2_$$ & echo $! > /tmp/irc_test_disc_pid2_$$; sleep 30) &
CLIENT2_PID=$(cat /tmp/irc_test_disc_pid2_$$)

# Attendre que les deux clients rejoignent le canal
sleep 5

echo "Test 1: Déconnexion brutale d'un client"
# Envoyer un message pour prouver que le client est actif
echo "PRIVMSG #testdisconnect :I'm about to disconnect" | nc -w 1 localhost $PORT -p 1234 > /dev/null

# Vérifier que le témoin reçoit le message
sleep 2
witness_output=$(cat /tmp/irc_test_disc_output2_$$)
if echo "$witness_output" | grep -q "JOIN #testdisconnect"; then
    echo "✅ Communication initiale réussie"
else
    echo "❌ Échec de la communication initiale"
    echo "$witness_output"
    exit 1
fi

# Tuer brutalement le client qui se déconnecte
echo "Déconnexion brutale du client 1 (PID: $CLIENT1_PID)..."
kill -9 $CLIENT1_PID
sleep 3

# Vérifier que le serveur détecte la déconnexion
NEW_OUTPUT=$(cat /tmp/irc_test_disc_output2_$$ | grep -A 100 "JOIN #testdisconnect")
echo "Sortie du témoin après déconnexion:"
echo "$NEW_OUTPUT"

if echo "$NEW_OUTPUT" | grep -q "QUIT"; then
    echo "✅ Test 1 réussi: Le serveur a détecté la déconnexion brutale"
else
    echo "❌ Test 1 échoué: Le serveur n'a pas détecté la déconnexion"
    exit 1
fi

# Test 2: Le serveur reste opérationnel après la déconnexion
echo "Test 2: Le serveur reste opérationnel après la déconnexion"

# Connexion d'un nouveau client pour vérifier que le serveur accepte encore des connexions
echo "PASS $PASSWORD" > /tmp/irc_test_disc_new_$$
echo "NICK newcomer" >> /tmp/irc_test_disc_new_$$
echo "USER new new new :New User" >> /tmp/irc_test_disc_new_$$
echo "JOIN #testdisconnect" >> /tmp/irc_test_disc_new_$$

NEW_OUTPUT=$(nc -w 3 localhost $PORT < /tmp/irc_test_disc_new_$$)

if echo "$NEW_OUTPUT" | grep -q "JOIN #testdisconnect"; then
    echo "✅ Test 2 réussi: Le serveur accepte encore des connexions"
else
    echo "❌ Test 2 échoué: Le serveur ne fonctionne plus correctement"
    echo "$NEW_OUTPUT"
    exit 1
fi

echo "Tous les tests de déconnexion ont réussi!"
exit 0