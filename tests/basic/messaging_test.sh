#!/bin/bash
# tests/basic/messaging_test.sh

PORT=${1:-6667}
PASSWORD=${2:-"testpassword"}

# Find the script directory regardless of how it's called
if [[ "$0" == /* ]]; then
    SCRIPT_DIR=$(dirname "$0")
else
    SCRIPT_DIR=$(dirname "$(readlink -f "$0")")
fi
ROOT_DIR=$(cd "$SCRIPT_DIR/../.." && pwd)

# Function to cleanup resources
cleanup() {
    echo "Cleaning up resources..."
    [[ -n "$SERVER_PID" ]] && kill $SERVER_PID 2>/dev/null
    pkill -f "nc localhost $PORT" 2>/dev/null
    rm -f /tmp/irc_test_*_$$ 2>/dev/null
}
trap cleanup EXIT INT TERM

# Check if server is already running (from run_all_tests.sh)
if ! nc -z localhost $PORT 2>/dev/null; then
    cd "$ROOT_DIR"
    echo "Starting server..."
    ./ircserv $PORT $PASSWORD > /dev/null 2>&1 &
    SERVER_PID=$!
    sleep 1
fi

echo "Testing messaging functionality..."

# Test 1: Private message between users
echo "Test 1: Private messaging"

# Set up client2 listening connection
echo "PASS $PASSWORD" > /tmp/irc_test_client2_$$
echo "NICK client2" >> /tmp/irc_test_client2_$$
echo "USER user2 host2 server2 :Real Name 2" >> /tmp/irc_test_client2_$$

# Start client2 in background
(nc localhost $PORT < /tmp/irc_test_client2_$$ > /tmp/irc_test_output2_$$ & echo $! > /tmp/irc_test_pid2_$$; sleep 10) &

# Wait for client2 to connect
sleep 2

# Client1 sends message to client2
TEST_MESSAGE="Hello from client1 to client2"
echo "Sending private message..."
printf "PASS %s\r\nNICK client1\r\nUSER user1 host1 server1 :Real Name 1\r\nPRIVMSG client2 :%s\r\n" "$PASSWORD" "$TEST_MESSAGE" | nc -w 2 localhost $PORT > /dev/null

# Wait for message to be received
sleep 2

# Check if client2 received the message
CLIENT2_OUTPUT=$(cat /tmp/irc_test_output2_$$)
if echo "$CLIENT2_OUTPUT" | grep -q "$TEST_MESSAGE"; then
    echo "✅ Test 1 passed: Private message received"
else
    echo "❌ Test 1 failed: Private message not received"
    echo "Client2 output:"
    echo "$CLIENT2_OUTPUT"
    exit 1
fi

# Kill client2 background process
kill $(cat /tmp/irc_test_pid2_$$) 2>/dev/null || true

# Test 2: Channel messaging
echo "Test 2: Channel messaging"

# Create a channel with two clients
echo "Creating channel with two clients..."

# Join with client1
echo "PASS $PASSWORD" > /tmp/irc_test_join1_$$
echo "NICK client1" >> /tmp/irc_test_join1_$$
echo "USER user1 host1 server1 :Real Name 1" >> /tmp/irc_test_join1_$$
echo "JOIN #testchannel" >> /tmp/irc_test_join1_$$
nc -w 2 localhost $PORT < /tmp/irc_test_join1_$$ > /dev/null

# Start client2 to listen for channel messages
echo "PASS $PASSWORD" > /tmp/irc_test_channel2_$$
echo "NICK client2" >> /tmp/irc_test_channel2_$$
echo "USER user2 host2 server2 :Real Name 2" >> /tmp/irc_test_channel2_$$
echo "JOIN #testchannel" >> /tmp/irc_test_channel2_$$

# Start client2 in background
(nc localhost $PORT < /tmp/irc_test_channel2_$$ > /tmp/irc_test_channel_output2_$$ & echo $! > /tmp/irc_test_channel_pid2_$$; sleep 10) &

# Wait for client2 to join
sleep 2

# Client1 sends message to channel
CHANNEL_MESSAGE="Hello everyone in #testchannel!"
echo "Sending channel message..."
printf "PASS %s\r\nNICK client1\r\nUSER user1 host1 server1 :Real Name 1\r\nJOIN #testchannel\r\nPRIVMSG #testchannel :%s\r\n" "$PASSWORD" "$CHANNEL_MESSAGE" | nc -w 2 localhost $PORT > /dev/null

# Wait for message to be received
sleep 2

# Check if client2 received the channel message
CHANNEL_OUTPUT=$(cat /tmp/irc_test_channel_output2_$$)
if echo "$CHANNEL_OUTPUT" | grep -q "$CHANNEL_MESSAGE"; then
    echo "✅ Test 2 passed: Channel message received"
else
    echo "❌ Test 2 failed: Channel message not received"
    echo "Channel output:"
    echo "$CHANNEL_OUTPUT"
    exit 1
fi

# Kill client2 background process
kill $(cat /tmp/irc_test_channel_pid2_$$) 2>/dev/null || true

echo "All messaging tests passed!"
exit 0