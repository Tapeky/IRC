/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CommandsChannel.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tsadouk <tsadouk@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/05 09:08:19 by tsadouk           #+#    #+#             */
/*   Updated: 2025/03/05 10:09:45 by tsadouk          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CommandExecutor.hpp"

void CommandExecutor::handleJoin(Client* client, const Command& cmd) {
    if (cmd.params.empty()) {
        client->sendReply("461", "JOIN :Not enough parameters");
        return;
    }

    const std::string& channelName = cmd.params[0];
    
    // Check Format
    if (channelName[0] != '#' && channelName[0] != '&') {
        client->sendReply("403", channelName + " :No such channel");
        return;
    }
    
    // Check Length
    if (channelName.length() > 50) {
        client->sendReply("403", channelName + " :Channel name too long");
        return;
    }
    
    // Check unvalid characters
    for (size_t i = 1; i < channelName.length(); ++i) {
        if (!isalnum(channelName[i]) && channelName[i] != '-' && channelName[i] != '_') {
            client->sendReply("403", channelName + " :Invalid channel name");
            return;
        }
    }
    
    Channel* channel = Server::getInstance().getOrCreateChannel(cmd.params[0]);
    
    // Check if key is required and if it is correct
    if (!channel->getKey().empty()) {
        if (cmd.params.size() < 2 || cmd.params[1] != channel->getKey()) {
            client->sendReply("475", channelName + " :Cannot join channel (+k) - bad key");
            return;
        }
    }
    
    // Check if channel is invite only and if client is invited
	if (channel->isInviteOnly()) {
        if (!channel->isOperator(client) && !channel->isInvited(client)) {
            client->sendReply("473", channelName + " :Cannot join channel (+i) - you must be invited");
            return;
        }
        // if invite used remove it
        if (!channel->isOperator(client)) {
            channel->removeInvite(client);
        }
    }
    
    // Check if channel is full
    if (channel->getUserLimit() > 0 && channel->getClients().size() >= channel->getUserLimit()) {
        client->sendReply("471", channelName + " :Cannot join channel (+l) - channel is full");
        return;
    }
    
    bool wasEmpty = channel->getClients().empty();
    channel->addClient(client);
    
    // Add operator if channel was empty
    if (wasEmpty) {
        channel->addOperator(client);
    }
    
    // Inform all the clients
    std::string joinMsg = ":" + client->getNickname() + " JOIN " + channelName;
    const std::vector<Client*>& clients = channel->getClients();
    for (size_t i = 0; i < clients.size(); ++i) {
        clients[i]->sendMessage(joinMsg);
    }
    
    // Display the list of clients in the channel
    std::string namesList = "";
    for (size_t i = 0; i < clients.size(); ++i) {
        if (channel->isOperator(clients[i])) {
            namesList += "@";
        }
        namesList += clients[i]->getNickname();
        if (i < clients.size() - 1) {
            namesList += " ";
        }
    }
    
    client->sendReply("353", "= " + channelName + " :" + namesList);
    client->sendReply("366", channelName + " :End of /NAMES list");
    
    // Display the topic
    if (!channel->getTopic().empty()) {
        client->sendReply("332", channelName + " :" + channel->getTopic());
    }
}

void CommandExecutor::handlePart(Client *client, const Command& cmd) {
	if (cmd.params.empty()) {
		client->sendReply("461", "PART :Not enough parameters");
		return;
	}

	const std::string& channelName = cmd.params[0];
	Channel* channel = Server::getInstance().getChannel(channelName);

	if (!channel) {
		client->sendReply("403", channelName + " :No such channel");
		return;
	}

	std::string partMsg = ":" + client->getNickname() + " PART " + channelName;
	if (cmd.params.size() > 1)
		partMsg += " :" + cmd.params[1];

	const std::vector<Client*>& clients = channel->getClients();
	for (size_t i = 0; i < clients.size(); ++i) 
		clients[i]->sendMessage(partMsg);

	channel->removeClient(client);
}

void CommandExecutor::handleTopic(Client *client, const Command &cmd)
{
	if (cmd.params.empty()) {
		client->sendReply("461", "TOPIC :Not enough parameters");
		return;
	}
	
	const std::string& channelName = cmd.params[0];
	Channel* channel = Server::getInstance().getChannel(channelName);
	
	if (!channel) {
		client->sendReply("403", channelName + " :No such channel");
		return;
	}
	
	// Check if client is in the channel
	if (!channel->isClientinChannel(client)) {
		client->sendReply("442", channelName + " :You're not on that channel");
		return;
	}
	
	// If no second parameter, display the topic
	if (cmd.params.size() == 1) {
		if (channel->getTopic().empty()) {
			client->sendReply("331", channelName + " :No topic is set");
		} else {
			client->sendReply("332", channelName + " :" + channel->getTopic());
		}
		return;
	}
	
	// To modify the topic, check restrictions
	if (channel->isTopicRestricted() && !channel->isOperator(client)) {
		client->sendReply("482", channelName + " :You're not channel operator");
		return;
	}
	
	// Process to clear or modify the topic
	if (cmd.params[1] == ":") {
		channel->setTopic("");
		std::string topicMsg = ":" + client->getNickname() + " TOPIC " + channelName + " :";
		const std::vector<Client*>& clients = channel->getClients();
		for (size_t i = 0; i < clients.size(); ++i) 
			clients[i]->sendMessage(topicMsg);
	} else {
		std::string topic = cmd.params[1];
		if (topic[0] == ':')
			topic = topic.substr(1);
		
		channel->setTopic(topic);
		std::string topicMsg = ":" + client->getNickname() + " TOPIC " + channelName + " :" + topic;
		const std::vector<Client*>& clients = channel->getClients();
		for (size_t i = 0; i < clients.size(); ++i) 
			clients[i]->sendMessage(topicMsg);
	}
}
