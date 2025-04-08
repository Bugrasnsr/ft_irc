#include "../include/Client.hpp"
#include "../include/Channel.hpp"
#include "../include/Utils.hpp"
#include <sstream>

Client::Client(int fd) : _fd(fd), _registered(false), _authenticated(false), _mode("") {
    _hostname = Utils::getIpAddress(fd);
}

Client::~Client() {
    // Clean up channels
    for (std::set<Channel*>::iterator it = _channels.begin(); it != _channels.end(); ++it) {
        (*it)->removeClient(this);
    }
    _channels.clear();
}

// Getters
int Client::getFd() const { return _fd; }
const std::string& Client::getNickname() const { return _nickname; }
const std::string& Client::getUsername() const { return _username; }
const std::string& Client::getRealname() const { return _realname; }
const std::string& Client::getHostname() const { return _hostname; }
const std::string& Client::getBuffer() const { return _buffer; }
bool Client::isRegistered() const { return _registered; }
bool Client::isAuthenticated() const { return _authenticated; }
const std::set<Channel*>& Client::getChannels() const { return _channels; }
const std::string& Client::getMode() const { return _mode; }

// Setters
void Client::setNickname(const std::string& nickname) { _nickname = nickname; }
void Client::setUsername(const std::string& username) { _username = username; }
void Client::setRealname(const std::string& realname) { _realname = realname; }
void Client::setHostname(const std::string& hostname) { _hostname = hostname; }
void Client::setRegistered(bool registered) { _registered = registered; }
void Client::setAuthenticated(bool authenticated) { _authenticated = authenticated; }
void Client::setMode(const std::string& mode) { _mode = mode; }

// Channel operations
void Client::addChannel(Channel* channel) {
    if (channel)
        _channels.insert(channel);
}

void Client::removeChannel(Channel* channel) {
    if (channel)
        _channels.erase(channel);
}

bool Client::isInChannel(const std::string& channelName) const {
    for (std::set<Channel*>::const_iterator it = _channels.begin(); it != _channels.end(); ++it) {
        if ((*it)->getName() == channelName)
            return true;
    }
    return false;
}

// Buffer operations
void Client::appendToBuffer(const std::string& data) {
    _buffer += data;
}

void Client::clearBuffer() {
    _buffer.clear();
}

bool Client::hasCompleteCommand() const {
    return _buffer.find("\r\n") != std::string::npos;
}

std::string Client::getNextCommand() {
    size_t pos = _buffer.find("\r\n");
    if (pos == std::string::npos)
        return "";

    std::string command = _buffer.substr(0, pos);
    _buffer = _buffer.substr(pos + 2);
    return command;
}

// Mode operations
bool Client::hasMode(char mode) const {
    return _mode.find(mode) != std::string::npos;
}

void Client::addMode(char mode) {
    if (!hasMode(mode))
        _mode += mode;
}

void Client::removeMode(char mode) {
    size_t pos = _mode.find(mode);
    if (pos != std::string::npos)
        _mode.erase(pos, 1);
} 