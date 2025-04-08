#include "../include/Command.hpp"
#include "../include/Client.hpp"
#include "../include/Server.hpp"
#include "../include/Utils.hpp"
#include <sstream>

Command::Command(const std::string& rawCommand, Client* client, Server* server)
    : _client(client), _server(server) {
    std::vector<std::string> parts = parseCommand(rawCommand);
    if (!parts.empty()) {
        _name = Utils::toUpper(parts[0]);
        _args.assign(parts.begin() + 1, parts.end());
    }
}

Command::~Command() {}

// Getters
const std::string& Command::getName() const { return _name; }
const std::vector<std::string>& Command::getArgs() const { return _args; }
Client* Command::getClient() const { return _client; }
Server* Command::getServer() const { return _server; }

// Command parsing
std::vector<std::string> Command::parseCommand(const std::string& rawCommand) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(rawCommand);
    
    while (std::getline(tokenStream, token, ' ')) {
        token = Utils::trim(token);
        if (!token.empty())
            tokens.push_back(token);
    }
    
    return tokens;
}

bool Command::isValidNickname(const std::string& nickname) {
    return Utils::isValidNickname(nickname);
}

bool Command::isValidChannelName(const std::string& channelName) {
    return Utils::isValidChannelName(channelName);
}

// Command execution
void Command::execute() {
    if (_name == "PASS") executePass();
    else if (_name == "NICK") executeNick();
    else if (_name == "USER") executeUser();
    else if (_name == "QUIT") executeQuit();
    else if (_name == "JOIN") executeJoin();
    else if (_name == "PART") executePart();
    else if (_name == "PRIVMSG") executePrivmsg();
    else if (_name == "NOTICE") executeNotice();
    else if (_name == "KICK") executeKick();
    else if (_name == "INVITE") executeInvite();
    else if (_name == "TOPIC") executeTopic();
    else if (_name == "MODE") executeMode();
    else if (_name == "PING") executePing();
    else if (_name == "PONG") executePong();
    else {
        std::string error = ERR_UNKNOWNCOMMAND(_name);
        send(_client->getFd(), error.c_str(), error.length(), 0);
    }
}

void Command::executePass() {
    if (_client->isAuthenticated()) {
        std::string error = ERR_ALREADYREGISTERED;
        send(_client->getFd(), error.c_str(), error.length(), 0);
        return;
    }

    if (_args.empty()) {
        std::string error = ERR_NEEDMOREPARAMS("PASS");
        send(_client->getFd(), error.c_str(), error.length(), 0);
        return;
    }

    if (_args[0] == _server->getPassword()) {
        _client->setAuthenticated(true);
    } else {
        std::string error = ERR_PASSWDMISMATCH;
        send(_client->getFd(), error.c_str(), error.length(), 0);
    }
}

void Command::executeNick() {
    if (_args.empty()) {
        std::string error = ERR_NONICKNAMEGIVEN;
        send(_client->getFd(), error.c_str(), error.length(), 0);
        return;
    }

    std::string newNick = _args[0];
    if (!isValidNickname(newNick)) {
        std::string error = ERR_ERRONEUSNICKNAME(newNick);
        send(_client->getFd(), error.c_str(), error.length(), 0);
        return;
    }

    if (_server->isNicknameInUse(newNick)) {
        std::string error = ERR_NICKNAMEINUSE(newNick);
        send(_client->getFd(), error.c_str(), error.length(), 0);
        return;
    }

    std::string oldNick = _client->getNickname();
    _client->setNickname(newNick);

    if (!oldNick.empty()) {
        std::string message = ":" + oldNick + " NICK " + newNick + "\r\n";
        _server->broadcastToAll(message, _client);
    }
}

void Command::executeUser() {
    if (_client->isRegistered()) {
        std::string error = ERR_ALREADYREGISTERED;
        send(_client->getFd(), error.c_str(), error.length(), 0);
        return;
    }

    if (_args.size() < 4) {
        std::string error = ERR_NEEDMOREPARAMS("USER");
        send(_client->getFd(), error.c_str(), error.length(), 0);
        return;
    }

    _client->setUsername(_args[0]);
    _client->setRealname(_args[3]);
    _client->setRegistered(true);

    // Send welcome messages
    std::string welcome = RPL_WELCOME(_client->getNickname());
    std::string yourHost = RPL_YOURHOST(SERVER_NAME, SERVER_VERSION);
    std::string created = RPL_CREATED(Utils::getCurrentTimestamp());
    std::string myInfo = RPL_MYINFO(SERVER_NAME, SERVER_VERSION, "aiwro", "Oov");

    send(_client->getFd(), welcome.c_str(), welcome.length(), 0);
    send(_client->getFd(), yourHost.c_str(), yourHost.length(), 0);
    send(_client->getFd(), created.c_str(), created.length(), 0);
    send(_client->getFd(), myInfo.c_str(), myInfo.length(), 0);
}

