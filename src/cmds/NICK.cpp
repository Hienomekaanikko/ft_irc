#include "Server.hpp"

#include <sstream>
#include <unordered_set>

/*
** Handle NICK command
** Sets the client's nickname
*/
void Server::handleNICK(Client &client, const std::vector<std::string_view> &params)
{
	// 1) No nickname given
	if (params.empty())
	{
		sendNumeric(client, 431, "No nickname given"); // ERR_NONICKNAMEGIVEN
		return;
	}

	std::string newNick(params[0]);

	// 2) If same as current nickname, do nothing
	if (client.hasNickname() && client.getNickname() == newNick)
		return;

	// 3) Nick already in use by someone else?
	if (nickInUse(newNick))
	{
		// server 433 <currentnick> <newnick> :Nickname is already in use
		sendNumeric(client, 433, "* " + newNick, "Nickname is already in use");
		return;
	}

	bool hadNickBefore = client.hasNickname();
	std::string oldNick;
	if (hadNickBefore)
		oldNick = client.getNickname();

	// 4) Update nickname
	client.setNickname(newNick);

	// 5) If registered and actually changing nick, broadcast:
	//    :oldNick!user@host NICK :newNick for irssi to pick up the change
	if (hadNickBefore && oldNick != newNick)
	{
		std::ostringstream oss;
		oss << ":" << oldNick;
		if (client.hasUsername())
			oss << "!" << client.getUsername() << "@" << getClientHost(client.getFd());

		oss << " NICK :" << newNick << "\r\n";
		std::string msg = oss.str();

		// Send to all connected clients (including the one who changed nick)
		// Send to self and users in shared channels
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
//:ft_irc_server 433 * Zorma :Nickname is already in use

bool Server::nickInUse(std::string_view nick) {
	if (_clients.empty())
		return false;
	for (auto it = _clients.begin(); it != _clients.end(); ++it) {
		if (it->second.getNickname() == nick)
			return true;
	}
	return false;
}
