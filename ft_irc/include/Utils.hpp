#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <vector>

namespace Utils {
    // String operations
    std::string trim(const std::string& str);
    std::vector<std::string> split(const std::string& str, char delimiter);
    std::string toUpper(const std::string& str);
    std::string toLower(const std::string& str);
    bool startsWith(const std::string& str, const std::string& prefix);
    bool endsWith(const std::string& str, const std::string& suffix);

    // IRC specific
    bool isValidNickname(const std::string& nickname);
    bool isValidChannelName(const std::string& channelName);
    std::string getCurrentTimestamp();
    std::string formatMessage(const std::string& prefix, const std::string& command, const std::string& params);
    std::string formatReply(const std::string& code, const std::string& target, const std::string& message);

    // Network operations
    void setNonBlocking(int fd);
    std::string getHostname();
    std::string getIpAddress(int fd);
    int getPort(int fd);

    // Error handling
    void handleError(const std::string& message);
    void handleSignal(int signal);
}

#endif // UTILS_HPP 