void Command::executeQuit() {
    std::string message = "QUIT";
    if (!_args.empty())
        message += " :" + _args[0];
    message += "\r\n";

    _server->broadcastToAll(message, _client);
    _server->removeClient(_client);
}

void Command::executeJoin() {
    if (_args.empty()) {
        std::string error = ERR_NEEDMOREPARAMS("JOIN");
        send(_client->getFd(), error.c_str(), error.length(), 0);
        return;
    }

    std::vector<std::string> channels = Utils::split(_args[0], ',');
    std::vector<std::string> keys = _args.size() > 1 ? Utils::split(_args[1], ',') : std::vector<std::string>();

    for (size_t i = 0; i < channels.size(); ++i) {
        std::string channelName = channels[i];
        if (!isValidChannelName(channelName)) {
            continue;
        }

        Channel* channel = _server->getChannel(channelName);
        if (!channel) {
            channel = _server->createChannel(channelName);
        }

        if (channel->isInviteOnly() && !channel->isOperator(_client)) {
            std::string error = ERR_INVITEONLYCHAN(channelName);
            send(_client->getFd(), error.c_str(), error.length(), 0);
            continue;
        }

        if (!channel->getKey().empty() && (i >= keys.size() || keys[i] != channel->getKey())) {
            std::string error = ERR_BADCHANNELKEY(channelName);
            send(_client->getFd(), error.c_str(), error.length(), 0);
            continue;
        }

        if (channel->getUserLimit() > 0 && channel->getClients().size() >= channel->getUserLimit()) {
            std::string error = ERR_CHANNELISFULL(channelName);
            send(_client->getFd(), error.c_str(), error.length(), 0);
            continue;
        }

        channel->addClient(_client);
        if (channel->getClients().size() == 1) {
            channel->addOperator(_client);
        }

        std::string joinMessage = ":" + _client->getNickname() + " JOIN " + channelName + "\r\n";
        channel->broadcast(joinMessage);

        // Send channel info
        std::string topic = channel->getTopic().empty() ? RPL_NOTOPIC(channelName) : RPL_TOPIC(channelName, channel->getTopic());
        send(_client->getFd(), topic.c_str(), topic.length(), 0);

        std::string names;
        for (std::set<Client*>::const_iterator it = channel->getClients().begin(); it != channel->getClients().end(); ++it) {
            if (it != channel->getClients().begin())
                names += " ";
            if (channel->isOperator(*it))
                names += "@";
            names += (*it)->getNickname();
        }
        std::string namesReply = RPL_NAMREPLY(channelName, names);
        std::string endNames = RPL_ENDOFNAMES(channelName);
        send(_client->getFd(), namesReply.c_str(), namesReply.length(), 0);
        send(_client->getFd(), endNames.c_str(), endNames.length(), 0);
    }
}

void Command::executePart() {
    if (_args.empty()) {
        std::string error = ERR_NEEDMOREPARAMS("PART");
        send(_client->getFd(), error.c_str(), error.length(), 0);
        return;
    }

    std::vector<std::string> channels = Utils::split(_args[0], ',');
    std::string reason = _args.size() > 1 ? _args[1] : "";

    for (size_t i = 0; i < channels.size(); ++i) {
        Channel* channel = _server->getChannel(channels[i]);
        if (!channel) {
            std::string error = ERR_NOSUCHCHANNEL(channels[i]);
            send(_client->getFd(), error.c_str(), error.length(), 0);
            continue;
        }

        if (!channel->hasClient(_client)) {
            std::string error = ERR_NOTONCHANNEL(channels[i]);
            send(_client->getFd(), error.c_str(), error.length(), 0);
            continue;
        }

        std::string partMessage = ":" + _client->getNickname() + " PART " + channels[i];
        if (!reason.empty())
            partMessage += " :" + reason;
        partMessage += "\r\n";

        channel->broadcast(partMessage);
        channel->removeClient(_client);
    }
}

