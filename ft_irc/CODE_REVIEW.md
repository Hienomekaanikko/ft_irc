# ft_irc Code Review

## Overview
This is a comprehensive review of your IRC server implementation. The project compiles successfully with C++17 and implements a basic IRC server with support for multiple clients, channels, and various IRC commands.

## ✅ Strengths

### 1. **Architecture & Design**
- **Clean separation of concerns**: Server, Client, and Channel classes are well-separated
- **Non-blocking I/O**: Properly uses `poll()` for handling multiple clients
- **RAII principles**: Resources are properly managed in constructors/destructors
- **Modern C++**: Uses C++17 features like `std::string_view`, `std::unordered_map`, etc.

### 2. **Core Functionality**
- **Registration flow**: PASS → NICK → USER sequence is properly implemented
- **Channel management**: Supports channel creation, joining, and messaging
- **Mode system**: Implements channel modes (i, t, k, o, l)
- **Message routing**: PRIVMSG works for both channels and private messages
- **Signal handling**: Graceful shutdown on SIGINT

### 3. **Code Quality**
- **Consistent style**: Code follows a consistent formatting style
- **Good comments**: Functions are documented with purpose
- **Error handling**: Uses exceptions and error codes appropriately

## ⚠️ Issues Found

### **CRITICAL Issues**

#### 1. **Buffer Overflow Risk in JOIN Command** (Server.cpp:604)
```cpp
if (it->second.PasswordRequired()){
    if (params[1].empty() || it->second.getPassword() != params[1]) {
```
**Problem**: Accessing `params[1]` without checking if `params.size() > 1` can cause a crash.

**Fix**:
```cpp
if (it->second.PasswordRequired()){
    if (params.size() < 2 || it->second.getPassword() != params[1]) {
        sendNumeric(client, 475, _channelName + " :Password required/Invalid password");
        return;
    }
}
```

#### 2. **Missing JOIN Success Messages**
After successfully joining a channel, the server should send:
- JOIN confirmation to all channel members
- RPL_TOPIC (332) if topic is set
- RPL_NAMREPLY (353) with list of users
- RPL_ENDOFNAMES (366)

**Current**: The server silently adds users to channels without any feedback.

#### 3. **PRIVMSG Missing Hostname**
The hostname is hardcoded as empty in Messages.cpp:49 and 66:
```cpp
prefix += client.getUsername() + "@"; //+ client.getHostname();
```
Should be:
```cpp
prefix += client.getUsername() + "@localhost";
```

#### 4. **Channel Password Not Cleared on Unset** (Channel.cpp:226-229)
```cpp
void Channel::unsetPassword() {
    _password = "";
    std::cout << "Channel password was disabled" << std::endl;
}
```
**Problem**: Missing `_passwordRequired = false;`

**Fix**:
```cpp
void Channel::unsetPassword() {
    _password = "";
    _passwordRequired = false;
    std::cout << "Channel password was disabled" << std::endl;
}
```

### **HIGH Priority Issues**

#### 5. **Incomplete NICK Change Broadcast**
When a user changes their nickname, the server broadcasts to all clients (Server.cpp:533), but it should only broadcast to:
- The user themselves
- Users in the same channels

#### 6. **Missing PART Command**
Users cannot leave channels. You need to implement the PART command.

#### 7. **Missing KICK Command**
Channel operators cannot kick users. This is a standard IRC feature.

#### 8. **Missing INVITE Command**
The invite-only mode ('i') is implemented, but there's no INVITE command to actually invite users.

#### 9. **Missing TOPIC Command**
Topic protection mode ('t') is implemented, but there's no TOPIC command to view/set topics.

#### 10. **User Limit Not Decremented** (Channel.cpp:134-137)
```cpp
void Channel::unsetUserlimit() {
    _userLimit = -1;
    std::cout << "User limit was unset" << std::endl;
}
```
**Problem**: Missing `_limitSet = false;`

### **MEDIUM Priority Issues**

#### 11. **Numeric Reply Format Issues**
Some numeric replies don't follow IRC RFC format. For example:
- Server.cpp:339: Should include the target nickname
- Server.cpp:353: Parameter order seems incorrect

#### 12. **MODE Command Validation**
The MODE command (Server.cpp:646) checks if the target is a nickname:
```cpp
if (nickInUse(params[0])) {
    return ;
}
```
This is incorrect - it should check if it's NOT a channel name.

#### 13. **Error Messages to stdout Instead of Client**
Channel.cpp has many `std::cout` and `std::cerr` messages that should be sent to the client instead:
- Line 92-96: Operator add errors
- Line 101-110: Operator remove errors
- Line 123, 130, 136: User limit messages
- Line 223, 228: Password messages

#### 14. **Missing WHO Command**
Useful for listing channel members.

#### 15. **Missing NAMES Command**
Should list all users in a channel.

#### 16. **Unused RegistrationState Enum**
Client.hpp defines `RegistrationState` enum but it's never used.

