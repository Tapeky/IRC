#!/bin/bash
# tests/operator/kick_test.sh

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

echo "Test de la commande KICK..."

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

# Test 1: Opérateur crée un canal (devient automatiquement op)
echo "Test 1: Opérateur crée un canal"
send_to_client "op" "JOIN #kicktest"
sleep 1

op_output=$(cat /tmp/op_out)
if echo "$op_output" | grep -q "JOIN #kicktest"; then
    echo "✅ Test 1 réussi: Canal créé"
else
    echo "❌ Test 1 échoué: Impossible de créer le canal"
    kill $OP_PID $USER_PID $SERVER_PID
    rm -f /tmp/op_in /tmp/op_out /tmp/user_in /tmp/user_out
    exit 1
fi

# Vider les buffers
cat /tmp/op_out > /dev/null
cat /tmp/user_out > /dev/null

# Test 2: Utilisateur normal rejoint le canal
echo "Test 2: Utilisateur rejoint le canal"
send_to_client "user" "JOIN #kicktest"
sleep 1

user_output=$(cat /tmp/user_out)
if echo "$user_output" | grep -q "JOIN #kicktest"; then
    echo "✅ Test 2 réussi: Utilisateur a rejoint le canal"
else
    echo "❌ Test 2 échoué: Utilisateur n'a pas pu rejoindre"
    kill $OP_PID $USER_PID $SERVER_PID
    rm -f /tmp/op_in /tmp/op_out /tmp/user_in /tmp/user_out
    exit 1
fi

# Vider les buffers
cat /tmp/op_out > /dev/null
cat /tmp/user_out > /dev/null

# Test 3: Opérateur expulse l'utilisateur
echo "Test 3: Opérateur expulse l'utilisateur"
send_to_client "op" "KICK #kicktest regularuser :You are kicked!"
sleep 1

# Vérifier que l'opérateur a reçu confirmation
op_output=$(cat /tmp/op_out)
user_output=$(cat /tmp/user_out)

if echo "$op_output" | grep -q "KICK #kicktest regularuser" && echo "$user_output" | grep -q "KICK #kicktest regularuser"; then
    echo "✅ Test 3 réussi: Utilisateur expulsé"
else
    echo "❌ Test 3 échoué: Expulsion échouée"
    echo "Output opérateur: $op_output"
    echo "Output utilisateur: $user_output"
    kill $OP_PID $USER_PID $SERVER_PID
    rm -f /tmp/op_in /tmp/op_out /tmp/user_in /tmp/user_out
    exit 1
fi

# Test 4: Utilisateur non-opérateur ne peut pas expulser
echo "Test 4: Utilisateur non-opérateur tente d'expulser"
send_to_client "user" "JOIN #kicktest"
sleep 1
cat /tmp/user_out > /dev/null

send_to_client "user" "KICK #kicktest operator :Revenge!"
sleep 1

user_output=$(cat /tmp/user_out)
if echo "$user_output" | grep -q "482"; then
    echo "✅ Test 4 réussi: Non-opérateur ne peut pas expulser"
else
    echo "❌ Test 4 échoué: Non-opérateur a pu expulser ou erreur inattendue"
    echo "$user_output"
    kill $OP_PID $USER_PID $SERVER_PID
    rm -f /tmp/op_in /tmp/op_out /tmp/user_in /tmp/user_out
    exit 1
fi

# Nettoyage
kill $OP_PID $USER_PID $SERVER_PID
rm -f /tmp/op_in /tmp/op_out /tmp/user_in /tmp/user_out
echo "Tous les tests de KICK ont réussi!"
exit 0