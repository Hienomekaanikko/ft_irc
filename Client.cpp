/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: msuokas <msuokas@student.hive.fi>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/04 09:53:37 by msuokas           #+#    #+#             */
/*   Updated: 2025/11/04 10:05:46 by msuokas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include <iostream>
#include <vector>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

int main() {
    // creating socket
    int _fd = socket(AF_INET, SOCK_STREAM, 0);

    // specifying address
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(1024);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    // sending connection request
    connect(_fd, (struct sockaddr*)&serverAddress,
            sizeof(serverAddress));

    // sending data
    while (1) {
        std::string line;
        getline(std::cin, line);
        send(_fd, line.c_str(), line.size(), 0);
    }
    // closing socket
    close(_fd);
}
