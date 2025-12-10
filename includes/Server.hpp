#pragma once

#include "Client.hpp"
#include "Channel.hpp"
#include <vector>
#include <string_view>
#include <unordered_map>
#include <cstddef>
#include <poll.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

struct errs {
	int num;
	std::string msg;
};

class Server {
public:
	Server(int port, const std::string &password);
	~Server();

	Server(const Server &other) = delete;
	Server &operator=(const Server &other) = delete;

	void run();
	void shutdown();

	void handlePASS(Client &client, const std::vector<std::string_view> &params);
	void handleNICK(Client &client, const std::vector<std::string_view> &params);
	void handleUSER(Client &client, const std::vector<std::string_view> &params);
	void handlePING(Client &client, const std::vector<std::string_view> &params);
	void handleQUIT(Client &client, const std::vector<std::string_view> &params);
	void handleJOIN(Client &client, const std::vector<std::string_view> &params);
	void handleMODE(Client &client, const std::vector<std::string_view> &params);
	void handlePART(Client &client, const std::vector<std::string_view> &params);
	void handlePRIVMSG(Client &client, const std::vector<std::string_view> &params);
	void handleTOPIC(Client &client, const std::vector<std::string_view> &params);
	void handleCAP(Client &client, const std::vector<std::string_view> &params);
	void handleKICK(Client &client, const std::vector<std::string_view> &params);
	void handleINVITE(Client &client, const std::vector<std::string_view> &params);
	
private:
	static const int 				BUFFER_SIZE = 1024;
	int 							_port;
	int								_channelCount;
	std::string 					_password;
	std::string 					_serverName{"ft_irc_server"};
	int 							_serverFd{-1};
	struct sockaddr_in 				_address{};
	socklen_t 						_addrLen;
	std::vector<pollfd> 			_fds;
	std::unordered_map<int, Client> _clients;
	std::unordered_map<std::string, Channel> _channels;
	bool							_running{false};
	bool							_wasRegistered{false};
	
	// Main server functions
	void initSocket();
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
	
	void sendTo(Client &client, const std::string &message);
	void sendToChannel(Channel &channel, const std::string &message, Client *exclude);

	void maybeRegistered(Client &client);
	Client* findClientByNick(const std::string &nick);
	bool nickInUse(std::string_view nick);

	// Message sending
	void sendNumeric(Client &client, int numeric, const std::string_view msg);
	void sendNumeric(Client &client, int numeric, const std::string_view channel, const std::string_view msg);
	void clientErr(std::string msg, int fd);
	std::string getClientHost(int clientFd);
	std::string formatPrefix(const Client &client);

	// Client disconnection, cleanup
	void disconnectClient(int fd, std::string_view reason);
};
