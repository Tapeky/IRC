/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Bot.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tsadouk <tsadouk@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/06 15:36:24 by tsadouk           #+#    #+#             */
/*   Updated: 2025/03/06 15:39:05 by tsadouk          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Bot.hpp"

Bot::Bot(const std::string& nickname, const std::string& channel)
    : _socket(-1), _nickname(nickname), _channel(channel), _connected(false) {}

Bot::~Bot() {
    closeSocket();
}

void Bot::closeSocket() {
    if (_socket != -1) {
        close(_socket);
        _socket = -1;
    }
}

void Bot::connect(const std::string& host, int port, const std::string& password) {
    struct sockaddr_in addr;

    closeSocket();
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
    if (_channel[0] == '#')
        sendMessage("JOIN " + _channel);
    else
        sendMessage("JOIN #" + _channel);
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

    std::string channelToCheck = (_channel[0] == '#') ? _channel : "#" + _channel;

    size_t privmsgPos = message.find("PRIVMSG " + channelToCheck + " :");
    if (privmsgPos != std::string::npos) {
        size_t nickEndPos = message.find('!');
        if (nickEndPos != std::string::npos && nickEndPos > 1) {
            std::string sender = message.substr(1, nickEndPos - 1);
            if (sender == _nickname) {
                return;
            }
        }
        
        size_t contentPos = message.find(" :", privmsgPos);
        if (contentPos != std::string::npos) {
            std::string content = message.substr(contentPos + 2);
            
            if (content == "!hello") {
                sendMessage("PRIVMSG " + channelToCheck + " :Hello! I am a bot!");
            }
            else if (content == "!time") {
                time_t now = time(0);
                std::string timeStr = ctime(&now);
                timeStr.erase(timeStr.find('\n'));
                sendMessage("PRIVMSG " + channelToCheck + " :Current time is: " + timeStr);
            }
            else if (content == "!help") {
                sendMessage("PRIVMSG " + channelToCheck + " :Available commands: !hello, !time, !help");
            }
        }
    }
}

void Bot::disconnect() {
    if (_connected) {
        sendMessage("QUIT :Bot déconnecté");
        usleep(100000);
        closeSocket();     
        _connected = false;
        std::cout << "Bot déconnecté du serveur" << std::endl;
    }
}
