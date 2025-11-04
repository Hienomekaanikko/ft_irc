#pragma once

#include <string>

class Client {
public:
	Client();
	Client(int fd);
	Client(const Client& other);
	Client& operator=(const Client& other);
	~Client();

	
};
