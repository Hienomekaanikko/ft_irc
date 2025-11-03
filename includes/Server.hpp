#pragma once

#include "Client.hpp"
#include "Parser.hpp"
#include "Utils.hpp"

#include <vector>
#include <poll.h>
#include <unordered_map>
#include <string>

class Server
{
	public:
		Server(const std::string& port, const std::string& password);
		~Server();

		void run();

	private:
		int _listenFd;
		std::string _password;
		std::vector<struct pollfd> _pollFds;
		std::unordered_map<int, Client> _clients;
};



