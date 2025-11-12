#include <cstdlib>
#include <iostream>
#include <string>

#include "Server.hpp"

static bool parsePort(const char *s, int &portOut)
{
	char *end = nullptr;
	long p = std::strtol(s, &end, 10);
	if (*s == '\0' || *end != '\0' || p < 1 || p > 65535)
		return false;
	portOut = static_cast<int>(p);
	return true;
}

int main(int argc, char **argv)
{
	if (argc != 3)
	{
		std::cerr << "Usage: " << argv[0] << " <port> <password>\n";
		return EXIT_FAILURE;
	}

	int port;
	if (!parsePort(argv[1], port))
	{
		std::cerr << "Invalid port: " << argv[1] << '\n';
		return EXIT_FAILURE;
	}

	std::string password = argv[2];

	try
	{
		Server server(port, password);
		server.run();
	}
	catch (const std::exception &e)
	{
		std::cerr << "Fatal error: " << e.what() << '\n';
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
