#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <vector>
#include <set>

class Client;

class Channel {
private:
    std::string _name;
    std::string _topic;
    std::string _key;
    std::vector<Client*> _clients;
    std::set<Client*> _operators;
    std::set<Client*> _invited;
    bool _inviteOnly;
    bool _topicRestricted;
    int _userLimit;

public:
    Channel(const std::string& name, Client* creator);
    ~Channel();
    
    const std::string& getName() const;
    const std::string& getTopic() const;
    void setTopic(const std::string& topic);
    
    void addClient(Client* client);
    void removeClient(Client* client);
    bool hasClient(Client* client) const;
    const std::vector<Client*>& getClients() const;
    
    void addOperator(Client* client);
    void removeOperator(Client* client);
    bool isOperator(Client* client) const;
    
    void addInvited(Client* client);
    void removeInvited(Client* client);
    bool isInvited(Client* client) const;
    
    void setInviteOnly(bool inviteOnly);
    bool isInviteOnly() const;
    
    void setTopicRestricted(bool restricted);
    bool isTopicRestricted() const;
    
    void setUserLimit(int limit);
    int getUserLimit() const;
    
    void setKey(const std::string& key);
    const std::string& getKey() const;
    
    void broadcast(const std::string& message, Client* exclude = NULL);
};

#endif
