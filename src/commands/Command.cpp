/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Command.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tsadouk <tsadouk@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/04 00:09:10 by tsadouk           #+#    #+#             */
/*   Updated: 2025/02/04 00:14:57 by tsadouk          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Command.hpp"

Command CommandParser::parseCommand(const std::string& message) {
	Command cmd;
	std::string msg = message;
   
	// Parse prefix
	if (!msg.empty() && msg[0] == ':') {
		size_t spacePos = msg.find(' ');
		if (spacePos != std::string::npos) {
			cmd.prefix = msg.substr(1, spacePos - 1);
			msg = msg.substr(spacePos + 1);
		}
	}
   
	// Parse command
	size_t spacePos = msg.find(' ');
	if (spacePos != std::string::npos) {
		cmd.command = msg.substr(0, spacePos);
		msg = msg.substr(spacePos + 1);
	} else {
		cmd.command = msg;
		return cmd;
	}
   
	// Parse params
	while (!msg.empty()) {
		if (msg[0] == ':') {
			cmd.params.push_back(msg.substr(1));
			break;
		}
	   
		spacePos = msg.find(' ');
		if (spacePos != std::string::npos) {
			cmd.params.push_back(msg.substr(0, spacePos));
			msg = msg.substr(spacePos + 1);
		} else {
			cmd.params.push_back(msg);
			break;
		}
	}
   
	return cmd;
}
