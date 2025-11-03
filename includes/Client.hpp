#pragma once

#include <string>

class Client {
public:
	Client();
	explicit Client(int fd);
	~Client();

	int getFd() const;
	void setFd(int fd);

	const std::string& getNick() const;
	void setNick(const std::string& nick);

	void appendRecv(const std::string& s);
	std::string& recvBuffer();

	void appendSend(const std::string& s);
	std::string& sendBuffer();

	void closeSocket();

	bool isRegistered() const;
	void setRegistered(bool v);

private:
	int _fd;
	std::string _nick;
	std::string _recvBuffer;
	std::string _sendBuffer;
	bool _registered;
};