### **LOW Priority Issues**

#### 17. **Magic Numbers**
- Buffer size (1024) is defined but could be configurable
- Poll timeout (-1) is hardcoded

#### 18. **Incomplete Error Handling**
Some error cases don't send proper numeric replies:
- Server.cpp:646-648: MODE on nickname just returns silently

#### 19. **Memory Safety**
While the code uses smart containers, there are raw pointers to Clients in Channels. This is generally safe since Clients are stored in `_clients` map, but could be improved with weak_ptr or ensuring proper cleanup order.

#### 20. **Inconsistent Logging**
Some operations log to stdout, others don't. Consider a consistent logging strategy.

## 🔧 Recommended Fixes (Priority Order)

### 1. Fix Critical Buffer Overflow
```cpp
// In Server.cpp, handleJOIN function
void Server::handleJOIN(Client &client, const std::vector<std::string_view> &params)
{
    if (params.empty())
    {
        sendNumeric(client, 461, "JOIN :Not enough parameters");
        return;
    }

    std::string _channelName(params[0]);
    auto it = _channels.find(_channelName);
    if (it != _channels.end())
    {
        // Check password FIRST, before other checks
        if (it->second.PasswordRequired()) {
            if (params.size() < 2 || it->second.getPassword() != params[1]) {
                sendNumeric(client, 475, _channelName + " :Bad channel key");
                return;
            }
        }
        
        // Check if banned
        if (it->second.isBanned(&client)) {
            sendNumeric(client, 474, _channelName + " :Cannot join channel (+b)");
            return;
        }
        
        // Check invite-only
        if (it->second.isInviteOnly()) {
            if (!it->second.isInvited(&client)) {
                sendNumeric(client, 473, _channelName + " :Cannot join channel (+i)");
                return;
            }
        }
        
        // Check user limit
        if (it->second.UserlimitSet()) {
            if (it->second.getUserLimit() <= it->second.getCurrentUsers()) {
                sendNumeric(client, 471, _channelName + " :Cannot join channel (+l)");
                return;
            }
        }
        
        it->second.addClient(&client);
    }
    else
    {
        Channel newChannel(_channelName);
        newChannel.addClient(&client);
        newChannel.addOperator(&client);
        _channels.emplace(_channelName, newChannel);
    }
    
    // Send JOIN confirmation to all channel members
    Channel &chan = _channels[_channelName];
    std::ostringstream joinMsg;
    joinMsg << ":" << client.getNickname() << "!" 
            << client.getUsername() << "@localhost"
            << " JOIN :" << _channelName << "\r\n";
    sendToChannel(chan, joinMsg.str(), nullptr);
    
    // Send topic if set
    const std::string &topic = chan.getTopic();
    if (!topic.empty()) {
        sendNumeric(client, 332, _channelName, topic);
    }
    
    // Send names list
    std::ostringstream names;
    for (Client *member : chan.getMembers()) {
        if (chan.isOperator(member))
            names << "@";
        names << member->getNickname() << " ";
    }
    sendNumeric(client, 353, "= " + _channelName, names.str());
    sendNumeric(client, 366, _channelName, "End of /NAMES list");
}
```

### 2. Fix Password Unset
```cpp
// In Channel.cpp
void Channel::unsetPassword() {
    _password = "";
    _passwordRequired = false;  // ADD THIS LINE
    std::cout << "Channel password was disabled" << std::endl;
}
```

### 3. Fix User Limit Unset
```cpp
// In Channel.cpp
void Channel::unsetUserlimit() {
    _userLimit = -1;
    _limitSet = false;  // ADD THIS LINE
    std::cout << "User limit was unset" << std::endl;
}
```

### 4. Add Missing Commands

#### PART Command
```cpp
// In Server.hpp
void handlePART(Client &client, const std::vector<std::string_view> &params);

// In Server.cpp
void Server::handlePART(Client &client, const std::vector<std::string_view> &params)
{
    if (params.empty()) {
        sendNumeric(client, 461, "PART :Not enough parameters");
        return;
    }
    
    std::string channelName(params[0]);
    auto it = _channels.find(channelName);
    if (it == _channels.end()) {
        sendNumeric(client, 403, channelName + " :No such channel");
        return;
    }
    
    Channel &chan = it->second;
    if (!chan.isMember(&client)) {
        sendNumeric(client, 442, channelName + " :You're not on that channel");
        return;
    }
    
    std::string reason = "Leaving";
    if (params.size() > 1)
        reason = std::string(params[1]);
    
    // Broadcast PART to all channel members
    std::ostringstream partMsg;
    partMsg << ":" << client.getNickname() << "!" 
            << client.getUsername() << "@localhost"
            << " PART " << channelName << " :" << reason << "\r\n";
    sendToChannel(chan, partMsg.str(), nullptr);
    
    // Remove client from channel
    chan.removeClient(&client);
    
    // Remove empty channel
    if (chan.isEmpty())
        _channels.erase(channelName);
}

// Add to processLine():
else if (upper == "PART")
    handlePART(client, cmd.params);
```

