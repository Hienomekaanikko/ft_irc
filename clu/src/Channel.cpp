/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: msuokas <msuokas@student.hive.fi>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/05 16:38:05 by msuokas           #+#    #+#             */
/*   Updated: 2025/11/12 11:47:02 by msuokas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Channel.hpp"

// Creates the 'Channel' object
// Initializes all of these mode 'switches' to false:
// i: Set/remove Invite-only channel
// t: Set/remove the restrictions of the TOPIC command to channel operators
// k: Set/remove the channel key (password)
// o: Give/take channel operator privilege
// l: Set/remove the user limit to channel
Channel::Channel(const std::string& name) : _channelName(name) {
    std::string mode_list = "itkos";
    for (char mode : mode_list) {
        _modes[mode] = false;
    }
}

// Adds a 'client' to the _clients list.
void Channel::addClient(Client* client) {
    if (!_clients.insert(client).second)
        throw std::runtime_error("Client already in channel");
}

// Removes a 'client' to the _clients list.
void Channel::removeClient(Client* client) {
    if (_clients.erase(client) == 0)
        throw std::runtime_error("Client not found in channel");
}

// Returns boolean about the operator status of 'client'
bool Channel::isOperator(Client* client) const {
    return _operators.find(client) != _operators.end();
}

// Handles the MODE command actions:
// i: Set/remove Invite-only channel
// t: Set/remove the restrictions of the TOPIC command to channel operators
// k: Set/remove the channel key (password)
// o: Give/take channel operator privilege
// l: Set/remove the user limit to channel
void Channel::handleMode(const std::string& prompt, const Client& client) {
    (void)prompt;
    (void)client;
    /*
    The commands that have MODE +(i,t,k,o,s) or MODE -(i,t,k,o,s) will be handled here.
    if the client has permissions, then the changes will be made.
    */
}

// Will be called from the handleMode if the client permissions match
void Channel::setPassword(const std::string& password) {
    _password = password;
}

// Returns the channel topic
const std::string& Channel::getTopic() const {
    return _topic;
}

// This handles the setting of cmd TOPIC when its called. 
// If MODE t is true, then only moderators can change the channel topic.
// otherwise anyone can change it.
void Channel::setTopic(const std::string& topic) {
    _topic = topic;
}

// Returns the name of the channel
const std::string& Channel::getChannelName() const {
    return _channelName;
}

// !!May not be useful!!: We can look for clients inside the channel by nickname
Client* Channel::findClientByNickname(const std::string& name) const {
    for (auto* c : _clients) {
        if (c->getNickname() == name)
            return c;
    }
    return nullptr;
}

