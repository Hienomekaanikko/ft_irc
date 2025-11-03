#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <vector>

class Channel;

class Client {
private:
    int _fd;
    std::string _nickname;
    std::string _username;
    std::string _realname;
    std::string _hostname;
    std::string _buffer;
    bool _authenticated;
    bool _registered;
    std::vector<Channel*> _channels;

public:
    Client(int fd, const std::string& hostname);
    ~Client();
    
    int getFd() const;
    const std::string& getNickname() const;
    const std::string& getUsername() const;
    const std::string& getRealname() const;
    const std::string& getHostname() const;
    std::string getPrefix() const;
    bool isAuthenticated() const;
    bool isRegistered() const;
    
    void setNickname(const std::string& nickname);
    void setUsername(const std::string& username);
    void setRealname(const std::string& realname);
    void setAuthenticated(bool auth);
    void setRegistered(bool reg);
    
    void appendToBuffer(const std::string& data);
    std::string extractMessage();
    bool hasCompleteMessage() const;
    
    void joinChannel(Channel* channel);
    void leaveChannel(Channel* channel);
    const std::vector<Channel*>& getChannels() const;
    
    void sendMessage(const std::string& message);
};

#endif
