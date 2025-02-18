#include "Test.hpp"
#include "../src/commands/Command.hpp"
#include "TestCommands.hpp"
#include <iostream>

void testNICKCommand() {
    Test::startTest("NICK Command");
    
    TestClient client(1);
    client.setAuthenticated(true);

    // Test NICK sans paramètre
    Command cmd1;
    cmd1.command = "NICK";
    client.clearLastMessage();
    CommandExecutor::executeCommand(&client, cmd1);
    Test::assertTest(client.getLastMessage().find("431") != std::string::npos, 
        "NICK without params should return error 431",
        "Message containing error code '431'",
        client.getLastMessage());
    
    // Test NICK avec caractères invalides
    Command cmd2;
    cmd2.command = "NICK";
    cmd2.params.push_back("@invalid");
    client.clearLastMessage();
    CommandExecutor::executeCommand(&client, cmd2);
    Test::assertTest(client.getLastMessage().find("432") != std::string::npos, 
        "NICK with invalid characters should return error 432",
        "Message containing error code '432'",
        client.getLastMessage());
    
    // Test NICK trop long (>9 caractères)
    Command cmd3;
    cmd3.command = "NICK";
    cmd3.params.push_back("verylongnicknametest");
    client.clearLastMessage();
    CommandExecutor::executeCommand(&client, cmd3);
    Test::assertTest(client.getLastMessage().find("432") != std::string::npos, 
        "NICK too long should return error 432",
        "Message containing error code '432'",
        client.getLastMessage());

    // Test NICK valide
    Command cmd4;
    cmd4.command = "NICK";
    cmd4.params.push_back("validnick");
    client.clearLastMessage();
    CommandExecutor::executeCommand(&client, cmd4);
    Test::assertTest(client.getNickname() == "validnick", 
        "Nickname should be set to 'validnick'",
        "validnick",
        client.getNickname());
    Test::assertTest(client.getLastMessage().find("001") != std::string::npos, 
        "Should receive welcome message after valid NICK",
        "Message containing code '001'",
        client.getLastMessage());

    // Test NICK déjà utilisé
    TestClient client2(2);
    client2.setAuthenticated(true);
    Command cmd5;
    cmd5.command = "NICK";
    cmd5.params.push_back("validnick"); // même nickname que client1
    client2.clearLastMessage();
    CommandExecutor::executeCommand(&client2, cmd5);
    Test::assertTest(client2.getLastMessage().find("433") != std::string::npos,
        "NICK already in use should return error 433",
        "Message containing error code '433'",
        client2.getLastMessage());
}

void testJOINCommand() {
    Test::startTest("JOIN Command");
    
    TestClient client(1);
    client.setAuthenticated(true);
    client.setNickname("tester");

    // Test JOIN sans paramètre
    Command cmd1;
    cmd1.command = "JOIN";
    CommandExecutor::executeCommand(&client, cmd1);
    Test::assertTest(client.getLastMessage().find("461") != std::string::npos, 
        "JOIN without params should return error 461");

    // Test JOIN canal valide
    Command cmd2;
    cmd2.command = "JOIN";
    cmd2.params.push_back("#testchan");
    CommandExecutor::executeCommand(&client, cmd2);
    Test::assertTest(client.getLastMessage().find("JOIN #testchan") != std::string::npos, 
        "Should receive join confirmation message");
}

void testPRIVMSGCommand() {
    Test::startTest("PRIVMSG Command");
    
    TestClient sender(1);
    TestClient receiver(2);
    sender.setAuthenticated(true);
    sender.setNickname("sender");
    receiver.setNickname("receiver");

    // Test PRIVMSG sans paramètre
    Command cmd1;
    cmd1.command = "PRIVMSG";
    CommandExecutor::executeCommand(&sender, cmd1);
    Test::assertTest(sender.getLastMessage().find("411") != std::string::npos, 
        "PRIVMSG without params should return error 411");

    // Test PRIVMSG valide
    Command cmd2;
    cmd2.command = "PRIVMSG";
    cmd2.params.push_back("receiver");
    cmd2.params.push_back("Hello!");
    CommandExecutor::executeCommand(&sender, cmd2);
    Test::assertTest(receiver.getLastMessage().find("Hello!") != std::string::npos, 
        "Message should be received by target");
}

