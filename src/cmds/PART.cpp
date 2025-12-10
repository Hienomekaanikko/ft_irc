#include "Server.hpp"
#include <sstream>

/*
** Handle PART command
** Validates parameters
** Checks if client is in the channel
** Removes client from channel
** Sends PART message to client and channel members
** Removes channel if empty
*/
void Server::handlePART(Client &client, const std::vector<std::string_view> &params)
{
	if (params.empty()) {
		sendNumeric(client, 461, "PART :Not enough parameters");
		return;
	}
	
	std::string channelName(params[0]);

	auto it = _channels.find(channelName);
	if (it == _channels.end())
	{
		sendNumeric(client, 403, channelName + " :No such channel");
		return;
	}
	
	Channel &chan = it->second;

	if (!chan.isMember(&client))
	{
		sendNumeric(client, 442, channelName + " :You're not on that channel");
		return;
	}
	
	std::string reason = "";
	if (params.size() > 1)
	{
		std::ostringstream reasonStream;
		for (std::size_t i = 1; i < params.size(); ++i)
		{
			if (i > 1)
				reasonStream << " ";
			reasonStream << params[i];
		}
		reason = reasonStream.str();
	}
	try
	{
		std::string clientName = client.getNickname();
		chan.removeClient(clientName);
	}
	catch (errs &e)
	{
		sendNumeric(client, e.num, e.msg);
		return;
	}
	std::ostringstream partMsg;
	partMsg << ":" << client.getNickname();

	if (client.hasUsername())
		partMsg << "!" << client.getUsername() << "@" << getClientHost(client.getFd());
	partMsg << " PART " << channelName;

	if (!reason.empty())
		partMsg << " :" << reason;
	partMsg << "\r\n";

	sendTo(client, partMsg.str());
	sendToChannel(chan, partMsg.str(), nullptr);

	if (chan.isEmpty())
	{
		_channels.erase(channelName);
		_channelCount--;
	}
}
