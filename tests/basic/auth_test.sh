#!/bin/bash
# tests/basic/auth_test.sh

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
    rm -f /tmp/auth_test_*_$$ 2>/dev/null
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

echo "Testing authentication..."

# Test 1: Valid authentication
echo "Test 1: Complete valid authentication"
result=$(printf "PASS %s\r\nNICK tester1\r\nUSER test1 test1 test1 :Testing User\r\n" "$PASSWORD" | nc -w 2 localhost $PORT)

if echo "$result" | grep -q "001"; then
    echo "✅ Test 1 passed: Authentication accepted"
else
    echo "❌ Test 1 failed: Authentication rejected"
    echo "$result"
    exit 1
fi

# Test 2: Invalid password
echo "Test 2: Invalid password"
result=$(printf "PASS wrongpassword\r\nNICK tester2\r\nUSER test2 test2 test2 :Testing User\r\n" | nc -w 2 localhost $PORT)

if echo "$result" | grep -q "464"; then
    echo "✅ Test 2 passed: Incorrect password rejected"
else
    echo "❌ Test 2 failed: Incorrect password accepted"
    echo "$result"
    exit 1
fi

# Test 3: Attempt to reregister
echo "Test 3: Attempt to reregister"
result=$(printf "PASS %s\r\nNICK tester3\r\nUSER test3 test3 test3 :Test\r\nUSER test3_new test3_new test3_new :Test\r\n" "$PASSWORD" | nc -w 2 localhost $PORT)

if echo "$result" | grep -q "462"; then
    echo "✅ Test 3 passed: Reregistration rejected"
else
    echo "❌ Test 3 failed: Reregistration accepted"
    echo "$result"
    exit 1
fi

echo "All authentication tests passed!"
exit 0