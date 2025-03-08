/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CommandsUtility.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tsadouk <tsadouk@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/05 09:05:54 by tsadouk           #+#    #+#             */
/*   Updated: 2025/03/09 00:44:55 by tsadouk          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CommandExecutor.hpp"

void CommandExecutor::handleQuit(Client* client, const Command& cmd) {
	std::string quitMessage = cmd.params.empty() ? "Client Quit" : cmd.params[0];
    Server::getInstance().disconnectClient(client->getFd());
}

void CommandExecutor::handleHelp(Client* client, const Command& cmd) {
    if (cmd.params.empty()) {
        client->sendReply("705", ":Available commands:");
        
        client->sendReply("705", ":Registration commands:");
        client->sendReply("705", ":  PASS - Sets connection password");
        client->sendReply("705", ":  NICK - Sets your nickname");
        client->sendReply("705", ":  USER - Completes registration");
        
        client->sendReply("705", ":Channel operations:");
        client->sendReply("705", ":  JOIN - Joins a channel");
        client->sendReply("705", ":  PART - Leaves a channel");
        client->sendReply("705", ":  TOPIC - Sets/views channel topic");
        client->sendReply("705", ":  KICK - Removes user from channel");
        client->sendReply("705", ":  INVITE - Invites user to channel");
        client->sendReply("705", ":  MODE - Sets channel modes");
        
        client->sendReply("705", ":Communication:");
        client->sendReply("705", ":  PRIVMSG - Sends a message to users or channels");
        
        client->sendReply("705", ":Other commands:");
        client->sendReply("705", ":  QUIT - Disconnects from server");
        client->sendReply("705", ":  HELP - Shows this help message");
        if (_commands.find("DCC") != _commands.end()) {
            client->sendReply("705", ":  DCC - Direct Client Connection for file transfers");
        }
        
        client->sendReply("705", ":For help on a specific command, type: HELP <command>");
        return;
    }

    std::string command = cmd.params[0];
    std::transform(command.begin(), command.end(), command.begin(), ::toupper);
    
    std::map<std::string, CommandInfo>::iterator it = _commands.find(command);
    
    if (it != _commands.end()) {
        if (command == "PASS") {
            client->sendReply("705", ":PASS <password>");
            client->sendReply("705", ":Sets a connection password. Must be used before NICK/USER.");
        } 
        else if (command == "NICK") {
            client->sendReply("705", ":NICK <nickname>");
            client->sendReply("705", ":Sets or changes your nickname. Limited to 9 characters,");
            client->sendReply("705", ":must start with a letter and contain only letters, numbers, - or _.");
        } 
        else if (command == "USER") {
            client->sendReply("705", ":USER <username> <hostname> <servername> <realname>");
            client->sendReply("705", ":Specifies user information. Used during connection registration.");
        } 
        else if (command == "JOIN") {
            client->sendReply("705", ":JOIN <channel> [key]");
            client->sendReply("705", ":Joins the specified channel with optional key (password).");
            client->sendReply("705", ":Channels start with # or &, e.g.: JOIN #general");
        } 
        else if (command == "PART") {
            client->sendReply("705", ":PART <channel> [reason]");
            client->sendReply("705", ":Leaves the specified channel with optional reason.");
            client->sendReply("705", ":Example: PART #general :Going to lunch");
        } 
        else if (command == "PRIVMSG") {
            client->sendReply("705", ":PRIVMSG <target> :<message>");
            client->sendReply("705", ":Sends a message to a user or channel.");
            client->sendReply("705", ":Examples: PRIVMSG #channel :Hello everyone!");
            client->sendReply("705", ":          PRIVMSG nickname :Hi there!");
        } 
        else if (command == "TOPIC") {
            client->sendReply("705", ":TOPIC <channel> [:<topic>]");
            client->sendReply("705", ":Gets or sets the channel topic.");
            client->sendReply("705", ":- TOPIC #channel       - View the current topic");
            client->sendReply("705", ":- TOPIC #channel :     - Clear the topic");
            client->sendReply("705", ":- TOPIC #channel :text - Set the topic to 'text'");
            client->sendReply("705", ":Requires operator status if channel is +t.");
        } 
        else if (command == "KICK") {
            client->sendReply("705", ":KICK <channel> <user> [:<reason>]");
            client->sendReply("705", ":Removes a user from a channel. Requires operator status.");
            client->sendReply("705", ":Example: KICK #channel nickname :Misbehaving");
        } 
        else if (command == "INVITE") {
            client->sendReply("705", ":INVITE <user> <channel>");
            client->sendReply("705", ":Invites a user to a channel. If channel is +i,");
            client->sendReply("705", ":requires operator status to invite users.");
        } 
        else if (command == "MODE") {
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
        else if (command == "QUIT") {
            client->sendReply("705", ":QUIT [:<reason>]");
            client->sendReply("705", ":Disconnects from the server with optional reason.");
            client->sendReply("705", ":Example: QUIT :Gone to lunch");
        } 
        else if (command == "HELP") {
            client->sendReply("705", ":HELP [command]");
            client->sendReply("705", ":Without parameters, shows a list of available commands.");
            client->sendReply("705", ":With a command parameter, shows detailed help for that command.");
            client->sendReply("705", ":Example: HELP JOIN");
        }
        else if (command == "DCC") {
            client->sendReply("705", ":DCC SEND <filename> <target>");
            client->sendReply("705", ":Initiates file transfer to another user.");
            client->sendReply("705", ":This is a bonus feature and may not be supported by all clients.");
        }
        else {
            client->sendReply("705", ":" + command + ": " + it->second.helpShort);
        }
    } else {
        client->sendReply("705", ":No help available for command: " + cmd.params[0]);
        client->sendReply("705", ":Type HELP for a list of available commands.");
    }
}
