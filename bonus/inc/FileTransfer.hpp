/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   FileTransfer.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tsadouk <tsadouk@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/06 17:44:37 by tsadouk           #+#    #+#             */
/*   Updated: 2025/03/06 16:21:17 by tsadouk          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef FILETRANSFER_HPP
# define FILETRANSFER_HPP

# include <string>
# include <sys/socket.h>
# include <netinet/in.h>
# include <fstream>
# include <iostream>
# include <arpa/inet.h>
# include <unistd.h>
# include <cstring>
# include <sys/stat.h>

class FileTransfer {
	private:
		int				_socket;
		std::string		_filename;
		std::string		_ip;
		int				_port;
		size_t			_filesize;
		std::ifstream	_file;

	public:
		FileTransfer(const std::string& filename);
		~FileTransfer();

		void	initiateSend();
		void	acceptTransfer();

		const std::string& getFilename() const { return _filename; }
    	const std::string& getIP() const { return _ip; }
    	int getPort() const { return _port; }
    	size_t getFilesize() const { return _filesize; }

	private:
    	void setupSocket();
    	void sendFile();
    	void receiveFile();
	
};

#endif