#include "Server.hpp"

#include <sstream>

void Server::handlePART(Client &client, const std::vector<std::string_view> &params)
{
	if (params.empty()) {
		sendNumeric(client, 461, "PART :Not enough parameters");
		return;
	}
	
	std::string channelName(params[0]);

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

	// Broadcast PART to all channel members
	std::ostringstream partMsg;
	partMsg << ":" << client.getNickname();
	if (client.hasUsername())
		partMsg << "!" << client.getUsername() << "@" << getClientHost(client.getFd());
	partMsg << " PART " << channelName;
	if (!reason.empty())
		partMsg << " :" << reason;
	partMsg << "\r\n";

	sendToChannel(chan, partMsg.str(), nullptr);

	// Remove client from channel
	chan.removeClient(&client);
}
