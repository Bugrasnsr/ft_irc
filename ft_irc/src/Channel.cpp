#include "../include/Channel.hpp"
#include "../include/Client.hpp"
#include "../include/Utils.hpp"
#include <sstream>

Channel::Channel(const std::string& name) : _name(name), _userLimit(0) {
    _mode = "+n"; // Default mode: no external messages
}

Channel::~Channel() {
    // Clean up clients
    for (std::set<Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
        (*it)->removeChannel(this);
    }
    _clients.clear();
    _operators.clear();
}

// Getters
const std::string& Channel::getName() const { return _name; }
const std::string& Channel::getTopic() const { return _topic; }
const std::string& Channel::getKey() const { return _key; }
const std::set<Client*>& Channel::getClients() const { return _clients; }
const std::set<Client*>& Channel::getOperators() const { return _operators; }
const std::string& Channel::getMode() const { return _mode; }
size_t Channel::getUserLimit() const { return _userLimit; }

// Setters
void Channel::setTopic(const std::string& topic) { _topic = topic; }
void Channel::setKey(const std::string& key) { _key = key; }
void Channel::setMode(const std::string& mode) { _mode = mode; }
void Channel::setUserLimit(size_t limit) { _userLimit = limit; }

// Client operations
void Channel::addClient(Client* client) {
    if (client) {
        _clients.insert(client);
        client->addChannel(this);
    }
}

void Channel::removeClient(Client* client) {
    if (client) {
        _clients.erase(client);
        _operators.erase(client);
        client->removeChannel(this);
    }
}

bool Channel::hasClient(Client* client) const {
    return _clients.find(client) != _clients.end();
}

bool Channel::isOperator(Client* client) const {
    return _operators.find(client) != _operators.end();
}

void Channel::addOperator(Client* client) {
    if (client && hasClient(client))
        _operators.insert(client);
}

void Channel::removeOperator(Client* client) {
    if (client)
        _operators.erase(client);
}

// Mode operations
bool Channel::hasMode(char mode) const {
    return _mode.find(mode) != std::string::npos;
}

void Channel::addMode(char mode) {
    if (!hasMode(mode))
        _mode += mode;
}

void Channel::removeMode(char mode) {
    size_t pos = _mode.find(mode);
    if (pos != std::string::npos)
        _mode.erase(pos, 1);
}

// Channel operations
void Channel::broadcast(const std::string& message, Client* sender) {
    std::string prefix;
    if (sender)
        prefix = sender->getNickname() + "!" + sender->getUsername() + "@" + sender->getHostname();

    for (std::set<Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
        if (*it != sender) {
            std::string fullMessage = ":" + prefix + " " + message + "\r\n";
            send((*it)->getFd(), fullMessage.c_str(), fullMessage.length(), 0);
        }
    }
}

bool Channel::isInviteOnly() const {
    return hasMode('i');
}

bool Channel::isModerated() const {
    return hasMode('m');
}

bool Channel::isSecret() const {
    return hasMode('s');
}

bool Channel::isProtected() const {
    return hasMode('p');
} 