/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CommandsMsg.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tsadouk <tsadouk@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/05 09:12:00 by tsadouk           #+#    #+#             */
/*   Updated: 2025/03/05 10:09:49 by tsadouk          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CommandExecutor.hpp"

void CommandExecutor::handlePrivmsg(Client* client, const Command& cmd) {
	if (cmd.params.size() < 2) {
		client->sendReply("411", "PRIVMSG :Not enough parameters");
		return;
	}

	const std::string& target = cmd.params[0];
	const std::string& message = cmd.params[1];

	// Message to a channel
	if (target[0] == '#' || target[0] == '&') {
		Channel *channel = Server::getInstance().getChannel(target);
		if (!channel) {
			client->sendReply("401", target + " :No such channel");
			return;
		}

		// Check if the client is in the channel
		const std::vector<Client*>& clients = channel->getClients();
		bool isInChannel = channel->isClientinChannel(client);
		if (!isInChannel) {
			client->sendReply("404", target + " :Cannot send to channel");
			return;
		}

		// Send the message to all clients in the channel except the sender
		std::string fullMessage = ":" + client->getNickname() + " PRIVMSG " + target + " :" + message;
		for (size_t i = 0; i < clients.size(); ++i) {
			if (clients[i] != client)
				clients[i]->sendMessage(fullMessage);
		}
	}
	// Private message
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
