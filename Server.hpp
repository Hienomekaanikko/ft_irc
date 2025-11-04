/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: msuokas <msuokas@student.hive.fi>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/03 16:07:25 by msuokas           #+#    #+#             */
/*   Updated: 2025/11/04 12:57:16 by msuokas          ###   ########.fr       */
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

class Server {
    private:
        int _fd;
        int _port;
        std::string _password;
        std::vector<Client> _clients;
        std::vector<struct pollfd> _pollfds;
        sockaddr_in _serverData;
    public:
        Server(const int _Port, const std::string _password);
        void setServerData();
        sockaddr_in getServerData();
};
