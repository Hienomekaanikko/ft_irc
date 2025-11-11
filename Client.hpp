/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: msuokas <msuokas@student.hive.fi>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/04 11:06:40 by msuokas           #+#    #+#             */
/*   Updated: 2025/11/10 11:41:09 by msuokas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <netinet/in.h>
#include <string>
#include <iostream>
#include <vector>

enum clientState {
    OPERATOR,
    WAITING_USERNAME,
    WAITING_NICKNAME,
    READY
};

class Client {
    private:
        int _clientFd;
        std::string _msg;
        std::string _username;
        std::string _nickname;
        clientState _state;
        std::vector<std::string> _channels;
    public:
        Client(int _clientFd);
        void setClientFd(int fd);
        void setUsername(std::string& username);
        void setNickname(std::string& nickname);
        void setMsg(char msg[]);
        void joinChannel(std::string& channelName);

        std::string getUsername() const;
        std::string getNickname() const;
        std::string getMsg() const;
        clientState getState() const;
        void setState(clientState state);
        int getClientFd();
};
