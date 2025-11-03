#include "Server.hpp"
#include <iostream>
#include <cstdlib>
#include <csignal>

static Server* g_server = NULL;

void signalHandler(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        std::cout << "\nShutting down server..." << std::endl;
        if (g_server) {
            g_server->stop();
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <port> <password>" << std::endl;
        return 1;
    }
    
    int port = std::atoi(argv[1]);
    if (port <= 0 || port > 65535) {
        std::cerr << "Error: Invalid port number" << std::endl;
        return 1;
    }
    
    std::string password = argv[2];
    if (password.empty()) {
        std::cerr << "Error: Password cannot be empty" << std::endl;
        return 1;
    }
    
    try {
        Server server(port, password);
        g_server = &server;
        
        signal(SIGINT, signalHandler);
        signal(SIGTERM, signalHandler);
        
        server.start();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
