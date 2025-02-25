/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CommandExecutor.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tsadouk <tsadouk@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/04 00:09:39 by tsadouk           #+#    #+#             */
/*   Updated: 2025/02/25 08:12:12 by tsadouk          ###   ########.fr       */
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

class CommandExecutor {
	public:
		static void	executeCommand(Client* client, const Command& cmd);
	private:
		static void	handleNick(Client* client, const Command& cmd);
		static void	handleUser(Client* client, const Command& cmd);
		static void	handleQuit(Client* client, const Command& cmd);
		static void	handlePass(Client* client, const Command& cmd);
		static void handleJoin(Client* client, const Command& cmd);
		static void handlePrivmsg(Client* client, const Command& cmd);
		static void handlePart(Client* client, const Command& cmd);
		static void handleTopic(Client* client, const Command& cmd);
		static void handleKick(Client* client, const Command& cmd);
		static void handleInvite(Client* client, const Command& cmd);
		static void handleMode(Client* client, const Command& cmd);
		static void handleHelp(Client* client, const Command& cmd);

		// BONUS
		static void handleDCC(Client* client, const Command& cmd);
};

#endif