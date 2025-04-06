#!/bin/bash
# tests/basic/channel_test.sh

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
    rm -f /tmp/channel_test_*_$$ 2>/dev/null
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

echo "Testing channel functionality..."

# Test 1: Join a channel
echo "Test 1: Join a channel"
result=$(printf "PASS %s\r\nNICK tester1\r\nUSER test1 test1 test1 :Testing User\r\nJOIN #testchannel\r\n" "$PASSWORD" | nc -w 2 localhost $PORT)

if echo "$result" | grep -q "JOIN #testchannel"; then
    if echo "$result" | grep -q "353" && echo "$result" | grep -q "366"; then
        echo "✅ Test 1 passed: Channel joined successfully with NAMES list"
    else
        echo "❌ Test 1 failed: No NAMES list after JOIN"
        echo "$result"
        exit 1
    fi
else
    echo "❌ Test 1 failed: Could not join channel"
    echo "$result"
    exit 1
fi

# Test 2: Set and get topic
echo "Test 2: Set and retrieve topic"
result=$(printf "PASS %s\r\nNICK tester2\r\nUSER test2 test2 test2 :Testing User\r\nJOIN #testchannel\r\nTOPIC #testchannel :Test Topic\r\nTOPIC #testchannel\r\n" "$PASSWORD" | nc -w 2 localhost $PORT)

if echo "$result" | grep -q "332" && echo "$result" | grep -q "Test Topic"; then
    echo "✅ Test 2 passed: Topic set and retrieved"
else
    echo "❌ Test 2 failed: Problem with TOPIC command"
    echo "$result"
    exit 1
fi

# Test 3: Leave channel with PART
echo "Test 3: Leave channel"
result=$(printf "PASS %s\r\nNICK tester3\r\nUSER test3 test3 test3 :Testing User\r\nJOIN #testchannel\r\nPART #testchannel :Goodbye\r\n" "$PASSWORD" | nc -w 2 localhost $PORT)

if echo "$result" | grep -q "PART #testchannel"; then
    echo "✅ Test 3 passed: Channel left successfully"
else
    echo "❌ Test 3 failed: Problem with PART command"
    echo "$result"
    exit 1
fi

# Test 4: Invalid channel format
echo "Test 4: Invalid channel format"
result=$(printf "PASS %s\r\nNICK tester4\r\nUSER test4 test4 test4 :Testing User\r\nJOIN testchannel\r\n" "$PASSWORD" | nc -w 2 localhost $PORT)

if echo "$result" | grep -q "403"; then
    echo "✅ Test 4 passed: Invalid channel format rejected"
else
    echo "❌ Test 4 failed: Invalid channel format accepted"
    echo "$result"
    exit 1
fi

echo "All channel tests passed!"
exit 0