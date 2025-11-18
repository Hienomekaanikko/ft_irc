/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: clu <clu@student.hive.fi>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/05 16:38:05 by msuokas           #+#    #+#             */
/*   Updated: 2025/11/17 17:33:04 by clu              ###   ########.fr       */
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
	_limitSet = false;
	_currentUsers = 0;
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

const std::unordered_set<Client*>& Channel::getMembers() const {
		return _clients;
	}

bool Channel::isMember(Client *client) {
	auto it = _clients.find(client);
	if (it == _clients.end())
		return false;
	else
		return true;
}

void Channel::setInviteOnly() {
	_inviteOnly = true;
	std::cout << _channelName << ": set to invite only" << std::endl;
}

void Channel::unsetInviteOnly() {
	_inviteOnly = false;
	std::cout << _channelName << ": invite only disabled" << std::endl;
}

// Adds a 'client' to the _clients list.
void Channel::addClient(Client* client) 
{
	if (_limitSet) {
		if (_currentUsers == _userLimit) {
			throw std::runtime_error("The channel's userlimit is full");
		}
	}
	if (!_clients.insert(client).second)
		throw std::runtime_error("Client already in channel");
	_currentUsers++;
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

void Channel::unsetTopicProtection() {
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
	_limitSet = true;
}

void Channel::unsetUserlimit() {
	_userLimit = -1;
	std::cout << "User limit was unset" << std::endl;
}

// Handles the MODE command actions:
// i: Set/remove Invite-only channel
// t: Set/remove the restrictions of the TOPIC command to channel operators
// k: Set/remove the channel key (password)
// o: Give/take channel operator privilege
// l: Set/remove the user limit to channel
void Channel::setMode(const std::vector<std::string_view>& params)
{
	if (params.empty())
		throw std::runtime_error("Missing mode string");
		
	const std::string flags = "itkol";
	const std::string flagsWithParams = "kol";

	std::string_view modeString = params[0];
	size_t paramIndex = 1;  // start of additional parameters

	char action = 0; // '+' or '-'

	for (char c : modeString) {
		if (c == '+' || c == '-') {
			action = c;
			continue;
		}

		if (flags.find(c) == std::string::npos)
			throw std::runtime_error(std::string(1, c) + " :is unknown mode char to me");

		bool adding = (action == '+');
		std::string param;
		if (adding && flagsWithParams.find(c) != std::string::npos) {
			if (paramIndex >= params.size())
				throw std::runtime_error(std::string("MODE +") + c + " requires a parameter");
			param = std::string(params[paramIndex++]);
		}
		switch (c) {
			case 'i':
				adding ? setInviteOnly() : unsetInviteOnly();
				break;
			case 't':
				adding ? setTopicProtection() : unsetTopicProtection();
				break;
			case 'k':
				adding ? setPassword(param) : unsetPassword();
				break;
			case 'o':
				adding ? addOperator(findClientByNickname(param))
					   : removeOperator(findClientByNickname(param));
				break;
			case 'l':
				adding ? setUserlimit(param) : unsetUserlimit();
				break;
		}
		_modes[c] = adding;
	}
}

// Will be called from the setMode if the client permissions match
void Channel::setPassword(const std::string& password) {
	_password = password;
	std::cout << "Channel password was set to: " << password << std::endl;
}

void Channel::unsetPassword() {
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

int Channel::getCurrentUsers() const {
	return _currentUsers;
}