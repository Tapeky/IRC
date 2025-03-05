/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tsadouk <tsadouk@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/01 12:09:09 by tsadouk           #+#    #+#             */
/*   Updated: 2025/03/05 10:13:14 by tsadouk          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "Server.hpp"
# include "Client.hpp"
# include <fcntl.h>
# include <unistd.h>
# include <string.h>
# include <sstream>
# include <cstring>
# include <signal.h>

Server* Server::_instance = NULL;
bool	Server::_running = true;

void Server::handleSignal(int signum) {
	if (signum == SIGINT) {
		std::cout << "\nShutting down server... " << signum << std::endl;
		_running = false;
	}
}

Server::Server(int port, std::string& password)
	: _serverSocket(-1), _port(port), _password(password) {
		if (port < 0 || port > 65535)
			throw std::runtime_error("Invalid port number");
}

Server::~Server() {
	for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it)
		delete it->second;
	_clients.clear();

	for (std::map<std::string, Channel*>::iterator it = _channels.begin(); it != _channels.end(); ++it)
		delete it->second;
	_channels.clear();

	if (_serverSocket != -1)
		close(_serverSocket);

	for (size_t i =  0; i < _pollfds.size(); ++i) {
		if (_pollfds[i].fd != -1) {
			close(_pollfds[i].fd);
			_pollfds[i].fd = -1;
		}
	}
	_pollfds.clear();
}

void Server::setupServerSocket() {
	// Create the socket
	_serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (_serverSocket == -1)
		throw std::runtime_error("Failed to create server socket");
	
	// Set the socket to non-blocking mode
	int flags = fcntl(_serverSocket, F_GETFL, 0);
	if (flags == -1)
		throw std::runtime_error("Failed to get server socket flags");
	if (fcntl(_serverSocket, F_SETFL, flags | O_NONBLOCK) == -1)
		throw std::runtime_error("Failed to set server socket to non-blocking mode");

	// Allow address reuse
	int opt = 1;
	if (setsockopt(_serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
		throw std::runtime_error("Failed to set server socket options");

	// Configure the server address
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(_port);
	addr.sin_addr.s_addr = INADDR_ANY;

	// Bind the socket to the address
	if (bind(_serverSocket, (struct sockaddr*)&addr, sizeof(addr)) == -1)
		throw std::runtime_error("Failed to bind server socket");

	// Set the socket to listen mode
	if (listen(_serverSocket, 10) == -1)
		throw std::runtime_error("Failed to listen on server socket");

	// Add the socket to the list of sockets to monitor
	struct pollfd pfd;
	pfd.fd = _serverSocket;
	pfd.events = POLLIN;
	pfd.revents = 0;
	_pollfds.push_back(pfd);	
}

void Server::handleNewConnection() {
	struct sockaddr_in	clientAdrdr;
	socklen_t			clientAddrLen = sizeof(clientAdrdr);

	// Accept the new connection
	int clientSocket = accept(_serverSocket, (struct sockaddr*)&clientAdrdr, &clientAddrLen);
	if (clientSocket == -1) {
		if (errno != EWOULDBLOCK)
			std::cerr << "Failed to accept new connection: " << std::endl;
		return;
	}

	// Set the socket to non-blocking mode
	int	flags = fcntl(clientSocket, F_GETFL, 0);
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

	// Create a new client
	Client	*client = 0;
	try {
		client = new Client(clientSocket);
	} catch (const std::bad_alloc &e) {
		std::cerr << "Failed to create new client: " << e.what() << std::endl;
		close(clientSocket);
		return;
	}
	_clients[clientSocket] = client;

	// Add the socket to the list of sockets to monitor
	struct pollfd pfd;
	pfd.fd = clientSocket;
	pfd.events = POLLIN;
	pfd.revents = 0;
	_pollfds.push_back(pfd);

	std::cout << "New client connected on socket " << clientSocket << std::endl;
}

void Server::start() {
	setupServerSocket();
	std::cout << "Server is listening on port " << _port << std::endl;
	run();
}

void Server::run() {
	while (_running) {
		for (size_t i = 0; i < _pollfds.size(); ++i)
			_pollfds[i].revents = 0;

		// Wait for events on the sockets
		int ready = poll(_pollfds.data(), _pollfds.size(), -1);
		if (ready == -1) {
			if (errno == EINTR)
				continue;
			throw std::runtime_error("Failed to poll sockets");
		}

		// Handle the events
		for (size_t i = 0; i < _pollfds.size(); ++i) {
			if (_pollfds[i].revents & POLLIN) {
				if (_pollfds[i].fd == _serverSocket)
					handleNewConnection();
				else
					handleClientData(_pollfds[i].fd);
			}
		}
	}
	for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
		delete it->second;
	}
	_clients.clear();

	for (size_t i = 0; i < _pollfds.size(); ++i) {
		if (_pollfds[i].fd != -1) {
			close(_pollfds[i].fd);
		}
	}
	_pollfds.clear();
}

void Server::handleClientData(int clientfd) {
	Client *client = _clients[clientfd];
	if (!client) {
		std::cerr << "Error: client not found for fd " << clientfd << std::endl;
		disconnectClient(clientfd);
		return;
	}

	char buffer[1024];
	ssize_t nbytes = recv(clientfd, buffer, sizeof(buffer) - 1, 0);
		
	if (nbytes <= 0) {
		if (nbytes < 0)
			std::cerr << "Error reading from client " << clientfd << ": " << strerror(errno) << std::endl;
		disconnectClient(clientfd);
		return;
	}

	// Add the data to the client's buffer
	try {
		client->appendToBuffer(std::string(buffer, nbytes));

		// Process all complete messages
		while (client->hasCompleteMessage()) {
			std::string message = client->getNextMessage();
			
			// Syntax error fixed here
			if (message.find("QUIT") == 0) {
				Command cmd = CommandParser::parseCommand(message);
				handleClientMessage(client, message);
				return;
			} else {
				handleClientMessage(client, message);
				
				// Syntax errors fixed here
				if (_clients.find(clientfd) == _clients.end() || _clients[clientfd] != client) {
					return;
				}
			}
		}
	} catch (const std::exception& e) {
		std::cerr << "Error processing client data: " << e.what() << std::endl;
		disconnectClient(clientfd);
	}
}

void Server::disconnectClient(int clientfd) {
	std::stringstream ss;
	ss << clientfd;
	logInfo("Disconnecting client: " + ss.str());

	// Inform other clients if necessary
	std::map<int, Client*>::iterator it = _clients.find(clientfd);
	if (it != _clients.end() && it->second) {
		if (!it->second->getNickname().empty()) {
			broadcastMessage(":" + it->second->getNickname() + " QUIT :Client disconnected");
		}
		delete it->second;
		_clients.erase(it);
	}

	// Close the socket and remove it from poll
	for (size_t i = 0; i < _pollfds.size(); ++i) {
		if (_pollfds[i].fd == clientfd) {
			close(clientfd);
			_pollfds.erase(_pollfds.begin() + i);
			break;
		}
	}
}

void Server::handleClientMessage(Client* client, const std::string& message) {
	Command cmd = CommandParser::parseCommand(message);
	CommandExecutor::executeCommand(client, cmd);
}


/*-------------------UTILS------------------------*/

