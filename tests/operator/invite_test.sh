#!/bin/bash
# tests/operator/invite_test.sh

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

echo "Test de la commande INVITE..."

# Créer les fifos pour les clients
mkfifo /tmp/op_in /tmp/op_out
mkfifo /tmp/user_in /tmp/user_out

# Démarrer les clients
nc localhost $PORT < /tmp/op_in > /tmp/op_out &
OP_PID=$!
nc localhost $PORT < /tmp/user_in > /tmp/user_out &
USER_PID=$!

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

send_to_client "user" "PASS $PASSWORD"
send_to_client "user" "NICK regularuser"
send_to_client "user" "USER user user user :Regular User"

# Attendre l'authentification
sleep 2

# Test 1: Opérateur crée un canal en mode invite-only
echo "Test 1: Opérateur crée un canal en mode invite-only"
send_to_client "op" "JOIN #invitetest"
sleep 1
cat /tmp/op_out > /dev/null

send_to_client "op" "MODE #invitetest +i"
sleep 1

op_output=$(cat /tmp/op_out)
if echo "$op_output" | grep -q "MODE #invitetest +i"; then
    echo "✅ Test 1 réussi: Canal en mode invite-only"
else
    echo "❌ Test 1 échoué: Impossible de définir le mode invite-only"
    echo "$op_output"
    kill $OP_PID $USER_PID $SERVER_PID
    rm -f /tmp/op_in /tmp/op_out /tmp/user_in /tmp/user_out
    exit 1
fi

# Test 2: Utilisateur ne peut pas rejoindre le canal
echo "Test 2: Utilisateur ne peut pas rejoindre le canal"
send_to_client "user" "JOIN #invitetest"
sleep 1

user_output=$(cat /tmp/user_out)
if echo "$user_output" | grep -q "473"; then
    echo "✅ Test 2 réussi: Accès refusé (invite-only)"
else
    echo "❌ Test 2 échoué: Utilisateur a pu rejoindre un canal invite-only"
    echo "$user_output"
    kill $OP_PID $USER_PID $SERVER_PID
    rm -f /tmp/op_in /tmp/op_out /tmp/user_in /tmp/user_out
    exit 1
fi

# Test 3: Opérateur invite l'utilisateur
echo "Test 3: Opérateur invite l'utilisateur"
send_to_client "op" "INVITE regularuser #invitetest"
sleep 1

op_output=$(cat /tmp/op_out)
user_output=$(cat /tmp/user_out)

if echo "$op_output" | grep -q "341" && echo "$user_output" | grep -q "INVITE"; then
    echo "✅ Test 3 réussi: Invitation envoyée et reçue"
else
    echo "❌ Test 3 échoué: Problème avec l'invitation"
    echo "Op output: $op_output"
    echo "User output: $user_output"
    kill $OP_PID $USER_PID $SERVER_PID
    rm -f /tmp/op_in /tmp/op_out /tmp/user_in /tmp/user_out
    exit 1
fi

# Test 4: Utilisateur peut maintenant rejoindre le canal
echo "Test 4: Utilisateur peut maintenant rejoindre le canal"
send_to_client "user" "JOIN #invitetest"
sleep 1

user_output=$(cat /tmp/user_out)
if echo "$user_output" | grep -q "JOIN #invitetest"; then
    echo "✅ Test 4 réussi: Utilisateur a rejoint le canal après invitation"
else
    echo "❌ Test 4 échoué: Impossible de rejoindre malgré l'invitation"
    echo "$user_output"
    kill $OP_PID $USER_PID $SERVER_PID
    rm -f /tmp/op_in /tmp/op_out /tmp/user_in /tmp/user_out
    exit 1
fi

# Test 5: Utilisateur non-opérateur ne peut pas inviter
echo "Test 5: Utilisateur non-opérateur ne peut pas inviter"
send_to_client "op" "PART #invitetest"
sleep 1
cat /tmp/op_out > /dev/null

send_to_client "user" "INVITE operator #invitetest"
sleep 1

user_output=$(cat /tmp/user_out)
if echo "$user_output" | grep -q "482"; then
    echo "✅ Test 5 réussi: Non-opérateur ne peut pas inviter"
else
    echo "❌ Test 5 échoué: Non-opérateur a pu inviter ou erreur inattendue"
    echo "$user_output"
    kill $OP_PID $USER_PID $SERVER_PID
    rm -f /tmp/op_in /tmp/op_out /tmp/user_in /tmp/user_out
    exit 1
fi

# Nettoyage
kill $OP_PID $USER_PID $SERVER_PID
rm -f /tmp/op_in /tmp/op_out /tmp/user_in /tmp/user_out
echo "Tous les tests d'INVITE ont réussi!"
exit 0