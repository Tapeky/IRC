/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   FileTransfer.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tsadouk <tsadouk@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/06 17:49:11 by tsadouk           #+#    #+#             */
/*   Updated: 2025/03/06 16:21:15 by tsadouk          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/FileTransfer.hpp"

FileTransfer::FileTransfer(const std::string& filename) 
    : _socket(-1), _filename(filename), _port(0), _filesize(0) {
    struct stat st;

    if (filename.empty())
        throw std::runtime_error("Empty filename");
    if (filename[0] != '/')
        throw std::runtime_error("Path must be absolute");
    if (stat(filename.c_str(), &st) != 0)
        throw std::runtime_error("File not found");
    if (access(filename.c_str(), R_OK) != 0)
        throw std::runtime_error("File not readable");
    if (!S_ISREG(st.st_mode))
        throw std::runtime_error("Not a regular file");

    _filesize = st.st_size;
}

FileTransfer::~FileTransfer() {
	if (_socket != -1)
		close(_socket);
	if (_file.is_open())
		_file.close();
}

void FileTransfer::setupSocket() {
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);

    _socket = socket(AF_INET, SOCK_STREAM, 0);
    if (_socket == -1)
        throw std::runtime_error("Failed to create socket");

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = 0;

    if (bind(_socket, (struct sockaddr*)&addr, sizeof(addr)) == -1)
        throw std::runtime_error("Failed to bind socket");

    if (getsockname(_socket, (struct sockaddr*)&addr, &len) == -1)
        throw std::runtime_error("Failed to get socket name");

    _port = ntohs(addr.sin_port);
    _ip = "127.0.0.1";

    if (listen(_socket, 1) == -1)
        throw std::runtime_error("Failed to listen");
}

void FileTransfer::initiateSend() {
	setupSocket();
	
	_file.open(_filename.c_str(), std::ios::binary);
	if (!_file.is_open())
		throw std::runtime_error("Failed to open file");

	// Wait for connection
	struct sockaddr_in clientAddr;
	socklen_t clientLen = sizeof(clientAddr);
	int clientSocket = accept(_socket, (struct sockaddr*)&clientAddr, &clientLen);
	if (clientSocket == -1)
		throw std::runtime_error("Failed to accept connection");

	// send file
	char buffer[4096];
	while (!_file.eof()) {
		_file.read(buffer, sizeof(buffer));
		ssize_t bytesRead = _file.gcount();
		if (send(clientSocket, buffer, bytesRead, 0) == -1)
			throw std::runtime_error("Failed to send file data");
	}

	close(clientSocket);
}

void FileTransfer::acceptTransfer() {
	struct sockaddr_in addr;
	
	
	_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (_socket == -1)
		throw std::runtime_error("Failed to create socket");

	addr.sin_family = AF_INET;
	addr.sin_port = htons(_port);
	if (inet_pton(AF_INET, _ip.c_str(), &addr.sin_addr) <= 0)
		throw std::runtime_error("Invalid address");

	if (connect(_socket, (struct sockaddr*)&addr, sizeof(addr)) == -1)
		throw std::runtime_error("Failed to connect");

	std::ofstream outFile(_filename.c_str(), std::ios::binary);
	if (!outFile.is_open())
		throw std::runtime_error("Failed to create output file");

	char buffer[4096];
	ssize_t bytesRead;
	while ((bytesRead = recv(_socket, buffer, sizeof(buffer), 0)) > 0) {
		outFile.write(buffer, bytesRead);
	}
}
