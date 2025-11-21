#include "Server.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>
#include <string_view>

std::string joinParams(const std::vector<std::string_view> &params, size_t start = 1) {
	if (params.size() <= start) return "";

	std::ostringstream oss;
	oss << params[start];  // first word

	for (size_t i = start + 1; i < params.size(); ++i) {
		oss << " " << params[i];  // add remaining words with space
	}

	return oss.str();
}

/*
** Handle message sending inside a channel
*/
void Server::handlePRIVMSG(Client &client, const std::vector<std::string_view> &params)
{
	if (params.size() < 2) {
		sendNumeric(client, 412, ":No text to send"); // ERR_NOTEXTTOSEND
		return;
	}

	std::string target(params[0]);
	std::string msg = joinParams(params, 1);
	
	if (target[0] == '#') {
		// Message to channel
		auto it = _channels.find(target);
		if (it == _channels.end()) {
			sendNumeric(client, 403, target + " :No such channel");
			return;
		}
		Channel &chan = it->second;

		if (!chan.isMember(&client)) {
			sendNumeric(client, 442, target + " :You're not on that channel"); // ERR_NOTONCHANNEL
			return;
		}
		// Broadcast to everyone except sender (should probably add the getHostname() but i dont fully understand what it means yet.)
		std::string prefix = ":" + formatPrefix(client) + "!";
		prefix += client.getUsername() + "@"; //+ client.getHostname();

		std::string message = prefix + " PRIVMSG " + target + " :" + msg + "\r\n";

		for (Client *member : chan.getMembers()) {
			if (member->getFd() != client.getFd())    // do not send back to sender
				sendTo(*member, message);
		}
	} else {
		// Private message to a nickname
		Client *targetClient = findClientByNick(target);
		if (!targetClient) {
			sendNumeric(client, 401, target + " :No such nick"); // ERR_NOSUCHNICK
			return;
		}

		std::string prefix = ":" + formatPrefix(client) + "!";
		prefix += client.getUsername() + "@"; //+ client.getHostname();

		//same thing here (should probably add the getHostname() but i dont fully understand what it means yet.)
		std::string message = prefix + " PRIVMSG " + target + " :" + msg + "\r\n";

		sendTo(*targetClient, message);
	}
}

Client* Server::findClientByNick(const std::string &nick) {
	for (auto &pair : _clients) {
		Client &c = pair.second;
		if (c.hasNickname() && c.getNickname() == nick) {
			return &c;  // return pointer to the matching client
		}
	}
	return nullptr;  // not found
}
