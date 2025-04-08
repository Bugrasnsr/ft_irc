#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "IRC.hpp"
#include <string>

class Channel;

class Client {
private:
    int _fd;
    std::string _nickname;
    std::string _username;
    std::string _realname;
    std::string _hostname;
    std::string _buffer;
    bool _registered;
    bool _authenticated;
    std::set<Channel*> _channels;
    std::string _mode;

public:
    Client(int fd);
    ~Client();

    // Getters
    int getFd() const;
    const std::string& getNickname() const;
    const std::string& getUsername() const;
    const std::string& getRealname() const;
    const std::string& getHostname() const;
    const std::string& getBuffer() const;
    bool isRegistered() const;
    bool isAuthenticated() const;
    const std::set<Channel*>& getChannels() const;
    const std::string& getMode() const;

    // Setters
    void setNickname(const std::string& nickname);
    void setUsername(const std::string& username);
    void setRealname(const std::string& realname);
    void setHostname(const std::string& hostname);
    void setRegistered(bool registered);
    void setAuthenticated(bool authenticated);
    void setMode(const std::string& mode);

    // Channel operations
    void addChannel(Channel* channel);
    void removeChannel(Channel* channel);
    bool isInChannel(const std::string& channelName) const;

    // Buffer operations
    void appendToBuffer(const std::string& data);
    void clearBuffer();
    bool hasCompleteCommand() const;
    std::string getNextCommand();

    // Mode operations
    bool hasMode(char mode) const;
    void addMode(char mode);
    void removeMode(char mode);
};

#endif // CLIENT_HPP 