/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tsadouk <tsadouk@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/01 12:02:57 by tsadouk           #+#    #+#             */
/*   Updated: 2025/02/26 16:36:10 by tsadouk          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include <iostream>
#include <cstdlib>
#include <signal.h>

static int  checkArg1(const std::string& arg) {
    for (size_t i = 0; i < arg.length(); ++i) {
        if (!std::isdigit(arg[i]))
            return 1;
    }
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <port> <password>" << std::endl;
        return 1;
    }
    if (checkArg1(argv[1])) {
        std::cerr << "Error: Port must be a number" << std::endl;
        return 1;
    }

    try {
        signal(SIGINT, Server::handleSignal);
        int port = std::atoi(argv[1]);
        std::string password = argv[2];
        Server::initInstance(port, password);
        Server::getInstance().start();
        Server::destroyInstance();
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
