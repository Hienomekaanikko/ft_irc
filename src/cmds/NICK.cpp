#include "Server.hpp"
#include <sstream>
#include <unordered_set>

/*
** Handle NICK command
** Validates parameters
** Checks if nickname is already in use
** Updates nickname if valid
*/
void Server::handleNICK(Client &client, const std::vector<std::string_view> &params)
{
	if (!client.hasPassword())
	{
		sendNumeric(client, 451, "You have not registered");
		return;
	}
	if (params.empty())
	{
		sendNumeric(client, 431, "No nickname given");
		return;
	}
	std::string newNick(params[0]);
	if (client.hasNickname() && client.getNickname() == newNick)
		return;
	if (nickInUse(newNick))
	{
		sendNumeric(client, 433, "* " + newNick, "Nickname is already in use");
		return;
	}
	bool hadNickBefore = client.hasNickname();
	std::string oldNick;
	if (hadNickBefore)
		oldNick = client.getNickname();
	client.setNickname(newNick);
	if (hadNickBefore && oldNick != newNick)
	{
		std::ostringstream oss;
		oss << ":" << oldNick;
		if (client.hasUsername())
			oss << "!" << client.getUsername() << "@" << getClientHost(client.getFd());

		oss << " NICK :" << newNick << "\r\n";
		std::string msg = oss.str();
		std::unordered_set<Client*> recipients;
		recipients.insert(&client);
		for (auto &chanPair : _channels) {
			Channel &chan = chanPair.second;
			if (chan.isMember(&client)) {
				const auto &members = chan.getMembers();
				recipients.insert(members.begin(), members.end());
			}
		}
		for (Client *recipient : recipients)
			sendTo(*recipient, msg);
	}
	maybeRegistered(client);
}

/*
** Checks if nickname is already in use
*/
bool Server::nickInUse(std::string_view nick) {
	if (_clients.empty())
		return false;
	for (auto it = _clients.begin(); it != _clients.end(); ++it)
	{
		if (it->second.getNickname() == nick)
			return true;
	}
	return false;
}
