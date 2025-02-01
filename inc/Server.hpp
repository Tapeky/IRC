/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tsadouk <tsadouk@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/01 11:57:05 by tsadouk           #+#    #+#             */
/*   Updated: 2025/02/01 12:07:26 by tsadouk          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

# include <iostream>
# include <vector>
# include <sys/socket.h>
# include <netinet/in.h>
# include <poll.h>
# include <cstring>

class Client;
class Channel;

class Server {
	private:
		int						_serverSocket;
		int						_port;
		std::string				_password;
		std::vector<Client>		_clients;
		std::vector<Channel>	_channels;
		std::vector<pollfd>		_pollfds;

	public:
		Server(int pord, std::string& password);
		~Server();

		void	start();
		void	run();

	private:
		void	setupServerSocket();
		void	handleNewConnection();
		void	handleClientData(int clientfd);

};


#endif