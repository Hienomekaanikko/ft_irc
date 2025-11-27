#include "Server.hpp"

/*
** Handle PING command
** Responds with a PONG message
*/
void Server::handlePING(Client &client, const std::vector<std::string_view> &params)
{
	if (params.empty())
	{
		sendNumeric(client, 409, "No origin specified");
		return;
	}
	std::string pongMsg = "PONG :" + std::string(params[0]) + "\r\n";
	sendTo(client, pongMsg);
}
