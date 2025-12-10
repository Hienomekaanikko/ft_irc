#include "Server.hpp"

/*
** Handle USER command
** Validates parameters
** Checks if client is already registered
** Sets client's username and fullname
** Updates client state
*/
void Server::handleUSER(Client &client, const std::vector<std::string_view> &params)
{
	if (!client.hasPassword())
	{
		sendNumeric(client, 451, "You have not registered");
		return;
	}
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
