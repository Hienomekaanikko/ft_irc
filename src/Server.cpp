#include "Server.hpp"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstring>
#include <cstdlib>
#include <cerrno>
#include <unistd.h>
#include <fcntl.h>
#include <algorithm>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

/// Constructor ///
Server::Server(int port, const std::string &password)
	: _port(port), _password(password), _addrLen(sizeof(_address))
{
	_channelCount = 0;
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

	std::vector<int> fds;
	fds.reserve(_clients.size());
	for (std::unordered_map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
		fds.push_back(it->first);
	for (int fd : fds)
		disconnectClient(fd, "Server shutting down");
	if (_serverFd >= 0)
	{
		close(_serverFd);
		_serverFd = -1;
	}
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
					break;
				continue;
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
** Sets up the socket
** Set socket to non-blocking mode
** Set socket options to reuse address
** Bind the socket to the specified port
** Start listening for connections
*/
void Server::initSocket()
{
	_serverFd = socket(AF_INET, SOCK_STREAM, 0);
	if (_serverFd < 0)
		throw std::runtime_error("Socket creation failed: " + std::string(strerror(errno)));

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
	if (::fcntl(clientFd, F_SETFL, O_NONBLOCK) < 0)
		throw std::runtime_error("Set non-blocking mode failed: " + std::string(strerror(errno)));
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

			while ((pos = readBuffer.find("\r\n")) != std::string::npos)
			{
				std::string line = readBuffer.substr(0, pos);
				readBuffer.erase(0, pos + 2);
				processLine(clientFd, line);
				if (_clients.find(clientFd) == _clients.end()) {
					return;
				}
			}
		}
		else if (bytes == 0)
		{
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
		_fds[index].events &= ~POLLOUT;
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
 
	bool preRegAllowed = (upper == "PASS" ||
						  upper == "NICK" ||
						  upper == "USER" ||
						  upper == "CAP"  ||
						  upper == "QUIT" ||
						  upper == "PING");

	if (!client.isRegistered() && !preRegAllowed)
	{
		sendNumeric(client, 451, ":You have not registered");
		return;
	}
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
	else if (upper == "TOPIC")
		handleTOPIC(client, cmd.params);
	else if (upper == "KICK")
		handleKICK(client, cmd.params);
	else if (upper == "INVITE")
		handleINVITE(client, cmd.params);
	else if (upper == "MODE")
		handleMODE(client, cmd.params);
	else if (upper == "PRIVMSG")
		handlePRIVMSG(client, cmd.params);
	else if (upper == "PART")
		handlePART(client, cmd.params);
	else if (upper == "TOPIC")
		handleTOPIC(client, cmd.params);
	else if (upper == "CAP" || upper == "WHO" || upper == "WHOIS")
		return;
	else
		sendNumeric(client, 421, std::string(cmd.command), "Unknown command");
}

/*
** Parse a command line into command and parameters
** Returns a ParsedCommand struct, containing the command and a vector of parameters
*/
Server::ParsedCommand Server::parseCommand(std::string_view line)
{
	ParsedCommand result;

	while (!line.empty() && std::isspace(static_cast<unsigned char>(line.front())))
		line.remove_prefix(1);

	auto spacePos = line.find(' ');
	if (spacePos == std::string_view::npos)
	{
		result.command = line;
		return result;
	}
	result.command = line.substr(0, spacePos);
	line.remove_prefix(spacePos + 1);

	while (!line.empty())
	{
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
	return result;
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
		<< formatPrefix(client) << " " <<  channel << " " << msg << "\r\n";
	sendTo(client, oss.str());
}

/*
** Format the prefix for messages from the server
*/
std::string Server::formatPrefix(const Client &client)
{
	return client.getNickname();
}

/*
** Send a message to a client
*/
void Server::sendTo(Client &client, const std::string &message)
{
	if (client.getFd() < 0)
		return;
	client.queueMsg(message);
	for (std::size_t i = 0; i < _fds.size(); ++i)
	{
		if (_fds[i].fd == client.getFd())
		{
			_fds[i].events |= POLLOUT;
			break;
		}
	}
}

/*
** Send a message to a channel
*/
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
** Disconnect a client and clean up resources
*/
void Server::disconnectClient(int fd, std::string_view reason)
{
	auto it = _clients.find(fd);
	if (it == _clients.end())
		return;

	Client &client = it->second;
	std::string nickname = client.hasNickname() ? client.getNickname() : "<unknown>";

	std::cout << "Disconnecting client [" << nickname << "] fd=" << fd
			  << " reason: " << reason << std::endl;

	::close(fd);

	auto newEnd = std::remove_if(_fds.begin(), _fds.end(),
								 [fd](const pollfd &pfd)
								 { return pfd.fd == fd; });
	_fds.erase(newEnd, _fds.end());

	std::vector<std::string> emptyChannels;
	for (auto &channelPair : _channels)
	{
		Channel &channel = channelPair.second;
		try
		{
			channel.removeClient(client.getNickname());
		}
		catch (const errs &)
		{
		}
		if (channel.isEmpty())
			emptyChannels.push_back(channel.getChannelName());
	}

	_clients.erase(it);

	for (const std::string &chanName : emptyChannels)
		_channels.erase(chanName);
	std::cout << "Client " << nickname << " disconnected successfully." << std::endl;
}

/*
** Get the client's host name
** Returns "unknown" if failed
*/
std::string Server::getClientHost(int clientFd)
{
	struct sockaddr_storage addr;
	socklen_t addrLen = sizeof(addr);

	if (getpeername(clientFd, reinterpret_cast<struct sockaddr *>(&addr), &addrLen) < 0)
	{
		std::cerr << "getpeername failed: " << strerror(errno) << std::endl;
		return "unknown";
	}

	char host[NI_MAXHOST];
	char service[NI_MAXSERV];

	if (getnameinfo(reinterpret_cast<struct sockaddr *>(&addr), addrLen,
					host, sizeof(host), service, sizeof(service),
					NI_NUMERICSERV) == 0)
		return std::string(host); 
	else
	{
		std::cerr << "getnameinfo failed: " << strerror(errno) << std::endl;
		return "unknown";
	}
}
