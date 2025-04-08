#ifndef COMMAND_HPP
#define COMMAND_HPP

#include "IRC.hpp"
#include <string>
#include <vector>

class Client;
class Server;

class Command {
private:
    std::string _name;
    std::vector<std::string> _args;
    Client* _client;
    Server* _server;

public:
    Command(const std::string& rawCommand, Client* client, Server* server);
    ~Command();

    // Getters
    const std::string& getName() const;
    const std::vector<std::string>& getArgs() const;
    Client* getClient() const;
    Server* getServer() const;

    // Command parsing
    static std::vector<std::string> parseCommand(const std::string& rawCommand);
    static bool isValidNickname(const std::string& nickname);
    static bool isValidChannelName(const std::string& channelName);

    // Command execution
    void execute();
    void executePass();
    void executeNick();
    void executeUser();
    void executeQuit();
    void executeJoin();
    void executePart();
    void executePrivmsg();
    void executeNotice();
    void executeKick();
    void executeInvite();
    void executeTopic();
    void executeMode();
    void executePing();
    void executePong();
};

#endif // COMMAND_HPP 