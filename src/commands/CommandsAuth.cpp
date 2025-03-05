/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CommandsAuth.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tsadouk <tsadouk@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/05 09:04:15 by tsadouk           #+#    #+#             */
/*   Updated: 2025/03/05 10:08:03 by tsadouk          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CommandExecutor.hpp"

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

	if (!Server::getInstance().isNicknameAvailable(nickname)) {
		client->sendReply("433", nickname + " Nickname is already in use");
		return;
	}

    std::string oldNick = client->getNickname();
    client->setNickname(nickname);
    
    if (oldNick.empty()) {
        client->sendReply("001", ":Nickname successfully registered");
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
