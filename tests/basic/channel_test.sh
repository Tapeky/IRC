#!/bin/bash
# tests/basic/channel_test.sh

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

echo "Test des fonctionnalités de canal..."

# Test 1: Rejoindre un canal
echo "Test 1: Rejoindre un canal"
result=$(printf "PASS %s\r\nNICK tester1\r\nUSER test1 test1 test1 :Testing User\r\nJOIN #testchannel\r\n" "$PASSWORD" | nc -w 2 localhost $PORT)

if echo "$result" | grep -q "JOIN #testchannel"; then
    if echo "$result" | grep -q "353" && echo "$result" | grep -q "366"; then
        echo "✅ Test 1 réussi: Canal rejoint avec succès"
    else
        echo "❌ Test 1 échoué: Pas de réponse NAMES après JOIN"
        kill $SERVER_PID
        exit 1
    fi
else
    echo "❌ Test 1 échoué: Impossible de rejoindre le canal"
    echo "$result"
    kill $SERVER_PID
    exit 1
fi

# Test 2: Définir et récupérer un sujet
echo "Test 2: Définir et récupérer un sujet"
result=$(printf "PASS %s\r\nNICK tester2\r\nUSER test2 test2 test2 :Testing User\r\nJOIN #testchannel\r\nTOPIC #testchannel :Test Topic\r\nTOPIC #testchannel\r\n" "$PASSWORD" | nc -w 2 localhost $PORT)

if echo "$result" | grep -q "332" && echo "$result" | grep -q "Test Topic"; then
    echo "✅ Test 2 réussi: Sujet défini et récupéré"
else
    echo "❌ Test 2 échoué: Problème avec la commande TOPIC"
    echo "$result"
    kill $SERVER_PID
    exit 1
fi

# Test 3: Quitter un canal avec PART
echo "Test 3: Quitter un canal"
result=$(printf "PASS %s\r\nNICK tester3\r\nUSER test3 test3 test3 :Testing User\r\nJOIN #testchannel\r\nPART #testchannel :Goodbye\r\n" "$PASSWORD" | nc -w 2 localhost $PORT)

if echo "$result" | grep -q "PART #testchannel"; then
    echo "✅ Test 3 réussi: Canal quitté avec succès"
else
    echo "❌ Test 3 échoué: Problème avec la commande PART"
    echo "$result"
    kill $SERVER_PID
    exit 1
fi

# Test 4: Format de canal invalide
echo "Test 4: Format de canal invalide"
result=$(printf "PASS %s\r\nNICK tester4\r\nUSER test4 test4 test4 :Testing User\r\nJOIN testchannel\r\n" "$PASSWORD" | nc -w 2 localhost $PORT)

if echo "$result" | grep -q "403"; then
    echo "✅ Test 4 réussi: Format de canal invalide rejeté"
else
    echo "❌ Test 4 échoué: Format de canal invalide accepté"
    echo "$result"
    kill $SERVER_PID
    exit 1
fi

# Nettoyage
kill $SERVER_PID
echo "Tous les tests de canal ont réussi!"
exit 0