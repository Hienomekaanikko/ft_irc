#include "Server.hpp"
#include "Client.hpp"
#include "Channel.hpp"
#include "Utils.hpp"
#include <sstream>

void Server::cmdPass(Client* client, const std::vector<std::string>& params) {
    if (client->isRegistered()) {
        client->sendMessage(ERR_ALREADYREGISTRED(client->getNickname()));
        return;
    }
    
    if (params.empty()) {
        client->sendMessage(ERR_NEEDMOREPARAMS("*", "PASS"));
        return;
    }
    
    if (params[0] == _password) {
        client->setAuthenticated(true);
    } else {
        client->sendMessage(ERR_PASSWDMISMATCH("*"));
    }
}

void Server::cmdNick(Client* client, const std::vector<std::string>& params) {
    if (params.empty()) {
        std::string nick = client->getNickname().empty() ? "*" : client->getNickname();
        client->sendMessage(ERR_NONICKNAMEGIVEN(nick));
        return;
    }
    
    std::string newNick = params[0];
    
    if (!isValidNickname(newNick)) {
        std::string nick = client->getNickname().empty() ? "*" : client->getNickname();
        client->sendMessage(ERR_ERRONEUSNICKNAME(nick, newNick));
        return;
    }
    
    if (getClientByNick(newNick) != NULL) {
        std::string nick = client->getNickname().empty() ? "*" : client->getNickname();
        client->sendMessage(ERR_NICKNAMEINUSE(nick, newNick));
        return;
    }
    
    std::string oldNick = client->getNickname();
    client->setNickname(newNick);
    
    if (!oldNick.empty()) {
        std::string message = ":" + oldNick + " NICK :" + newNick + "\r\n";
        client->sendMessage(message);
    }
}

void Server::cmdUser(Client* client, const std::vector<std::string>& params) {
    if (client->isRegistered()) {
        client->sendMessage(ERR_ALREADYREGISTRED(client->getNickname()));
        return;
    }
    
    if (params.size() < 4) {
        std::string nick = client->getNickname().empty() ? "*" : client->getNickname();
        client->sendMessage(ERR_NEEDMOREPARAMS(nick, "USER"));
        return;
    }
    
    if (!client->isAuthenticated()) {
        std::string nick = client->getNickname().empty() ? "*" : client->getNickname();
        client->sendMessage(ERR_PASSWDMISMATCH(nick));
        return;
    }
    
    client->setUsername(params[0]);
    client->setRealname(params[3]);
    
    if (!client->getNickname().empty()) {
        client->setRegistered(true);
        client->sendMessage(RPL_WELCOME(client->getNickname(), client->getUsername(), client->getHostname()));
    }
}

void Server::cmdJoin(Client* client, const std::vector<std::string>& params) {
    if (!client->isRegistered()) {
        client->sendMessage(ERR_NOTREGISTERED(client->getNickname()));
        return;
    }
    
    if (params.empty()) {
        client->sendMessage(ERR_NEEDMOREPARAMS(client->getNickname(), "JOIN"));
        return;
    }
    
    std::vector<std::string> channels = split(params[0], ',');
    std::vector<std::string> keys;
    
    if (params.size() > 1) {
        keys = split(params[1], ',');
    }
    
    for (size_t i = 0; i < channels.size(); i++) {
        std::string channelName = channels[i];
        
        if (!isValidChannelName(channelName)) {
            client->sendMessage(ERR_NOSUCHCHANNEL(client->getNickname(), channelName));
            continue;
        }
        
        Channel* channel = getChannel(channelName);
        
        if (channel == NULL) {
            channel = createChannel(channelName, client);
            client->joinChannel(channel);
        } else {
            if (channel->hasClient(client)) {
                continue;
            }
            
            if (channel->isInviteOnly() && !channel->isInvited(client)) {
                client->sendMessage(ERR_INVITEONLYCHAN(client->getNickname(), channelName));
                continue;
            }
            
            if (!channel->getKey().empty()) {
                std::string key = (i < keys.size()) ? keys[i] : "";
                if (key != channel->getKey()) {
                    client->sendMessage(ERR_BADCHANNELKEY(client->getNickname(), channelName));
                    continue;
                }
            }
            
            if (channel->getUserLimit() > 0 && (int)channel->getClients().size() >= channel->getUserLimit()) {
                client->sendMessage(ERR_CHANNELISFULL(client->getNickname(), channelName));
                continue;
            }
            
            channel->addClient(client);
            client->joinChannel(channel);
            channel->removeInvited(client);
        }
        
        std::string joinMsg = ":" + client->getPrefix() + " JOIN " + channelName + "\r\n";
        channel->broadcast(joinMsg);
        
        if (!channel->getTopic().empty()) {
            client->sendMessage(RPL_TOPIC(client->getNickname(), channelName, channel->getTopic()));
        } else {
            client->sendMessage(RPL_NOTOPIC(client->getNickname(), channelName));
        }
        
        std::string names;
        const std::vector<Client*>& clients = channel->getClients();
        for (size_t j = 0; j < clients.size(); j++) {
            if (channel->isOperator(clients[j])) {
                names += "@";
            }
            names += clients[j]->getNickname();
            if (j < clients.size() - 1) {
                names += " ";
            }
        }
        
        client->sendMessage(RPL_NAMREPLY(client->getNickname(), channelName, names));
        client->sendMessage(RPL_ENDOFNAMES(client->getNickname(), channelName));
    }
}

