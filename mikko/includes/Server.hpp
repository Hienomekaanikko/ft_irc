#pragma once

#include "Client.hpp"

#include <vector>
#include <string_view>
#include <unordered_map>
#include <cstddef>
#include <poll.h>
#include <netinet/in.h>

class Server
{
public:
	Server(int port, const std::string &password);
	~Server();

	// Disabling copy constructor and assignment operator, 1 server only
	Server(const Server &other) = delete;
	Server &operator=(const Server &other) = delete;


	void run();

private:
	static const int BUFFER_SIZE = 1024;

	void initSocket();
	void setNonBlocking(int fd);
	void mainLoop();

	void handleNewConnection();
	void handleClientRead(std::size_t index);
	void handleClientWrite(std::size_t index);
	void processLine(int clientFd, std::string_view line);

	int 							_port;
	std::string 					_password;
	int 							_serverFd{-1}; 	// Listening socket file descriptor
	struct sockaddr_in 				_address{};
	socklen_t 						_addrLen;
	std::vector<pollfd> 			_fds;	  		// index 0 = server
	std::unordered_map<int, Client> _clients;  		// fd -> Client
};
