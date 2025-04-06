#!/bin/bash
# tests/edge_cases/partial_commands.sh

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
    rm -f /tmp/partial_test_*_$$
}
trap cleanup EXIT INT TERM

cd "$ROOT_DIR"
echo "Démarrage du serveur..."
./ircserv $PORT $PASSWORD > /dev/null 2>&1 &
SERVER_PID=$!
sleep 2

echo "Test de commandes partielles..."

# Créer un script Python temporaire pour envoyer des données partielles sur la même connexion
cat > /tmp/partial_test_script_$$ << 'EOF'
#!/usr/bin/env python3
import socket
import sys
import time

host = 'localhost'
port = int(sys.argv[1])

# Créer une socket
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((host, port))

# Envoyer chaque fragment avec un délai
for fragment in sys.argv[2:]:
    print(f"Sending fragment: '{fragment}'")
    s.sendall(fragment.encode())
    time.sleep(0.5)

# Attendre une réponse (délai plus long pour la réponse complète)
s.settimeout(5)
response = ""
try:
    while True:
        chunk = s.recv(4096).decode()
        if not chunk:
            break
        response += chunk
        # Si on a reçu assez de données, on peut s'arrêter
        if "001" in response and "Welcome" in response:
            break
except socket.timeout:
    pass

print(response)
s.close()
EOF

chmod +x /tmp/partial_test_script_$$

# Test 1: Envoi d'une commande d'authentification complète en fragments
echo "Test 1: Authentification complète fragmentée"

# Exécuter le script Python avec une séquence d'authentification complète
echo "Envoi d'une séquence d'authentification en fragments"
response=$(/tmp/partial_test_script_$$ $PORT "PA" "SS $PASSWORD\r\n" "NI" "CK testuser\r\n" "US" "ER test test test :Real" " name\r\n")

echo "Réponse du serveur:"
echo "$response"

if echo "$response" | grep -q "001"; then
    echo "✅ Test 1 réussi: Authentification fragmentée acceptée"
else
    echo "❌ Test 1 échoué: Authentification fragmentée rejetée"
    exit 1
fi

# Test 2: Envoi de commandes avec des pauses significatives
echo "Test 2: Commande avec pause longue"

# Créer un script Python avec pause plus longue
cat > /tmp/partial_test_script2_$$ << 'EOF'
#!/usr/bin/env python3
import socket
import sys
import time

host = 'localhost'
port = int(sys.argv[1])
password = sys.argv[2]

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((host, port))

# Envoyer l'authentification
s.sendall(f"PASS {password}\r\nNICK pausetest\r\nUSER pause pause pause :Pause Test\r\n".encode())
time.sleep(1)

# Envoyer une commande JOIN incomplète
s.sendall("JOIN ".encode())
print("Pause de 3 secondes au milieu de la commande JOIN...")
time.sleep(3)  # Pause significative au milieu de la commande
s.sendall("#testpause\r\n".encode())

# Attendre la réponse
s.settimeout(5)
response = ""
try:
    while True:
        chunk = s.recv(4096).decode()
        if not chunk:
            break
        response += chunk
        if "JOIN #testpause" in response:
            break
except socket.timeout:
    pass

print(response)
s.close()
EOF

chmod +x /tmp/partial_test_script2_$$

# Exécuter le script avec pause
echo "Envoi d'une commande JOIN avec pause de 3 secondes au milieu"
response=$(/tmp/partial_test_script2_$$ $PORT "$PASSWORD")

echo "Réponse du serveur:"
echo "$response"

if echo "$response" | grep -q "JOIN #testpause"; then
    echo "✅ Test 2 réussi: Commande avec pause longue acceptée"
else
    echo "❌ Test 2 échoué: Problème avec commande avec pause longue"
    exit 1
fi

# Test 3: Messages multiples dans un seul paquet TCP
echo "Test 3: Messages multiples dans un seul paquet"

response=$(printf "PASS %s\r\nNICK multitest\r\nUSER multi multi multi :Multiple Commands\r\nJOIN #test1\r\nJOIN #test2\r\n" "$PASSWORD" | nc -w 3 localhost $PORT)

echo "Réponse du serveur:"
echo "$response"

if echo "$response" | grep -q "JOIN #test1" && echo "$response" | grep -q "JOIN #test2"; then
    echo "✅ Test 3 réussi: Traitement de commandes multiples dans un seul paquet"
else
    echo "❌ Test 3 échoué: Problème avec les commandes multiples"
    exit 1
fi

echo "Tous les tests de commandes partielles ont réussi!"
exit 0