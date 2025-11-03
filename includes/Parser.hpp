#pragma once

#include <string>
#include <vector>

class Parser {
public:
	Parser();
	~Parser();

	// feed raw data for a client; returns complete lines (without CRLF)
	std::vector<std::string> feed(const std::string& data);

	// optionally parse a single IRC line into command and params
	std::vector<std::string> splitLine(const std::string& line) const;
};

