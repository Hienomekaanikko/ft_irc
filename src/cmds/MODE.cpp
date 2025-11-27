#include "Server.hpp"

#include <sstream>

/*
** Handle MODE command
** Sets the MODE for channel
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
	if (params.size() < 2) {
		sendNumeric(client, 461, "MODE :Not enough parameters");
		return;
	}
	auto it = _channels.find(channelName);
	if (it == _channels.end()) {
		sendNumeric(client, 403, channelName, " No such channel with" + std::string(params[0])); // ERR_NOSUCHCHANNEL
		return;
	}
	Channel &chan = it->second;
	if (!chan.isOperator(&client)) {
		sendNumeric(client, 482, channelName, "You're not channel operator"); // ERR_CHANOPRIVSNEEDED
		return;
	}
	// Mode params that we pass down to Channel (everything after channel name)s
	std::vector<std::string_view> modeParams(params.begin() + 1, params.end());

	try {
		chan.setMode(modeParams); // modifies internal flags, password, limit, ops...
	} catch (const errs &e) {
		sendNumeric(client, e.num, e.msg);
		return;
	}

	// If we reached here, the mode change succeeded.
	// Now we must announce it to ALL channel members.

	std::ostringstream oss;
	// Prefix: :nick!user@host
	oss << ":" << client.getNickname();
	if (client.hasUsername())
		oss << "!" << client.getUsername() << "@" << getClientHost(client.getFd());

	oss << " MODE " << channelName;

	// Re-attach the exact mode string and its params (e.g. "+ik key 10 nick")
	for (std::size_t i = 0; i < modeParams.size(); ++i) {
		oss << " " << modeParams[i];
	}
	oss << "\r\n";

	// Broadcast to everyone in channel (including the setter)
	sendToChannel(chan, oss.str(), nullptr);
}
