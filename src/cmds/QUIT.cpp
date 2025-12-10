#include "Server.hpp"

/*
** Handle QUIT command
** Validates parameters
** Disconnects client
*/
void Server::handleQUIT(Client &client, const std::vector<std::string_view> &params)
{
	std::string reason = "Client Quit";
	if (!params.empty())
		reason = std::string(params[0]);
	disconnectClient(client.getFd(), reason);
}
