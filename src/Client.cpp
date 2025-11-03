#include "Client.hpp"
#include "Channel.hpp"
#include <unistd.h>
#include <sys/socket.h>
#include <algorithm>

Client::Client(int fd, const std::string& hostname)
    : _fd(fd), _hostname(hostname), _authenticated(false), _registered(false) {
}

Client::~Client() {
}

int Client::getFd() const {
    return _fd;
}

const std::string& Client::getNickname() const {
    return _nickname;
}

const std::string& Client::getUsername() const {
    return _username;
}

const std::string& Client::getRealname() const {
    return _realname;
}

const std::string& Client::getHostname() const {
    return _hostname;
}

std::string Client::getPrefix() const {
    return _nickname + "!" + _username + "@" + _hostname;
}

bool Client::isAuthenticated() const {
    return _authenticated;
}

bool Client::isRegistered() const {
    return _registered;
}

void Client::setNickname(const std::string& nickname) {
    _nickname = nickname;
}

void Client::setUsername(const std::string& username) {
    _username = username;
}

void Client::setRealname(const std::string& realname) {
    _realname = realname;
}

void Client::setAuthenticated(bool auth) {
    _authenticated = auth;
}

void Client::setRegistered(bool reg) {
    _registered = reg;
}

void Client::appendToBuffer(const std::string& data) {
    _buffer += data;
}

std::string Client::extractMessage() {
    size_t pos = _buffer.find("\r\n");
    if (pos == std::string::npos) {
        pos = _buffer.find("\n");
    }
    
    if (pos != std::string::npos) {
        std::string message = _buffer.substr(0, pos);
        _buffer.erase(0, pos + (_buffer[pos] == '\r' ? 2 : 1));
        return message;
    }
    
    return "";
}

bool Client::hasCompleteMessage() const {
    return _buffer.find("\r\n") != std::string::npos || _buffer.find("\n") != std::string::npos;
}

void Client::joinChannel(Channel* channel) {
    _channels.push_back(channel);
}

void Client::leaveChannel(Channel* channel) {
    std::vector<Channel*>::iterator it = std::find(_channels.begin(), _channels.end(), channel);
    if (it != _channels.end()) {
        _channels.erase(it);
    }
}

const std::vector<Channel*>& Client::getChannels() const {
    return _channels;
}

void Client::sendMessage(const std::string& message) {
    std::string fullMessage = message;
    if (fullMessage.substr(fullMessage.length() - 2) != "\r\n") {
        fullMessage += "\r\n";
    }
    send(_fd, fullMessage.c_str(), fullMessage.length(), 0);
}
