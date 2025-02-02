/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tsadouk <tsadouk@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/01 18:15:03 by tsadouk           #+#    #+#             */
/*   Updated: 2025/02/02 09:11:39 by tsadouk          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
#define CLIENT_HPP

# include <string>
# include <queue>

class Client {
	private:
		int			_fd;				// Socket du client
		std::string	_nickname;			// Pseudo IRC
		std::string	_username;			// Nom d'utilisateur
		std::string	_buffer;			// Buffer pour les messages partiels
		bool		_authenticated;		// Authentification
		bool		_registrationDone;	// Enregistrement complet (NICK + USER)

	public:
		Client(int fd);
		~Client();

		// Getters
		int			getFd() const { return _fd; }
		std::string	getNickname() const { return _nickname; }
		std::string	getUsername() const { return _username; }
		bool		isAuthenticated() const { return _authenticated; }
		bool		isRegistrationDone() const { return _registrationDone; }

		// Setters
		void	setNickname(const std::string& nickname) { _nickname = nickname; }
		void	setUsername(const std::string& username) { _username = username; }
		void	setAuthenticated(bool status) { _authenticated = status; }

		// Gestion des messages
		void		appendToBuffer(const std::string& data);
		std::string	getNextMessage();
		bool		hasCompleteMessage() const;

		// Envoi des messages
		void	sendMessage(const std::string& message);
		void	sendReply(const std::string& code, const std::string& message);

	private:
		void	checkRegistration();

};

#endif