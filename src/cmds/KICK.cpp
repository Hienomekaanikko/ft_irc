/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   KICK.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: msuokas <msuokas@student.hive.fi>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/27 14:48:52 by msuokas           #+#    #+#             */
/*   Updated: 2025/11/27 14:48:53 by msuokas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include <sstream>

void Server::handleKICK(Client &client, const std::vector<std::string_view> &params)
{
    if (params.size() < 2) {
        sendNumeric(client, 461, "KICK :Not enough parameters");
        return;
    }

    std::string channelName(params[0]);
    std::string targetNick(params[1]);
    std::string comment = "";
    if (params.size() > 2) {
        std::ostringstream commentStream;
        commentStream << params[2];
        for (size_t i = 3; i < params.size(); ++i) {
            commentStream << " " << params[i];
        }
        comment = commentStream.str();
    }

    auto it = _channels.find(channelName);
    if (it == _channels.end()) {
        sendNumeric(client, 403, channelName + " :No such channel");
        return;
    }

    Channel &chan = it->second;

    if (!chan.isMember(&client)) {
        sendNumeric(client, 442, channelName + " :You're not on that channel");
        return;
    }

    if (!chan.isOperator(&client)) {
        sendNumeric(client, 482, channelName + " :You're not channel operator");
        return;
    }

    Client *target = chan.findClientByNickname(targetNick);
    if (!target) {
        sendNumeric(client, 441, targetNick + " " + channelName + " :They aren't on that channel");
        return;
    }

    // Broadcast KICK message
    std::ostringstream kickMsg;
    kickMsg << ":" << client.getNickname() << "!" 
            << client.getUsername() << "@" << getClientHost(client.getFd())
            << " KICK " << channelName << " " << targetNick;
    if (!comment.empty())
        kickMsg << " :" << comment;
    kickMsg << "\r\n";
    
    sendToChannel(chan, kickMsg.str(), nullptr);

    // Remove user from channel
    chan.removeClient(target);
    
    if (chan.isEmpty()) {
        _channels.erase(channelName);
    }
}