void testPARTCommand() {
    Test::startTest("PART Command");
    
    TestClient client(1);
    client.setAuthenticated(true);
    client.setNickname("tester");

    // Test PART sans paramètre
    Command cmd1;
    cmd1.command = "PART";
    CommandExecutor::executeCommand(&client, cmd1);
    Test::assertTest(client.getLastMessage().find("461") != std::string::npos, 
        "PART without params should return error 461");

    // Test PART sur un canal non existant
    Command cmd2;
    cmd2.command = "PART";
    cmd2.params.push_back("#nonexistent");
    CommandExecutor::executeCommand(&client, cmd2);
    Test::assertTest(client.getLastMessage().find("403") != std::string::npos, 
        "PART on non-existent channel should return error 403");

    // Test PART valide
    Command joinCmd;
    joinCmd.command = "JOIN";
    joinCmd.params.push_back("#testchan");
    CommandExecutor::executeCommand(&client, joinCmd);
    client.clearLastMessage();

    Command cmd3;
    cmd3.command = "PART";
    cmd3.params.push_back("#testchan");
    CommandExecutor::executeCommand(&client, cmd3);
    Test::assertTest(client.getLastMessage().find("PART #testchan") != std::string::npos, 
        "Should receive part confirmation message");
}

void testTOPICCommand() {
    Test::startTest("TOPIC Command");
    
    TestClient client(1);
    client.setAuthenticated(true);
    client.setNickname("tester");

    // Test TOPIC sans paramètre
    Command cmd1;
    cmd1.command = "TOPIC";
    CommandExecutor::executeCommand(&client, cmd1);
    Test::assertTest(client.getLastMessage().find("461") != std::string::npos, 
        "TOPIC without params should return error 461");

    // Joindre un canal d'abord
    Command joinCmd;
    joinCmd.command = "JOIN";
    joinCmd.params.push_back("#testchan");
    CommandExecutor::executeCommand(&client, joinCmd);
    client.clearLastMessage();

    // Test consultation du topic
    Command cmd2;
    cmd2.command = "TOPIC";
    cmd2.params.push_back("#testchan");
    CommandExecutor::executeCommand(&client, cmd2);
    Test::assertTest(client.getLastMessage().find("topic") != std::string::npos, 
        "Should receive topic information");

    // Test modification du topic
    Command cmd3;
    cmd3.command = "TOPIC";
    cmd3.params.push_back("#testchan");
    cmd3.params.push_back(":New Topic");
    CommandExecutor::executeCommand(&client, cmd3);
    Test::assertTest(client.getLastMessage().find("New Topic") != std::string::npos, 
        "Should receive topic change confirmation");
}

void testKICKCommand() {
    Test::startTest("KICK Command");
    
    TestClient operator_client(1);
    TestClient target_client(2);
    operator_client.setAuthenticated(true);
    operator_client.setNickname("operator");
    target_client.setAuthenticated(true);
    target_client.setNickname("target");

    // Test KICK sans paramètres suffisants
    Command cmd1;
    cmd1.command = "KICK";
    CommandExecutor::executeCommand(&operator_client, cmd1);
    Test::assertTest(operator_client.getLastMessage().find("461") != std::string::npos, 
        "KICK without params should return error 461");

    // Préparer le canal
    Command joinCmd;
    joinCmd.command = "JOIN";
    joinCmd.params.push_back("#testchan");
    CommandExecutor::executeCommand(&operator_client, joinCmd);
    CommandExecutor::executeCommand(&target_client, joinCmd);
    operator_client.clearLastMessage();

    // Test KICK par non-opérateur
    Command cmd2;
    cmd2.command = "KICK";
    cmd2.params.push_back("#testchan");
    cmd2.params.push_back("target");
    CommandExecutor::executeCommand(&target_client, cmd2);
    Test::assertTest(target_client.getLastMessage().find("482") != std::string::npos, 
        "KICK by non-operator should return error 482");

    // Test KICK valide
    Command cmd3;
    cmd3.command = "KICK";
    cmd3.params.push_back("#testchan");
    cmd3.params.push_back("target");
    CommandExecutor::executeCommand(&operator_client, cmd3);
    Test::assertTest(target_client.getLastMessage().find("kicked") != std::string::npos, 
        "Target should receive kick message");
}

