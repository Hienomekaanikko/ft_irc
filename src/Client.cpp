#include "Client.hpp"

// Constructor
Client::Client(int fd) : _fd(fd) {}

// Getters
int Client::getFd() const noexcept { return _fd; }

const std::string& Client::getNickname() const noexcept { return _nickname; }

const std::string& Client::getUsername() const noexcept { return _username; }

const std::string& Client::getFullname() const noexcept { return _fullname; }

// Read and write buffers
std::string& Client::getReadBuffer() noexcept { return _readBuffer; }

const std::string& Client::getReadBuffer() const noexcept { return _readBuffer; }

std::string& Client::getWriteBuffer() noexcept { return _writeBuffer; }

const std::string& Client::getWriteBuffer() const noexcept { return _writeBuffer; }

// Setters
void Client::setFd(int fd) noexcept { _fd = fd; }

void Client::setNickname(std::string nickname)
{
	trimCrLf(nickname);
	_nickname = nickname;
	_hasNickname = true;
}

void Client::setUsername(std::string username)
{
	trimCrLf(username);
	_username = username;
	_hasUsername = true;
}

void Client::setFullname(std::string fullname)
{
	trimCrLf(fullname);
	_fullname = fullname;
	_hasFullname = true;
}

void Client::setHasPassword(bool hasPassword) noexcept { _hasPassword = hasPassword; }

void Client::setIsRegistered(bool isRegistered) noexcept { _isRegistered = isRegistered; }

// Client state information
bool Client::hasPassword() const noexcept { return _hasPassword; }

bool Client::hasNickname() const noexcept { return _hasNickname; }

bool Client::hasUsername() const noexcept { return _hasUsername; }

bool Client::hasFullname() const noexcept { return _hasFullname; }

bool Client::isRegistered() const noexcept { return _isRegistered; }

// Check if there is data to write
bool Client::dataToWrite() const noexcept { return !_writeBuffer.empty(); }

void Client::queueMsg(const std::string& msg) { _writeBuffer += msg; }

/// Private member functions ///
// Trim CRLF from the end of a string
void Client::trimCrLf(std::string& str)
{
	while (!str.empty() && (str.back() == '\r' || str.back() == '\n'))
		str.pop_back();
}
