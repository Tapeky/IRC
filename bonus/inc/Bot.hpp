/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Bot.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tsadouk <tsadouk@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/06 15:30:02 by tsadouk           #+#    #+#             */
/*   Updated: 2025/03/06 15:39:32 by tsadouk          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef BOT_HPP
# define BOT_HPP

# include <string>
# include <sys/socket.h>
# include <netinet/in.h>
# include <iostream>
# include <arpa/inet.h>
# include <unistd.h>
# include <cstring>

class Bot {
	private:
		int			_socket;
		std::string	_nickname;
		std::string	_username;
		std::string	_channel;
		std::string	_buffer;
		bool		_connected;
	
	public:
		Bot(const std::string& nickname, const std::string& channel);
		~Bot();

		// Connection
		void	connect(const std::string& server, int port, const std::string& password);
		void	disconnect();

		// Message Handling
		void	sendMessage(const std::string& message);
		void	processMessages();
		void	joinChannel();

	private:
		void	authenticate(const std::string& password);
		void	handleCommand(const std::string& message);
		void	closeSocket();

};

#endif