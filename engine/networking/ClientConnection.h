#pragma once
#include <functional>
#include <memory>
#include <string>

#include "NetworkMessage.h"

namespace fresh
{
namespace networking
{

/**
 * @brief Callback type for receiving messages
 */
using MessageCallback = std::function<void(std::unique_ptr<NetworkMessage>)>;

/**
 * @brief Represents a connected client
 */
class ClientConnection
{
public:
    ClientConnection(int socketFd, const std::string& address);
    ~ClientConnection();

    // Connection info
    int getSocketFd() const
    {
        return socketFd;
    }
    const std::string& getAddress() const
    {
        return address;
    }
    uint32_t getClientId() const
    {
        return clientId;
    }
    void setClientId(uint32_t id)
    {
        clientId = id;
    }

    // Player info
    const std::string& getPlayerName() const
    {
        return playerName;
    }
    void setPlayerName(const std::string& name)
    {
        playerName = name;
    }

    // Send/receive messages
    bool sendMessage(const NetworkMessage& message);
    std::unique_ptr<NetworkMessage> receiveMessage();

    // Connection state
    bool isConnected() const
    {
        return connected;
    }
    void disconnect();

    // Timeout management
    void updateLastActivity();
    double getTimeSinceLastActivity() const;

private:
    int socketFd;
    std::string address;
    uint32_t clientId;
    std::string playerName;
    bool connected;
    double lastActivityTime;

    bool sendBytes(const uint8_t* data, size_t length);
    std::vector<uint8_t> receiveBytes(size_t length);
};

} // namespace networking
} // namespace fresh