void Command::executePrivmsg() {
    if (_args.empty()) {
        std::string error = ERR_NORECIPIENT("PRIVMSG");
        send(_client->getFd(), error.c_str(), error.length(), 0);
        return;
    }

    if (_args.size() < 2) {
        std::string error = ERR_NOTEXTTOSEND;
        send(_client->getFd(), error.c_str(), error.length(), 0);
        return;
    }

    std::vector<std::string> targets = Utils::split(_args[0], ',');
    std::string message = _args[1];

    for (size_t i = 0; i < targets.size(); ++i) {
        if (targets[i][0] == '#' || targets[i][0] == '&') {
            Channel* channel = _server->getChannel(targets[i]);
            if (!channel) {
                std::string error = ERR_NOSUCHCHANNEL(targets[i]);
                send(_client->getFd(), error.c_str(), error.length(), 0);
                continue;
            }

            if (!channel->hasClient(_client)) {
                std::string error = ERR_CANNOTSENDTOCHAN(targets[i]);
                send(_client->getFd(), error.c_str(), error.length(), 0);
                continue;
            }

            channel->broadcast("PRIVMSG " + targets[i] + " :" + message, _client);
        } else {
            Client* target = _server->getClient(targets[i]);
            if (!target) {
                std::string error = ERR_NOSUCHNICK(targets[i]);
                send(_client->getFd(), error.c_str(), error.length(), 0);
                continue;
            }

            std::string privmsg = ":" + _client->getNickname() + " PRIVMSG " + targets[i] + " :" + message + "\r\n";
            send(target->getFd(), privmsg.c_str(), privmsg.length(), 0);
        }
    }
}

void Command::executeNotice() {
    // Similar to PRIVMSG but without error replies
    if (_args.size() < 2)
        return;

    std::vector<std::string> targets = Utils::split(_args[0], ',');
    std::string message = _args[1];

    for (size_t i = 0; i < targets.size(); ++i) {
        if (targets[i][0] == '#' || targets[i][0] == '&') {
            Channel* channel = _server->getChannel(targets[i]);
            if (channel && channel->hasClient(_client)) {
                channel->broadcast("NOTICE " + targets[i] + " :" + message, _client);
            }
        } else {
            Client* target = _server->getClient(targets[i]);
            if (target) {
                std::string notice = ":" + _client->getNickname() + " NOTICE " + targets[i] + " :" + message + "\r\n";
                send(target->getFd(), notice.c_str(), notice.length(), 0);
            }
        }
    }
}

void Command::executeKick() {
    if (_args.size() < 2) {
        std::string error = ERR_NEEDMOREPARAMS("KICK");
        send(_client->getFd(), error.c_str(), error.length(), 0);
        return;
    }

    Channel* channel = _server->getChannel(_args[0]);
    if (!channel) {
        std::string error = ERR_NOSUCHCHANNEL(_args[0]);
        send(_client->getFd(), error.c_str(), error.length(), 0);
        return;
    }

    if (!channel->isOperator(_client)) {
        std::string error = ERR_CHANOPRIVSNEEDED(_args[0]);
        send(_client->getFd(), error.c_str(), error.length(), 0);
        return;
    }

    Client* target = _server->getClient(_args[1]);
    if (!target || !channel->hasClient(target)) {
        std::string error = ERR_NOTONCHANNEL(_args[0]);
        send(_client->getFd(), error.c_str(), error.length(), 0);
        return;
    }

    std::string reason = _args.size() > 2 ? _args[2] : _client->getNickname();
    std::string kickMessage = ":" + _client->getNickname() + " KICK " + _args[0] + " " + _args[1] + " :" + reason + "\r\n";
    channel->broadcast(kickMessage);
    channel->removeClient(target);
}

void Command::executeInvite() {
    if (_args.size() < 2) {
        std::string error = ERR_NEEDMOREPARAMS("INVITE");
        send(_client->getFd(), error.c_str(), error.length(), 0);
        return;
    }

    Client* target = _server->getClient(_args[0]);
    if (!target) {
        std::string error = ERR_NOSUCHNICK(_args[0]);
        send(_client->getFd(), error.c_str(), error.length(), 0);
        return;
    }

    Channel* channel = _server->getChannel(_args[1]);
    if (!channel) {
        std::string error = ERR_NOSUCHCHANNEL(_args[1]);
        send(_client->getFd(), error.c_str(), error.length(), 0);
        return;
    }

    if (!channel->isOperator(_client)) {
        std::string error = ERR_CHANOPRIVSNEEDED(_args[1]);
        send(_client->getFd(), error.c_str(), error.length(), 0);
        return;
    }

    if (channel->hasClient(target)) {
        std::string error = "443 " + _args[0] + " " + _args[1] + " :is already on channel";
        send(_client->getFd(), error.c_str(), error.length(), 0);
        return;
    }

    std::string inviteMessage = ":" + _client->getNickname() + " INVITE " + _args[0] + " " + _args[1] + "\r\n";
    send(target->getFd(), inviteMessage.c_str(), inviteMessage.length(), 0);
}