void testINVITECommand() {
    Test::startTest("INVITE Command");
    
    TestClient operator_client(1);
    TestClient target_client(2);
    operator_client.setAuthenticated(true);
    operator_client.setNickname("operator");
    target_client.setAuthenticated(true);
    target_client.setNickname("target");

    // Test INVITE sans paramètres suffisants
    Command cmd1;
    cmd1.command = "INVITE";
    CommandExecutor::executeCommand(&operator_client, cmd1);
    Test::assertTest(operator_client.getLastMessage().find("461") != std::string::npos, 
        "INVITE without params should return error 461");

    // Préparer le canal
    Command joinCmd;
    joinCmd.command = "JOIN";
    joinCmd.params.push_back("#testchan");
    CommandExecutor::executeCommand(&operator_client, joinCmd);
    operator_client.clearLastMessage();

    // Test INVITE valide
    Command cmd2;
    cmd2.command = "INVITE";
    cmd2.params.push_back("target");
    cmd2.params.push_back("#testchan");
    CommandExecutor::executeCommand(&operator_client, cmd2);
    Test::assertTest(target_client.getLastMessage().find("invited") != std::string::npos, 
        "Target should receive invite message");
}

void testMODECommand() {
    Test::startTest("MODE Command");
    
    TestClient operator_client(1);
    operator_client.setAuthenticated(true);
    operator_client.setNickname("operator");

    // Test MODE sans paramètres suffisants
    Command cmd1;
    cmd1.command = "MODE";
    CommandExecutor::executeCommand(&operator_client, cmd1);
    Test::assertTest(operator_client.getLastMessage().find("461") != std::string::npos, 
        "MODE without params should return error 461");

    // Préparer le canal
    Command joinCmd;
    joinCmd.command = "JOIN";
    joinCmd.params.push_back("#testchan");
    CommandExecutor::executeCommand(&operator_client, joinCmd);
    operator_client.clearLastMessage();

    // Test des différents modes
    Command cmd2;
    cmd2.command = "MODE";
    cmd2.params.push_back("#testchan");
    cmd2.params.push_back("+i");
    CommandExecutor::executeCommand(&operator_client, cmd2);
    Test::assertTest(operator_client.getLastMessage().find("MODE #testchan +i") != std::string::npos, 
        "Should receive mode change confirmation");
}

void testPASSCommand() {
    Test::startTest("PASS Command");
    
    TestClient client(1);

    // Test PASS sans paramètre
    Command cmd1;
    cmd1.command = "PASS";
    CommandExecutor::executeCommand(&client, cmd1);
    Test::assertTest(client.getLastMessage().find("461") != std::string::npos, 
        "PASS without params should return error 461");

    // Test PASS valide
    Command cmd2;
    cmd2.command = "PASS";
    cmd2.params.push_back(Server::getInstance().getPassword());
    CommandExecutor::executeCommand(&client, cmd2);
    Test::assertTest(client.isAuthenticated(), 
        "Client should be authenticated after correct password");
}

void testUSERCommand() {
    Test::startTest("USER Command");
    
    TestClient client(1);
    client.setAuthenticated(true);

    // Test USER sans paramètre
    Command cmd1;
    cmd1.command = "USER";
    CommandExecutor::executeCommand(&client, cmd1);
    Test::assertTest(client.getLastMessage().find("461") != std::string::npos, 
        "USER without params should return error 461");

    // Test USER valide
    Command cmd2;
    cmd2.command = "USER";
    cmd2.params.push_back("testuser");
    CommandExecutor::executeCommand(&client, cmd2);
    Test::assertTest(client.getUsername() == "testuser", 
        "Username should be set to 'testuser'");
}

