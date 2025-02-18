#ifndef TEST_HPP
#define TEST_HPP

#include <iostream>
#include <string>
#include <vector>

class Test {
private:
    static int totalTests;
    static int passedTests;
    static int currentSuiteTests;
    static int currentSuitePassed;
    static std::string currentSuite;
    
    // Codes ANSI pour les couleurs
    static const std::string GREEN;
    static const std::string RED;
    static const std::string YELLOW;
    static const std::string BLUE;
    static const std::string RESET;
    
public:
    static void startTestSuite(const std::string& suiteName) {
        if (!currentSuite.empty()) {
            printSuiteResults();
        }
        currentSuite = suiteName;
        currentSuiteTests = 0;
        currentSuitePassed = 0;
        std::cout << "\n" << BLUE << "=== Running " << suiteName << " Tests ===" << RESET << "\n" << std::endl;
    }
    
    static void startTest(const std::string& testName) {
        std::cout << YELLOW << "Running test: " << testName << RESET << std::endl;
        totalTests++;
        currentSuiteTests++;
    }
    
	static void assertTest(bool condition, const std::string& message, 
						const std::string& expected = "", const std::string& actual = "") {
		if (condition) {
			std::cout << GREEN << "✓ " << message << RESET << std::endl;
			passedTests++;
			currentSuitePassed++;
		} else {
			std::cout << RED << "✗ " << message << RESET << std::endl;
			if (!expected.empty() || !actual.empty()) {
				std::cout << RED << "  Expected: [" << expected << "]" << RESET << std::endl;
				std::cout << RED << "  Actual:   [" << actual << "]" << RESET << std::endl;
			}
		}
	}
    
    static void printSuiteResults() {
        std::cout << "\n" << BLUE << "Results for " << currentSuite << ":" << RESET << std::endl;
        std::cout << "Passed: " << currentSuitePassed << "/" << currentSuiteTests 
                  << " (" << (currentSuiteTests > 0 ? (currentSuitePassed * 100 / currentSuiteTests) : 0) 
                  << "%)" << std::endl;
    }
    
    static void printFinalResults() {
        if (!currentSuite.empty()) {
            printSuiteResults();
        }
        
        std::cout << "\n" << BLUE << "=== Final Test Results ===" << RESET << std::endl;
        std::cout << "Total Tests Passed: " << passedTests << "/" << totalTests 
                  << " (" << (totalTests > 0 ? (passedTests * 100 / totalTests) : 0) 
                  << "%)" << std::endl;
    }
};

#endif