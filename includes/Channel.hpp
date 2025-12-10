#pragma once

#include "Client.hpp"
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <stdexcept>
#include <algorithm>
#include <iostream>
#include <limits>
#include <vector>
#include <ctime>

class Client;

class Channel
{
public:
	Channel(const std::string &name);

	void addClient(Client *client);
	void addOperator(const std::string& name);
	void inviteUser(Client *client);
	bool isOperator(Client *client) const;
	bool isMember(Client *client);

	void unsetPassword();
	void unsetInviteOnly();
	void unsetTopicProtection();
	void unsetUserlimit();

	void setPassword(const std::string &password);
	void setInviteOnly();
	void setTopicProtection();
	void setTopic(const std::string &topic);
	void setMode(const std::vector<std::string_view> &params);
	void setUserlimit(const std::string limit);

	void removeClient(const std::string& nickname);
	void removeOperator(const std::string& nickname);

	const std::string &getChannelName() const;
	const std::string &getTopic() const;
	int getCurrentUsers() const;
	const std::unordered_set<Client*>& getMembers() const;
	std::string getPassword() const;
	int getUserLimit() const;
	std::string getModeString() const;

	bool PasswordRequired() const;
	bool UserlimitSet() const;
	bool isInviteOnly() const;
	bool isEmpty() const;
	bool isInvited(Client* client) const;
	bool isTopicProtected() const;

	// Creation time handling
	void setCreationTime(const std::time_t time);
	std::time_t getCreationTime() const;

	Client *findClientByNickname(const std::string &nickname) const;

private:
	std::string _password;
	std::string _channelName;
	std::string _topic;
	bool _inviteOnly;
	bool _passwordRequired;
	bool _topicProtected;
	bool _limitSet;
	int	_userLimit;
	int _currentUsers;
	std::time_t _creationTime;

	std::unordered_map<char, bool> _modes;
	std::unordered_set<Client *> _clients;
	std::unordered_set<Client *> _operators;
	std::unordered_set<Client *> _invited;
};