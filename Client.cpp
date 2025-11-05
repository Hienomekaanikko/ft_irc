/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: msuokas <msuokas@student.hive.fi>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/04 09:53:37 by msuokas           #+#    #+#             */
/*   Updated: 2025/11/05 09:54:22 by msuokas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

Client::Client(int fd): _clientFd(fd) {}

void Client::setUsername(std::string& username) {
    _username = username;
}

void Client::setNickname(std::string& nickname) {
    _nickname = nickname;
}

void Client::setClientFd(const int fd) {
    _clientFd = fd;
}

int Client::getClientFd() {
    return _clientFd;
}