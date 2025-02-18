#include "Test.hpp"
#include "../inc/Channel.hpp"
#include "../inc/Client.hpp"

void testChannelCreation() {
    Test::startTest("Channel Creation");
    
    Channel channel("#test");
    Test::assertTest(channel.getName() == "#test", "Channel name should be '#test'");
    Test::assertTest(!channel.isInviteOnly(), "Channel should not be invite-only by default");
    Test::assertTest(!channel.isTopicRestricted(), "Channel should not have topic restrictions by default");
    Test::assertTest(channel.getUserLimit() == 0, "Channel should have no user limit by default");
    Test::assertTest(channel.getKey().empty(), "Channel should have no key by default");
}

void testClientOperations() {
    Test::startTest("Client Operations");
    
    Channel channel("#test");
    Client client(1);
    
    // Test adding client
    channel.addClient(&client);
    Test::assertTest(channel.isClientinChannel(&client), "Client should be in channel after adding");
    Test::assertTest(channel.getClients().size() == 1, "Channel should have exactly one client");
    
    // Test adding same client again
    channel.addClient(&client);
    Test::assertTest(channel.getClients().size() == 1, "Channel should still have one client after duplicate add");
    
    // Test removing client
    channel.removeClient(&client);
    Test::assertTest(!channel.isClientinChannel(&client), "Client should not be in channel after removal");
    Test::assertTest(channel.getClients().empty(), "Channel should be empty after removing only client");
}

void testOperatorManagement() {
    Test::startTest("Operator Management");
    
    Channel channel("#test");
    Client client(1);
    
    // Test operator status
    Test::assertTest(!channel.isOperator(&client), "Client should not be operator by default");
    
    // Test adding operator
    channel.addOperator(&client);
    Test::assertTest(channel.isOperator(&client), "Client should be operator after being added");
    
    // Test adding operator twice
    channel.addOperator(&client);
    
    // Test removing operator
    channel.removeOperator(&client);
    Test::assertTest(!channel.isOperator(&client), "Client should not be operator after removal");
}

void testChannelModes() {
    Test::startTest("Channel Modes");
    
    Channel channel("#test");
    
    // Test invite-only mode
    channel.setInviteOnly(true);
    Test::assertTest(channel.isInviteOnly(), "Channel should be invite-only");
    channel.setInviteOnly(false);
    Test::assertTest(!channel.isInviteOnly(), "Channel should not be invite-only");
    
    // Test topic restriction
    channel.setTopicRestricted(true);
    Test::assertTest(channel.isTopicRestricted(), "Channel should have topic restricted");
    channel.setTopicRestricted(false);
    Test::assertTest(!channel.isTopicRestricted(), "Channel should not have topic restricted");
    
    // Test key
    channel.setKey("testkey");
    Test::assertTest(channel.getKey() == "testkey", "Channel key should be 'testkey'");
    channel.setKey("");
    Test::assertTest(channel.getKey().empty(), "Channel should have no key");
    
    // Test user limit
    channel.setUserLimit(10);
    Test::assertTest(channel.getUserLimit() == 10, "Channel should have user limit of 10");
    channel.setUserLimit(0);
    Test::assertTest(channel.getUserLimit() == 0, "Channel should have no user limit");
}

void testTopicManagement() {
    Test::startTest("Topic Management");
    
    Channel channel("#test");
    
    // Test initial topic
    Test::assertTest(channel.getTopic().empty(), "Channel should have no topic by default");
    
    // Test setting topic
    channel.setTopic("Test Topic");
    Test::assertTest(channel.getTopic() == "Test Topic", "Channel topic should be 'Test Topic'");
    
    // Test clearing topic
    channel.setTopic("");
    Test::assertTest(channel.getTopic().empty(), "Channel topic should be empty after clearing");
}

void runChannelTests() {
    Test::startTestSuite("Channel");
    
    testChannelCreation();
    testClientOperations();
    testOperatorManagement();
    testChannelModes();
    testTopicManagement();
}