void Command::executeTopic() {
    if (_args.empty()) {
        std::string error = ERR_NEEDMOREPARAMS("TOPIC");
        send(_client->getFd(), error.c_str(), error.length(), 0);
        return;
    }

    Channel* channel = _server->getChannel(_args[0]);
    if (!channel) {
        std::string error = ERR_NOSUCHCHANNEL(_args[0]);
        send(_client->getFd(), error.c_str(), error.length(), 0);
        return;
    }

    if (!channel->hasClient(_client)) {
        std::string error = ERR_NOTONCHANNEL(_args[0]);
        send(_client->getFd(), error.c_str(), error.length(), 0);
        return;
    }

    if (_args.size() == 1) {
        std::string reply = channel->getTopic().empty() ? 
            RPL_NOTOPIC(_args[0]) : RPL_TOPIC(_args[0], channel->getTopic());
        send(_client->getFd(), reply.c_str(), reply.length(), 0);
        return;
    }

    if (channel->hasMode('t') && !channel->isOperator(_client)) {
        std::string error = ERR_CHANOPRIVSNEEDED(_args[0]);
        send(_client->getFd(), error.c_str(), error.length(), 0);
        return;
    }

    std::string newTopic = _args[1];
    channel->setTopic(newTopic);

    std::string topicMessage = ":" + _client->getNickname() + " TOPIC " + _args[0] + " :" + newTopic + "\r\n";
    channel->broadcast(topicMessage);
}

void Command::executeMode() {
    if (_args.empty()) {
        std::string error = ERR_NEEDMOREPARAMS("MODE");
        send(_client->getFd(), error.c_str(), error.length(), 0);
        return;
    }

    if (_args[0][0] == '#' || _args[0][0] == '&') {
        Channel* channel = _server->getChannel(_args[0]);
        if (!channel) {
            std::string error = ERR_NOSUCHCHANNEL(_args[0]);
            send(_client->getFd(), error.c_str(), error.length(), 0);
            return;
        }

        if (!channel->isOperator(_client)) {
            std::string error = ERR_CHANOPRIVSNEEDED(_args[0]);
            send(_client->getFd(), error.c_str(), error.length(), 0);
            return;
        }

        if (_args.size() == 1) {
            std::string modeReply = RPL_CHANNELMODEIS(_args[0], channel->getMode());
            send(_client->getFd(), modeReply.c_str(), modeReply.length(), 0);
            return;
        }

        std::string modes = _args[1];
        bool adding = true;
        std::string modeChanges;

        for (size_t i = 0; i < modes.length(); ++i) {
            if (modes[i] == '+')
                adding = true;
            else if (modes[i] == '-')
                adding = false;
            else {
                switch (modes[i]) {
                    case 'i':
                        if (adding)
                            channel->addMode('i');
                        else
                            channel->removeMode('i');
                        modeChanges += (adding ? "+" : "-") + std::string(1, modes[i]);
                        break;
                    case 't':
                        if (adding)
                            channel->addMode('t');
                        else
                            channel->removeMode('t');
                        modeChanges += (adding ? "+" : "-") + std::string(1, modes[i]);
                        break;
                    case 'k':
                        if (adding) {
                            if (i + 1 < modes.length()) {
                                channel->setKey(modes.substr(i + 1));
                                modeChanges += "+k " + modes.substr(i + 1);
                                i++;
                            }
                        } else {
                            channel->setKey("");
                            modeChanges += "-k";
                        }
                        break;
                    case 'o':
                        if (i + 1 < modes.length()) {
                            Client* target = _server->getClient(modes.substr(i + 1));
                            if (target) {
                                if (adding)
                                    channel->addOperator(target);
                                else
                                    channel->removeOperator(target);
                                modeChanges += (adding ? "+" : "-") + std::string(1, modes[i]) + " " + target->getNickname();
                            }
                            i++;
                        }
                        break;
                    case 'l':
                        if (adding) {
                            if (i + 1 < modes.length()) {
                                size_t limit = std::atoi(modes.substr(i + 1).c_str());
                                channel->setUserLimit(limit);
                                modeChanges += "+l " + modes.substr(i + 1);
                                i++;
                            }
                        } else {
                            channel->setUserLimit(0);
                            modeChanges += "-l";
                        }
                        break;
                }
            }
        }

        if (!modeChanges.empty()) {
            std::string modeMessage = ":" + _client->getNickname() + " MODE " + _args[0] + " " + modeChanges + "\r\n";
            channel->broadcast(modeMessage);
        }
    } else {
        // User modes (not implemented in this basic version)
        std::string error = ERR_USERSDONTMATCH;
        send(_client->getFd(), error.c_str(), error.length(), 0);
    }
}

void Command::executePing() {
    if (_args.empty())
        return;

    std::string pong = "PONG " + SERVER_NAME + " :" + _args[0] + "\r\n";
    send(_client->getFd(), pong.c_str(), pong.length(), 0);
}

void Command::executePong() {
    // PONG is just acknowledged, no response needed
} 