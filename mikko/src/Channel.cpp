/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: msuokas <msuokas@student.hive.fi>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/05 16:38:05 by msuokas           #+#    #+#             */
/*   Updated: 2025/11/14 16:53:54 by msuokas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Channel.hpp"

// Creates the 'Channel' object
// Initializes all of these mode 'switches' to false:
// i: Set/remove Invite-only channel
// t: Set/remove the restrictions of the TOPIC command to channel operators
// k: Set/remove the channel key (password)
// o: Give/take channel operator privilege
// l: Set/remove the user limit to channel
Channel::Channel(const std::string& name) : _channelName(name)
{
    std::string mode_list = "itkol";
    for (char mode : mode_list) 
    {
        _modes[mode] = false;
    }

    std::cout << "Modes at the construction: " << std::endl;
    for (const auto& [key, value] : _modes) {
        std::cout << key << " = " << value << "\n";
    }
}

void Channel::addOperator(Client *client) {
    if (client == nullptr)
        std::cerr << "Such client does not exist" << std::endl;
    else if (!_operators.insert(client).second)
        std::cerr << "Client already an operator" << std::endl;
    else
        std::cout << getChannelName() << ": " << client->getNickname() << " was given operator rights" << std::endl;
}

void Channel::removeOperator(Client *client) {
    if (client == nullptr) {
        std::cerr << "Such client does not exist" << std::endl;
        return;
    }
    auto it = _operators.find(client);
    if (it == _operators.end()) {
        std::cerr << "Client is not an operator" << std::endl;
    } else {
        _operators.erase(it);
        std::cout << getChannelName() << ": " << client->getNickname() 
                  << " had operator rights removed" << std::endl;
    }
}

void Channel::setInviteOnly() {
    _inviteOnly = true;
    std::cout << _channelName << ": set to invite only" << std::endl;
}

void Channel::disableInviteOnly() {
    _inviteOnly = false;
    std::cout << _channelName << ": invite only disabled" << std::endl;
}

// Adds a 'client' to the _clients list.
void Channel::addClient(Client* client) 
{
    if (!_clients.insert(client).second)
        throw std::runtime_error("Client already in channel");
}

// Removes a 'client' to the _clients list.
void Channel::removeClient(Client* client) 
{
    if (_clients.erase(client) == 0)
        throw std::runtime_error("Client not found in channel");
}

void Channel::setTopicProtection() {
    std::cout << _channelName << ": topic-protection enabled" << std::endl;
}

void Channel::disableTopicProtection() {
    std::cout << _channelName << ": topic-protection disabled" << std::endl;
}

// Returns boolean about the operator status of 'client'
bool Channel::isOperator(Client* client) const 
{
    return _operators.find(client) != _operators.end();
}

void Channel::setUserlimit(const std::string limit) {
    long long n;
    try {
        n = stoll(limit);
    } catch (std::exception &e) {
        std::cerr << "Invalid user limit" << std::endl;
        return ;
    }
    if (n > std::numeric_limits<int>::max() || n < std::numeric_limits<int>::min())
        throw "Invalid value";
    int val = static_cast<int>(n);
    _userLimit = val;
    std::cout << "User limit was set to " << limit << std::endl;
}

// Handles the MODE command actions:
// i: Set/remove Invite-only channel
// t: Set/remove the restrictions of the TOPIC command to channel operators
// k: Set/remove the channel key (password)
// o: Give/take channel operator privilege
// l: Set/remove the user limit to channel

void Channel::setMode(const std::vector<std::string_view> &params) 
{
    bool add = false;
    bool flags_set = false;
    std::vector<char> requiresParams;
    std::string flags = "itkol";
    
    size_t i = 0;
    for (; i < params.size(); ++i) {
        std::string_view string = params[i];
        if (string[0] != '+' && string[0] != '-')
            break;
        {
            if (string[0] == '+')
                add = true;
            string.remove_prefix(1);
            for (auto it = string.begin(); it != string.end(); ++it) {
                auto pos = find(flags.begin(), flags.end(), *it);
                if (pos == flags.end()) {
                    char invalidMode = *it;
                    throw std::runtime_error(std::string(1, invalidMode) + " :is unknown mode char to me");
                }
                else {
                    if (add) {
                        if (*it == 'k' || *it == 'o' || *it == 'l')
                            requiresParams.push_back(*it);
                        else if (*it == 'i')
                            setInviteOnly();
                        else if (*it == 't')
                            setTopicProtection();
                        _modes[*it] = true;
                    }
                    else {
                        if (*it == 'k')
                            removePassword();
                        _modes[*it] = false;
                    }
                }
            }
            add = false;
        }
        flags_set = true;
    }
    if (flags_set == false) {
        char invalidMode = params[0][0];
        throw std::runtime_error(std::string(1, invalidMode) + " :is unknown mode char to me");
    }
    if (!requiresParams.empty()) {
        for (size_t j = 0; j < requiresParams.size() && i < params.size(); ++j, ++i) {
            char mode = requiresParams[j];
            std::string param(params[i]); 
            switch (mode) {
                case 'k':
                    setPassword(param);
                    break;
                case 'o':
                    addOperator(findClientByNickname(param));
                    break;
                case 'l':
                    setUserlimit(param);
                    break;
            }
        }
    }
    // std::cout << "Modes after setting up: " << std::endl;
    // for (const auto& [key, value] : _modes) {
    //     std::cout << key << " = " << value << "\n";
    // }
}

// Will be called from the setMode if the client permissions match
void Channel::setPassword(const std::string& password) {
    _password = password;
    std::cout << "Channel password was set to: " << password << std::endl;
}

void Channel::removePassword() {
    _password = "";
    std::cout << "Channel password was disabled" << std::endl;
}

// Returns the channel topic
const std::string& Channel::getTopic() const { return _topic; }

// This handles the setting of cmd TOPIC when its called. 
// If MODE t is true, then only moderators can change the channel topic.
// otherwise anyone can change it.
void Channel::setTopic(const std::string& topic) { _topic = topic; }

// Returns the name of the channel
const std::string& Channel::getChannelName() const { return _channelName; }

// Check empty channel
bool Channel::isEmpty() const { return _clients.empty(); }

// !!May not be useful!!: We can look for clients inside the channel by nickname
Client* Channel::findClientByNickname(const std::string& name) const
{
    for (auto* c : _clients) {
        if (c->getNickname() == name)
            return c;
    }
    return nullptr;
}

