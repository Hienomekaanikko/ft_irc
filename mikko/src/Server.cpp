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
	std::vector<int>fds;
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
				continue; // Otherwise, continue polling
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
				if (revents & POLLOUT)
					handleClientWrite(i);
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

	setNonBlocking(_serverFd);

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

void Server::setNonBlocking(int fd)
{
	if (::fcntl(fd, F_SETFL, O_NONBLOCK) < 0)
		throw std::runtime_error("Set non-blocking mode failed: " + std::string(strerror(errno)));
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

	setNonBlocking(clientFd);

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
		_fds[index].events = POLLIN | POLLOUT;
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

/*
** Handle NICK command
** Sets the client's nickname
*/
void Server::handleNICK(Client &client, const std::vector<std::string_view> &params)
{
	if (params.empty())
	{
		sendNumeric(client, 431, "No nickname given");
		return;
	}
	client.setNickname(std::string(params[0]));
	maybeRegistered(client);
}

/*
** Handle MODE command
** Sets the MODE for channel
*/
void Server::handleMODE(Client &client, const std::vector<std::string_view> &params)
{
	if (params.size() < 2) {
		sendNumeric(client, 461, "MODE :Not enough parameters");
		return;
	}
	std::string	channel(params[0]);
	auto it = _channels.find(channel);
	if (it == _channels.end()) {
		sendNumeric(client, 403, "MODE :No such channel");
	}
	else {
		if (it->second.isOperator(&client)){
			try {
				it->second.setMode(std::vector<std::string_view>(params.begin() + 1, params.end()));
			} catch (std::exception &e) {
				sendNumeric(client, 472, e.what());
			}
		}
		else
			sendNumeric(client, 482, "MODE :You're not channel operator");
	}
}


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
	client.queueMsg(pongMsg);
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
	if (client.hasNickname())
		std::cout << client.getNickname() << " joining channel: " << _channelName << std::endl;
	else
		std::cout << "Unknown client joining channel: " << _channelName << std::endl;
	
	auto it = _channels.find(_channelName);
	if (it != _channels.end())
		it->second.addClient(&client);
	else
	{
		Channel newChannel(_channelName);
		newChannel.addClient(&client);
		newChannel.addOperator(&client);
		_channels.emplace(_channelName, newChannel);
	}
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
		<< numeric << " " << formatPrefix(client) << " :" << msg << "\r\n";
	client.queueMsg(oss.str());
}
/*
** Format the prefix for messages from the server
*/
std::string Server::formatPrefix(const Client &client) const
{
	if (client.hasNickname())
		return client.getNickname();
	return "anonymous";
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

	::close(fd);

	// Clean up empty channels
	for (const std::string &chanName : emptyChannels)
		_channels.erase(chanName);

	std::cout << "Client " << nickname << " disconnected successfully." << std::endl;
}
