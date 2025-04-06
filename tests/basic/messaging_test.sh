#!/bin/bash
# tests/basic/messaging_test.sh
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
    rm -f /tmp/irc_test_*_$$
}
trap cleanup EXIT INT TERM

cd "$ROOT_DIR"
echo "Démarrage du serveur..."
./ircserv $PORT $PASSWORD > /dev/null 2>&1 &
SERVER_PID=$!
sleep 2

# Vérifier si le serveur a démarré
if ! ps -p $SERVER_PID > /dev/null; then
    echo "❌ Le serveur n'a pas démarré."
    exit 1
fi
echo "Serveur démarré avec PID $SERVER_PID"

echo "Test de messagerie..."

# Approche alternative: utiliser des processus séquentiels plutôt que des FIFOs

# Client 1: Authentification
echo "1. Authentification du client1..."
CLIENT1_AUTH=$(printf "PASS %s\r\nNICK client1\r\nUSER user1 host1 server1 :Real Name 1\r\n" "$PASSWORD" | nc -w 2 localhost $PORT)
if ! echo "$CLIENT1_AUTH" | grep -q "001"; then
    echo "❌ Authentification échouée pour client1"
    echo "$CLIENT1_AUTH"
    exit 1
fi
echo "✅ Client1 authentifié"

# Client 2: Authentification
echo "2. Authentification du client2..."
CLIENT2_AUTH=$(printf "PASS %s\r\nNICK client2\r\nUSER user2 host2 server2 :Real Name 2\r\n" "$PASSWORD" | nc -w 2 localhost $PORT)
if ! echo "$CLIENT2_AUTH" | grep -q "001"; then
    echo "❌ Authentification échouée pour client2"
    echo "$CLIENT2_AUTH"
    exit 1
fi
echo "✅ Client2 authentifié"

# Test 1: Message privé entre utilisateurs
echo "3. Test de message privé..."
# Démarrer client2 en arrière-plan pour écouter
echo "PASS $PASSWORD" > /tmp/irc_test_client2_$$
echo "NICK client2" >> /tmp/irc_test_client2_$$
echo "USER user2 host2 server2 :Real Name 2" >> /tmp/irc_test_client2_$$
# Ajouter un sleep pour attendre le message
(nc localhost $PORT < /tmp/irc_test_client2_$$ > /tmp/irc_test_output2_$$ & echo $! > /tmp/irc_test_pid2_$$; sleep 5) &

# Attendre que client2 soit connecté
sleep 2

# Client1 envoie un message à client2
echo "4. Envoi du message privé..."
TEST_MESSAGE="Hello from client1 to client2"
CLIENT1_MSG=$(printf "PASS %s\r\nNICK client1\r\nUSER user1 host1 server1 :Real Name 1\r\nPRIVMSG client2 :%s\r\n" "$PASSWORD" "$TEST_MESSAGE" | nc -w 2 localhost $PORT)

# Attendre que le message soit reçu
sleep 2

# Arrêter client2
kill $(cat /tmp/irc_test_pid2_$$) 2>/dev/null || true

# Vérifier si client2 a reçu le message
CLIENT2_OUTPUT=$(cat /tmp/irc_test_output2_$$)
echo "Réponse de client2:"
echo "$CLIENT2_OUTPUT"

if echo "$CLIENT2_OUTPUT" | grep -q "$TEST_MESSAGE"; then
    echo "✅ Test 1 réussi: Message privé reçu"
else
    echo "❌ Test 1 échoué: Message privé non reçu"
    exit 1
fi

# Test 2: Message de canal
echo "5. Test de message dans un canal..."

# Créer un canal et joindre les deux clients
echo "PASS $PASSWORD" > /tmp/irc_test_join1_$$
echo "NICK client1" >> /tmp/irc_test_join1_$$
echo "USER user1 host1 server1 :Real Name 1" >> /tmp/irc_test_join1_$$
echo "JOIN #testchannel" >> /tmp/irc_test_join1_$$
nc -w 2 localhost $PORT < /tmp/irc_test_join1_$$ > /dev/null

echo "PASS $PASSWORD" > /tmp/irc_test_join2_$$
echo "NICK client2" >> /tmp/irc_test_join2_$$
echo "USER user2 host2 server2 :Real Name 2" >> /tmp/irc_test_join2_$$
echo "JOIN #testchannel" >> /tmp/irc_test_join2_$$
nc -w 2 localhost $PORT < /tmp/irc_test_join2_$$ > /dev/null

# Démarrer client2 pour écouter les messages du canal
echo "PASS $PASSWORD" > /tmp/irc_test_channel2_$$
echo "NICK client2" >> /tmp/irc_test_channel2_$$
echo "USER user2 host2 server2 :Real Name 2" >> /tmp/irc_test_channel2_$$
echo "JOIN #testchannel" >> /tmp/irc_test_channel2_$$
(nc localhost $PORT < /tmp/irc_test_channel2_$$ > /tmp/irc_test_channel_output2_$$ & echo $! > /tmp/irc_test_channel_pid2_$$; sleep 5) &

sleep 2

# Client1 envoie un message au canal
CHANNEL_MESSAGE="Hello everyone in #testchannel!"
echo "PASS $PASSWORD" > /tmp/irc_test_channel1_$$
echo "NICK client1" >> /tmp/irc_test_channel1_$$
echo "USER user1 host1 server1 :Real Name 1" >> /tmp/irc_test_channel1_$$
echo "JOIN #testchannel" >> /tmp/irc_test_channel1_$$
echo "PRIVMSG #testchannel :$CHANNEL_MESSAGE" >> /tmp/irc_test_channel1_$$
nc -w 2 localhost $PORT < /tmp/irc_test_channel1_$$ > /dev/null

sleep 2

# Arrêter client2
kill $(cat /tmp/irc_test_channel_pid2_$$) 2>/dev/null || true

# Vérifier si client2 a reçu le message du canal
CHANNEL_OUTPUT=$(cat /tmp/irc_test_channel_output2_$$)
echo "Réponse de client2 pour le canal:"
echo "$CHANNEL_OUTPUT"

if echo "$CHANNEL_OUTPUT" | grep -q "$CHANNEL_MESSAGE"; then
    echo "✅ Test 2 réussi: Message de canal reçu"
else
    echo "❌ Test 2 échoué: Message de canal non reçu"
    exit 1
fi

echo "Tous les tests de messagerie ont réussi!"
exit 0