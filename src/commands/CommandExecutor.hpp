/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CommandExecutor.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tsadouk <tsadouk@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/04 00:09:27 by tsadouk           #+#    #+#             */
/*   Updated: 2025/03/05 10:07:59 by tsadouk          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef COMMANDEXECUTOR_HPP
# define COMMANDEXECUTOR_HPP

#include "Command.hpp"
#include "../../inc/Client.hpp"
#include "../../inc/Server.hpp"
#include "../../bonus/inc/FileTransfer.hpp"
#include <cstdlib>
#include <algorithm>
#include <map>
#include <set>
#include <string>
#include <vector>

enum CommandAuth {
	AUTH_NONE,           // No authentication required (ex: PASS)
	AUTH_BASIC,          // Basic authentication required
	AUTH_REGISTERED      // Full registration required (NICK + USER)
};

class CommandExecutor {
	private:
		// Function pointer type definition for command handlers
		typedef void (CommandExecutor::*CommandHandler)(Client*, const Command&);
		
		// Structure to store information about a command
		struct CommandInfo {
			CommandHandler handler;    // Function that handles the command
			CommandAuth authLevel;     // Required authorization level
			std::string helpShort;     // Short description for help
			
			CommandInfo() : handler(NULL), authLevel(AUTH_NONE), helpShort("") {}
			CommandInfo(CommandHandler h, CommandAuth a, const std::string& help) 
				: handler(h), authLevel(a), helpShort(help) {}
		};
		// Map to store command handlers
		static std::map<std::string, CommandInfo> _commands;
		
		// Auth commands (CommandsAuth.cpp)
		void handlePass(Client* client, const Command& cmd);
		void handleNick(Client* client, const Command& cmd);
		void handleUser(Client* client, const Command& cmd);
		
		// Channel commands (CommandsChannel.cpp)
		void handleJoin(Client* client, const Command& cmd);
		void handlePart(Client* client, const Command& cmd);
		void handleTopic(Client* client, const Command& cmd);
		
		// Operator commands (CommandsOper.cpp)
		void handleKick(Client* client, const Command& cmd);
		void handleInvite(Client* client, const Command& cmd);
		void handleMode(Client* client, const Command& cmd);
		
		// Messaging commands (CommandsMessaging.cpp)
		void handlePrivmsg(Client* client, const Command& cmd);
		
		// Utility commands (CommandsUtility.cpp)
		void handleQuit(Client* client, const Command& cmd);
		void handleHelp(Client* client, const Command& cmd);
		
		// Bonus commands (CommandsBonus.cpp)
		void handleDCC(Client* client, const Command& cmd);
		
		bool checkAuth(Client* client, CommandAuth requiredAuth);
		CommandExecutor() {}
		
	public:
		static void initCommands();
		static void executeCommand(Client* client, const Command& cmd);
		static std::vector<std::string> getCommandList();
		static std::string getCommandHelp(const std::string& command);
};

#endif