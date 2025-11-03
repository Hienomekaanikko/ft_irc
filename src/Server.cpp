#include "Server.hpp"
#include "Client.hpp"
#include "Channel.hpp"
#include "Utils.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <iostream>
#include <sstream>

Server::Server(int port, const std::string& password)
    : _serverSocket(-1), _port(port), _password(password), _running(false) {
}

Server::~Server() {
    stop();
    
    for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
        delete it->second;
    }
    _clients.clear();
    
    for (std::map<std::string, Channel*>::iterator it = _channels.begin(); it != _channels.end(); ++it) {
        delete it->second;
    }
    _channels.clear();
}

void Server::start() {
    _serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (_serverSocket < 0) {
        throw std::runtime_error("Failed to create socket");
    }
    
    int opt = 1;
    if (setsockopt(_serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        close(_serverSocket);
        throw std::runtime_error("Failed to set socket options");
    }
    
    fcntl(_serverSocket, F_SETFL, O_NONBLOCK);
    
    struct sockaddr_in serverAddr;
    std::memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(_port);
    
    if (bind(_serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        close(_serverSocket);
        throw std::runtime_error("Failed to bind socket");
    }
    
    if (listen(_serverSocket, 10) < 0) {
        close(_serverSocket);
        throw std::runtime_error("Failed to listen on socket");
    }
    
    struct pollfd serverPollfd;
    serverPollfd.fd = _serverSocket;
    serverPollfd.events = POLLIN;
    serverPollfd.revents = 0;
    _pollfds.push_back(serverPollfd);
    
    _running = true;
    std::cout << "Server started on port " << _port << std::endl;
    
    while (_running) {
        int ret = poll(&_pollfds[0], _pollfds.size(), -1);
        if (ret < 0) {
            break;
        }
        
        for (size_t i = 0; i < _pollfds.size(); i++) {
            if (_pollfds[i].revents & POLLIN) {
                if (_pollfds[i].fd == _serverSocket) {
                    acceptNewClient();
                } else {
                    handleClientData(_pollfds[i].fd);
                }
            }
        }
    }
}

void Server::stop() {
    _running = false;
    if (_serverSocket >= 0) {
        close(_serverSocket);
        _serverSocket = -1;
    }
}

void Server::acceptNewClient() {
    struct sockaddr_in clientAddr;
    socklen_t clientLen = sizeof(clientAddr);
    
    int clientSocket = accept(_serverSocket, (struct sockaddr*)&clientAddr, &clientLen);
    if (clientSocket < 0) {
        return;
    }
    
    fcntl(clientSocket, F_SETFL, O_NONBLOCK);
    
    std::string hostname = inet_ntoa(clientAddr.sin_addr);
    Client* client = new Client(clientSocket, hostname);
    _clients[clientSocket] = client;
    
    struct pollfd clientPollfd;
    clientPollfd.fd = clientSocket;
    clientPollfd.events = POLLIN;
    clientPollfd.revents = 0;
    _pollfds.push_back(clientPollfd);
    
    std::cout << "New client connected: " << clientSocket << std::endl;
}

void Server::handleClientData(int fd) {
    Client* client = getClient(fd);
    if (!client) {
        return;
    }
    
    char buffer[512];
    ssize_t bytesRead = recv(fd, buffer, sizeof(buffer) - 1, 0);
    
    if (bytesRead <= 0) {
        removeClient(fd);
        return;
    }
    
    buffer[bytesRead] = '\0';
    client->appendToBuffer(std::string(buffer));
    
    while (client->hasCompleteMessage()) {
        std::string message = client->extractMessage();
        if (!message.empty()) {
            processCommand(client, message);
        }
    }
}

void Server::removeClient(int fd) {
    Client* client = getClient(fd);
    if (client) {
        std::cout << "Client disconnected: " << fd << std::endl;
        
        const std::vector<Channel*>& channels = client->getChannels();
        std::vector<Channel*> channelsCopy = channels;
        
        for (size_t i = 0; i < channelsCopy.size(); i++) {
            channelsCopy[i]->removeClient(client);
            if (channelsCopy[i]->getClients().empty()) {
                removeChannel(channelsCopy[i]->getName());
            }
        }
        
        _clients.erase(fd);
        delete client;
    }
    
    for (size_t i = 0; i < _pollfds.size(); i++) {
        if (_pollfds[i].fd == fd) {
            _pollfds.erase(_pollfds.begin() + i);
            break;
        }
    }
    
    close(fd);
}

void Server::processCommand(Client* client, const std::string& message) {
    std::string trimmedMsg = trim(message);
    if (trimmedMsg.empty()) {
        return;
    }
    
    std::vector<std::string> tokens;
    std::string token;
    bool inTrailing = false;
    
    for (size_t i = 0; i < trimmedMsg.length(); i++) {
        if (inTrailing) {
            token += trimmedMsg[i];
        } else if (trimmedMsg[i] == ':' && (i == 0 || trimmedMsg[i - 1] == ' ')) {
            if (!token.empty()) {
                tokens.push_back(token);
                token.clear();
            }
            inTrailing = true;
        } else if (trimmedMsg[i] == ' ') {
            if (!token.empty()) {
                tokens.push_back(token);
                token.clear();
            }
        } else {
            token += trimmedMsg[i];
        }
    }
    
    if (!token.empty()) {
        tokens.push_back(token);
    }
    
    if (tokens.empty()) {
        return;
    }
    
    std::string command = toUpper(tokens[0]);
    std::vector<std::string> params(tokens.begin() + 1, tokens.end());
    
    if (command == "PASS") {
        cmdPass(client, params);
    } else if (command == "NICK") {
        cmdNick(client, params);
    } else if (command == "USER") {
        cmdUser(client, params);
    } else if (command == "PING") {
        cmdPing(client, params);
    } else if (command == "JOIN") {
        cmdJoin(client, params);
    } else if (command == "PART") {
        cmdPart(client, params);
    } else if (command == "PRIVMSG") {
        cmdPrivmsg(client, params);
    } else if (command == "QUIT") {
        cmdQuit(client, params);
    } else if (command == "KICK") {
        cmdKick(client, params);
    } else if (command == "INVITE") {
        cmdInvite(client, params);
    } else if (command == "TOPIC") {
        cmdTopic(client, params);
    } else if (command == "MODE") {
        cmdMode(client, params);
    }
}

Client* Server::getClient(int fd) {
    std::map<int, Client*>::iterator it = _clients.find(fd);
    if (it != _clients.end()) {
        return it->second;
    }
    return NULL;
}

Client* Server::getClientByNick(const std::string& nickname) {
    for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
        if (it->second->getNickname() == nickname) {
            return it->second;
        }
    }
    return NULL;
}

Channel* Server::getChannel(const std::string& name) {
    std::map<std::string, Channel*>::iterator it = _channels.find(name);
    if (it != _channels.end()) {
        return it->second;
    }
    return NULL;
}

Channel* Server::createChannel(const std::string& name, Client* creator) {
    Channel* channel = new Channel(name, creator);
    _channels[name] = channel;
    return channel;
}

void Server::removeChannel(const std::string& name) {
    std::map<std::string, Channel*>::iterator it = _channels.find(name);
    if (it != _channels.end()) {
        delete it->second;
        _channels.erase(it);
    }
}

const std::string& Server::getPassword() const {
    return _password;
}
