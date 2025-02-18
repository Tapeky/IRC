/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tsadouk <tsadouk@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/05 14:26:02 by tsadouk           #+#    #+#             */
/*   Updated: 2025/02/18 13:40:30 by tsadouk          ###   ########.fr       */
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

bool Channel::isOperator(Client* client) const {
    for (size_t i = 0; i < _operators.size(); ++i) {
        if (_operators[i] == client)
            return true;
    }
    return false;
}

void Channel::addOperator(Client* client) {
    if (!isOperator(client))
        _operators.push_back(client);
}

void Channel::removeOperator(Client* client) {
    for (size_t i = 0; i < _operators.size(); ++i) {
        if (_operators[i] == client) {
            _operators.erase(_operators.begin() + i);
            return;
        }
    }
}
