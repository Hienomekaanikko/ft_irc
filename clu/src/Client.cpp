#include "Client.hpp"

// Constructor
Client::Client(int fd) : _fd(fd) {}

// Getters and setters
int Client::getFd() const noexcept
{
	return _fd;
}

void Client::setFd(int fd) noexcept
{
	_fd = fd;
}

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

bool Client::dataToWrite() const noexcept
{
	return !_writeBuffer.empty();
}

void Client::queueMsg(const std::string& msg)
{
	_writeBuffer += msg;
}
