/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tsadouk <tsadouk@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/03 15:45:52 by tsadouk           #+#    #+#             */
/*   Updated: 2025/02/26 16:53:06 by tsadouk          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

// Channel.hpp
#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <vector>
#include "Client.hpp"

class Channel {
	private:
		std::string             _name;
		std::string             _topic;
		std::vector<Client*>    _clients;
		std::vector<Client*>    _operators;
		std::string             _key;
		bool                    _inviteOnly;
		bool                    _topicRestricted;
		size_t                  _userLimit;
		std::vector<Client*>    _invitedUsers;
   
	public:
		Channel(const std::string& name);
		~Channel();
   
		// Getters
		const std::string&          getName() const { return _name; }
		const std::string&          getTopic() const { return _topic; }
		const std::vector<Client*>& getClients() const { return _clients; }
		const std::string&          getKey() const { return _key; }
		bool                        isInviteOnly() const { return _inviteOnly; }
		bool                        isTopicRestricted() const { return _topicRestricted; }
		size_t                      getUserLimit() const { return _userLimit; }
    
		// Setters
		void    setTopic(const std::string& topic) { this->_topic = topic ;}
		void    setKey(const std::string& key) { _key = key; }
		void    setInviteOnly(bool status) { _inviteOnly = status; }
		void    setTopicRestricted(bool status) { _topicRestricted = status; }
		void    setUserLimit(size_t limit) { _userLimit = limit; };

		// Channel operations
		void    addClient(Client* client);
		void    removeClient(Client* client);
		bool    isOperator(Client* client) const;
		void    addOperator(Client* client);
		void    removeOperator(Client* client);
		bool    isClientinChannel(Client *client);
		void	inviteClient(Client *client);
		bool	isInvited(Client *client) const;
		void	removeInvite(Client *client);
	};

#endif
