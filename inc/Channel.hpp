/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brguicho <brguicho@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/03 15:45:52 by tsadouk           #+#    #+#             */
/*   Updated: 2025/02/10 17:06:14 by brguicho         ###   ########.fr       */
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
   
      // Channel operations
      void addClient(Client* client);
      void removeClient(Client* client);
      bool isOperator(Client* client) const;
      void addOperator(Client* client);
      void removeOperator(Client* client);
      void setTopic(const std::string& topic);
      void setKey(const std::string& key);
      void setInviteOnly(bool status);
      void setTopicRestricted(bool status);
      void setUserLimit(size_t limit);
      bool isClientinChannel(Client *client);
};

#endif