void Server::cmdPart(Client* client, const std::vector<std::string>& params) {
    if (!client->isRegistered()) {
        client->sendMessage(ERR_NOTREGISTERED(client->getNickname()));
        return;
    }
    
    if (params.empty()) {
        client->sendMessage(ERR_NEEDMOREPARAMS(client->getNickname(), "PART"));
        return;
    }
    
    std::vector<std::string> channels = split(params[0], ',');
    std::string reason = (params.size() > 1) ? params[1] : "";
    
    for (size_t i = 0; i < channels.size(); i++) {
        std::string channelName = channels[i];
        Channel* channel = getChannel(channelName);
        
        if (channel == NULL) {
            client->sendMessage(ERR_NOSUCHCHANNEL(client->getNickname(), channelName));
            continue;
        }
        
        if (!channel->hasClient(client)) {
            client->sendMessage(ERR_NOTONCHANNEL(client->getNickname(), channelName));
            continue;
        }
        
        std::string partMsg = ":" + client->getPrefix() + " PART " + channelName;
        if (!reason.empty()) {
            partMsg += " :" + reason;
        }
        partMsg += "\r\n";
        
        channel->broadcast(partMsg);
        
        channel->removeClient(client);
        client->leaveChannel(channel);
        
        if (channel->getClients().empty()) {
            removeChannel(channelName);
        }
    }
}

void Server::cmdPrivmsg(Client* client, const std::vector<std::string>& params) {
    if (!client->isRegistered()) {
        client->sendMessage(ERR_NOTREGISTERED(client->getNickname()));
        return;
    }
    
    if (params.size() < 2) {
        client->sendMessage(ERR_NEEDMOREPARAMS(client->getNickname(), "PRIVMSG"));
        return;
    }
    
    std::string target = params[0];
    std::string message = params[1];
    
    if (target[0] == '#' || target[0] == '&') {
        Channel* channel = getChannel(target);
        if (channel == NULL) {
            client->sendMessage(ERR_NOSUCHCHANNEL(client->getNickname(), target));
            return;
        }
        
        if (!channel->hasClient(client)) {
            client->sendMessage(ERR_CANNOTSENDTOCHAN(client->getNickname(), target));
            return;
        }
        
        std::string msg = ":" + client->getPrefix() + " PRIVMSG " + target + " :" + message + "\r\n";
        channel->broadcast(msg, client);
    } else {
        Client* targetClient = getClientByNick(target);
        if (targetClient == NULL) {
            client->sendMessage(ERR_NOSUCHNICK(client->getNickname(), target));
            return;
        }
        
        std::string msg = ":" + client->getPrefix() + " PRIVMSG " + target + " :" + message + "\r\n";
        targetClient->sendMessage(msg);
    }
}

