#include "Server.hpp"

#include <iostream>
#include <cstring>
#include <string_view>
#include <cstdlib>
#include <cerrno>
#include <unistd.h>
#include <fcntl.h>
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
	std::cout << "Server is running..." << std::endl;
	mainLoop(); // Enter the main server loop
}

/// Member functions ///

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
	int flags = ::fcntl(fd, F_GETFL, 0);
	if (flags < 0)
		throw std::runtime_error("Get file descriptor flags failed: " + std::string(strerror(errno)));
	
	if (::fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0)
		throw std::runtime_error("Set non-blocking mode failed: " + std::string(strerror(errno)));
}

/*
** Main server loop
** Uses poll to monitor multiple file descriptors
** Handles new connections and client read/write events
*/
void Server::mainLoop()
{
	// Main server loop
	while (true)
	{
		int ready = ::poll(_fds.data(), _fds.size(), -1);
		if (ready < 0)
		{
			if (errno == EINTR)
				continue; // Interrupted by signal, retry
			throw std::runtime_error("Poll failed: " + std::string(strerror(errno)));
		}

		for (std::size_t i = 0; i < _fds.size() && ready > 0; ++i)
		{
			short revents = _fds[i].revents;
			if (revents == 0)
				continue;
			--ready;

			if (_fds[i].fd == _serverFd && (revents & POLLIN)){
				handleNewConnection();
			}
			else
			{
				if (revents & POLLIN) {
					std::cout << "Handling client read" << std::endl;
					handleClientRead(i);
				}
				if (revents & POLLOUT) {
					std::cout << "Handling client read" << std::endl;
					handleClientWrite(i);
				}
			}
		}
	}
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

			std::string& readBuffer = client.getReadBuffer();
			std::size_t pos;

			// Process complete lines
			while ((pos = readBuffer.find("\n")) != std::string::npos)
			{
				std::string_view line(readBuffer.data(), pos);
				readBuffer.erase(0, pos + 2); // Remove processed line
				processLine(clientFd, line);
			}
		}
		else if (bytes == 0)
		{
			// Client disconnected
			std::cout << "Client disconnected: fd = " << clientFd << std::endl;
			::close(clientFd);
			_clients.erase(clientFd);
			_fds[index] = _fds.back();
			_fds.pop_back();
			return;
		}
		else
		{
			if (errno == EWOULDBLOCK || errno == EAGAIN)
				break; // No more data to read
			
			std::perror("Recv failed");
			::close(clientFd);
			_clients.erase(clientFd);
			_fds[index] = _fds.back();
			_fds.pop_back();
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
			::close(clientFd);
			_clients.erase(clientFd);
			_fds[index] = _fds.back();
			_fds.pop_back();
			return;
		}
	}

	if (!client.dataToWrite())
		_fds[index].events = POLLIN;
}

/*
** Process a complete line received from a client
*/
void Server::processLine(int clientFd, std::string_view line)
{
	// Handle the received line (e.g., command parsing, response generation)
	std::cout << "Received command from client " << clientFd << ": " << line << std::endl;

	// For demonstration, echo the line back to the client
	Client &client = _clients.at(clientFd);
	client.queueMsg(std::string(line) + "\r\n");
}
