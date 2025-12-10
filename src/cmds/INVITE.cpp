#include "Server.hpp"
#include <sstream>

/*
** Handle INVITE command
** Validates parameters
** Checks if target client exists
** Checks if channel exists
** Checks if client is in the channel
** Checks if channel is invite-only
** Checks if client is channel operator
** Checks if target client is already in the channel
** Adds target client to channel invite list
** Sends INVITE message to target client
** Sends 341 numeric to inviting client
*/
void Server::handleINVITE(Client &client, const std::vector<std::string_view> &params)
{
    if (params.size() < 2)
    {
        sendNumeric(client, 461, "INVITE :Not enough parameters");
        return;
    }

    std::string targetNick(params[0]);
    std::string channelName(params[1]);

    Client *target = findClientByNick(targetNick);
    if (!target)
    {
        sendNumeric(client, 401, targetNick + " :No such nick");
        return;
    }

    auto it = _channels.find(channelName);
    if (it != _channels.end())
    {
        Channel &chan = it->second;

        if (!chan.isMember(&client))
        {
            sendNumeric(client, 442, channelName + " :You're not on that channel");
            return;
        }

        if (chan.isInviteOnly() && !chan.isOperator(&client))
        {
            sendNumeric(client, 482, channelName + " :You're not channel operator");
            return;
        }

        if (chan.isMember(target))
        {
            sendNumeric(client, 443, targetNick + " " + channelName + " :is already on channel");
            return;
        }
        
        chan.inviteUser(target);
    }
    else
    {
        sendNumeric(client, 403, channelName + " :No such channel");
        return;
    }
    std::ostringstream inviteMsg;
    inviteMsg << ":" << client.getNickname() << "!" 
              << client.getUsername() << "@" << getClientHost(client.getFd())
              << " INVITE " << targetNick << " :" << channelName << "\r\n";
    sendTo(*target, inviteMsg.str());
    
    sendNumeric(client, 341, targetNick + " " + channelName);
}
