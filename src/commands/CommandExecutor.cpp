/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CommandExecutor.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tsadouk <tsadouk@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/04 00:09:27 by tsadouk           #+#    #+#             */
/*   Updated: 2025/02/06 18:37:22 by tsadouk          ###   ########.fr       */
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
    if (cmd.params.empty()) {
        client->sendReply("461", "USER :Not enough parameters");
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
    for (size_t i = 0; i < clients.size(); ++i)
        clients[i]->sendMessage(joinMsg);
    
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
		bool isInChannel = false;
		for (size_t i = 0; i < clients.size(); ++i) {
			if (clients[i] == client) {
				isInChannel = true;
				break;
			}
		}

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
