/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: msuokas <msuokas@student.hive.fi>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/04 09:53:37 by msuokas           #+#    #+#             */
/*   Updated: 2025/11/10 11:29:51 by msuokas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

Client::Client(int fd): _clientFd(fd) {
    _state = WAITING_USERNAME;
}

void Client::setUsername(std::string& username) {
    while (!username.empty() && (username.back() == '\n' || username.back() == '\r'))
        username.pop_back();
    _username = username;
}

void Client::setNickname(std::string& nickname) {
    while (!nickname.empty() && (nickname.back() == '\n' || nickname.back() == '\r'))
        nickname.pop_back();
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

std::string Client::getMsg() const {
    return _msg;
}

void Client::setMsg(char msg[]) {
    std::string cleaned_msg = msg;
    while (!cleaned_msg.empty() && (cleaned_msg.back() == '\n' || cleaned_msg.back() == '\r'))
        cleaned_msg.pop_back();
    _msg = "<" + _username + ">" + ": " + cleaned_msg + "\n"; 
}

void Client::joinChannel(std::string& channelName) {
    this->_channels.push_back(channelName);
}