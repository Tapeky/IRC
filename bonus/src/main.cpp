/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tsadouk <tsadouk@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/06 15:37:31 by tsadouk           #+#    #+#             */
/*   Updated: 2025/02/06 18:44:38 by tsadouk          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Bot.hpp"
#include <iostream>
#include <cstdlib>

int main(int argc, char *argv[]) {
    if (argc != 5) {
        std::cerr << "Usage: " << argv[0] << " <host> <port> <password> <channel>" << std::endl;
        return 1;
    }

    try {
        std::string host = argv[1];
        int port = std::atoi(argv[2]);
        std::string password = argv[3];
        std::string channel = argv[4];

        Bot bot("IRCBot", channel);
        bot.connect(host, port, password);
        
        while (1) {
            bot.processMessages();
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
