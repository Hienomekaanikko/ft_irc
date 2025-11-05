/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: msuokas <msuokas@student.hive.fi>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/04 11:06:40 by msuokas           #+#    #+#             */
/*   Updated: 2025/11/05 10:05:51 by msuokas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <netinet/in.h>
#include <string>

class Client {
    private:
        int _clientFd;
        std::string _username;
        std::string _nickname;
    public:
        Client(int _clientFd);
        void setClientFd(int fd);
        void setUsername(std::string& username);
        void setNickname(std::string& nickname);
        int getClientFd();

        //void sendMsg();
};
