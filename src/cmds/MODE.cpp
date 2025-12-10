#include "Server.hpp"
#include <sstream>

/*
** Handle MODE command
** Validates parameters
** Checks if channel exists
** Checks if client is channel operator
** Sets channel mode
** Sends MODE message to channel members
*/
void Server::handleMODE(Client &client, const std::vector<std::string_view> &params)
{
	if (params.empty()) {
		sendNumeric(client, 461, "MODE :Not enough parameters");
		return;
	}
	
	std::string target(params[0]);
	if (target[0] != '#') {
		if (params.size() == 1) {
			sendNumeric(client, 221, "+");
			return;
		}
		return;
	}
	std::string channelName = target;
	
	auto it = _channels.find(channelName);
	if (it == _channels.end()) {
		sendNumeric(client, 403, channelName, ":No such channel");
		return;
	}
	
	Channel &chan = it->second;
	if (params.size() < 2)
	{
		sendNumeric(client, 324, channelName, chan.getModeString());
		return;
	}
	if (!chan.isOperator(&client))
	{
		sendNumeric(client, 482, channelName, ":You're not channel operator");
		return;
	}
	std::vector<std::string_view> modeParams(params.begin() + 1, params.end());
	try	
	{
		chan.setMode(modeParams);
	}
	catch (errs &e)
	{
		sendNumeric(client, e.num, e.msg);
		return;
	}
	std::ostringstream oss;
	oss << ":" << client.getNickname();
	if (client.hasUsername())
		oss << "!" << client.getUsername() << "@" << getClientHost(client.getFd());

	oss << " MODE " << channelName;
	for (std::size_t i = 0; i < modeParams.size(); ++i)
		oss << " " << modeParams[i];
	oss << "\r\n";
	sendToChannel(chan, oss.str(), nullptr);
}
