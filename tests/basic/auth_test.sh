#!/bin/bash
# tests/basic/auth_test.sh

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

# Attendre que le serveur démarre
sleep 1

echo "Test d'authentification..."

# Test 1: Mot de passe correct, NICK et USER valides
echo -e "Test 1: Authentification complète valide"
result=$(printf "PASS %s\r\nNICK tester1\r\nUSER test1 test1 test1 :Testing User\r\n" "$PASSWORD" | nc -w 2 localhost $PORT)

if echo "$result" | grep -q "001"; then
    echo "✅ Test 1 réussi: Authentification acceptée"
else
    echo "❌ Test 1 échoué: L'authentification a échoué"
    echo "$result"
    kill $SERVER_PID
    exit 1
fi

# Test 2: Mot de passe incorrect
echo -e "\nTest 2: Mot de passe incorrect"
result=$(printf "PASS wrongpassword\r\nNICK tester2\r\nUSER test2 test2 test2 :Testing User\r\n" | nc -w 2 localhost $PORT)

if echo "$result" | grep -q "464"; then
    echo "✅ Test 2 réussi: Mot de passe incorrect rejeté"
else
    echo "❌ Test 2 échoué: Mot de passe incorrect accepté"
    kill $SERVER_PID
    exit 1
fi

# Test 3: Tentative de réenregistrement (USER)
echo -e "\nTest 3: Tentative de réenregistrement"
result=$(printf "PASS %s\r\nNICK tester3\r\nUSER test3 test3 test3 :Test\r\nUSER test3_new test3_new test3_new :Test\r\n" "$PASSWORD" | nc -w 2 localhost $PORT)

if echo "$result" | grep -q "462"; then
    echo "✅ Test 3 réussi: Réenregistrement rejeté"
else
    echo "❌ Test 3 échoué: Réenregistrement accepté"
    kill $SERVER_PID
    exit 1
fi

# Nettoyage
kill $SERVER_PID
echo "Tous les tests d'authentification ont réussi!"
exit 0