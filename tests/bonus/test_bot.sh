#!/bin/bash
# tests/bonus/bot_test.sh

PORT=${1:-6667}
PASSWORD=${2:-"testpassword"}

# Démarrer le serveur
cd ../../
./ircserv $PORT $PASSWORD > /dev/null 2>&1 &
SERVER_PID=$!

# Attendre que le serveur démarre
sleep 1

echo "Test du bot IRC..."

# Vérifier si le bot existe
if [ ! -f "./ircbot" ]; then
    echo "❌ Test échoué: Le bot (ircbot) n'existe pas"
    kill $SERVER_PID
    exit 1
fi

# Démarrer le bot en arrière-plan
./ircbot $PORT $PASSWORD > /dev/null 2>&1 &
BOT_PID=$!

# Attendre que le bot se connecte
sleep 2

# Créer un fifo pour le client de test
FIFO_IN=/tmp/client_in_$$.fifo
FIFO_OUT=/tmp/client_out_$$.fifo
mkfifo $FIFO_IN
mkfifo $FIFO_OUT

# Démarrer le client de test
nc localhost $PORT < $FIFO_IN > $FIFO_OUT &
CLIENT_PID=$!

# Authentifier le client
echo -e "PASS $PASSWORD\r\nNICK tester\r\nUSER test test test :Test User\r\n" > $FIFO_IN
sleep 1

# Vider le buffer de sortie initial
cat $FIFO_OUT > /dev/null

# Test 1: Rejoindre le même canal que le bot
echo "Test 1: Rejoindre le même canal que le bot"
echo -e "JOIN #bottest\r\n" > $FIFO_IN
sleep 2

client_output=$(cat $FIFO_OUT)
if echo "$client_output" | grep -q "JOIN #bottest"; then
    # Vérifier si le bot est visible dans la liste des noms
    if echo "$client_output" | grep -q "353" && echo "$client_output" | grep -qi "bot"; then
        echo "✅ Test 1 réussi: Bot trouvé dans le canal"
    else
        echo "❌ Test 1 échoué: Bot non trouvé dans le canal"
        echo "$client_output"
        kill $CLIENT_PID $BOT_PID $SERVER_PID
        rm -f $FIFO_IN $FIFO_OUT
        exit 1
    fi
else
    echo "❌ Test 1 échoué: Impossible de rejoindre le canal"
    echo "$client_output"
    kill $CLIENT_PID $BOT_PID $SERVER_PID
    rm -f $FIFO_IN $FIFO_OUT
    exit 1
fi

# Test 2: Interagir avec le bot
echo "Test 2: Interagir avec le bot"
echo -e "PRIVMSG #bottest :!help\r\n" > $FIFO_IN
sleep 2

client_output=$(cat $FIFO_OUT)
# Vérifier si le bot répond à une commande simple
if echo "$client_output" | grep -qi "PRIVMSG" && echo "$client_output" | grep -qi "!help"; then
    echo "✅ Test 2 réussi: Bot a répondu à une commande"
else
    echo "❌ Test 2 échoué: Bot n'a pas répondu"
    echo "$client_output"
    kill $CLIENT_PID $BOT_PID $SERVER_PID
    rm -f $FIFO_IN $FIFO_OUT
    exit 1
fi

# Test 3: Vérifier une fonction spécifique du bot
echo "Test 3: Tester une fonction spécifique du bot"
echo -e "PRIVMSG #bottest :!time\r\n" > $FIFO_IN
sleep 2

client_output=$(cat $FIFO_OUT)
if echo "$client_output" | grep -qi "time" || echo "$client_output" | grep -qi "date" || echo "$client_output" | grep -qi "hour"; then
    echo "✅ Test 3 réussi: Bot a fourni une information de temps"
else
    echo "❌ Test 3 échoué: Fonction spécifique non disponible"
    echo "$client_output"
    
    # Ce test n'est pas critique, continuer malgré l'échec
    echo "Note: Ce test peut échouer si le bot n'offre pas cette commande spécifique"
fi

# Test 4: Le bot reste connecté sur une longue période
echo "Test 4: Vérifier que le bot reste connecté"
sleep 5
echo -e "PRIVMSG #bottest :!ping\r\n" > $FIFO_IN
sleep 2

client_output=$(cat $FIFO_OUT)
if echo "$client_output" | grep -qi "PRIVMSG"; then
    echo "✅ Test 4 réussi: Bot est toujours actif"
else
    echo "❌ Test 4 échoué: Bot ne répond plus"
    echo "$client_output"
    kill $CLIENT_PID $BOT_PID $SERVER_PID
    rm -f $FIFO_IN $FIFO_OUT
    exit 1
fi

# Nettoyage
kill $CLIENT_PID $BOT_PID $SERVER_PID
rm -f $FIFO_IN $FIFO_OUT
echo "Tous les tests du bot ont réussi!"
exit 0