// TestCommands.hpp
#ifndef TEST_COMMANDS_HPP
#define TEST_COMMANDS_HPP

#include "Test.hpp"
#include "../src/commands/CommandExecutor.hpp"
#include "../inc/Server.hpp"
#include "../inc/Client.hpp"
#include "../inc/Channel.hpp"
#include <sstream>

class TestClient : public Client {
	public:
		TestClient(int fd) : Client(fd), _lastMessage("") {
			std::cout << "TestClient created with fd " << fd << std::endl;
		}
	
		virtual void sendMessage(const std::string& message) override {
			std::cout << "TestClient::sendMessage received: [" << message << "]" << std::endl;
			_lastMessage = message;
		}
	
		std::string getLastMessage() const { 
			std::cout << "Getting last message: [" << _lastMessage << "]" << std::endl;
			return _lastMessage; 
		}
	
		void clearLastMessage() {
			std::cout << "Clearing last message" << std::endl;
			_lastMessage = "";
		}
	
	private:
		std::string _lastMessage;
};

// Déclarations des fonctions de test
void testNICKCommand();
void testJOINCommand();
void testPRIVMSGCommand();
void testPARTCommand();
void testTOPICCommand();
void testKICKCommand();
void testINVITECommand();
void testMODECommand();
void testPASSCommand();
void testUSERCommand();
void testQUITCommand();
void runCommandsTests();

#endif