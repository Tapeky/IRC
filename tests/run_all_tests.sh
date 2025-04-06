#!/bin/bash
# tests/run_all_tests.sh

# Couleurs pour améliorer la lisibilité
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${YELLOW}====== DÉMARRAGE DES TESTS IRC SERVER ======${NC}"
echo "Exécution dans: $(pwd)"

# Variables globales
PORT=6667
PASSWORD="testpassword"
BASE_DIR="$(dirname "$0")"
FAILED_TESTS=0
TOTAL_TESTS=0

# Fonction pour exécuter un ensemble de tests
run_test_suite() {
    local suite_name=$1
    local suite_dir=$2
    
    echo -e "\n${YELLOW}=== Test Suite: $suite_name ===${NC}"
    
    for test_script in "$suite_dir"/*.sh; do
        if [ -f "$test_script" ] && [ "$test_script" != "$0" ]; then
            TOTAL_TESTS=$((TOTAL_TESTS + 1))
            echo -e "\n${YELLOW}Exécution: $(basename "$test_script")${NC}"
            chmod +x "$test_script"
            bash "$test_script" "$PORT" "$PASSWORD"
            
            if [ $? -eq 0 ]; then
                echo -e "${GREEN}✓ Test réussi: $(basename "$test_script")${NC}"
            else
                echo -e "${RED}✗ Test échoué: $(basename "$test_script")${NC}"
                FAILED_TESTS=$((FAILED_TESTS + 1))
            fi
        fi
    done
}

# Exécution de tous les ensembles de tests
run_test_suite "Tests de base" "$BASE_DIR/basic"
run_test_suite "Tests de cas limites" "$BASE_DIR/edge_cases"
run_test_suite "Tests d'opérateur" "$BASE_DIR/operator"
run_test_suite "Tests bonus" "$BASE_DIR/bonus"

# Affichage des résultats
echo -e "\n${YELLOW}====== RÉSULTATS DES TESTS ======${NC}"
PASSED_TESTS=$((TOTAL_TESTS - FAILED_TESTS))
echo -e "Tests réussis: ${GREEN}$PASSED_TESTS${NC}/$TOTAL_TESTS"
echo -e "Tests échoués: ${RED}$FAILED_TESTS${NC}/$TOTAL_TESTS"

if [ $FAILED_TESTS -eq 0 ]; then
    echo -e "${GREEN}Tous les tests ont réussi!${NC}"
    exit 0
else
    echo -e "${RED}Certains tests ont échoué.${NC}"
    exit 1
fi