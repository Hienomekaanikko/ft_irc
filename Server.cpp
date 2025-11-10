/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: msuokas <msuokas@student.hive.fi>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/03 14:12:16 by msuokas           #+#    #+#             */
/*   Updated: 2025/11/10 11:35:39 by msuokas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "Client.hpp"

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
                                    std::cout << "this is the message: " << msg;
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