void Server::cmdQuit(Client* client, const std::vector<std::string>& params) {
    std::string reason = params.empty() ? "Client quit" : params[0];
    std::string quitMsg = ":" + client->getPrefix() + " QUIT :" + reason + "\r\n";
    
    const std::vector<Channel*>& channels = client->getChannels();
    for (size_t i = 0; i < channels.size(); i++) {
        channels[i]->broadcast(quitMsg);
        channels[i]->removeClient(client);
    }
    
    removeClient(client->getFd());
}

void Server::cmdPing(Client* client, const std::vector<std::string>& params) {
    if (params.empty()) {
        client->sendMessage(ERR_NEEDMOREPARAMS(client->getNickname(), "PING"));
        return;
    }
    
    std::string pongMsg = ":localhost PONG localhost :" + params[0] + "\r\n";
    client->sendMessage(pongMsg);
}

void Server::cmdKick(Client* client, const std::vector<std::string>& params) {
    if (!client->isRegistered()) {
        client->sendMessage(ERR_NOTREGISTERED(client->getNickname()));
        return;
    }
    
    if (params.size() < 2) {
        client->sendMessage(ERR_NEEDMOREPARAMS(client->getNickname(), "KICK"));
        return;
    }
    
    std::string channelName = params[0];
    std::string targetNick = params[1];
    std::string reason = (params.size() > 2) ? params[2] : client->getNickname();
    
    Channel* channel = getChannel(channelName);
    if (channel == NULL) {
        client->sendMessage(ERR_NOSUCHCHANNEL(client->getNickname(), channelName));
        return;
    }
    
    if (!channel->hasClient(client)) {
        client->sendMessage(ERR_NOTONCHANNEL(client->getNickname(), channelName));
        return;
    }
    
    if (!channel->isOperator(client)) {
        client->sendMessage(ERR_CHANOPRIVSNEEDED(client->getNickname(), channelName));
        return;
    }
    
    Client* targetClient = getClientByNick(targetNick);
    if (targetClient == NULL || !channel->hasClient(targetClient)) {
        client->sendMessage(ERR_USERNOTINCHANNEL(client->getNickname(), targetNick, channelName));
        return;
    }
    
    std::string kickMsg = ":" + client->getPrefix() + " KICK " + channelName + " " + targetNick + " :" + reason + "\r\n";
    channel->broadcast(kickMsg);
    
    channel->removeClient(targetClient);
    targetClient->leaveChannel(channel);
    
    if (channel->getClients().empty()) {
        removeChannel(channelName);
    }
}

void Server::cmdInvite(Client* client, const std::vector<std::string>& params) {
    if (!client->isRegistered()) {
        client->sendMessage(ERR_NOTREGISTERED(client->getNickname()));
        return;
    }
    
    if (params.size() < 2) {
        client->sendMessage(ERR_NEEDMOREPARAMS(client->getNickname(), "INVITE"));
        return;
    }
    
    std::string targetNick = params[0];
    std::string channelName = params[1];
    
    Channel* channel = getChannel(channelName);
    if (channel == NULL) {
        client->sendMessage(ERR_NOSUCHCHANNEL(client->getNickname(), channelName));
        return;
    }
    
    if (!channel->hasClient(client)) {
        client->sendMessage(ERR_NOTONCHANNEL(client->getNickname(), channelName));
        return;
    }
    
    if (channel->isInviteOnly() && !channel->isOperator(client)) {
        client->sendMessage(ERR_CHANOPRIVSNEEDED(client->getNickname(), channelName));
        return;
    }
    
    Client* targetClient = getClientByNick(targetNick);
    if (targetClient == NULL) {
        client->sendMessage(ERR_NOSUCHNICK(client->getNickname(), targetNick));
        return;
    }
    
    if (channel->hasClient(targetClient)) {
        client->sendMessage(ERR_USERONCHANNEL(client->getNickname(), targetNick, channelName));
        return;
    }
    
    channel->addInvited(targetClient);
    
    std::string inviteMsg = ":" + client->getPrefix() + " INVITE " + targetNick + " " + channelName + "\r\n";
    targetClient->sendMessage(inviteMsg);
}

