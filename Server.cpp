/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: msuokas <msuokas@student.hive.fi>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/03 14:12:16 by msuokas           #+#    #+#             */
/*   Updated: 2025/11/05 10:21:08 by msuokas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "Client.hpp"

void Server::setServerData() {
    _serverData.sin_family = AF_INET;
    _serverData.sin_port = htons(_port);
    _serverData.sin_addr.s_addr = INADDR_ANY;
}

sockaddr_in Server::getServerData() {
    return _serverData;
}

void setUserData(int clientSocket, Client newClient) {
    char buffer[256];

    std::string prompt = "Set username: ";
    send(clientSocket, prompt.c_str(), prompt.size(), 0);

    memset(buffer, 0, sizeof(buffer));
    ssize_t bytes = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    if (bytes <= 0) {
        std::cerr << "Didn't receive username" << "\n";
    }
    std::string username(buffer);
    newClient.setUsername(username);

    prompt = "Set nickname: ";
    send(clientSocket, prompt.c_str(), prompt.size(), 0);
    
    memset(buffer, 0, sizeof(buffer));
    bytes = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    if (bytes <= 0) {
        std::cerr << "Didnt receive nickname" << "\n";
    }
    std::string nickname(buffer);
    newClient.setNickname(nickname);
}

Server::Server(const int port, const std::string password): _port(port), _password(password){
    setServerData();
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
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
                    if (clientSocket < 0)
                        continue;
                    std::cout << "New client connected: " << clientSocket << std::endl;
                    Client newClient(clientSocket);
                    _clients.push_back(newClient);
                    pollfd newPoll{};
                    newPoll.fd = clientSocket;
                    newPoll.events = POLLIN;
                    _pollfds.push_back(newPoll);
                    setUserData(clientSocket, newClient);
                }
                else if (pfd.revents & POLLIN) {
                    std::cout << "Data is available on fd " << pfd.fd << "!\n";
                    char buf[100] = { 0 };
                    recv(pfd.fd, buf, sizeof(buf), 0);
                    std::cout << "Message from client: " << buf << std::endl;
                    for (auto& client: _clients) {
                        if (client.getClientFd() != pfd.fd)
                            send(client.getClientFd(), buf, sizeof(buf), 0);
                    }
                }
            }
        }
    }
    close(serverSocket);
}