#### TOPIC Command
```cpp
void Server::handleTOPIC(Client &client, const std::vector<std::string_view> &params)
{
    if (params.empty()) {
        sendNumeric(client, 461, "TOPIC :Not enough parameters");
        return;
    }
    
    std::string channelName(params[0]);
    auto it = _channels.find(channelName);
    if (it == _channels.end()) {
        sendNumeric(client, 403, channelName + " :No such channel");
        return;
    }
    
    Channel &chan = it->second;
    if (!chan.isMember(&client)) {
        sendNumeric(client, 442, channelName + " :You're not on that channel");
        return;
    }
    
    // If no topic parameter, show current topic
    if (params.size() == 1) {
        const std::string &topic = chan.getTopic();
        if (topic.empty())
            sendNumeric(client, 331, channelName, "No topic is set");
        else
            sendNumeric(client, 332, channelName, topic);
        return;
    }
    
    // Setting topic - check permissions
    if (chan.isTopicProtected() && !chan.isOperator(&client)) {
        sendNumeric(client, 482, channelName + " :You're not channel operator");
        return;
    }
    
    std::string newTopic(params[1]);
    chan.setTopic(newTopic);
    
    // Broadcast topic change
    std::ostringstream topicMsg;
    topicMsg << ":" << client.getNickname() << "!" 
             << client.getUsername() << "@localhost"
             << " TOPIC " << channelName << " :" << newTopic << "\r\n";
    sendToChannel(chan, topicMsg.str(), nullptr);
}
```

## 📋 Testing Recommendations

### 1. Test with Real IRC Clients
- **irssi**: `irssi -c localhost -p <port> -w <password>`
- **WeeChat**: `weechat`
- **HexChat**: GUI client for visual testing

### 2. Test Cases to Cover
- [ ] Multiple clients connecting simultaneously
- [ ] Password authentication (correct and incorrect)
- [ ] Nickname conflicts
- [ ] Channel creation and joining
- [ ] Channel modes (i, t, k, o, l)
- [ ] Private messages between users
- [ ] Channel messages
- [ ] User disconnection and cleanup
- [ ] Empty channel cleanup
- [ ] Signal handling (Ctrl+C)
- [ ] Buffer overflow scenarios
- [ ] Very long messages
- [ ] Rapid command sending

### 3. Edge Cases
- [ ] Empty channel names
- [ ] Invalid channel names (not starting with #)
- [ ] Very long nicknames
- [ ] Special characters in nicknames
- [ ] Joining multiple channels
- [ ] Changing nickname while in channels
- [ ] Operator privileges
- [ ] User limit edge cases (exactly at limit)

## 🎯 Additional Features to Consider

1. **WHOIS command**: Get information about a user
2. **LIST command**: List all channels
3. **AWAY command**: Set away status
4. **NOTICE command**: Like PRIVMSG but without auto-reply
5. **Channel bans**: Implement ban list management
6. **Better hostname resolution**: Instead of hardcoded "localhost"
7. **Server password**: Separate from channel passwords
8. **Multiple channel join**: JOIN #chan1,#chan2,#chan3
9. **Logging**: Log all commands and errors to a file
10. **Configuration file**: Port, password, server name from config

## 📊 Code Metrics

- **Total Lines**: ~1,200 lines
- **Files**: 8 files (3 headers, 5 implementations)
- **Classes**: 3 (Server, Client, Channel)
- **Commands Implemented**: 8 (PASS, NICK, USER, PING, QUIT, JOIN, MODE, PRIVMSG)
- **Commands Missing**: 6+ (PART, KICK, INVITE, TOPIC, WHO, NAMES, etc.)

## 🏆 Overall Assessment

**Grade: B+ (Good, with room for improvement)**

### What's Working Well:
- Core IRC functionality is solid
- Code is clean and well-organized
- Proper use of modern C++ features
- Non-blocking I/O implementation is correct

### What Needs Work:
- Critical buffer overflow bug in JOIN
- Missing essential IRC commands (PART, TOPIC, KICK, INVITE)
- Incomplete message broadcasting
- Some numeric reply format issues

### Estimated Time to Fix Critical Issues: 2-4 hours
### Estimated Time to Add Missing Commands: 4-6 hours

## 🚀 Next Steps

1. **Immediate**: Fix the buffer overflow in JOIN command
2. **Short-term**: Implement PART, TOPIC, and fix JOIN messages
3. **Medium-term**: Add KICK, INVITE, WHO, NAMES commands
4. **Long-term**: Improve error handling and add logging

---

**Note**: This review is based on the IRC RFC 1459 and RFC 2812 standards. Your implementation covers the basic requirements well but would benefit from the fixes and additions mentioned above.
