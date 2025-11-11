#pragma once

#include <string>

class Client {
private:
	int _fd = -1;
	std::string _readBuffer;
	std::string _writeBuffer;

	// Identity & State
	std::string _nickname;
	std::string _username;
	std::string _fullname;

	bool _hasPassword = false;
	bool _hasNickname = false;
	bool _hasUsername = false;
	bool _isRegistered = false;

public:
	Client() = default;
	Client(int fd);

	Client(const Client &other) = default;
	Client &operator=(const Client &other) = default;

	~Client() = default;

	/// Getters ///
	// File descriptor
	int getFd() const noexcept;
	
	// Identity getters
	const std::string &getNickname() const noexcept;
	const std::string &getUsername() const noexcept;
	const std::string &getFullname() const noexcept;

	// Read line buffer
	std::string &getReadBuffer() noexcept;
	const std::string &getReadBuffer() const noexcept;

	std::string &getWriteBuffer() noexcept;
	const std::string &getWriteBuffer() const noexcept;

	// Setters
	void setFd(int fd) noexcept;
	void setNickname(const std::string &nickname);
	void setUsername(const std::string &username);
	void setFullname(const std::string &fullname);

	// Client state information
	bool hasPassword() const noexcept;
	bool hasNickname() const noexcept;
	bool hasUsername() const noexcept;
	bool isRegistered() const noexcept;


	bool dataToWrite() const noexcept;
	void queueMsg(const std::string &msg);
};
