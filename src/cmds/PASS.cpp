#include "Server.hpp"

/*
** Handle PASS command
** Validates parameters
** Verifies the password and updates client state
*/
void Server::handlePASS(Client &client, const std::vector<std::string_view> &params)
{
	if (client.isRegistered())
	{
		sendNumeric(client, 462, "You may not reregister");
		return;
	}
	if (params.empty())
	{
		sendNumeric(client, 461, "PASS :Not enough parameters");
		return;
	}
	if (std::string(params[0]) != _password)
	{
		sendNumeric(client, 464, "Password incorrect");
		return;
	}
	client.setHasPassword(true);
	maybeRegistered(client);
}