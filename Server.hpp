/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: msuokas <msuokas@student.hive.fi>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/03 16:07:25 by msuokas           #+#    #+#             */
/*   Updated: 2025/11/10 16:12:56 by msuokas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include <vector>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include "Client.hpp"
#include <poll.h>
#include <fcntl.h>
#include "Channel.hpp"
#include <algorithm>

enum Commands {
    JOIN,
};

class Server {
    private:
        int _fd;
        int _port;
        std::string _password;
        std::vector<Client> _clients;
        std::vector<struct pollfd> _pollfds;
        std::vector<Channel> _channels;
        sockaddr_in _serverData;
    public:
        Server(const int _Port, const std::string _password);
        void addChannel(std::string& channelName);
        void joinHandler(std::string& channelName, Client& user);
        void setServerData();
        void msgHandler(Client& sender, std::string& msg);
        sockaddr_in getServerData();
        Client* findClientByFd(int fd);
};
