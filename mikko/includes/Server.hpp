#pragma once

#include "Client.hpp"
#include "Channel.hpp"

#include <vector>
#include <string_view>
#include <unordered_map>
#include <cstddef>
#include <poll.h>
#include <netinet/in.h>

class Server {
public:
	Server(int port, const std::string &password);
	~Server();

	// Disabling copy constructor and assignment operator, 1 server only
	Server(const Server &other) = delete;
	Server &operator=(const Server &other) = delete;

	void run();

private:
	static const int 				BUFFER_SIZE = 1024;
	int 							_port;
	std::string 					_password;
	std::string 					_serverName{"ft_irc_server"};
	int 							_serverFd{-1}; 	// Listening socket file descriptor
	struct sockaddr_in 				_address{};
	socklen_t 						_addrLen;
	std::vector<pollfd> 			_fds;	  		// index 0 = server
	std::unordered_map<int, Client> _clients;  		// fd -> Client
	std::unordered_map<std::string, Channel> _channels;

	// Main server functions
	void initSocket();
	void setNonBlocking(int fd);
	void mainLoop();

	// Event handlers
	void handleNewConnection();
	void handleClientRead(std::size_t index);
	void handleClientWrite(std::size_t index);
	
	// Command processing
	void processLine(int clientFd, std::string_view line);

	struct ParsedCommand {
		std::string_view command;
		std::vector<std::string_view> params;
	};
	ParsedCommand parseCommand(std::string_view line);

	void handlePASS(Client &client, const std::vector<std::string_view> &params);
	void handleNICK(Client &client, const std::vector<std::string_view> &params);
	void handleUSER(Client &client, const std::vector<std::string_view> &params);
	void handlePING(Client &client, const std::vector<std::string_view> &params);
	void handleQUIT(Client &client, const std::vector<std::string_view> &params);
	void handleJOIN(Client &client, const std::vector<std::string_view> &params);

	void maybeRegistered(Client &client);

	// Message sending
	void sendNumeric(Client &client, int numeric, const std::string_view msg);
	std::string formatPrefix(const Client &client) const;

	// Client disconnection, cleanup
	void disconnectClient(int fd, std::string_view reason);
};