void Server::cmdTopic(Client* client, const std::vector<std::string>& params) {
    if (!client->isRegistered()) {
        client->sendMessage(ERR_NOTREGISTERED(client->getNickname()));
        return;
    }
    
    if (params.empty()) {
        client->sendMessage(ERR_NEEDMOREPARAMS(client->getNickname(), "TOPIC"));
        return;
    }
    
    std::string channelName = params[0];
    Channel* channel = getChannel(channelName);
    
    if (channel == NULL) {
        client->sendMessage(ERR_NOSUCHCHANNEL(client->getNickname(), channelName));
        return;
    }
    
    if (!channel->hasClient(client)) {
        client->sendMessage(ERR_NOTONCHANNEL(client->getNickname(), channelName));
        return;
    }
    
    if (params.size() == 1) {
        if (channel->getTopic().empty()) {
            client->sendMessage(RPL_NOTOPIC(client->getNickname(), channelName));
        } else {
            client->sendMessage(RPL_TOPIC(client->getNickname(), channelName, channel->getTopic()));
        }
    } else {
        if (channel->isTopicRestricted() && !channel->isOperator(client)) {
            client->sendMessage(ERR_CHANOPRIVSNEEDED(client->getNickname(), channelName));
            return;
        }
        
        std::string newTopic = params[1];
        channel->setTopic(newTopic);
        
        std::string topicMsg = ":" + client->getPrefix() + " TOPIC " + channelName + " :" + newTopic + "\r\n";
        channel->broadcast(topicMsg);
    }
}

void Server::cmdMode(Client* client, const std::vector<std::string>& params) {
    if (!client->isRegistered()) {
        client->sendMessage(ERR_NOTREGISTERED(client->getNickname()));
        return;
    }
    
    if (params.empty()) {
        client->sendMessage(ERR_NEEDMOREPARAMS(client->getNickname(), "MODE"));
        return;
    }
    
    std::string target = params[0];
    
    if (target[0] != '#' && target[0] != '&') {
        return;
    }
    
    Channel* channel = getChannel(target);
    if (channel == NULL) {
        client->sendMessage(ERR_NOSUCHCHANNEL(client->getNickname(), target));
        return;
    }
    
    if (!channel->hasClient(client)) {
        client->sendMessage(ERR_NOTONCHANNEL(client->getNickname(), target));
        return;
    }
    
    if (params.size() == 1) {
        return;
    }
    
    if (!channel->isOperator(client)) {
        client->sendMessage(ERR_CHANOPRIVSNEEDED(client->getNickname(), target));
        return;
    }
    
    std::string modes = params[1];
    size_t paramIndex = 2;
    bool adding = true;
    
    for (size_t i = 0; i < modes.length(); i++) {
        char mode = modes[i];
        
        if (mode == '+') {
            adding = true;
        } else if (mode == '-') {
            adding = false;
        } else if (mode == 'i') {
            channel->setInviteOnly(adding);
        } else if (mode == 't') {
            channel->setTopicRestricted(adding);
        } else if (mode == 'k') {
            if (adding && paramIndex < params.size()) {
                channel->setKey(params[paramIndex++]);
            } else if (!adding) {
                channel->setKey("");
            }
        } else if (mode == 'o') {
            if (paramIndex < params.size()) {
                std::string targetNick = params[paramIndex++];
                Client* targetClient = getClientByNick(targetNick);
                if (targetClient && channel->hasClient(targetClient)) {
                    if (adding) {
                        channel->addOperator(targetClient);
                    } else {
                        channel->removeOperator(targetClient);
                    }
                }
            }
        } else if (mode == 'l') {
            if (adding && paramIndex < params.size()) {
                std::stringstream ss(params[paramIndex++]);
                int limit;
                ss >> limit;
                channel->setUserLimit(limit);
            } else if (!adding) {
                channel->setUserLimit(-1);
            }
        }
    }
    
    std::string modeMsg = ":" + client->getPrefix() + " MODE " + target + " " + modes + "\r\n";
    channel->broadcast(modeMsg);
}
