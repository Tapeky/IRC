/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Bot.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tsadouk <tsadouk@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/06 15:36:24 by tsadouk           #+#    #+#             */
/*   Updated: 2025/03/05 10:11:41 by tsadouk          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Bot.hpp"

Bot::Bot(const std::string& nickname, const std::string& channel)
    : _socket(-1), _nickname(nickname), _channel(channel), _connected(false) {}

Bot::~Bot() {
    if (_socket != -1) {
        close(_socket);
    }
}

void Bot::connect(const std::string& host, int port, const std::string& password) {
    struct sockaddr_in addr;

    _socket = socket(AF_INET, SOCK_STREAM, 0);
    if (_socket == -1)
        throw std::runtime_error("Failed to create socket");

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(host.c_str());

    if (::connect(_socket, (struct sockaddr*)&addr, sizeof(addr)) == -1)
        throw std::runtime_error("Failed to connect");

    authenticate(password);
    _connected = true;
    joinChannel();
}

void Bot::authenticate(const std::string& password) {
    sendMessage("PASS " + password);
    sendMessage("NICK " + _nickname);
    sendMessage("USER " + _nickname + " 0 * :" + _nickname);
}

void Bot::sendMessage(const std::string& message) {
    std::string fullMessage = message + "\r\n";
    send(_socket, fullMessage.c_str(), fullMessage.length(), 0);
}

void Bot::joinChannel() {
    sendMessage("JOIN " + _channel);
}

void Bot::processMessages() {
    char buffer[4096];
    ssize_t bytesRead = recv(_socket, buffer, sizeof(buffer) - 1, 0);
    
    if (bytesRead <= 0) {
        if (bytesRead == 0)
            throw std::runtime_error("Server disconnected");
        throw std::runtime_error("Error receiving data");
    }

    buffer[bytesRead] = '\0';
    _buffer += buffer;

    size_t pos;
    while ((pos = _buffer.find("\r\n")) != std::string::npos) {
        std::string message = _buffer.substr(0, pos);
        _buffer.erase(0, pos + 2);
        handleCommand(message);
    }
}

void Bot::handleCommand(const std::string& message) {
    std::cout << "Message reçu: " << message << std::endl;
    
    if (message.substr(0, 4) == "PING") {
        sendMessage("PONG" + message.substr(4));
        return;
    }

    size_t spacePos = message.find(' ');
    if (spacePos == std::string::npos)
        return;

	if (message.find("PRIVMSG " + _channel) != std::string::npos) {
    	if (message.find(":!hello") != std::string::npos) {
    	    sendMessage("PRIVMSG " + _channel + " :Hello! I am a bot!");
    	}
    	else if (message.find(":!time") != std::string::npos) {
    	    time_t now = time(0);
    	    sendMessage("PRIVMSG " + _channel + " :Current time is: " + ctime(&now));
    	}
    	else if (message.find(":!help") != std::string::npos) {
    	    sendMessage("PRIVMSG " + _channel + " :Available commands: !hello, !time, !help");
    }
	}
}
