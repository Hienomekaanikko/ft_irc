#include "Server.hpp"
#include <sstream>

void Server::handleINVITE(Client &client, const std::vector<std::string_view> &params)
{
    if (params.size() < 2) {
        sendNumeric(client, 461, "INVITE :Not enough parameters");
        return;
    }

    std::string targetNick(params[0]);
    std::string channelName(params[1]);

    Client *target = findClientByNick(targetNick);
    if (!target) {
        sendNumeric(client, 401, targetNick + " :No such nick");
        return;
    }

    auto it = _channels.find(channelName);
    if (it != _channels.end()) {
        Channel &chan = it->second;

        if (!chan.isMember(&client)) {
            sendNumeric(client, 442, channelName + " :You're not on that channel");
            return;
        }

        if (chan.isInviteOnly() && !chan.isOperator(&client)) {
            sendNumeric(client, 482, channelName + " :You're not channel operator");
            return;
        }

        if (chan.isMember(target)) {
            sendNumeric(client, 443, targetNick + " " + channelName + " :is already on channel");
            return;
        }
        
        chan.inviteUser(target);
    }
    
    // Send INVITE message to target
    std::ostringstream inviteMsg;
    inviteMsg << ":" << client.getNickname() << "!" 
              << client.getUsername() << "@" << getClientHost(client.getFd())
              << " INVITE " << targetNick << " :" << channelName << "\r\n";
    sendTo(*target, inviteMsg.str());
    
    sendNumeric(client, 341, targetNick + " " + channelName);
}
