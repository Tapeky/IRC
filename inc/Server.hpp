/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tsadouk <tsadouk@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/01 11:57:05 by tsadouk           #+#    #+#             */
/*   Updated: 2025/02/04 00:46:45 by tsadouk          ###   ########.fr       */
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
		int							_serverSocket;
		int							_port;
		std::string					_password;
		std::map<int, Client*>		_clients; // paires de socket et client
		//std::vector<Channel>		_channels;
		std::vector<pollfd>			_pollfds;

	public:
		Server(int pord, std::string& password);
		~Server();

		void	start();
		void	run();

	private:
		void	setupServerSocket();
		void	handleNewConnection();
		void	handleClientData(int clientfd);
		void    disconnectClient(int clientfd);
		void    handleClientMessage(Client* client, const std::string& message);

		// Utils
		bool	isClientAuthenticated(int fd) const;
		void	broadcastMessage(const std::string& message);
		bool    isNicknameAvailable(const std::string& nickname) const;
		Client* getClientByNickname(const std::string& nickname) const;
		void	logError(const std::string& message);
		void	logInfo(const std::string& message);

};

#endif