/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: msuokas <msuokas@student.hive.fi>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/04 11:06:40 by msuokas           #+#    #+#             */
/*   Updated: 2025/11/05 11:20:06 by msuokas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <netinet/in.h>
#include <string>

enum clientState {
    WAITING_USERNAME,
    WAITING_NICKNAME,
    READY
};

class Client {
    private:
        int _clientFd;
        std::string _username;
        std::string _nickname;
        clientState _state;
    public:
        Client(int _clientFd);
        void setClientFd(int fd);
        void setUsername(std::string& username);
        void setNickname(std::string& nickname);

        std::string getUsername() const;
        std::string getNickname() const;
        clientState getState() const;
        void setState(clientState state);
        int getClientFd();
};
