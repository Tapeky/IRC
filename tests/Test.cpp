#include "Test.hpp"

int Test::totalTests = 0;
int Test::passedTests = 0;
int Test::currentSuiteTests = 0;
int Test::currentSuitePassed = 0;
std::string Test::currentSuite = "";

const std::string Test::GREEN = "\033[32m";
const std::string Test::RED = "\033[31m";
const std::string Test::YELLOW = "\033[33m";
const std::string Test::BLUE = "\033[34m";
const std::string Test::RESET = "\033[0m";