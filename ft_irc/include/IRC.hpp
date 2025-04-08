#ifndef IRC_HPP
#define IRC_HPP

#include <string>
#include <vector>
#include <map>
#include <set>
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>

#define MAX_CLIENTS 1024
#define BUFFER_SIZE 512
#define SERVER_NAME "irc.42.fr"
#define SERVER_VERSION "1.0"

// Forward declarations
class Client;
class Channel;
class Server;

// Common types
typedef std::map<std::string, Channel*> ChannelMap;
typedef std::map<int, Client*> ClientMap;
typedef std::set<Client*> ClientSet;

// Error codes
#define ERR_NOSUCHNICK(nick) "401 " + nick + " :No such nick/channel"
#define ERR_NOSUCHCHANNEL(channel) "403 " + channel + " :No such channel"
#define ERR_CANNOTSENDTOCHAN(channel) "404 " + channel + " :Cannot send to channel"
#define ERR_NORECIPIENT(command) "411 :No recipient given (" + command + ")"
#define ERR_NOTEXTTOSEND "412 :No text to send"
#define ERR_UNKNOWNCOMMAND(command) "421 " + command + " :Unknown command"
#define ERR_NONICKNAMEGIVEN "431 :No nickname given"
#define ERR_ERRONEUSNICKNAME(nick) "432 " + nick + " :Erroneous nickname"
#define ERR_NICKNAMEINUSE(nick) "433 " + nick + " :Nickname is already in use"
#define ERR_NOTONCHANNEL(channel) "442 " + channel + " :You're not on that channel"
#define ERR_NEEDMOREPARAMS(command) "461 " + command + " :Not enough parameters"
#define ERR_ALREADYREGISTERED "462 :You may not reregister"
#define ERR_PASSWDMISMATCH "464 :Password incorrect"
#define ERR_CHANNELISFULL(channel) "471 " + channel + " :Cannot join channel (+l)"
#define ERR_INVITEONLYCHAN(channel) "473 " + channel + " :Cannot join channel (+i)"
#define ERR_BADCHANNELKEY(channel) "475 " + channel + " :Cannot join channel (+k)"
#define ERR_CHANOPRIVSNEEDED(channel) "482 " + channel + " :You're not channel operator"

// Reply codes
#define RPL_WELCOME(nick) "001 " + nick + " :Welcome to the IRC Network " + nick + "!" + nick + "@" + SERVER_NAME
#define RPL_YOURHOST(servername, version) "002 :Your host is " + servername + ", running version " + version
#define RPL_CREATED(date) "003 :This server was created " + date
#define RPL_MYINFO(servername, version, usermodes, chanmodes) "004 " + servername + " " + version + " " + usermodes + " " + chanmodes
#define RPL_TOPIC(channel, topic) "332 " + channel + " :" + topic
#define RPL_NOTOPIC(channel) "331 " + channel + " :No topic is set"
#define RPL_NAMREPLY(channel, names) "353 " + channel + " :" + names
#define RPL_ENDOFNAMES(channel) "366 " + channel + " :End of /NAMES list"
#define RPL_MOTD(text) "372 :- " + text
#define RPL_MOTDSTART(servername) "375 :- " + servername + " Message of the day - "
#define RPL_ENDOFMOTD "376 :End of /MOTD command."

#endif // IRC_HPP 