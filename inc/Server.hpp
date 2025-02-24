/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tsadouk <tsadouk@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/01 11:57:05 by tsadouk           #+#    #+#             */
/*   Updated: 2025/02/24 13:48:18 by tsadouk          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

# include <iostream>
# include <vector>
# include <sys/socket.h>
# include <netinet/in.h>
# include <poll.h>
# include <map>
# include <cerrno>
# include "Channel.hpp"
# include "Client.hpp"
# include "../src/commands/Command.hpp"
# include "../src/commands/CommandExecutor.hpp"

class Client;
class Channel;

class Server {
	private:
		static bool					_running;
		static Server*				_instance;
		int							_serverSocket;
		int							_port;
		std::string					_password;
		std::map<int, Client*>		_clients;
		std::vector<pollfd>			_pollfds;
		std::map<std::string, Channel*>	_channels;

		// Constructeur privé pour Singleton
		Server(int port, std::string& password);
		Server(const Server&);				
		void operator=(const Server&);		

	public:
		~Server();

		// Méthodes Singleton
		static Server& getInstance() {
			if (!_instance)
				throw std::runtime_error("Server not initialized");
			return *_instance;
		}

		static void initInstance(int port, std::string& password) {
			if (!_instance)
				_instance = new Server(port, password);
		}

		static void destroyInstance() {
			delete _instance;
			_instance = NULL;
		}

		// Méthodes existantes
		void		start();
		void		run();
		bool		isClientAuthenticated(int fd) const;
		void		broadcastMessage(const std::string& message);
		bool		isNicknameAvailable(const std::string& nickname) const;
		Client*		getClientByNickname(const std::string& nickname) const;
		void		disconnectClient(int clientfd);
		static void	handleSignal(int signum);

		// Getters
		const std::string& getPassword() const { return _password; }

		// Channel operations
		Channel *getOrCreateChannel(const std::string& name);
		Channel *getChannel(const std::string& name);

	private:
		void	setupServerSocket();
		void	handleNewConnection();
		void	handleClientData(int clientfd);
		void	handleClientMessage(Client* client, const std::string& message);
		void	logError(const std::string& message);
		void	logInfo(const std::string& message);
};

#endif
