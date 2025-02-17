/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brguicho <brguicho@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/05 14:26:02 by tsadouk           #+#    #+#             */
/*   Updated: 2025/02/10 17:06:02 by brguicho         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Channel.hpp"

Channel::Channel(const std::string& name) : _name(name), _inviteOnly(false), _topicRestricted(false), _userLimit(0){}
Channel::~Channel() {}

void Channel::addClient(Client* client) {
	for (size_t i = 0; i < _clients.size(); ++i)
		if (_clients[i] == client)
			return;
	_clients.push_back(client);
}

void Channel::removeClient(Client* client) {
	for (size_t i = 0; i < _clients.size(); ++i) {
		if (_clients[i] == client) {
			_clients.erase(_clients.begin() + i);
			return;
		}
	}
}

bool Channel::isClientinChannel(Client *client)
{
	const std::vector<Client*>& clients = this->getClients();
	bool isClientInChannel = false;
	for (size_t i = 0; i < clients.size(); ++i) {
		if (clients[i] == client) {
			isClientInChannel = true;
			break;
		}
	}
	return (isClientInChannel);
}

void Channel::setTopic(const std::string& topic)
{
	this->_topic = topic;
}