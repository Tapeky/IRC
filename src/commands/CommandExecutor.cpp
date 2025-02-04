/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CommandExecutor.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tsadouk <tsadouk@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/04 00:09:27 by tsadouk           #+#    #+#             */
/*   Updated: 2025/02/04 15:15:19 by tsadouk          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CommandExecutor.hpp"

void CommandExecutor::executeCommand(Client* client, const Command& cmd) {
	if (cmd.command == "NICK")
		handleNick(client, cmd);
	// else if (cmd.command == "USER")
	// 	handleUser(client, cmd);
	// else if (cmd.command == "QUIT")
	// 	handleQuit(client, cmd);
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
		client->sendReply("433", nickname + "Nickname is already in use");
		return;
	}

	std::string oldNickname = client->getNickname();
	client->setNickname(nickname);

	if (!oldNickname.empty()) {
		Server::getInstance().broadcastMessage(":" + oldNickname + " NICK " + nickname);
	}

}
