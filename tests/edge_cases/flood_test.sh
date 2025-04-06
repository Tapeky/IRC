#!/bin/bash
# tests/edge_cases/flood_test.sh

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
    rm -f /tmp/flood_test_*_$$
}
trap cleanup EXIT INT TERM

cd "$ROOT_DIR"
echo "Démarrage du serveur..."
./ircserv $PORT $PASSWORD > /dev/null 2>&1 &
SERVER_PID=$!
sleep 2

echo "Test de surcharge du serveur..."

# Fonction pour créer un client qui envoie beaucoup de messages
generate_flood_messages() {
    local client_num=$1
    local message_count=$2
    
    echo "PASS $PASSWORD" > /tmp/flood_test_flood${client_num}_$$
    echo "NICK flood${client_num}" >> /tmp/flood_test_flood${client_num}_$$
    echo "USER flood${client_num} flood${client_num} flood${client_num} :Flood Tester" >> /tmp/flood_test_flood${client_num}_$$
    echo "JOIN #flood" >> /tmp/flood_test_flood${client_num}_$$
    
    # Générer beaucoup de messages
    for ((i=1; i<=$message_count; i++)); do
        echo "PRIVMSG #flood :Message $i from client $client_num" >> /tmp/flood_test_flood${client_num}_$$
    done
}

# Préparer les clients d'écoute
echo "PASS $PASSWORD" > /tmp/flood_test_listener1_$$
echo "NICK listener1" >> /tmp/flood_test_listener1_$$
echo "USER listener1 host1 serv1 :Listener 1" >> /tmp/flood_test_listener1_$$
echo "JOIN #flood" >> /tmp/flood_test_listener1_$$

# Démarrer le client d'écoute qui va recevoir les messages
(nc localhost $PORT < /tmp/flood_test_listener1_$$ > /tmp/flood_test_listener1_out_$$ & echo $! > /tmp/flood_test_listener1_pid_$$; sleep 30) &
LISTENER_PID=$(cat /tmp/flood_test_listener1_pid_$$)

# Attendre que le client rejoigne le canal
sleep 3

# Test 1: Un seul client qui envoie beaucoup de messages
echo "Test 1: Un client envoie 50 messages"
generate_flood_messages 1 50
nc -w 5 localhost $PORT < /tmp/flood_test_flood1_$$ > /dev/null

# Attendre que tous les messages soient traités
sleep 5

# Vérifier que le client d'écoute a reçu des messages
listener_output=$(cat /tmp/flood_test_listener1_out_$$)
message_count=$(echo "$listener_output" | grep -c "PRIVMSG #flood")

echo "Le client d'écoute a reçu $message_count messages"

if [ $message_count -gt 0 ]; then
    echo "✅ Test 1 réussi: Listener a reçu $message_count messages"
else
    echo "❌ Test 1 échoué: Aucun message reçu"
    cat /tmp/flood_test_listener1_out_$$
    exit 1
fi

# Test 2: Plusieurs clients qui envoient simultanément
echo "Test 2: 3 clients envoient simultanément 20 messages chacun"

# Réinitialiser le client d'écoute
kill $LISTENER_PID 2>/dev/null || true
(nc localhost $PORT < /tmp/flood_test_listener1_$$ > /tmp/flood_test_listener2_out_$$ & echo $! > /tmp/flood_test_listener2_pid_$$; sleep 30) &
LISTENER_PID=$(cat /tmp/flood_test_listener2_pid_$$)
sleep 3

# Générer les messages pour 3 clients différents
generate_flood_messages 2 20
generate_flood_messages 3 20
generate_flood_messages 4 20

# Lancer les 3 clients en parallèle
(nc -w 5 localhost $PORT < /tmp/flood_test_flood2_$$ > /dev/null) &
(nc -w 5 localhost $PORT < /tmp/flood_test_flood3_$$ > /dev/null) &
(nc -w 5 localhost $PORT < /tmp/flood_test_flood4_$$ > /dev/null) &

# Attendre que tous les messages soient traités
sleep 10

# Vérifier que le client d'écoute a reçu des messages
listener_output=$(cat /tmp/flood_test_listener2_out_$$)
message_count=$(echo "$listener_output" | grep -c "PRIVMSG #flood")

echo "Le client d'écoute a reçu $message_count messages"

if [ $message_count -gt 30 ]; then
    echo "✅ Test 2 réussi: Listener a reçu $message_count messages"
else
    echo "❌ Test 2 échoué: Pas assez de messages reçus"
    cat /tmp/flood_test_listener2_out_$$
    exit 1
fi

echo "Tous les tests de surcharge ont réussi!"
exit 0