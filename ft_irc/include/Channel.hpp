#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include "IRC.hpp"
#include <string>

class Client;

class Channel {
private:
    std::string _name;
    std::string _topic;
    std::string _key;
    std::set<Client*> _clients;
    std::set<Client*> _operators;
    std::string _mode;
    size_t _userLimit;

public:
    Channel(const std::string& name);
    ~Channel();

    // Getters
    const std::string& getName() const;
    const std::string& getTopic() const;
    const std::string& getKey() const;
    const std::set<Client*>& getClients() const;
    const std::set<Client*>& getOperators() const;
    const std::string& getMode() const;
    size_t getUserLimit() const;

    // Setters
    void setTopic(const std::string& topic);
    void setKey(const std::string& key);
    void setMode(const std::string& mode);
    void setUserLimit(size_t limit);

    // Client operations
    void addClient(Client* client);
    void removeClient(Client* client);
    bool hasClient(Client* client) const;
    bool isOperator(Client* client) const;
    void addOperator(Client* client);
    void removeOperator(Client* client);

    // Mode operations
    bool hasMode(char mode) const;
    void addMode(char mode);
    void removeMode(char mode);

    // Channel operations
    void broadcast(const std::string& message, Client* sender = NULL);
    bool isInviteOnly() const;
    bool isModerated() const;
    bool isSecret() const;
    bool isProtected() const;
};

#endif // CHANNEL_HPP 