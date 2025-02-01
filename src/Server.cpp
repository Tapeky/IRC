/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tsadouk <tsadouk@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/01 12:09:09 by tsadouk           #+#    #+#             */
/*   Updated: 2025/02/01 12:36:46 by tsadouk          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "Server.hpp"
# include <fcntl.h>
# include <unistd.h>
# include <string.h>

Server::Server(int port, std::string& password)
	: _serverSocket(-1), _port(port), _password(password) {
		if (port < 0 || port > 65535)
			throw std::runtime_error("Invalid port number");
}

Server::~Server() {
	if (_serverSocket != -1)
		close(_serverSocket);

	// Close all client sockets
	for (size_t i =  0; i < _pollfds.size(); ++i)
			close(_pollfds[i].fd);
}

void Server::setupServerSocket() {
	// Cree les sockets
	_serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (_serverSocket == -1)
		throw std::runtime_error("Failed to create server socket");
	
	// Configurer le socket en mode non-bloquant
	int flags = fcntl(_serverSocket, F_GETFL, 0);
	if (flags == -1)
		throw std::runtime_error("Failed to get server socket flags");
	if (fcntl(_serverSocket, F_SETFL, flags | O_NONBLOCK) == -1)
		throw std::runtime_error("Failed to set server socket to non-blocking mode");

	// Permettre la reutilisation de l'adresse
	int opt = 1;
	if (setsockopt(_serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
		throw std::runtime_error("Failed to set server socket options");

	// Configurer l'adresse du serveur
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(_port);
	addr.sin_addr.s_addr = INADDR_ANY;

	// Lier le socket a l'adresse
	if (bind(_serverSocket, (struct sockaddr*)&addr, sizeof(addr)) == -1)
		throw std::runtime_error("Failed to bind server socket");

	// Mettre le socket en mode ecoute
	if (listen(_serverSocket, 10) == -1)
		throw std::runtime_error("Failed to listen on server socket");

	// Ajouter le socket a la liste des sockets a surveiller
	struct pollfd pfd;
	pfd.fd = _serverSocket;
	pfd.events = POLLIN;
	_pollfds.push_back(pfd);	
}

void Server::handleNewConnection() {
	struct sockaddr_in	clientAdrdr;
	socklen_t			clientAddrLen = sizeof(clientAdrdr);

	// Accepter la nouvelle connexion
	int clientSocket = accept(_serverSocket, (struct sockaddr*)&clientAdrdr, &clientAddrLen);
	if (clientSocket == -1) {
		if (errno != EWOULDBLOCK)
			std::cerr << "Failed to accept new connection: " << std::endl;
		return;
	}

	// Configurer le socket en mode non-bloquant
	int flags = fcntl(clientSocket, F_GETFL, 0);
	if (flags == -1) {
		std::cerr << "Failed to get client socket flags" << std::endl;
		close(clientSocket);
		return;
	}
	if (fcntl(clientSocket, F_SETFL, flags | O_NONBLOCK) == -1) {
		std::cerr << "Failed to set client socket to non-blocking mode" << std::endl;
		close(clientSocket);
		return;
	}

	// Ajouter le socket a la liste des sockets a surveiller
	struct pollfd pfd;
	pfd.fd = clientSocket;
	pfd.events = POLLIN;
	_pollfds.push_back(pfd);

	// Envoyer un message de bienvenue
	std::cout << "New client connected" << std::endl;
}

void Server::start() {
	setupServerSocket();
	std::cout << "Server is listening on port " << _port << std::endl;
	run();
}

void Server::run() {
	while (true) {
		// Attendre des evenements sur les sockets
		int ready = poll(_pollfds.data(), _pollfds.size(), -1);
		if (ready == -1) {
			if (errno == EINTR)
				continue;
			throw std::runtime_error("Failed to poll sockets");
		}

		// Traiter les evenements
		for (size_t i = 0; i < _pollfds.size(); ++i) {
			if (_pollfds[i].revents & POLLIN) {
				if (_pollfds[i].fd == _serverSocket)
					handleNewConnection();
				else
					handleClientData(_pollfds[i].fd);
			}
		}
	}
}

void Server::handleClientData(int clientfd) {
	char buffer[1024];
	ssize_t nbytes = recv(clientfd, buffer, sizeof(buffer), 0);
	
	if (nbytes <= 0) {
		if (nbytes < 0)
			std::cerr << "Error reading from client" << std::endl;
		close(clientfd);
		for (size_t i = 0; i < _pollfds.size(); ++i) {
			if (_pollfds[i].fd == clientfd) {
				_pollfds.erase(_pollfds.begin() + i);
				break;
			}
		}
		std::cout << "Client disconnected" << std::endl;
		return;
	}

	// Traiter les donnees recues
	buffer[nbytes] = '\0';
	std::cout << "Received: " << buffer << std::endl;	
}
