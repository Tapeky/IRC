#!/bin/bash
# tests/operator/mode_test.sh

PORT=${1:-6667}
PASSWORD=${2:-"testpassword"}

# Démarrer le serveur - NOUVELLE MÉTHODE
if [[ "$0" == /* ]]; then
    # Chemin absolu
    SCRIPT_DIR=$(dirname "$0")
else
    # Chemin relatif
    SCRIPT_DIR=$(dirname "$(pwd)/$0")
fi
ROOT_DIR=$(cd "$SCRIPT_DIR/../.." && pwd)

cd "$ROOT_DIR"
./ircserv $PORT $PASSWORD > /dev/null 2>&1 &
SERVER_PID=$!

sleep 1

echo "Test de la commande MODE..."

# Créer les fifos pour les clients
mkfifo /tmp/op_in /tmp/op_out
mkfifo /tmp/user1_in /tmp/user1_out
mkfifo /tmp/user2_in /tmp/user2_out

# Démarrer les clients
nc localhost $PORT < /tmp/op_in > /tmp/op_out &
OP_PID=$!
nc localhost $PORT < /tmp/user1_in > /tmp/user1_out &
USER1_PID=$!
nc localhost $PORT < /tmp/user2_in > /tmp/user2_out &
USER2_PID=$!

# Fonction pour envoyer des commandes
send_to_client() {
    local client=$1
    local message=$2
    echo -e "$message\r" > /tmp/${client}_in
    sleep 0.5
}

# Authentifier les clients
send_to_client "op" "PASS $PASSWORD"
send_to_client "op" "NICK operator"
send_to_client "op" "USER op op op :Channel Operator"

send_to_client "user1" "PASS $PASSWORD"
send_to_client "user1" "NICK user1"
send_to_client "user1" "USER user1 user1 user1 :Regular User 1"

send_to_client "user2" "PASS $PASSWORD"
send_to_client "user2" "NICK user2"
send_to_client "user2" "USER user2 user2 user2 :Regular User 2"

# Attendre l'authentification
sleep 2

# Test 1: Opérateur crée un canal et définit le mode invite-only
echo "Test 1: Opérateur crée un canal et définit le mode invite-only"
send_to_client "op" "JOIN #modetest"
sleep 1
cat /tmp/op_out > /dev/null

send_to_client "op" "MODE #modetest +i"
sleep 1

op_output=$(cat /tmp/op_out)
if echo "$op_output" | grep -q "MODE #modetest +i"; then
    echo "✅ Test 1 réussi: Mode invite-only défini"
else
    echo "❌ Test 1 échoué: Impossible de définir le mode invite-only"
    echo "$op_output"
    kill $OP_PID $USER1_PID $USER2_PID $SERVER_PID
    rm -f /tmp/op_in /tmp/op_out /tmp/user1_in /tmp/user1_out /tmp/user2_in /tmp/user2_out
    exit 1
fi

# Test 2: Utilisateur ne peut pas rejoindre un canal invite-only
echo "Test 2: Utilisateur ne peut pas rejoindre un canal invite-only"
send_to_client "user1" "JOIN #modetest"
sleep 1

user1_output=$(cat /tmp/user1_out)
if echo "$user1_output" | grep -q "473"; then
    echo "✅ Test 2 réussi: Utilisateur ne peut pas rejoindre le canal"
else
    echo "❌ Test 2 échoué: Utilisateur a pu rejoindre un canal invite-only"
    echo "$user1_output"
    kill $OP_PID $USER1_PID $USER2_PID $SERVER_PID
    rm -f /tmp/op_in /tmp/op_out /tmp/user1_in /tmp/user1_out /tmp/user2_in /tmp/user2_out
    exit 1
fi

# Test 3: Opérateur donne des privilèges d'opérateur à un utilisateur
echo "Test 3: Opérateur invite et donne des privilèges à un utilisateur"
send_to_client "op" "INVITE user1 #modetest"
sleep 1
send_to_client "user1" "JOIN #modetest"
sleep 1
cat /tmp/user1_out > /dev/null

send_to_client "op" "MODE #modetest +o user1"
sleep 1

op_output=$(cat /tmp/op_out)
user1_output=$(cat /tmp/user1_out)

if echo "$op_output" | grep -q "MODE #modetest +o user1" && 
   (echo "$user1_output" | grep -q "MODE #modetest +o user1" || echo "$user1_output" | grep -q "@user1"); then
    echo "✅ Test 3 réussi: Privilèges d'opérateur accordés"
else
    echo "❌ Test 3 échoué: Impossible d'accorder les privilèges d'opérateur"
    echo "Op output: $op_output"
    echo "User1 output: $user1_output"
    kill $OP_PID $USER1_PID $USER2_PID $SERVER_PID
    rm -f /tmp/op_in /tmp/op_out /tmp/user1_in /tmp/user1_out /tmp/user2_in /tmp/user2_out
    exit 1
fi

# Test 4: Définir un mot de passe de canal
echo "Test 4: Définir un mot de passe de canal"
send_to_client "op" "MODE #modetest -i"  # Désactiver invite-only
sleep 1
send_to_client "op" "MODE #modetest +k secretpass"
sleep 1

op_output=$(cat /tmp/op_out)
if echo "$op_output" | grep -q "MODE #modetest +k"; then
    echo "✅ Test 4 réussi: Mot de passe de canal défini"
else
    echo "❌ Test 4 échoué: Impossible de définir le mot de passe"
    echo "$op_output"
    kill $OP_PID $USER1_PID $USER2_PID $SERVER_PID
    rm -f /tmp/op_in /tmp/op_out /tmp/user1_in /tmp/user1_out /tmp/user2_in /tmp/user2_out
    exit 1
fi

# Test 5: Utilisateur doit fournir le mot de passe pour rejoindre
echo "Test 5: Utilisateur doit fournir le mot de passe"
send_to_client "user2" "JOIN #modetest"
sleep 1
user2_output=$(cat /tmp/user2_out)

if echo "$user2_output" | grep -q "475"; then
    echo "✅ Test 5 réussi: Accès refusé sans mot de passe"
else
    echo "❌ Test 5 échoué: Utilisateur a pu rejoindre sans mot de passe"
    echo "$user2_output"
    kill $OP_PID $USER1_PID $USER2_PID $SERVER_PID
    rm -f /tmp/op_in /tmp/op_out /tmp/user1_in /tmp/user1_out /tmp/user2_in /tmp/user2_out
    exit 1
fi

send_to_client "user2" "JOIN #modetest secretpass"
sleep 1
user2_output=$(cat /tmp/user2_out)

if echo "$user2_output" | grep -q "JOIN #modetest"; then
    echo "✅ Test 5 suite: Accès autorisé avec mot de passe"
else
    echo "❌ Test 5 suite: Impossible de rejoindre même avec mot de passe"
    echo "$user2_output"
    kill $OP_PID $USER1_PID $USER2_PID $SERVER_PID
    rm -f /tmp/op_in /tmp/op_out /tmp/user1_in /tmp/user1_out /tmp/user2_in /tmp/user2_out
    exit 1
fi

# Nettoyage
kill $OP_PID $USER1_PID $USER2_PID $SERVER_PID
rm -f /tmp/op_in /tmp/op_out /tmp/user1_in /tmp/user1_out /tmp/user2_in /tmp/user2_out
echo "Tous les tests de MODE ont réussi!"
exit 0