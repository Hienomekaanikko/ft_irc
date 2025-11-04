/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: msuokas <msuokas@student.hive.fi>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/03 14:12:16 by msuokas           #+#    #+#             */
/*   Updated: 2025/11/04 10:12:21 by msuokas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

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
    
    while (1) {
        listen(serverSocket, 5);
        int clientSocket = accept(serverSocket, nullptr, nullptr);
        if (clientSocket < 0)
            continue ;
        char buffer[1024] = { 0 };
        while (1) { 
            recv(clientSocket, buffer, sizeof(buffer), 0);
            std::cout << "Message from client: " << buffer << std::endl;
        }
    }
    close(serverSocket);
}
