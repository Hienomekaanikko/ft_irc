#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <stdexcept>
#include "Client.hpp"
#include <iostream>

class Client;

class Channel
{
public:
	Channel(const std::string &name);

	void addClient(Client *client);
	void removeClient(Client *client);
	void addOperator(Client *client);
	bool isOperator(Client *client) const;

	void setPassword(const std::string &password);
	void setTopic(const std::string &topic);
	void setMode(const std::vector<std::string_view> &params);

	const std::string &getChannelName() const;
	const std::string &getTopic() const;

	bool isEmpty() const;

	Client *findClientByNickname(const std::string &nickname) const;

private:
	std::string _password;
	std::string _channelName;
	std::string _topic;

	std::unordered_map<char, bool> _modes;
	std::unordered_set<Client *> _clients;
	std::unordered_set<Client *> _operators;
};