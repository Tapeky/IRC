/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tsadouk <tsadouk@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/02 09:10:13 by tsadouk           #+#    #+#             */
/*   Updated: 2025/02/24 13:39:10 by tsadouk          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"
#include <sys/socket.h>
#include <unistd.h>
#include <sstream>

Client::Client(int fd) : _fd(fd), _authenticated(false), _registrationDone(false) {}

Client::~Client() {
	if (_fd != -1)
		close(_fd);
}

void	Client::appendToBuffer(const std::string& data) {
	_buffer += data;
}

bool	Client::hasCompleteMessage() const {
	return _buffer.find("\r\n") != std::string::npos; // Dans le protocole IRC, les messages se terminent par \r\n
}

std::string Client::getNextMessage() {
	size_t pos = _buffer.find("\r\n");
	if (pos == std::string::npos)
		return ""; // On a pas trouver d autre message

	std::string message = _buffer.substr(0, pos); // On extrait le next msg
	_buffer.erase(0, pos + 2);
	return message;
}

void Client::sendMessage(const std::string& message) {
	std::string fullMessage = message + "\r\n";
	send(_fd, fullMessage.c_str(), fullMessage.length(), 0);
}

void Client::sendReply(const std::string& code, const std::string &message) {
	std::stringstream ss;
	ss << ":" << "server" << " " << code << " " << (_nickname.empty() ? "*" : _nickname) << " " << message;
	sendMessage(ss.str());
}

void Client::checkRegistration() {
	if (!_nickname.empty() && !_username.empty() && _authenticated)
		_registrationDone = true;
}
