/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CommandsBonus.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tsadouk <tsadouk@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/05 09:13:09 by tsadouk           #+#    #+#             */
/*   Updated: 2025/03/05 10:08:04 by tsadouk          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CommandExecutor.hpp"

void CommandExecutor::handleDCC(Client* client, const Command& cmd) {
	std::cout << "DCC command received with " << cmd.params.size() << " parameters" << std::endl;
    if (cmd.params.size() < 2) {
        client->sendReply("461", "DCC :Not enough parameters");
        return;
    }

    if (cmd.params[0] == "SEND") {
        try {
            FileTransfer transfer(cmd.params[1]);
            
            struct in_addr addr;
            inet_aton(transfer.getIP().c_str(), &addr);
            unsigned long ipNum = ntohl(addr.s_addr);

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
