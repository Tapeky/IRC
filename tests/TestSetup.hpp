#ifndef TEST_SETUP_HPP
#define TEST_SETUP_HPP

#include "../inc/Server.hpp"

class TestSetup {
public:
    static void init() {
        std::string password = "testpass";
        Server::initInstance(6667, password);
    }
    
    static void cleanup() {
        Server::destroyInstance();
    }
};

#endif