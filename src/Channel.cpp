#include "Channel.hpp"
#include "Client.hpp"
#include <algorithm>

Channel::Channel(const std::string& name, Client* creator)
    : _name(name), _inviteOnly(false), _topicRestricted(true), _userLimit(-1) {
    if (creator) {
        addClient(creator);
        addOperator(creator);
    }
}

Channel::~Channel() {
}

const std::string& Channel::getName() const {
    return _name;
}

const std::string& Channel::getTopic() const {
    return _topic;
}

void Channel::setTopic(const std::string& topic) {
    _topic = topic;
}

void Channel::addClient(Client* client) {
    _clients.push_back(client);
}

void Channel::removeClient(Client* client) {
    std::vector<Client*>::iterator it = std::find(_clients.begin(), _clients.end(), client);
    if (it != _clients.end()) {
        _clients.erase(it);
    }
    _operators.erase(client);
    _invited.erase(client);
}

bool Channel::hasClient(Client* client) const {
    return std::find(_clients.begin(), _clients.end(), client) != _clients.end();
}

const std::vector<Client*>& Channel::getClients() const {
    return _clients;
}

void Channel::addOperator(Client* client) {
    _operators.insert(client);
}

void Channel::removeOperator(Client* client) {
    _operators.erase(client);
}

bool Channel::isOperator(Client* client) const {
    return _operators.find(client) != _operators.end();
}

void Channel::addInvited(Client* client) {
    _invited.insert(client);
}

void Channel::removeInvited(Client* client) {
    _invited.erase(client);
}

bool Channel::isInvited(Client* client) const {
    return _invited.find(client) != _invited.end();
}

void Channel::setInviteOnly(bool inviteOnly) {
    _inviteOnly = inviteOnly;
}

bool Channel::isInviteOnly() const {
    return _inviteOnly;
}

void Channel::setTopicRestricted(bool restricted) {
    _topicRestricted = restricted;
}

bool Channel::isTopicRestricted() const {
    return _topicRestricted;
}

void Channel::setUserLimit(int limit) {
    _userLimit = limit;
}

int Channel::getUserLimit() const {
    return _userLimit;
}

void Channel::setKey(const std::string& key) {
    _key = key;
}

const std::string& Channel::getKey() const {
    return _key;
}

void Channel::broadcast(const std::string& message, Client* exclude) {
    for (size_t i = 0; i < _clients.size(); i++) {
        if (_clients[i] != exclude) {
            _clients[i]->sendMessage(message);
        }
    }
}
