/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tsadouk <tsadouk@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/06 15:37:31 by tsadouk           #+#    #+#             */
/*   Updated: 2025/03/06 15:34:58 by tsadouk          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Bot.hpp"
#include <iostream>
#include <cstdlib>
#include <signal.h>

Bot *g_bot = NULL;
bool g_running = true;

void sigintHandler(int sig) {
    if (sig == SIGINT && g_bot) {
        std::cout << "\nShutting down bot..." << std::endl;
        g_bot->disconnect();
        g_running = false;
    }
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        std::cerr << "Usage: " << argv[0] << " <host> <port> <password> <channel>" << std::endl;
        return 1;
    }

    try {
        signal(SIGINT, sigintHandler);
        std::string host = argv[1];
        int port = std::atoi(argv[2]);
        if (port <= 0 || port > 65535) {
            std::cerr << "Error: Invalid port number" << std::endl;
            return 1;
        }
        std::string password = argv[3];
        std::string channel = argv[4];

        Bot bot("IRCBot", channel);
        g_bot = &bot;
        bot.connect(host, port, password);
        
        while (g_running) {
            try {
                bot.processMessages();
            }
            catch (const std::exception& e) {
                std::cerr << "Error: processing messages" << e.what() << std::endl;
                g_running = false;
            }
        }
        return 0;
    }
    catch (const std::exception& e) {
        if (g_bot) {
            g_bot->disconnect();
        }
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
