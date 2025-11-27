#include "Server.hpp"

/*
** Handle USER command
** Sets the client's username and fullname
*/
void Server::handleUSER(Client &client, const std::vector<std::string_view> &params)
{
	if (params.size() < 4)
	{
		sendNumeric(client, 461, "USER :Not enough parameters");
		return;
	}
	if (client.isRegistered())
	{
		sendNumeric(client, 462, "You may not reregister");
		return;
	}
	client.setUsername(std::string(params[0]));
	client.setFullname(std::string(params[3]));
	maybeRegistered(client);
}
