#include "../include/Server.hpp"
#include "../include/Client.hpp"
#include "../include/Channel.hpp"
#include "../include/Command.hpp"
#include "../include/Utils.hpp"
#include <iostream>
#include <sstream>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

Server::Server(int port, const std::string& password) : _password(password), _running(false) {
    setupServer(port);
}

Server::~Server() {
    // Clean up clients
    for (ClientMap::iterator it = _clients.begin(); it != _clients.end(); ++it) {
        delete it->second;
    }
    _clients.clear();

    // Clean up channels
    for (ChannelMap::iterator it = _channels.begin(); it != _channels.end(); ++it) {
        delete it->second;
    }
    _channels.clear();

    // Close server socket
    if (_serverSocket != -1)
        close(_serverSocket);
}

void Server::setupServer(int port) {
    // Create socket
    _serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (_serverSocket == -1)
        throw std::runtime_error("Failed to create socket");

    // Set socket options
    int opt = 1;
    if (setsockopt(_serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
        throw std::runtime_error("Failed to set socket options");

    // Set non-blocking mode
    Utils::setNonBlocking(_serverSocket);

    // Bind socket
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    if (bind(_serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1)
        throw std::runtime_error("Failed to bind socket");

    // Listen for connections
    if (listen(_serverSocket, SOMAXCONN) == -1)
        throw std::runtime_error("Failed to listen on socket");

    // Add server socket to poll set
    struct pollfd pfd;
    pfd.fd = _serverSocket;
    pfd.events = POLLIN;
    _pollfds.push_back(pfd);
}

void Server::start() {
    _running = true;
    std::cout << "Server started on port " << ntohs(((struct sockaddr_in*)&_serverSocket)->sin_port) << std::endl;
}

void Server::stop() {
    _running = false;
}

void Server::run() {
    while (_running) {
        int ready = poll(_pollfds.data(), _pollfds.size(), -1);
        if (ready == -1) {
            if (errno == EINTR)
                continue;
            throw std::runtime_error("Poll failed");
        }

        for (size_t i = 0; i < _pollfds.size(); ++i) {
            if (_pollfds[i].revents & POLLIN) {
                if (_pollfds[i].fd == _serverSocket) {
                    handleNewConnection();
                } else {
                    Client* client = _clients[_pollfds[i].fd];
                    if (client)
                        handleClientData(client);
                }
            }
        }
    }
}

void Server::handleNewConnection() {
    struct sockaddr_in clientAddr;
    socklen_t clientLen = sizeof(clientAddr);
    int clientFd = accept(_serverSocket, (struct sockaddr*)&clientAddr, &clientLen);

    if (clientFd == -1) {
        if (errno != EAGAIN && errno != EWOULDBLOCK)
            std::cerr << "Failed to accept connection" << std::endl;
        return;
    }

    // Set non-blocking mode
    Utils::setNonBlocking(clientFd);

    // Add to poll set
    struct pollfd pfd;
    pfd.fd = clientFd;
    pfd.events = POLLIN;
    _pollfds.push_back(pfd);

    // Create new client
    addClient(clientFd);
}

void Server::handleClientData(Client* client) {
    char buffer[BUFFER_SIZE];
    ssize_t bytesRead = recv(client->getFd(), buffer, BUFFER_SIZE - 1, 0);

    if (bytesRead <= 0) {
        if (bytesRead == 0 || (bytesRead == -1 && (errno == EAGAIN || errno == EWOULDBLOCK))) {
            handleClientDisconnect(client);
        }
        return;
    }

    buffer[bytesRead] = '\0';
    client->appendToBuffer(buffer);

    while (client->hasCompleteCommand()) {
        std::string command = client->getNextCommand();
        processCommand(client, command);
    }
}

void Server::handleClientDisconnect(Client* client) {
    // Remove from poll set
    for (std::vector<pollfd>::iterator it = _pollfds.begin(); it != _pollfds.end(); ++it) {
        if (it->fd == client->getFd()) {
            _pollfds.erase(it);
            break;
        }
    }

    // Remove client
    removeClient(client);
}

void Server::processCommand(Client* client, const std::string& command) {
    Command cmd(command, client, this);
    cmd.execute();
}

void Server::addClient(int fd) {
    Client* client = new Client(fd);
    _clients[fd] = client;
}

void Server::removeClient(Client* client) {
    if (!client)
        return;

    // Remove from poll set
    for (std::vector<pollfd>::iterator it = _pollfds.begin(); it != _pollfds.end(); ++it) {
        if (it->fd == client->getFd()) {
            _pollfds.erase(it);
            break;
        }
    }

    // Remove from clients map
    _clients.erase(client->getFd());

    // Close socket
    close(client->getFd());

    // Delete client
    delete client;
}

Client* Server::getClient(const std::string& nickname) {
    for (ClientMap::iterator it = _clients.begin(); it != _clients.end(); ++it) {
        if (it->second->getNickname() == nickname)
            return it->second;
    }
    return NULL;
}

bool Server::isNicknameInUse(const std::string& nickname) const {
    for (ClientMap::const_iterator it = _clients.begin(); it != _clients.end(); ++it) {
        if (it->second->getNickname() == nickname)
            return true;
    }
    return false;
}

Channel* Server::getChannel(const std::string& name) {
    ChannelMap::iterator it = _channels.find(name);
    return it != _channels.end() ? it->second : NULL;
}

Channel* Server::createChannel(const std::string& name) {
    Channel* channel = new Channel(name);
    _channels[name] = channel;
    return channel;
}

void Server::removeChannel(Channel* channel) {
    if (!channel)
        return;

    _channels.erase(channel->getName());
    delete channel;
}

bool Server::isChannelNameValid(const std::string& name) const {
    return Utils::isValidChannelName(name);
}

void Server::broadcastToAll(const std::string& message, Client* sender) {
    for (ClientMap::iterator it = _clients.begin(); it != _clients.end(); ++it) {
        if (it->second != sender) {
            send(it->second->getFd(), message.c_str(), message.length(), 0);
        }
    }
}

const std::string& Server::getPassword() const {
    return _password;
} 