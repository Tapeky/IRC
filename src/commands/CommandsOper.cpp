/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CommandsOper.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tsadouk <tsadouk@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/05 09:10:19 by tsadouk           #+#    #+#             */
/*   Updated: 2025/03/05 10:10:16 by tsadouk          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CommandExecutor.hpp"

void CommandExecutor::handleKick(Client* client, const Command& cmd)
{  
	// Check for required parameters: channel and target nick
	if (cmd.params.size() < 2) {
		client->sendReply("461", "KICK :Not enough parameters");
		return;
	}
	
	const std::string& channelName = cmd.params[0];
	const std::string& target = cmd.params[1];
	
	// Validate channel exists
	Channel* channel = Server::getInstance().getChannel(channelName);
	if (!channel) {
		client->sendReply("403", channelName + " :No such channel");
		return;
	}
	
	// Validate target user exists
	Client* targetClient = Server::getInstance().getClientByNickname(target);
	if (!targetClient) {
		client->sendReply("401", target + " :No such nick/channel");
		return;
	}
	
	// Verify kicker is in the channel
	if (!channel->isClientinChannel(client)) {
		client->sendReply("442", channelName + " :You're not on that channel");
		return;
	}
	
	// Verify kicker has operator privileges
	if (!channel->isOperator(client)) {
		client->sendReply("482", channelName + " :You're not channel operator");
		return;
	}
	
	// Verify target is in the channel
	if (!channel->isClientinChannel(targetClient)) {
		client->sendReply("441", target + " " + channelName + " :They aren't on that channel");
		return;
	}
	
	// Process kick reason if provided
	std::string reason = cmd.params.size() > 2 ? cmd.params[2] : "No reason specified";
	if (reason[0] == ':') reason = reason.substr(1);
	
	// Broadcast kick message to all channel members
	std::string kickMsg = ":" + client->getNickname() + " KICK " + channelName + " " + target + " :" + reason;
	const std::vector<Client*>& clients = channel->getClients();
	for (size_t i = 0; i < clients.size(); ++i) {
		clients[i]->sendMessage(kickMsg);
	}
	
	// Remove target from channel
	channel->removeClient(targetClient);
}

void CommandExecutor::handleInvite(Client* client, const Command& cmd)
{
	// Check for required parameters: target nick and channel
	if (cmd.params.size() < 2) {
		client->sendReply("461", "INVITE :Not enough parameters");
		return;
	}
	
	const std::string& target = cmd.params[0];
	const std::string& channelName = cmd.params[1];
	
	// Validate target user exists
	Client* targetClient = Server::getInstance().getClientByNickname(target);
	if (!targetClient) {
		client->sendReply("401", target + " :No such nick/channel");
		return;
	}
	
	// Validate channel exists
	Channel* channel = Server::getInstance().getChannel(channelName);
	if (!channel) {
		client->sendReply("403", channelName + " :No such channel");
		return;
	}
	
	// Verify inviter is in the channel
	if (!channel->isClientinChannel(client)) {
		client->sendReply("442", channelName + " :You're not on that channel");
		return;
	}
	
	// Check operator status for invite-only channels
	if (channel->isInviteOnly() && !channel->isOperator(client)) {
		client->sendReply("482", channelName + " :You're not channel operator");
		return;
	}
	
	// Verify target is not already in the channel
	if (channel->isClientinChannel(targetClient)) {
		client->sendReply("443", target + " " + channelName + " :is already on channel");
		return;
	}
	
	// Add target to the invited users list
	channel->inviteClient(targetClient);
	
	// Send invitation to target
	std::string inviteMsg = ":" + client->getNickname() + " INVITE " + target + " :" + channelName;
	targetClient->sendMessage(inviteMsg);
	
	// Confirm invitation to sender
	client->sendReply("341", target + " " + channelName);
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

	const std::vector<Client*>& clients = channel->getClients();

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
            if (adding) {
                channel->addOperator(targetClient);
			}
            else {
                channel->removeOperator(targetClient);
			}
			std::string modeChange = adding ? "+o" : "-o";
			std::string modeMsg = ":" + client->getNickname() + " MODE " + channelName + " " +
					modeChange + " " + targetClient->getNickname();
			for (size_t j = 0; j < clients.size(); ++j)
				clients[j]->sendMessage(modeMsg);
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
			for (size_t i = 0; i < clients.size(); ++i) {
				clients[i]->sendReply("353", "= " + channelName + " :" + namesList);
				clients[i]->sendReply("366", channelName + " :End of /NAMES list");
			}			
			
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
}
