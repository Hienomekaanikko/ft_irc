#include "Utils.hpp"
#include <sstream>
#include <algorithm>
#include <cctype>

std::vector<std::string> split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;
    
    while (std::getline(ss, token, delimiter)) {
        if (!token.empty()) {
            tokens.push_back(token);
        }
    }
    return tokens;
}

std::string trim(const std::string& str) {
    size_t start = 0;
    size_t end = str.length();
    
    while (start < end && std::isspace(str[start])) {
        start++;
    }
    
    while (end > start && std::isspace(str[end - 1])) {
        end--;
    }
    
    return str.substr(start, end - start);
}

std::string toUpper(const std::string& str) {
    std::string result = str;
    for (size_t i = 0; i < result.length(); i++) {
        result[i] = std::toupper(static_cast<unsigned char>(result[i]));
    }
    return result;
}

bool isValidNickname(const std::string& nickname) {
    if (nickname.empty() || nickname.length() > 9) {
        return false;
    }
    
    if (!std::isalpha(nickname[0])) {
        return false;
    }
    
    for (size_t i = 0; i < nickname.length(); i++) {
        char c = nickname[i];
        if (!std::isalnum(c) && c != '_' && c != '-') {
            return false;
        }
    }
    
    return true;
}

bool isValidChannelName(const std::string& name) {
    if (name.empty() || name.length() > 50) {
        return false;
    }
    
    if (name[0] != '#' && name[0] != '&') {
        return false;
    }
    
    for (size_t i = 1; i < name.length(); i++) {
        char c = name[i];
        if (std::isspace(c) || c == ',' || c == ':') {
            return false;
        }
    }
    
    return true;
}
