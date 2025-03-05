/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CommandExecutor.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tsadouk <tsadouk@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/04 00:09:27 by tsadouk           #+#    #+#             */
/*   Updated: 2025/03/05 10:07:57 by tsadouk          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CommandExecutor.hpp"

std::map<std::string, CommandExecutor::CommandInfo> CommandExecutor::_commands;
//CommandExecutor::CommandExecutor() {}

void CommandExecutor::initCommands() {
    _commands["PASS"] = CommandInfo(&CommandExecutor::handlePass, AUTH_NONE, "Sets connection password");
    _commands["NICK"] = CommandInfo(&CommandExecutor::handleNick, AUTH_BASIC, "Sets your nickname");
    _commands["USER"] = CommandInfo(&CommandExecutor::handleUser, AUTH_BASIC, "Completes registration");
    _commands["QUIT"] = CommandInfo(&CommandExecutor::handleQuit, AUTH_BASIC, "Disconnects from server");
    _commands["JOIN"] = CommandInfo(&CommandExecutor::handleJoin, AUTH_REGISTERED, "Joins a channel");
    _commands["PRIVMSG"] = CommandInfo(&CommandExecutor::handlePrivmsg, AUTH_REGISTERED, "Sends a message");
    _commands["PART"] = CommandInfo(&CommandExecutor::handlePart, AUTH_REGISTERED, "Leaves a channel");
    _commands["TOPIC"] = CommandInfo(&CommandExecutor::handleTopic, AUTH_REGISTERED, "Sets/views channel topic");
    _commands["KICK"] = CommandInfo(&CommandExecutor::handleKick, AUTH_REGISTERED, "Removes user from channel");
    _commands["INVITE"] = CommandInfo(&CommandExecutor::handleInvite, AUTH_REGISTERED, "Invites user to channel");
    _commands["MODE"] = CommandInfo(&CommandExecutor::handleMode, AUTH_REGISTERED, "Sets channel modes");
    _commands["HELP"] = CommandInfo(&CommandExecutor::handleHelp, AUTH_BASIC, "Shows help information");
    _commands["DCC"] = CommandInfo(&CommandExecutor::handleDCC, AUTH_REGISTERED, "Direct Client Connection");
}

bool CommandExecutor::checkAuth(Client* client, CommandAuth requiredAuth) {
    switch (requiredAuth) {
        case AUTH_NONE:
            return true;
        case AUTH_BASIC:
            return client->isAuthenticated();
        case AUTH_REGISTERED:
            return client->isAuthenticated() && !client->getUsername().empty();
        default:
            return false;
    }
}

void CommandExecutor::executeCommand(Client* client, const Command& cmd) {
    CommandExecutor executor;
    
    std::map<std::string, CommandInfo>::iterator it = _commands.find(cmd.command);
    if (it == _commands.end()) {
        client->sendReply("421", cmd.command + " :Unknown command");
        return;
    }
    
    if (!executor.checkAuth(client, it->second.authLevel)) {
        if (it->second.authLevel == AUTH_BASIC) {
            client->sendReply("451", ":You have not registered");
        } else if (it->second.authLevel == AUTH_REGISTERED) {
            client->sendReply("451", ":You need to set a username (USER command)");
        }
        return;
    }
    
    (executor.*(it->second.handler))(client, cmd);
}

std::vector<std::string> CommandExecutor::getCommandList() {
    std::vector<std::string> result;
    for (std::map<std::string, CommandInfo>::const_iterator it = _commands.begin(); 
         it != _commands.end(); ++it) {
        result.push_back(it->first + ": " + it->second.helpShort);
    }
    return result;
}