void testQUITCommand() {
    Test::startTest("QUIT Command");
    
    TestClient client(1);
    client.setAuthenticated(true);
    client.setNickname("quitter");

    // Test QUIT sans message
    Command cmd1;
    cmd1.command = "QUIT";
    CommandExecutor::executeCommand(&client, cmd1);
    Test::assertTest(client.getLastMessage().find("QUIT") != std::string::npos, 
        "Should receive quit confirmation");

    // Test QUIT avec message
    client.setAuthenticated(true); // Réinitialiser pour le test
    Command cmd2;
    cmd2.command = "QUIT";
    cmd2.params.push_back("Goodbye!");
    CommandExecutor::executeCommand(&client, cmd2);
    Test::assertTest(client.getLastMessage().find("Goodbye!") != std::string::npos, 
        "Should receive quit message");
}

void testSimpleCommand() {
    Test::startTest("Simple Command Parsing");
    Command cmd = CommandParser::parseCommand("NICK testuser");
    Test::assertTest(cmd.command == "NICK", "Command name should be NICK");
    Test::assertTest(cmd.params.size() == 1, "Should have exactly 1 parameter");
    Test::assertTest(cmd.params[0] == "testuser", "Parameter should be 'testuser'");
    Test::assertTest(cmd.prefix.empty(), "Prefix should be empty");
}

void testCommandWithPrefix() {
    Test::startTest("Command With Prefix");

    Command cmd = CommandParser::parseCommand(":user PRIVMSG #channel :Hello World");
    Test::assertTest(cmd.prefix == "user", "Prefix is user");
    Test::assertTest(cmd.command == "PRIVMSG", "Command is PRIVMSG");
    Test::assertTest(cmd.params.size() == 2, "Has 2 parameters");
    Test::assertTest(cmd.params[0] == "#channel", "First parameter is #channel");
    Test::assertTest(cmd.params[1] == "Hello World", "Second parameter is message");
}

void testCommandWithMultipleParams() {
    Test::startTest("Command With Multiple Parameters");

    Command cmd = CommandParser::parseCommand("MODE #channel +o user1");
    Test::assertTest(cmd.command == "MODE", "Command is MODE");
    Test::assertTest(cmd.params.size() == 3, "Has 3 parameters");
    Test::assertTest(cmd.params[0] == "#channel", "First parameter is channel");
    Test::assertTest(cmd.params[1] == "+o", "Second parameter is mode");
    Test::assertTest(cmd.params[2] == "user1", "Third parameter is username");
}

void testEmptyCommand() {
    Test::startTest("Empty Command");

    Command cmd = CommandParser::parseCommand("");
    Test::assertTest(cmd.command.empty(), "Command is empty");
    Test::assertTest(cmd.params.empty(), "No parameters");
    Test::assertTest(cmd.prefix.empty(), "No prefix");
}

void testCommandWithColon() {
    Test::startTest("Command With Colon Message");

    Command cmd = CommandParser::parseCommand("PRIVMSG #channel :This is a long message with spaces");
    Test::assertTest(cmd.command == "PRIVMSG", "Command is PRIVMSG");
    Test::assertTest(cmd.params.size() == 2, "Has 2 parameters");
    Test::assertTest(cmd.params[0] == "#channel", "First parameter is channel");
    Test::assertTest(cmd.params[1] == "This is a long message with spaces", "Second parameter is full message");
}

// Fonction à appeler depuis TestMain.cpp
void runCommandTests() {
    std::cout << "\n=== Running Command Parser Tests ===\n" << std::endl;
    
    testSimpleCommand();
    testCommandWithPrefix();
    testCommandWithMultipleParams();
    testEmptyCommand();
    testCommandWithColon();

	Test::startTestSuite("IRC Commands");
    
   testNICKCommand();
   // testJOINCommand();
    testPRIVMSGCommand();
    testPARTCommand();
    testTOPICCommand();
    testKICKCommand();
    testINVITECommand();
    testMODECommand();
    testPASSCommand();
    testUSERCommand();
    testQUITCommand();
}
