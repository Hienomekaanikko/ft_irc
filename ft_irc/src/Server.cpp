#include "Server.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstring>
#include <string_view>
#include <cstdlib>
#include <cerrno>
#include <unistd.h>
#include <fcntl.h>
#include <algorithm>
#include <sys/socket.h>

/// Constructor ///
Server::Server(int port, const std::string &password)
	: _port(port), _password(password), _addrLen(sizeof(_address))
{
	initSocket();
}

/// Destructor ///
Server::~Server()
{
	for (auto &pollFd : _fds)
		close(pollFd.fd);
}

/// Public member functions ///

/*
** Start the server
*/
void Server::run()
{
	_running = true;
	std::cout << "Server is running..." << std::endl;
	mainLoop();
}

void Server::shutdown()
{
	if (!_running)
		return;
	std::cout << "\nShutting down server..." << std::endl;
	_running = false;

	// Disconnect all clients
	std::vector<int> fds;
	fds.reserve(_clients.size());
	for (std::unordered_map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
		fds.push_back(it->first);
	for (int fd : fds)
		disconnectClient(fd, "Server shutting down");

	// Close server socket
	if (_serverFd >= 0)
	{
		close(_serverFd);
		_serverFd = -1;
	}

	// Cleanup containers
	_fds.clear();
	_clients.clear();

	std::cout << "Server shutdown successful." << std::endl;
}

/////////////////////////////////////////////////////////////////////////////////////////
/// Member functions ///

/*
** Main server loop
** Uses poll to monitor multiple file descriptors
** Handles new connections and client read/write events
*/
void Server::mainLoop()
{
	while (_running)
	{
		int ready = ::poll(_fds.data(), _fds.size(), -1);
		if (ready < 0)
		{
			if (errno == EINTR)
			{
				if (!_running)
					break; // Exit loop if server is shutting down
				continue;  // Otherwise, continue polling
			}
			throw std::runtime_error("Poll failed: " + std::string(strerror(errno)));
		}

		for (std::size_t i = 0; i < _fds.size() && ready > 0; ++i)
		{
			short revents = _fds[i].revents;
			if (revents == 0)
				continue;
			--ready;

			if (_fds[i].fd == _serverFd && (revents & POLLIN))
				handleNewConnection();
			else
			{
				if (revents & POLLIN)
					handleClientRead(i);
				if (revents & POLLOUT) {
					handleClientWrite(i);
				}
			}
		}
	}
}

/*
** Initialize server socket
** Sets up the socket, binds it to the specified port, and starts listening for connections
*/
void Server::initSocket()
{
	_serverFd = socket(AF_INET, SOCK_STREAM, 0);
	if (_serverFd < 0)
		throw std::runtime_error("Socket creation failed: " + std::string(strerror(errno)));

	// Set socket to non-blocking mode
	if (::fcntl(_serverFd, F_SETFL, O_NONBLOCK) < 0)
		throw std::runtime_error("Set non-blocking mode failed: " + std::string(strerror(errno)));

	int opt = 1;
	if (::setsockopt(_serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
		throw std::runtime_error("Set socket options failed: " + std::string(strerror(errno)));

	_address.sin_family = AF_INET;
	_address.sin_addr.s_addr = INADDR_ANY;
	_address.sin_port = htons(_port);

	if (::bind(_serverFd, reinterpret_cast<struct sockaddr *>(&_address), sizeof(_address)) < 0)
		throw std::runtime_error("Bind failed: " + std::string(strerror(errno)));

	if (::listen(_serverFd, 10) < 0)
		throw std::runtime_error("Listen failed: " + std::string(strerror(errno)));

	std::cout << "IRC Server is now listening on port " << _port
			  << " (password: " << _password << ")" << std::endl;

	// Add server fd to poll fds
	pollfd serverPollFd;
	serverPollFd.fd = _serverFd;
	serverPollFd.events = POLLIN;
	serverPollFd.revents = 0;
	_fds.push_back(serverPollFd);
}

/*
** Handle new client connections
** Accepts the connection, sets the socket to non-blocking,
** adds the client to the poll fds and the clients map
*/
void Server::handleNewConnection()
{
	// Accept new client connection
	_addrLen = sizeof(_address);

	int clientFd = ::accept(_serverFd, reinterpret_cast<struct sockaddr *>(&_address), &_addrLen);
	if (clientFd < 0)
	{
		if (errno != EWOULDBLOCK && errno != EAGAIN)
		{
			std::cerr << "Accept failed: " << strerror(errno) << std::endl;
			return;
		}
		std::perror("Accept failed");
		return;
	}

	// Set socket to non-blocking mode
	if (::fcntl(clientFd, F_SETFL, O_NONBLOCK) < 0)
		throw std::runtime_error("Set non-blocking mode failed: " + std::string(strerror(errno)));

	std::cout << "New client connected: fd = " << clientFd << std::endl;

	pollfd clientPollFd;
	clientPollFd.fd = clientFd;
	clientPollFd.events = POLLIN;
	clientPollFd.revents = 0;
	_fds.push_back(clientPollFd);

	_clients.emplace(clientFd, Client(clientFd));
}

/*
** Handle client read events
** Reads data from the client socket, processes complete lines,
** and handles client disconnections
*/
void Server::handleClientRead(std::size_t index)
{
	int clientFd = _fds[index].fd;
	Client &client = _clients.at(clientFd);

	char buffer[BUFFER_SIZE];

	while (true)
	{
		ssize_t bytes = ::recv(clientFd, buffer, BUFFER_SIZE, 0);
		if (bytes > 0)
		{
			client.getReadBuffer().append(buffer, static_cast<std::size_t>(bytes));

			std::string &readBuffer = client.getReadBuffer();
			std::size_t pos;

			// Process complete lines
			while ((pos = readBuffer.find("\r\n")) != std::string::npos)
			{
				std::string line = readBuffer.substr(0, pos);
				readBuffer.erase(0, pos + 2); // Remove processed line
				processLine(clientFd, line);
				if (_clients.find(clientFd) == _clients.end())
					return; // Client disconnected during processing
			}
		}
		else if (bytes == 0)
		{
			// Client disconnected
			disconnectClient(clientFd, "EOF");
			return;
		}
		else
		{
			if (errno == EWOULDBLOCK || errno == EAGAIN)
				break;

			std::perror("Recv failed");
			disconnectClient(clientFd, "Recv error");
			return;
		}
	}
	if (client.dataToWrite())
		_fds[index].events |= POLLOUT;
}

/*
** Handles client write events
*/
void Server::handleClientWrite(std::size_t index)
{
	int clientFd = _fds[index].fd;
	Client &client = _clients.at(clientFd);

	std::string &wb = client.getWriteBuffer();

	while (!wb.empty())
	{
		// std::cout << "Sending: " << wb.data() << std::endl;
		ssize_t sent = ::send(clientFd, wb.data(), wb.size(), 0);
		if (sent > 0)
		{
			wb.erase(0, static_cast<std::size_t>(sent));
		}
		else if (sent < 0)
		{
			if (errno == EWOULDBLOCK || errno == EAGAIN)
				break;
			std::perror("send");
			disconnectClient(clientFd, "Send error");
			return;
		}
	}
	if (!client.dataToWrite())
		_fds[index].events = POLLIN;
}

/////////////////////////////////////////////////////////////////////////////////////////
/// Command processing and message sending ///

/*
** Process a complete line received from a client
** Parses the command and dispatches to the appropriate handler
*/
void Server::processLine(int clientFd, std::string_view line)
{
	auto it = _clients.find(clientFd);
	if (it == _clients.end())
		return;
	Client &client = it->second;

	auto cmd = parseCommand(line);
	if (cmd.command.empty())
		return;

	std::string upper(cmd.command);
	for (char &c : upper)
		c = std::toupper(static_cast<unsigned char>(c));

	if (upper == "PASS")
		handlePASS(client, cmd.params);
	else if (upper == "NICK")
		handleNICK(client, cmd.params);
	else if (upper == "USER")
		handleUSER(client, cmd.params);
	else if (upper == "PING")
		handlePING(client, cmd.params);
	else if (upper == "QUIT")
		handleQUIT(client, cmd.params);
	else if (upper == "JOIN")
		handleJOIN(client, cmd.params);
	else if (upper == "MODE")
		handleMODE(client, cmd.params);
	else if (upper == "PRIVMSG")
		handlePRIVMSG(client, cmd.params);
	else
		std::cout << "Unknown command: " << upper << std::endl;
}

/*
** Parse a command line into command and parameters
** Returns a ParsedCommand struct, containing the command and a vector of parameters
*/
Server::ParsedCommand Server::parseCommand(std::string_view line)
{
	ParsedCommand result;

	// Trim leading spaces
	while (!line.empty() && std::isspace(static_cast<unsigned char>(line.front())))
		line.remove_prefix(1);

	// Extract command
	auto spacePos = line.find(' ');
	if (spacePos == std::string_view::npos)
	{
		result.command = line;
		return result;
	}
	result.command = line.substr(0, spacePos);
	line.remove_prefix(spacePos + 1);

	// Params extraction
	while (!line.empty())
	{
		// Trim leading spaces
		while (!line.empty() && std::isspace(static_cast<unsigned char>(line.front())))
			line.remove_prefix(1);
		if (line.empty())
			break;
		if (line.front() == ':')
		{
			line.remove_prefix(1);
			result.params.push_back(line);
			break;
		}
		auto nextSpace = line.find(' ');
		if (nextSpace == std::string_view::npos)
		{
			result.params.push_back(line);
			break;
		}
		else
		{
			result.params.push_back(line.substr(0, nextSpace));
			line.remove_prefix(nextSpace + 1);
		}
	}
	// No command was found
	return result;
}

void Server::sendTo(Client &client, const std::string &message)
{
	if (client.getFd() < 0)
		return; // invalid socket

	// Append to the client's write buffer
	client.queueMsg(message);

	// Ensure POLLOUT is set for this client in the server's poll fds
	for (std::size_t i = 0; i < _fds.size(); ++i)
	{
		if (_fds[i].fd == client.getFd())
		{
			_fds[i].events |= POLLOUT;
			break;
		}
	}
}

void Server::sendToChannel(Channel &channel, const std::string &message, Client *exclude)
{
	const std::unordered_set<Client *> &clients = channel.getMembers();
	for (Client *client : clients)
	{
		if (exclude && client == exclude)
			continue;
		if (!client)
			continue;
		sendTo(*client, message);
	}
}

// ...existing code...

/*
** Handle PASS command
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

bool Server::nickInUse(std::string_view nick) {
	if (_clients.empty())
		return false;
	for (auto it = _clients.begin(); it != _clients.end(); ++it) {
		if (it->second.getNickname() == nick)
			return true;
	}
	return false;
}

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
		sendNumeric(client, 433, newNick, "Nickname is already in use");
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
			oss << "!" << client.getUsername() << "@localhost"; // TODO: real host later

		oss << " NICK :" << newNick << "\r\n";
		std::string msg = oss.str();

		// Send to all connected clients (including the one who changed nick)
		for (std::unordered_map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
			sendTo(it->second, msg);
	}
}
//:ft_irc_server 433 * Zorma :Nickname is already in use


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

/*
** Handle QUIT command
*/
void Server::handleQUIT(Client &client, const std::vector<std::string_view> &params)
{
	std::string reason = "Client Quit";
	if (!params.empty())
		reason = std::string(params[0]);
	disconnectClient(client.getFd(), reason);
}

/*
** Handle JOIN command
** Joins client to a channel
*/
void Server::handleJOIN(Client &client, const std::vector<std::string_view> &params)
{
	if (params.empty())
	{
		sendNumeric(client, 461, "JOIN :Not enough parameters");
		return;
	}

	std::string _channelName(params[0]);
	// if (client.hasNickname())
	// 	std::cout << client.getNickname() << " joining channel: " << _channelName << std::endl;
	// else
	// 	std::cout << "Unknown client joining channel: " << _channelName << std::endl;
	auto it = _channels.find(_channelName);
	if (it != _channels.end())
	{
		if (it->second.getPasswordRequired()){
			if (params[1].empty() || it->second.getPassword() != params[1]) {
				sendNumeric(client, 475, _channelName + " :Password required/Invalid password");
				return ;
			}
		}
		try
		{
			it->second.addClient(&client);
		}
		catch (const std::runtime_error &e)
		{
			sendNumeric(client, 471, _channelName + " :Cannot join channel (possibly full)");
			return;
		}
	}
	else
	{
		Channel newChannel(_channelName);
		newChannel.addClient(&client);
		newChannel.addOperator(&client);
		_channels.emplace(_channelName, newChannel);
	}
}

/*
** Handle MODE command
** Sets the MODE for channel
*/
void Server::handleMODE(Client &client, const std::vector<std::string_view> &params)
{
	if (params.size() < 2) {
		sendNumeric(client, 461, " Not enough parameters"); // ERR_NEEDMOREPARAMS
		return;
	}
	if (nickInUse(params[0])) {
		return ;
	}
	std::string channelName(params[0]);
	auto it = _channels.find(channelName);
	if (it == _channels.end()) {
		sendNumeric(client, 403, channelName, " No such channel with" + std::string(params[0])); // ERR_NOSUCHCHANNEL
		return;
	}

	Channel &chan = it->second;

	if (!chan.isOperator(&client)) {
		sendNumeric(client, 482, channelName, " You're not channel operator"); // ERR_CHANOPRIVSNEEDED
		return;
	}

	// Mode params that we pass down to Channel (everything after channel name)
	std::vector<std::string_view> modeParams(params.begin() + 1, params.end());

	try {
		chan.setMode(modeParams); // modifies internal flags, password, limit, ops...
	} catch (const std::exception &e) {
		// Only the caller sees this error
		std::cout << "handling this->" << std::endl;
		sendNumeric(client, 472, e.what()); // ERR_UNKNOWNMODE or similar
		return;
	}

	// If we reached here, the mode change succeeded.
	// Now we must announce it to ALL channel members.

	std::ostringstream oss;
	// Prefix: :nick!user@host
	oss << ":" << client.getNickname();
	if (client.hasUsername())
		oss << "!" << client.getUsername() << "@localhost"; // TODO: real host later

	oss << " MODE " << channelName;

	// Re-attach the exact mode string and its params (e.g. "+ik key 10 nick")
	for (std::size_t i = 0; i < modeParams.size(); ++i) {
		oss << " " << modeParams[i];
	}
	oss << "\r\n";

	// Broadcast to everyone in channel (including the setter)
	sendToChannel(chan, oss.str(), nullptr);
}


/*
** Check if client has completed registration
** If so, mark as registered and send welcome messages
*/
void Server::maybeRegistered(Client &client)
{
	if (client.isRegistered())
		return;

	if (client.hasPassword() && client.hasNickname() && client.hasUsername())
	{
		client.setIsRegistered(true);
		sendNumeric(client, 001, "Welcome to the IRC Network, " + client.getNickname());
		sendNumeric(client, 002, "Your host is " + _serverName);
		sendNumeric(client, 003, "This server was created just now");
		sendNumeric(client, 004, _serverName + " ft_irc_server v1.0");
		_wasRegistered = true;
	}
}

/*
** Send a numeric reply to a client
** Formats and queues the message in the client's write buffer
*/
void Server::sendNumeric(Client &client, int numeric, const std::string_view msg)
{
	std::ostringstream oss;
	oss << ":" << _serverName << " " << std::setfill('0') << std::setw(3)
		<< numeric << " " << formatPrefix(client) << " " << msg << "\r\n";
	sendTo(client, oss.str());
}

/*
** Send a numeric reply to a client with channel context
*/
void Server::sendNumeric(Client &client, int numeric,
						 const std::string_view channel,
						 const std::string_view msg)
{
	std::ostringstream oss;
	oss << ":" << _serverName << " "
		<< std::setfill('0') << std::setw(3) << numeric << " "
		<< channel << " " << formatPrefix(client) << " :" << msg << "\r\n";
	sendTo(client, oss.str());
}

/*
** Format the prefix for messages from the server
*/
std::string Server::formatPrefix(const Client &client) const
{
	return client.getNickname();
}

/*
** Disconnect a client and clean up resources
*/
void Server::disconnectClient(int fd, std::string_view reason)
{
	auto it = _clients.find(fd);
	if (it == _clients.end())
		return; // Client not found

	Client &client = it->second;
	std::string nickname = client.hasNickname() ? client.getNickname() : "<unknown>";

	std::cout << "Disconnecting client [" << nickname << "] fd=" << fd
			  << " reason: " << reason << std::endl;

	// Close the socket and remove from clients map and fds vector
	::close(fd);

	// Remove from poll fds
	auto newEnd = std::remove_if(_fds.begin(), _fds.end(),
								 [fd](const pollfd &pfd)
								 { return pfd.fd == fd; });
	_fds.erase(newEnd, _fds.end());

	std::vector<std::string> emptyChannels;
	// Remove client from all channels
	for (auto &channelPair : _channels)
	{
		Channel &channel = channelPair.second;
		try
		{
			channel.removeClient(&client);
		}
		catch (const std::runtime_error &)
		{
			// Client was not in this channel, ignore
		}
		if (channel.isEmpty())
			emptyChannels.push_back(channel.getChannelName());
	}

	// Remove from clients map
	_clients.erase(it);

	// Clean up empty channels
	for (const std::string &chanName : emptyChannels)
		_channels.erase(chanName);

	std::cout << "Client " << nickname << " disconnected successfully." << std::endl;
}
