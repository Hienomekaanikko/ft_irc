#include "Server.hpp"
#include <sstream>

/*
** Handle JOIN command
** Validates parameters
** Checks if client has joined too many channels
** Checks if channel exists
** Checks if channel is password protected
** Checks if channel is invite-only
** Checks if channel is full
** Adds client to channel
*/
void Server::handleJOIN(Client &client, const std::vector<std::string_view> &params)
{
	if (params.empty())
	{
		sendNumeric(client, 461, "JOIN :Not enough parameters");
		return;
	}

	std::string _channelName(params[0]);
	if (client.getChannelCount() == 10)
	{
		sendNumeric(client, 405, _channelName + " :You have joined too many channels");
		return;
	}
	auto it = _channels.find(_channelName);
	if (it != _channels.end())
	{
		if (it->second.PasswordRequired())
		{
			if (params.size() < 2 || it->second.getPassword() != params[1])
			{
				sendNumeric(client, 475, _channelName + " :Cannot join channel (+k)");
				return ;
			}
		}
		if (it->second.UserlimitSet())
		{
			if (it->second.getUserLimit() == it->second.getCurrentUsers())
			{
				sendNumeric(client, 471, _channelName + " :Cannot join channel (+l)");
				return ;
			}
		}
		if (it->second.isInviteOnly())
		{
			if (!it->second.isInvited(&client))
			{
				sendNumeric(client, 473, _channelName + " :Cannot join channel (+i)");
				return ;
			}
		}
		it->second.addClient(&client);
	}
	else
	{
		if (_channelCount == 500) {
			sendNumeric(client, 600, _channelName + " :Channel not created. Too many channels exist");
			return ;
		}
		Channel newChannel(_channelName);
		newChannel.addClient(&client);
		newChannel.addOperator(client.getNickname());
		newChannel.setCreationTime(std::time(nullptr));
		_channels.emplace(_channelName, newChannel);
		_channelCount++;
	}
	Channel &chan = _channels.at(_channelName);
	std::ostringstream joinMsg;
	joinMsg << ":" << client.getNickname() << "!~" 
			<< client.getUsername() << "@" << getClientHost(client.getFd())
			<< " JOIN " << _channelName << "\r\n";
	sendToChannel(chan, joinMsg.str(), nullptr);
	
	const std::string &topic = chan.getTopic();
	if (!topic.empty())
		sendNumeric(client, 332, _channelName, topic);

	std::ostringstream names;
	for (Client *member : chan.getMembers())
	{
		if (chan.isOperator(member))
			names << "@";
		names << member->getNickname() << " ";
	}
	sendNumeric(client, 353, "= " + _channelName, ":" + names.str());

	std::ostringstream endNames;
	endNames << ":" << _serverName
			 << " 366 " << client.getNickname()
			 << " " << _channelName
			 << " :End of /NAMES list\r\n";
	sendTo(client, endNames.str());
	sendNumeric(client, 329, _channelName, std::to_string(chan.getCreationTime()));
}	
