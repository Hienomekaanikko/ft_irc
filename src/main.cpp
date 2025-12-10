#include "Server.hpp"

#include <cstdlib>
#include <iostream>
#include <string>
#include <csignal>

static void handleSignal(int signal);
static bool parsePort(const char *s, int &portOut);

/* Global Server pointer */
static Server* g_server = 0;

/* 
** Main
** Create server and run it
** If signal SIGINT is received, shutdown server
*/
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
		g_server = &server;
		std::signal(SIGINT, handleSignal);
		std::signal(SIGPIPE, SIG_IGN);
		server.run();
		g_server = 0;
	}
	catch (const std::exception &e)
	{
		std::cerr << "Fatal error: " << e.what() << '\n';
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

/* Parse port */
static bool parsePort(const char *s, int &portOut)
{
	char *end = nullptr;
	long p = std::strtol(s, &end, 10);
	if (*s == '\0' || *end != '\0' || p < 1 || p > 65535)
		return false;
	portOut = static_cast<int>(p);
	return true;
}

/* Signal handler SIGINT */
static void handleSignal(int signal)
{
	if (signal == SIGINT && g_server)
		g_server->shutdown();
}
