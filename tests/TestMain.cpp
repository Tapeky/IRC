#include "Test.hpp"
#include "TestSetup.hpp"

void runCommandTests();
void runChannelTests();

int main() {
	std::cout << "Initializing test environment..." << std::endl;
    
    TestSetup::init();
    
    std::cout << "Running tests..." << std::endl;
    
    // Exécuter les suites de tests
    runCommandTests();
    runChannelTests();
    
    Test::printFinalResults();
	TestSetup::cleanup();
    return 0;
}
