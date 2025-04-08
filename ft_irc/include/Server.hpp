#ifndef SERVER_HPP
#define SERVER_HPP

#include "IRC.hpp"
#include "Client.hpp"
#include "Channel.hpp"
#include <vector>

class Server {
private:
    int _serverSocket;
    std::string _password;
    std::vector<pollfd> _pollfds;
    ClientMap _clients;
    ChannelMap _channels;
    bool _running;

    // Private methods
    void setupServer(int port);
    void handleNewConnection();
    void handleClientData(Client* client);
    void handleClientDisconnect(Client* client);
    void processCommand(Client* client, const std::string& command);
    void executeCommand(Client* client, const std::string& command, const std::vector<std::string>& args);
    void broadcastToAll(const std::string& message, Client* sender = NULL);

public:
    Server(int port, const std::string& password);
    ~Server();

    // Server operations
    void start();
    void stop();
    void run();

    // Client operations
    void addClient(int fd);
    void removeClient(Client* client);
    Client* getClient(const std::string& nickname);
    bool isNicknameInUse(const std::string& nickname) const;

    // Channel operations
    Channel* getChannel(const std::string& name);
    Channel* createChannel(const std::string& name);
    void removeChannel(Channel* channel);
    bool isChannelNameValid(const std::string& name) const;

    // Command handlers
    void handlePass(Client* client, const std::vector<std::string>& args);
    void handleNick(Client* client, const std::vector<std::string>& args);
    void handleUser(Client* client, const std::vector<std::string>& args);
    void handleQuit(Client* client, const std::vector<std::string>& args);
    void handleJoin(Client* client, const std::vector<std::string>& args);
    void handlePart(Client* client, const std::vector<std::string>& args);
    void handlePrivmsg(Client* client, const std::vector<std::string>& args);
    void handleNotice(Client* client, const std::vector<std::string>& args);
    void handleKick(Client* client, const std::vector<std::string>& args);
    void handleInvite(Client* client, const std::vector<std::string>& args);
    void handleTopic(Client* client, const std::vector<std::string>& args);
    void handleMode(Client* client, const std::vector<std::string>& args);
    void handlePing(Client* client, const std::vector<std::string>& args);
    void handlePong(Client* client, const std::vector<std::string>& args);
};

#endif // SERVER_HPP 