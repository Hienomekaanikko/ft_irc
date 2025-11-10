/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: msuokas <msuokas@student.hive.fi>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/03 14:12:16 by msuokas           #+#    #+#             */
/*   Updated: 2025/11/10 16:21:15 by msuokas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

void Server::addChannel(std::string& channelName) {
    _channels.push_back(channelName);
}

void Server::joinHandler(std::string& channelName, Client& user) {
    std::cout << channelName << std::endl;
    if (!_channels.empty()) {
        for (auto& i : _channels) {
            if (i.getChannelName() == channelName)
            {
                user.joinChannel(channelName);
                return;
            }
        }
    }
    else {
        addChannel(channelName);
        user.joinChannel(channelName);   
    }
}

void Server::setServerData() {
    _serverData.sin_family = AF_INET;
    _serverData.sin_port = htons(_port);
    _serverData.sin_addr.s_addr = INADDR_ANY;
}

Client* Server::findClientByFd(int fd) {
    for (auto& client : _clients) {
        if (client.getClientFd() == fd)
            return &client;
    }
    return nullptr;
}

sockaddr_in Server::getServerData() {
    return _serverData;
}

std::string getHashtag(const std::string& msg) {
    size_t pos = msg.find('#');
    if (pos == std::string::npos)
        return "";
    size_t end = msg.find(' ', pos);
    if (end == std::string::npos)
        end = msg.size();
        
    return msg.substr(pos, end - pos);
}

void Server::msgHandler(Client& user, std::string& msg) {
    std::cout << "Inside msgHandler with: " << user.getMsg() << std::endl;

    std::vector<std::string> cmds = {"/join", "/msg", "/kick", "/invite", "/topic", "/mode"};
    std::string cmd;

    if (!msg.empty() && msg[0] == '/') {
        for (const auto& i : cmds) {
            if (msg.rfind(i, 0) == 0) {
                cmd = i;
                break;
            }
        }
    }
    std::string hash = getHashtag(msg);
    if (!cmd.empty() && !hash.empty()) {
        if (cmd == "/join"){
            joinHandler(hash, user);
        }
    } else {
        std::cout << "Regular message or unknown command." << std::endl;
    }
}

Server::Server(const int port, const std::string password): _port(port), _password(password){
    setServerData();
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    fcntl(serverSocket, F_SETFL, O_NONBLOCK);
    bind(serverSocket, (struct sockaddr*)&_serverData, sizeof(_serverData));
    listen(serverSocket, 5);
    pollfd serverPoll{};
    serverPoll.fd = serverSocket;
    serverPoll.events = POLLIN;
    _pollfds.push_back(serverPoll);
    while (1) {
        int timeout = 5000;
        int ret = poll(_pollfds.data(), _pollfds.size(), timeout);
        if (ret > 0) {
            for (auto& pfd : _pollfds) {
                 if (pfd.fd == serverSocket && (pfd.revents & POLLIN)) {
                    int clientSocket = accept(serverSocket, nullptr, nullptr);
                    fcntl(clientSocket, F_SETFL, O_NONBLOCK);
                    if (clientSocket < 0)
                        continue;
                    std::cout << "New client connected: " << clientSocket << std::endl;
                    Client newClient(clientSocket);
                    _clients.push_back(newClient);
                    pollfd newPoll{};
                    newPoll.fd = clientSocket;
                    newPoll.events = POLLIN;
                    _pollfds.push_back(newPoll);
                    send(clientSocket, "Set username: ", sizeof("Set username: "), 0);
                }
                else if (pfd.revents & POLLIN && pfd.fd) {
                    char buf[100];
                    int len = recv(pfd.fd, buf, sizeof(buf), 0);
                    Client *userPtr;
                    if (len > 0)
                    {
                        buf[len] = '\0';
                        userPtr = findClientByFd(pfd.fd);
                        userPtr->setMsg(buf);
                        std::string msg = userPtr->getMsg();
                        msgHandler(*userPtr, msg);
                        if (userPtr && userPtr->getState() == WAITING_USERNAME) {
                            std::string username(buf);
                            userPtr->setUsername(username);
                            userPtr->setState(WAITING_NICKNAME);
                            send(pfd.fd, "Set nickname: ", sizeof("Set nickname: "), 0);
                        }
                        else if (userPtr && userPtr->getState() == WAITING_NICKNAME) {
                            std::string nickname(buf);
                            userPtr->setNickname(nickname);
                            userPtr->setState(READY);
                            std::string intro = "Welcome to the server!\n";
                            send(pfd.fd, intro.c_str(), intro.size(), 0);
                        }
                        else {
                            for (auto& client: _clients) {
                                if (client.getClientFd() != pfd.fd) {
                                    std::string msg = userPtr->getMsg().c_str();
                                    send(client.getClientFd(), msg.c_str(), msg.size(), 0);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    close(serverSocket);
}