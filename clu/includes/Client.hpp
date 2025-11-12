#pragma once

#include <string>
#include <unordered_map>

enum class RegistrationState
{
	NeedPassNickUser,
	NeedNickUser,
	NeedUser,
	Registered
};

class Client {
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
	void setNickname(std::string nickname);
	void setUsername(std::string username);
	void setFullname(std::string fullname);

	// Client state information
	bool hasPassword() const noexcept;
	bool hasNickname() const noexcept;
	bool hasUsername() const noexcept;
	bool hasFullname() const noexcept;
	bool isRegistered() const noexcept;

	void setHasPassword(bool hasPassword) noexcept;
	void setIsRegistered(bool isRegistered) noexcept;

	bool dataToWrite() const noexcept;
	void queueMsg(const std::string &msg);

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
	bool _hasFullname = false;
	bool _isRegistered = false;

	static void trimCrLf(std::string &str);

	// std::unordered_map<std::string, Channel> _channels;
};
