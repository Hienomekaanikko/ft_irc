#pragma once

#include <string>

class Client {
public:
	Client() = default;
	Client(int fd);

	Client(const Client& other) = default;
	Client& operator=(const Client& other) = default;

	~Client() = default;

	int getFd() const noexcept;
	void setFd(int fd) noexcept;

	std::string& getReadBuffer() noexcept;
	const std::string& getReadBuffer() const noexcept;

	std::string& getWriteBuffer() noexcept;
	const std::string& getWriteBuffer() const noexcept;

	bool dataToWrite() const noexcept;
	void queueMsg(const std::string& msg);

private:
	int			_fd = -1;
	std::string _readBuffer;
	std::string _writeBuffer;
};
