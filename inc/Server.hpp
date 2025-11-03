#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <vector>
#include <map>
#include <poll.h>
#include <netinet/in.h>

class Client;
class Channel;

class Server {
private:
    int _serverSocket;
    int _port;
    std::string _password;
    std::vector<struct pollfd> _pollfds;
    std::map<int, Client*> _clients;
    std::map<std::string, Channel*> _channels;
    bool _running;

    void acceptNewClient();
    void handleClientData(int fd);
    void removeClient(int fd);
    void processCommand(Client* client, const std::string& message);
    
public:
    Server(int port, const std::string& password);
    ~Server();
    
    void start();
    void stop();
    
    Client* getClient(int fd);
    Client* getClientByNick(const std::string& nickname);
    Channel* getChannel(const std::string& name);
    Channel* createChannel(const std::string& name, Client* creator);
    void removeChannel(const std::string& name);
    
    const std::string& getPassword() const;
    
    // Command handlers
    void cmdPass(Client* client, const std::vector<std::string>& params);
    void cmdNick(Client* client, const std::vector<std::string>& params);
    void cmdUser(Client* client, const std::vector<std::string>& params);
    void cmdJoin(Client* client, const std::vector<std::string>& params);
    void cmdPart(Client* client, const std::vector<std::string>& params);
    void cmdPrivmsg(Client* client, const std::vector<std::string>& params);
    void cmdQuit(Client* client, const std::vector<std::string>& params);
    void cmdPing(Client* client, const std::vector<std::string>& params);
    void cmdKick(Client* client, const std::vector<std::string>& params);
    void cmdInvite(Client* client, const std::vector<std::string>& params);
    void cmdTopic(Client* client, const std::vector<std::string>& params);
    void cmdMode(Client* client, const std::vector<std::string>& params);
};

#endif
