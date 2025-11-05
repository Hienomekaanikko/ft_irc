/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: msuokas <msuokas@student.hive.fi>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/04 09:53:37 by msuokas           #+#    #+#             */
/*   Updated: 2025/11/05 11:19:52 by msuokas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

Client::Client(int fd): _clientFd(fd) {
    _state = WAITING_USERNAME;
}

void Client::setUsername(std::string& username) {
    _username = username;
}

void Client::setNickname(std::string& nickname) {
    _nickname = nickname;
}

void Client::setClientFd(const int fd) {
    _clientFd = fd;
}

std::string Client::getNickname() const {
    return _nickname;
}

std::string Client::getUsername() const {
    return _username;
}

int Client::getClientFd() {
    return _clientFd;
}

clientState Client::getState() const {
    return _state;
}

void Client::setState(clientState state) {
    _state = state;
}