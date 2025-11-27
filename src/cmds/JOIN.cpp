#include "Server.hpp"
#include <sstream>

/*
** Handle JOIN command
** Joins client to a channel
*/
void Server::handleJOIN(Client &client, const std::vector<std::string_view> &params)
{
	if (params.empty())
	{
		sendNumeric(client, 461, "JOIN :Not enough parameters");
		return;
	}

	std::string _channelName(params[0]);
	auto it = _channels.find(_channelName);
	if (it != _channels.end())
	{
		if (it->second.PasswordRequired()){
			if (params.size() < 2 || it->second.getPassword() != params[1]) {
				sendNumeric(client, 475, _channelName + " :Password required/Invalid password");
				return ;
			}
		}
		if (it->second.UserlimitSet()) {
			if (it->second.getUserLimit() == it->second.getCurrentUsers()) {
				sendNumeric(client, 471, _channelName + " :Channel is full");
				return ;
			}
		}
		if (it->second.isInviteOnly()) {
			if (!it->second.isInvited(&client)) {
				sendNumeric(client, 473, _channelName + " :Channel is invite only");
				return ;
			}
		}
		if (it->second.isBanned(&client)) {
			sendNumeric(client, 474, _channelName + " :Banned from channel");
			return ;
		}
		it->second.addClient(&client);
	}
	else
	{
		Channel newChannel(_channelName);
		newChannel.addClient(&client);
		newChannel.addOperator(&client);
		_channels.emplace(_channelName, newChannel);
	}

	// Send JOIN confirmation to all channel members
	Channel &chan = _channels.at(_channelName);
	std::ostringstream joinMsg;
	joinMsg << ":" << client.getNickname() << "!~" 
			<< client.getUsername() << "@" << getClientHost(client.getFd())
			<< " JOIN " << _channelName << "\r\n";
	sendToChannel(chan, joinMsg.str(), nullptr);
	
	// Send topic if set
	const std::string &topic = chan.getTopic();
	if (!topic.empty()) {
		sendNumeric(client, 332, _channelName, topic);
	}
	
	// Send names list
	std::ostringstream names;
	for (Client *member : chan.getMembers()) {
		if (chan.isOperator(member))
			names << "@";
		names << member->getNickname() << " ";
	}
	
	sendNumeric(client, 353, "= " + _channelName, ":" + names.str());
	sendNumeric(client, 366, _channelName, ":End of /NAMES list");
}
