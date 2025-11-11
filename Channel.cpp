/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: msuokas <msuokas@student.hive.fi>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/05 16:38:05 by msuokas           #+#    #+#             */
/*   Updated: 2025/11/10 15:52:48 by msuokas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Channel.hpp"

Channel::Channel(std::string& name): _channelName(name) {}

void Channel::addClient(Client& user) {
    for (auto& client : _clientList) {
        if (client.getClientFd() == user.getClientFd()) {
            throw std::runtime_error("Client already in the channel");
        }
        else if (client.getUsername() == user.getUsername()) {
            throw std::runtime_error("User of same username already in the channel");
        }
    }
    _clientList.push_back(user);
}

Client Channel::getClient(std::string& clientName) const {
    for (auto& client : _clientList) {
        if (client.getUsername() == clientName) {
            return client;
        }
    }
    throw std::runtime_error("Client not found in channel");
}

std::string& Channel::getChannelName() {
    return _channelName;
}