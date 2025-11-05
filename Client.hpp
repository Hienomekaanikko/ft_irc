/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: msuokas <msuokas@student.hive.fi>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/04 11:06:40 by msuokas           #+#    #+#             */
/*   Updated: 2025/11/05 15:57:14 by msuokas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <netinet/in.h>
#include <string>
#include <iostream>

enum clientState {
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
    public:
        Client(int _clientFd);
        void setClientFd(int fd);
        void setUsername(std::string& username);
        void setNickname(std::string& nickname);
        void setMsg(char msg[]);

        std::string getUsername() const;
        std::string getNickname() const;
        std::string getMsg() const;
        clientState getState() const;
        void setState(clientState state);
        int getClientFd();
};
