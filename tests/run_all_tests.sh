#!/bin/bash
# tests/run_all_tests.sh

GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo -e "${YELLOW}====== STARTING IRC SERVER TESTS ======${NC}"
echo "Running in: $(pwd)"

# Global variables
PORT=6667
PASSWORD="testpassword"
BASE_DIR="$(dirname "$(readlink -f "$0")")"
SERVER_PID=""
FAILED_TESTS=0
TOTAL_TESTS=0

# Function to cleanup resources
cleanup() {
    echo -e "\nCleaning up resources..."
    if [ -n "$SERVER_PID" ] && ps -p $SERVER_PID > /dev/null; then
        kill $SERVER_PID 2>/dev/null
        wait $SERVER_PID 2>/dev/null
    fi
    # Kill any remaining netcat processes
    pkill -f "nc localhost $PORT" 2>/dev/null
}
trap cleanup EXIT INT TERM

# Function to run a test suite
run_test_suite() {
    local suite_name=$1
    local suite_dir=$2
    
    echo -e "\n${YELLOW}=== Test Suite: $suite_name ===${NC}"
    
    for test_script in "$suite_dir"/*.sh; do
        if [ -f "$test_script" ] && [ "$test_script" != "$0" ]; then
            TOTAL_TESTS=$((TOTAL_TESTS + 1))
            echo -e "\n${YELLOW}Running: $(basename "$test_script")${NC}"
            chmod +x "$test_script"
            
            # Start a fresh server for each test
            if [ -n "$SERVER_PID" ] && ps -p $SERVER_PID > /dev/null; then
                kill $SERVER_PID 2>/dev/null
                wait $SERVER_PID 2>/dev/null
            fi
            
            cd "$BASE_DIR/.."
            ./ircserv $PORT $PASSWORD > /dev/null 2>&1 &
            SERVER_PID=$!
            sleep 1
            
            # Run the test
            bash "$test_script" "$PORT" "$PASSWORD"
            
            if [ $? -eq 0 ]; then
                echo -e "${GREEN}✓ Test passed: $(basename "$test_script")${NC}"
            else
                echo -e "${RED}✗ Test failed: $(basename "$test_script")${NC}"
                FAILED_TESTS=$((FAILED_TESTS + 1))
            fi
        fi
    done
}

# Run all test suites
run_test_suite "Basic Tests" "$BASE_DIR/basic"

echo -e "\n${YELLOW}====== TEST RESULTS ======${NC}"
PASSED_TESTS=$((TOTAL_TESTS - FAILED_TESTS))
echo -e "Tests passed: ${GREEN}$PASSED_TESTS${NC}/$TOTAL_TESTS"
echo -e "Tests failed: ${RED}$FAILED_TESTS${NC}/$TOTAL_TESTS"

if [ $FAILED_TESTS -eq 0 ]; then
    echo -e "${GREEN}All tests passed!${NC}"
    exit 0
else
    echo -e "${RED}Some tests failed.${NC}"
    exit 1
fi