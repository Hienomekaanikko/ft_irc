#include "Server.hpp"

#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cerrno>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>

// Constructor
Server::Server(int port, const std::string &password)
	: _port(port), _password(password), _addrLen(sizeof(_address))
{
	initSocket();
}