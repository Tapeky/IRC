/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tsadouk <tsadouk@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/02 09:10:13 by tsadouk           #+#    #+#             */
/*   Updated: 2025/02/24 17:10:27 by tsadouk          ###   ########.fr       */
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
	return _buffer.find("\r\n") != std::string::npos || _buffer.find("\n") != std::string::npos; // Dans le protocole IRC, les messages se terminent par \r\n
}

std::string Client::getNextMessage() {
    size_t pos_crlf = _buffer.find("\r\n");
    size_t pos_lf = _buffer.find("\n");
    
    // Déterminer quelle fin de ligne utiliser
    if (pos_crlf != std::string::npos && (pos_lf == std::string::npos || pos_crlf < pos_lf)) {
        std::string message = _buffer.substr(0, pos_crlf);
        _buffer.erase(0, pos_crlf + 2);
        return message;
    } else if (pos_lf != std::string::npos) {
        std::string message = _buffer.substr(0, pos_lf);
        _buffer.erase(0, pos_lf + 1);
        return message;
    }
    
    return "";
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
