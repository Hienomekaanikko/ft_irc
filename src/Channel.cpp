#include "Channel.hpp"
#include "Server.hpp"

// Channel constructor
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
	_inviteOnly = false;
	_topicProtected = false;
}

// Getters
const std::string& Channel::getChannelName() const { return _channelName; }

bool Channel::isEmpty() const { return _clients.empty(); }

int Channel::getCurrentUsers() const { return _currentUsers; }

const std::unordered_set<Client*>& Channel::getMembers() const { return _clients; }

bool Channel::isMember(Client *client)
{
	auto it = _clients.find(client);
	if (it == _clients.end())
		return false;
	else
		return true;
}

// Client handling
void Channel::addClient(Client* client) 
{
	client->incrementChannelCount();
	_clients.insert(client);
	_currentUsers++;
}

void Channel::removeClient(const std::string& nickname) 
{
	Client* client = findClientByNickname(nickname);
	if (client == nullptr) {
		throw errs { 401, nickname + " :Such client does not exist" };
	}
	if (_currentUsers > 0)
		_currentUsers--;
	if (isOperator(client))
		removeOperator(nickname);
	_clients.erase(client);
	client->decrementChannelCount();
}

// Find client by nickname
Client* Channel::findClientByNickname(const std::string& name) const
{
	for (Client* c : _clients) {
		if (c->getNickname() == name) {
			return c;
		}
	}
	return nullptr;
}

// Operator handling
void Channel::addOperator(const std::string& nickname) {
	Client* client = findClientByNickname(nickname);
	if (client == nullptr)
		throw errs { 401, nickname + " :Such client does not exist" };
	_operators.insert(client);
}

void Channel::removeOperator(const std::string& nickname) {
	Client* client = findClientByNickname(nickname);
	if (client == nullptr) {
		throw errs { 401, nickname + " :Such client does not exist" };
	}
	auto it = _operators.find(client);
	if (it != _operators.end()) {
		_operators.erase(it);
	}
}

bool Channel::isOperator(Client* client) const { return _operators.find(client) != _operators.end(); }

// Userlimit handling
void Channel::setUserlimit(const std::string limit) {
	long long n;
	try {
		n = stoll(limit);
	} catch (std::exception &e) {
		throw errs { 502, ":Cannot set channel limit: invalid value" };
	}
	if (n > std::numeric_limits<int>::max() || n < 0) {
		throw errs { 502, ":Cannot set channel limit: invalid value" };
	}
	int val = static_cast<int>(n);
	_userLimit = val;
	_limitSet = true;
}	

void Channel::unsetUserlimit()
{
	_userLimit = -1;
	_limitSet = false;
}

int Channel::getUserLimit() const { return _userLimit; }

bool Channel::UserlimitSet() const { return _limitSet; }

// Invite handling
void Channel::inviteUser(Client *client) { _invited.insert(client); }

void Channel::setInviteOnly() { _inviteOnly = true; }

void Channel::unsetInviteOnly() { _inviteOnly = false; }

bool Channel::isInviteOnly() const { return _inviteOnly; }

bool Channel::isInvited(Client* client) const
{
	if (_invited.find(client) != _invited.end())
		return true;
	else
		return false;
}

// Mode handling
void Channel::setMode(const std::vector<std::string_view>& params)
{
	if (params.empty())
		throw errs { 461, std::string("MODE") + " :Not enough parameters"};
		
	const std::string flags = "itkol";
	const std::string flagsWithParams = "kol";

	std::string_view modeString = params[0];
	size_t paramIndex = 1;

	char action = 0;

	for (char c : modeString)
	{
		if (c == '+' || c == '-')
		{
			action = c;
			continue;
		}
		if (flags.find(c) == std::string::npos)
			throw errs { 472, std::string(1, c) + " :is unknown mode char to me" };
		bool adding = (action == '+');
		std::string param;
		if ((adding || c == 'o') && flagsWithParams.find(c) != std::string::npos)
		{
			if (paramIndex >= params.size())
				throw errs { 461, std::string("MODE +") + c + " :Not enough parameters"};
			param = std::string(params[paramIndex++]);
		}
		switch (c)
		{
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
				adding ? addOperator(param) : removeOperator(param);
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

void Channel::setPassword(const std::string& password)
{
	if (!_password.empty())
		throw errs { 467, ":Channel key already set"};
	_password = password;
	_passwordRequired = true;
}

void Channel::unsetPassword()
{
	_password = "";
	_passwordRequired = false;
}

// Topic handling
void Channel::setTopicProtection() { _topicProtected = true; }
void Channel::unsetTopicProtection() { _topicProtected = false; }
const std::string& Channel::getTopic() const { return _topic; }
void Channel::setTopic(const std::string& topic) { _topic = topic; }
bool Channel::isTopicProtected() const { return _topicProtected; }

// Creation time handling
void Channel::setCreationTime(const std::time_t time)
{
	_creationTime = time;
}
std::time_t Channel::getCreationTime() const { return _creationTime; }

// Get current mode string
std::string Channel::getModeString() const
{
	std::string modeStr = "+";
	
	if (_inviteOnly)
		modeStr += "i";
	if (_topicProtected)
		modeStr += "t";
	if (_passwordRequired)
		modeStr += "k";
	if (_limitSet)
		modeStr += "l";
	if (modeStr == "+")
		return "+";
	
	return modeStr;
}
