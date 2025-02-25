/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CommandExecutor.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tsadouk <tsadouk@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/04 00:09:27 by tsadouk           #+#    #+#             */
/*   Updated: 2025/02/25 08:24:33 by tsadouk          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CommandExecutor.hpp"

void CommandExecutor::executeCommand(Client* client, const Command& cmd) {
	if (cmd.command == "PASS") {
		handlePass(client, cmd);
	}
	else if (!client->isAuthenticated()) {
		client->sendReply("451", ":You have not registered");
		return;
	}
	else if (cmd.command == "NICK") {
		handleNick(client, cmd);
	}
	else if (cmd.command == "USER") {
		handleUser(client, cmd);
	}
	else if (cmd.command == "QUIT") {
		handleQuit(client, cmd);
		return;
	}
	else if (cmd.command == "JOIN") {
		handleJoin(client, cmd);
	}
	else if (cmd.command == "PRIVMSG") {
		handlePrivmsg(client, cmd);
	}
	else if (cmd.command == "PART") {
		handlePart(client, cmd);
	}
	else if (cmd.command == "TOPIC") {
		handleTopic(client, cmd);
	}
	else if (cmd.command == "KICK") {
		handleKick(client, cmd);
	}
	else if (cmd.command == "INVITE") {
		handleInvite(client, cmd);
	}
	else if (cmd.command == "MODE") {
		handleMode(client, cmd);
	}
	else if (cmd.command == "HELP") {
		handleHelp(client, cmd);
	}
	else if (cmd.command == "DCC") {
		handleDCC(client, cmd);
		client->sendReply("8000", ":On est bien la");
	}
	// Autres commandes a faire...
}

void CommandExecutor::handleNick(Client* client, const Command& cmd) {
	if (cmd.params.empty()) {
		client->sendReply("431", "No nickname given");
		return;
	}
	const std::string& nickname = cmd.params[0];
	if (nickname.size() > 9 || !std::isalpha(nickname[0])) {
		client->sendReply("432", nickname +  "Erroneous nickname");
		return;
	}

	for (size_t i = 0; i < nickname.length(); ++i) {
		if (!std::isalnum(nickname[i]) && nickname[i] != '-' && nickname[i] != '_') {
			client->sendReply("432", nickname +  "Erroneous nickname");
			return;
		}
	}

	// Check if nickname is already in use
	if (!Server::getInstance().isNicknameAvailable(nickname)) {
		client->sendReply("433", nickname + " Nickname is already in use");
		return;
	}

    std::string oldNick = client->getNickname();
    client->setNickname(nickname);
    
    if (oldNick.empty()) {
        client->sendReply("001", ":Nickname successfully registered");
        // Si username est aussi défini
        if (!client->getUsername().empty()) {
            client->sendReply("002", ":Registration complete");
        }
    } else {
        Server::getInstance().broadcastMessage(":" + oldNick + " NICK " + nickname);
    }

}

void CommandExecutor::handlePass(Client* client, const Command& cmd) {
	if (cmd.params.empty()) {
		client->sendReply("461", "PASS :Not enough parameters");
		return;
	}

	if (cmd.params[0] == Server::getInstance().getPassword()) {
		client->setAuthenticated(true);
		client->sendReply("001", ":Authentication successful");
	} else {
		client->sendReply("464", ":Password Incorect");
	}
}

void CommandExecutor::handleUser(Client* client, const Command& cmd) {
    if (cmd.params.size() < 4) {
        client->sendReply("461", "USER :Not enough parameters\n Usage: <username> <hostname> <servername> <realname>");
        return;
    }
    
	if (!client->getUsername().empty()) {
        client->sendReply("462", ":You may not reregister");
        return;
    }

    client->setUsername(cmd.params[0]);
    
    if (!client->getNickname().empty()) {
        client->sendReply("001", ":Welcome to the Internet Relay Network " + 
                                client->getNickname() + "!" + client->getUsername() + "@" + "localhost");
        client->sendReply("002", ":Your host is ircserv, running version 1.0");
        client->sendReply("003", ":This server was created today");
        client->sendReply("004", "ircserv 1.0 o o");
    }
}

void CommandExecutor::handleQuit(Client* client, const Command& cmd) {
	std::string quitMessage = cmd.params.empty() ? "Client Quit" : cmd.params[0];
	Server::getInstance().broadcastMessage(":" + client->getNickname() + " QUIT :" + quitMessage);
    Server::getInstance().disconnectClient(client->getFd());
}

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
    channel->addClient(client);
	
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


void CommandExecutor::handlePrivmsg(Client* client, const Command& cmd) {
	if (cmd.params.size() < 2) {
		client->sendReply("411", "PRIVMSG :Not enough parameters");
		return;
	}

	const std::string& target = cmd.params[0];
	const std::string& message = cmd.params[1];

	// Message vers un channel
	if (target[0] == '#' || target[0] == '&') {
		Channel *channel = Server::getInstance().getChannel(target);
		if (!channel) {
			client->sendReply("401", target + " :No such channel");
			return;
		}

		// On verifie si le client est dans le channel
		const std::vector<Client*>& clients = channel->getClients();
		bool isInChannel = channel->isClientinChannel(client);
		if (!isInChannel) {
			client->sendReply("404", target + " :Cannot send to channel");
			return;
		}

		// On envoie le message a tous les clients du channel sauf l expediteur
		std::string fullMessage = ":" + client->getNickname() + " PRIVMSG " + target + " :" + message;
		for (size_t i = 0; i < clients.size(); ++i) {
			if (clients[i] != client)
				clients[i]->sendMessage(fullMessage);
		}
	}
	// Message prive
	else {
		Client* targetClient = Server::getInstance().getClientByNickname(target);
		if (!targetClient) {
			client->sendReply("401", target + " :No such nick/channel");
			return;
		}
		std::string fullMessage = ":" + client->getNickname() + " PRIVMSG " + target + " :" + message;
		targetClient->sendMessage(fullMessage);
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
    
    // Vérifier si le client est dans le canal
    if (!channel->isClientinChannel(client)) {
        client->sendReply("442", channelName + " :You're not on that channel");
        return;
    }
    
    // Si pas de second paramètre, on affiche le topic
    if (cmd.params.size() == 1) {
        if (channel->getTopic().empty()) {
            client->sendReply("331", channelName + " :No topic is set");
        } else {
            client->sendReply("332", channelName + " :" + channel->getTopic());
        }
        return;
    }
    
    // Pour modifier le topic, vérifier les restrictions
    if (channel->isTopicRestricted() && !channel->isOperator(client)) {
        client->sendReply("482", channelName + " :You're not channel operator");
        return;
    }
    
    // Traitement pour effacer ou modifier le topic
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

void CommandExecutor::handleKick(Client* client, const Command& cmd)
{  
    if (cmd.params.size() < 2) {
        client->sendReply("461", "KICK :Not enough parameters");
        return;
    }
    
    const std::string& channelName = cmd.params[0];
    const std::string& target = cmd.params[1];
    
    Channel* channel = Server::getInstance().getChannel(channelName);
    if (!channel) {
        client->sendReply("403", channelName + " :No such channel");
        return;
    }
    
    Client* targetClient = Server::getInstance().getClientByNickname(target);
    if (!targetClient) {
        client->sendReply("401", target + " :No such nick/channel");
        return;
    }
    
    // Vérifier si le client est dans le canal
    if (!channel->isClientinChannel(client)) {
        client->sendReply("442", channelName + " :You're not on that channel");
        return;
    }
    
    // Vérifier si le client est opérateur
    if (!channel->isOperator(client)) {
        client->sendReply("482", channelName + " :You're not channel operator");
        return;
    }
    
    // Vérifier si la cible est dans le canal
    if (!channel->isClientinChannel(targetClient)) {
        client->sendReply("441", target + " " + channelName + " :They aren't on that channel");
        return;
    }
    
    // Préparer le message de raison (optionnel)
    std::string reason = cmd.params.size() > 2 ? cmd.params[2] : "No reason specified";
    if (reason[0] == ':') reason = reason.substr(1);
    
    // Envoyer le message KICK à tous les clients du canal
    std::string kickMsg = ":" + client->getNickname() + " KICK " + channelName + " " + target + " :" + reason;
    const std::vector<Client*>& clients = channel->getClients();
    for (size_t i = 0; i < clients.size(); ++i) {
        clients[i]->sendMessage(kickMsg);
    }
    
    // Retirer la cible du canal
    channel->removeClient(targetClient);
}

void CommandExecutor::handleInvite(Client* client, const Command& cmd)
{
    if (cmd.params.size() < 2) {
        client->sendReply("461", "INVITE :Not enough parameters");
        return;
    }
    
    const std::string& target = cmd.params[0];
    const std::string& channelName = cmd.params[1];
    
    Client* targetClient = Server::getInstance().getClientByNickname(target);
    if (!targetClient) {
        client->sendReply("401", target + " :No such nick/channel");
        return;
    }
    
    Channel* channel = Server::getInstance().getChannel(channelName);
    if (!channel) {
        client->sendReply("403", channelName + " :No such channel");
        return;
    }
    
    // Vérifier si le client est dans le canal
    if (!channel->isClientinChannel(client)) {
        client->sendReply("442", channelName + " :You're not on that channel");
        return;
    }
    
    // Vérifier les permissions si le canal est en mode invite-only
    if (channel->isInviteOnly() && !channel->isOperator(client)) {
        client->sendReply("482", channelName + " :You're not channel operator");
        return;
    }
    
    // Vérifier si la cible est déjà dans le canal
    if (channel->isClientinChannel(targetClient)) {
        client->sendReply("443", target + " " + channelName + " :is already on channel");
        return;
    }
    
    // Envoyer l'invitation
    std::string inviteMsg = ":" + client->getNickname() + " INVITE " + target + " :" + channelName;
    targetClient->sendMessage(inviteMsg);
    
    // Confirmer l'invitation à l'expéditeur
    client->sendReply("341", target + " " + channelName);
}

void CommandExecutor::handleDCC(Client* client, const Command& cmd) {
	std::cout << "DCC command received with " << cmd.params.size() << " parameters" << std::endl;
    if (cmd.params.size() < 2) {
        client->sendReply("461", "DCC :Not enough parameters");
        return;
    }

    if (cmd.params[0] == "SEND") {
        try {
            FileTransfer transfer(cmd.params[1]);
            
            // Conversion de l'adresse IP en format numérique
            struct in_addr addr;
            inet_aton(transfer.getIP().c_str(), &addr);
            unsigned long ipNum = ntohl(addr.s_addr);

            // Construction du message DCC selon le protocole standard
            std::string message = ":";
            message += client->getNickname();
            message += " PRIVMSG ";
            message += cmd.params[2];
            message += " :\001DCC SEND \"";
            message += transfer.getFilename();
            message += "\" ";
            char ipStr[16];
            sprintf(ipStr, "%lu", ipNum);
            message += ipStr;
            message += " ";
            char portStr[8];
            sprintf(portStr, "%d", transfer.getPort());
            message += portStr;
            message += " ";
            char sizeStr[16];
            sprintf(sizeStr, "%lu", transfer.getFilesize());
            message += sizeStr;
            message += "\001";

            client->sendMessage(message);
            transfer.initiateSend();
        }
        catch (const std::exception& e) {
            client->sendReply("411", std::string(":") + e.what());
        }
    }
}


void CommandExecutor::handleMode(Client* client, const Command& cmd) {
    if (cmd.params.size() < 2) {
        client->sendReply("461", "MODE :Not enough parameters");
        return;
    }

    const std::string& channelName = cmd.params[0];
    const std::string& modeString = cmd.params[1];
    
    Channel* channel = Server::getInstance().getChannel(channelName);
    if (!channel) {
        client->sendReply("403", channelName + " :No such channel");
        return;
    }

    if (!channel->isOperator(client)) {
        client->sendReply("482", channelName + " :You're not channel operator");
        return;
    }

    bool adding = true;
    size_t paramIndex = 2;

    for (size_t i = 0; i < modeString.length(); ++i) {
        if (modeString[i] == '+') {
            adding = true;
            continue;
        }
        if (modeString[i] == '-') {
            adding = false;
            continue;
        }

        char mode = modeString[i];
        if (mode == 'i') {
            channel->setInviteOnly(adding);
        }
        else if (mode == 't') {
            channel->setTopicRestricted(adding);
        }
        else if (mode == 'k') {
            if (adding) {
                if (paramIndex >= cmd.params.size()) {
                    client->sendReply("461", "MODE +k :Not enough parameters");
                    continue;
                }
                channel->setKey(cmd.params[paramIndex++]);
            } else {
                channel->setKey("");
            }
        }
        else if (mode == 'o') {
            if (paramIndex >= cmd.params.size()) {
                client->sendReply("461", "MODE +/-o :Not enough parameters");
                continue;
            }
            Client* targetClient = Server::getInstance().getClientByNickname(cmd.params[paramIndex++]);
            if (!targetClient) {
                client->sendReply("401", cmd.params[paramIndex-1] + " :No such nick/channel");
                continue;
            }
            if (adding)
                channel->addOperator(targetClient);
            else
                channel->removeOperator(targetClient);
        }
        else if (mode == 'l') {
            if (adding) {
                if (paramIndex >= cmd.params.size()) {
                    client->sendReply("461", "MODE +l :Not enough parameters");
                    continue;
                }
                int limit = atoi(cmd.params[paramIndex++].c_str());
                channel->setUserLimit(limit);
            } else {
                channel->setUserLimit(0);
            }
        }
        else {
            client->sendReply("472", std::string(1, modeString[i]) + " :is unknown mode char to me");
        }
    }

    // Notifier tous les clients du canal du changement de mode
    std::string modeMsg = ":" + client->getNickname() + " MODE " + channelName + " " + modeString;
    for (size_t i = paramIndex; i < cmd.params.size(); ++i)
        modeMsg += " " + cmd.params[i];
    
    const std::vector<Client*>& clients = channel->getClients();
    for (size_t i = 0; i < clients.size(); ++i)
        clients[i]->sendMessage(modeMsg);
}

// Function to compare case-insensitive
namespace {
    bool iequals(const std::string& a, const std::string& b) {
        if (a.length() != b.length())
            return false;
        for (size_t i = 0; i < a.length(); ++i)
            if (tolower(a[i]) != tolower(b[i]))
                return false;
        return true;
    }
}

void CommandExecutor::handleHelp(Client* client, const Command& cmd) {
    if (cmd.params.empty()) {
        client->sendReply("705", ":Available commands:");
        client->sendReply("705", ":PASS, NICK, USER - Registration commands");
        client->sendReply("705", ":JOIN, PART, TOPIC, KICK, INVITE - Channel operations");
        client->sendReply("705", ":MODE - Change channel modes");
        client->sendReply("705", ":PRIVMSG - Send messages to users or channels");
        client->sendReply("705", ":QUIT - Disconnect from server");
        client->sendReply("705", ":HELP - This help message");
        client->sendReply("705", ":For help on a specific command, type: HELP <command>");
        return;
    }

    const std::string& command = cmd.params[0];
    
    if (iequals(command, "PASS")) {
        client->sendReply("705", ":PASS <password>");
        client->sendReply("705", ":Sets a connection password. Must be used before NICK/USER.");
    } 
    else if (iequals(command, "NICK")) {
        client->sendReply("705", ":NICK <nickname>");
        client->sendReply("705", ":Sets or changes your nickname. Limited to 9 characters,");
        client->sendReply("705", ":must start with a letter and contain only letters, numbers, - or _.");
    } 
    else if (iequals(command, "USER")) {
        client->sendReply("705", ":USER <username> <hostname> <servername> <realname>");
        client->sendReply("705", ":Specifies user information. Used during connection registration.");
    } 
    else if (iequals(command, "JOIN")) {
        client->sendReply("705", ":JOIN <channel> [key]");
        client->sendReply("705", ":Joins the specified channel with optional key (password).");
        client->sendReply("705", ":Channels start with # or &, e.g.: JOIN #general");
    } 
    else if (iequals(command, "PART")) {
        client->sendReply("705", ":PART <channel> [reason]");
        client->sendReply("705", ":Leaves the specified channel with optional reason.");
        client->sendReply("705", ":Example: PART #general :Going to lunch");
    } 
    else if (iequals(command, "PRIVMSG")) {
        client->sendReply("705", ":PRIVMSG <target> :<message>");
        client->sendReply("705", ":Sends a message to a user or channel.");
        client->sendReply("705", ":Examples: PRIVMSG #channel :Hello everyone!");
        client->sendReply("705", ":          PRIVMSG nickname :Hi there!");
    } 
    else if (iequals(command, "TOPIC")) {
        client->sendReply("705", ":TOPIC <channel> [:<topic>]");
        client->sendReply("705", ":Gets or sets the channel topic.");
        client->sendReply("705", ":- TOPIC #channel       - View the current topic");
        client->sendReply("705", ":- TOPIC #channel :     - Clear the topic");
        client->sendReply("705", ":- TOPIC #channel :text - Set the topic to 'text'");
        client->sendReply("705", ":Requires operator status if channel is +t.");
    } 
    else if (iequals(command, "KICK")) {
        client->sendReply("705", ":KICK <channel> <user> [:<reason>]");
        client->sendReply("705", ":Removes a user from a channel. Requires operator status.");
        client->sendReply("705", ":Example: KICK #channel nickname :Misbehaving");
    } 
    else if (iequals(command, "INVITE")) {
        client->sendReply("705", ":INVITE <user> <channel>");
        client->sendReply("705", ":Invites a user to a channel. If channel is +i,");
        client->sendReply("705", ":requires operator status to invite users.");
    } 
    else if (iequals(command, "MODE")) {
        client->sendReply("705", ":MODE <channel> <modes> [parameters]");
        client->sendReply("705", ":Sets channel modes. Requires operator status.");
        client->sendReply("705", ":Available modes:");
        client->sendReply("705", ":+i: Set channel to invite-only");
        client->sendReply("705", ":-i: Remove invite-only restriction");
        client->sendReply("705", ":+t: Only operators can change topic");
        client->sendReply("705", ":-t: Anyone can change topic");
        client->sendReply("705", ":+k <key>: Set channel password/key");
        client->sendReply("705", ":-k: Remove channel password");
        client->sendReply("705", ":+o <nick>: Give operator status to user");
        client->sendReply("705", ":-o <nick>: Remove operator status from user");
        client->sendReply("705", ":+l <limit>: Set user limit for channel");
        client->sendReply("705", ":-l: Remove user limit");
        client->sendReply("705", ":Examples: MODE #channel +t");
        client->sendReply("705", ":          MODE #channel +k password");
        client->sendReply("705", ":          MODE #channel +o nickname");
    } 
    else if (iequals(command, "QUIT")) {
        client->sendReply("705", ":QUIT [:<reason>]");
        client->sendReply("705", ":Disconnects from the server with optional reason.");
        client->sendReply("705", ":Example: QUIT :Gone to lunch");
    } 
    else if (iequals(command, "DCC")) {
        client->sendReply("705", ":DCC SEND <filename> <target>");
        client->sendReply("705", ":Initiates file transfer to another user.");
        client->sendReply("705", ":This is a bonus feature and may not be supported by all clients.");
    } 
    else {
        client->sendReply("705", ":No help available for command: " + command);
        client->sendReply("705", ":Type HELP for a list of available commands.");
    }
}