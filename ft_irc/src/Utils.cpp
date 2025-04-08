#include "../include/Utils.hpp"
#include <algorithm>
#include <cctype>
#include <ctime>
#include <sstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

namespace Utils {
    std::string trim(const std::string& str) {
        size_t first = str.find_first_not_of(" \t\r\n");
        if (first == std::string::npos)
            return "";
        
        size_t last = str.find_last_not_of(" \t\r\n");
        return str.substr(first, (last - first + 1));
    }

    std::vector<std::string> split(const std::string& str, char delimiter) {
        std::vector<std::string> tokens;
        std::string token;
        std::istringstream tokenStream(str);
        
        while (std::getline(tokenStream, token, delimiter)) {
            token = trim(token);
            if (!token.empty())
                tokens.push_back(token);
        }
        
        return tokens;
    }

    std::string toUpper(const std::string& str) {
        std::string result = str;
        std::transform(result.begin(), result.end(), result.begin(), ::toupper);
        return result;
    }

    std::string toLower(const std::string& str) {
        std::string result = str;
        std::transform(result.begin(), result.end(), result.begin(), ::tolower);
        return result;
    }

    bool startsWith(const std::string& str, const std::string& prefix) {
        return str.size() >= prefix.size() && str.substr(0, prefix.size()) == prefix;
    }

    bool endsWith(const std::string& str, const std::string& suffix) {
        return str.size() >= suffix.size() && str.substr(str.size() - suffix.size()) == suffix;
    }

    bool isValidNickname(const std::string& nickname) {
        if (nickname.empty() || nickname.length() > 9)
            return false;
        
        // First character must be a letter
        if (!std::isalpha(nickname[0]))
            return false;

        // Remaining characters can be letters, numbers, or special characters
        for (size_t i = 1; i < nickname.length(); ++i) {
            char c = nickname[i];
            if (!std::isalnum(c) && c != '-' && c != '_' && c != '[' && c != ']' && c != '\\' && c != '`' && c != '^' && c != '{' && c != '|' && c != '}')
                return false;
        }
        return true;
    }

    bool isValidChannelName(const std::string& channelName) {
        if (channelName.empty() || channelName.length() > 50)
            return false;

        // Channel names must start with # or &
        if (channelName[0] != '#' && channelName[0] != '&')
            return false;

        // Remaining characters can be letters, numbers, or special characters
        for (size_t i = 1; i < channelName.length(); ++i) {
            char c = channelName[i];
            if (!std::isalnum(c) && c != '-' && c != '_' && c != '[' && c != ']' && c != '\\' && c != '`' && c != '^' && c != '{' && c != '|' && c != '}')
                return false;
        }
        return true;
    }

    std::string getCurrentTimestamp() {
        time_t now = time(NULL);
        struct tm* timeinfo = localtime(&now);
        char buffer[20];
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
        return std::string(buffer);
    }

    std::string formatMessage(const std::string& prefix, const std::string& command, const std::string& params) {
        std::string message;
        if (!prefix.empty())
            message += ":" + prefix + " ";
        message += command;
        if (!params.empty())
            message += " " + params;
        message += "\r\n";
        return message;
    }

    std::string formatReply(const std::string& code, const std::string& target, const std::string& message) {
        return ":" + SERVER_NAME + " " + code + " " + target + " :" + message + "\r\n";
    }

    void setNonBlocking(int fd) {
        int flags = fcntl(fd, F_GETFL, 0);
        if (flags == -1)
            throw std::runtime_error("Failed to get socket flags");
        
        if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
            throw std::runtime_error("Failed to set non-blocking mode");
    }

    std::string getHostname() {
        char hostname[256];
        if (gethostname(hostname, sizeof(hostname)) == -1)
            throw std::runtime_error("gethostname failed");
        return std::string(hostname);
    }

    std::string getIpAddress(int fd) {
        struct sockaddr_in addr;
        socklen_t addr_len = sizeof(addr);
        if (getsockname(fd, (struct sockaddr*)&addr, &addr_len) == -1)
            throw std::runtime_error("getsockname failed");
        return std::string(inet_ntoa(addr.sin_addr));
    }

    int getPort(int fd) {
        struct sockaddr_in addr;
        socklen_t addr_len = sizeof(addr);
        if (getsockname(fd, (struct sockaddr*)&addr, &addr_len) == -1)
            throw std::runtime_error("getsockname failed");
        return ntohs(addr.sin_port);
    }

    void handleError(const std::string& message) {
        std::cerr << "Error: " << message << std::endl;
    }

    void handleSignal(int signal) {
        if (signal == SIGINT || signal == SIGTERM) {
            std::cout << "\nReceived signal " << signal << ", shutting down..." << std::endl;
            exit(0);
        }
    }

    bool isValidMode(const std::string& mode) {
        if (mode.empty())
            return false;
        
        char prefix = mode[0];
        if (prefix != '+' && prefix != '-')
            return false;
        
        // Valid channel modes
        std::string validModes = "itkol";
        
        for (size_t i = 1; i < mode.length(); ++i) {
            if (validModes.find(mode[i]) == std::string::npos)
                return false;
        }
        
        return true;
    }

    std::string formatMessage(const std::string& prefix, const std::string& command, 
                            const std::string& target, const std::string& message) {
        std::stringstream ss;
        if (!prefix.empty())
            ss << ":" << prefix << " ";
        ss << command;
        if (!target.empty())
            ss << " " << target;
        if (!message.empty())
            ss << " :" << message;
        ss << "\r\n";
        return ss.str();
    }
} 