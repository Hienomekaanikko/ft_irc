/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: msuokas <msuokas@student.hive.fi>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/03 14:12:16 by msuokas           #+#    #+#             */
/*   Updated: 2025/11/04 15:25:45 by msuokas          ###   ########.fr       */
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
                    if (clientSocket < 0) {
                        perror("accept");
                        continue;
                    }
                    std::cout << "New client connected: " << clientSocket << std::endl;
                    _clients.push_back(clientSocket);
                    pollfd newPoll{};
                    newPoll.fd = clientSocket;
                    newPoll.events = POLLIN;
                    _pollfds.push_back(newPoll);
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