bool Server::isClientAuthenticated(int fd) const {
	std::map<int, Client*>::const_iterator it = _clients.find(fd);
	return (it != _clients.end() && it->second && it->second->isAuthenticated());
}

void Server::broadcastMessage(const std::string& message) {
   for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
	   if (it->second && it->second->isAuthenticated()) {
		   try {
			   it->second->sendMessage(message);
		   } catch (const std::exception& e) {
			   std::stringstream ss;
			   ss << it->first;
			   logError("Failed to broadcast message to client " + ss.str() + ": " + e.what());
		   }            
	   }
   }
}

bool Server::isNicknameAvailable(const std::string& nickname) const {
	for (std::map<int, Client*>::const_iterator it = _clients.begin(); it != _clients.end(); ++it) {
		if (it->second && it->second->getNickname() == nickname)
			return false;
	}
	return true;
}

Client* Server::getClientByNickname(const std::string& nickname) const {
	for (std::map<int, Client*>::const_iterator it = _clients.begin(); it != _clients.end(); ++it) {
		if (it->second && it->second->getNickname() == nickname)
			return it->second;
	}
	return 0;
}

void Server::logError(const std::string& message) {
	std::cerr << "ERROR: " << message << std::endl;
}

void Server::logInfo(const std::string& message) {
	std::cout << "INFO: " << message << std::endl;
}


/*-------------------CHANNELS------------------------*/

Channel* Server::getOrCreateChannel(const std::string& name) {
	if (_channels.find(name) == _channels.end())
		_channels[name] = new Channel(name);
	return _channels[name];
}

Channel* Server::getChannel(const std::string& name) {
	if (_channels.find(name) == _channels.end())
		return 0;
	return _channels[name];
}
