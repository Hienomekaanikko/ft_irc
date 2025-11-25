/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: msuokas <msuokas@student.hive.fi>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/20 15:52:45 by msuokas           #+#    #+#             */
/*   Updated: 2025/11/25 16:53:42 by msuokas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Channel.hpp"
#include "Server.hpp"

// Channel handling

Channel::Channel(const std::string& name) : _channelName(name)
{
	std::string mode_list = "itkol";
	for (char mode : mode_list) 
	{
		_modes[mode] = false;
	}
	_limitSet = false;
	_currentUsers = 0;
	_passwordRequired = false;
	_limitSet = false;
	_inviteOnly = false;
}

const std::string& Channel::getChannelName() const { return _channelName; }

bool Channel::isEmpty() const { return _clients.empty(); }

int Channel::getCurrentUsers() const { return _currentUsers; }

// Client handling

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

void Channel::addClient(Client* client) 
{
	_clients.insert(client);
	_currentUsers++;
}

void Channel::removeClient(Client* client) 
{
	if (_clients.erase(client) == 0)
		throw std::runtime_error("Client not found in channel");
	if (_currentUsers > 0)
		_currentUsers--;
	if (isOperator(client))
		removeOperator(client);
}

Client* Channel::findClientByNickname(const std::string& name) const
{
	for (auto* c : _clients) {
		if (c->getNickname() == name)
			return c;
	}
	return nullptr;
}

// Ban handling

void Channel::banUser(Client *client) { _banned.emplace(client); }

bool Channel::isBanned(Client *client) const {
	auto it = _banned.find(client);
	if (it == _banned.end()) {
		return false;
	}
	return true;
}

// Operator handling

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

bool Channel::isOperator(Client* client) const { return _operators.find(client) != _operators.end(); }

// Userlimit handling

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

int Channel::getUserLimit() const {
	return _userLimit;
}

bool Channel::UserlimitSet() const {
	return _limitSet;
}

// Invite handling

void Channel::setInviteOnly() { _inviteOnly = true; }

void Channel::unsetInviteOnly() { _inviteOnly = false; }

bool Channel::isInviteOnly() const { return _inviteOnly; }

bool Channel::isInvited(Client* client) const {
	if (_invited.find(client) != _invited.end())
		return true;
	else
		return false;
}

//  Mode handling

void Channel::setMode(const std::vector<std::string_view>& params)
{
	if (params.empty())
		throw errs { 461, std::string("MODE") + " :Not enough parameters"};
		
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
			throw errs { 472, std::string(1, c) + " :is unknown mode char to me" };
		bool adding = (action == '+');
		std::string param;
		if (adding && flagsWithParams.find(c) != std::string::npos) {
			if (paramIndex >= params.size())
				throw errs { 461, std::string("MODE +") + c + " :Not enough parameters"};
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

// Password handling

std::string Channel::getPassword() const { return _password; }
bool Channel::PasswordRequired() const { return _passwordRequired; }

void Channel::setPassword(const std::string& password) {
	if (!_password.empty()) {
		throw errs { 467, ":Channel key already set"};
	}
	_password = password;
	_passwordRequired = true;
	std::cout << "Channel password was set to: " << password << std::endl;
}

void Channel::unsetPassword() {
	_password = "";
	std::cout << "Channel password was disabled" << std::endl;
}

// Topic handling

void Channel::setTopicProtection() { _topicProtected = true; }
void Channel::unsetTopicProtection() { _topicProtected = false; }
const std::string& Channel::getTopic() const { return _topic; }
void Channel::setTopic(const std::string& topic) { _topic = topic; }