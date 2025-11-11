#include "Client.hpp"

// Constructor
Client::Client(int fd) : _fd(fd) {}

// Getters
int Client::getFd() const noexcept
{
	return _fd;
}

const std::string& Client::getNickname() const noexcept
{
	return _nickname;
}

const std::string& Client::getUsername() const noexcept
{
	return _username;
}

const std::string& Client::getHostname() const noexcept
{
	return _hostname;
}

// Read and write buffers
std::string& Client::getReadBuffer() noexcept
{
	return _readBuffer;
}

const std::string& Client::getReadBuffer() const noexcept
{
	return _readBuffer;
}

std::string& Client::getWriteBuffer() noexcept
{
	return _writeBuffer;
}

const std::string& Client::getWriteBuffer() const noexcept
{
	return _writeBuffer;
}

// Setters
void Client::setFd(int fd) noexcept
{
	_fd = fd;
}

void Client::setNickname(const std::string& nickname)
{
	_nickname = nickname;
}

void Client::setUsername(const std::string& username)
{
	_username = username;
}

void Client::setFullname(const std::string& fullname)
{
	_fullname = fullname;
}

// Client state information
bool Client::hasPassword() const noexcept
{
	return _hasPassword;
}

bool Client::hasNickname() const noexcept
{
	return _hasNickname;
}

bool Client::hasUsername() const noexcept
{
	return _hasUsername;
}

bool Client::isRegistered() const noexcept
{
	return _isRegistered;
}

// Check if there is data to write
bool Client::dataToWrite() const noexcept
{
	return !_writeBuffer.empty();
}

void Client::queueMsg(const std::string& msg)
{
	_writeBuffer += msg;
}
