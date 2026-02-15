#include "networking/ClientConnection.h"

#include <chrono>
#include <cstring>

#ifdef _WIN32
    #ifndef NOMINMAX
    #define NOMINMAX
    #endif
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    #define close closesocket
// Windows doesn't have ssize_t, define it
typedef int ssize_t;
#else
    #include <sys/socket.h>

    #include <unistd.h>
#endif

namespace fresh
{
namespace networking
{

ClientConnection::ClientConnection(int socketFd, const std::string& address)
    : socketFd(socketFd), address(address), clientId(0), connected(true), lastActivityTime(0.0)
{
    updateLastActivity();
}

ClientConnection::~ClientConnection()
{
    disconnect();
}

bool ClientConnection::sendMessage(const NetworkMessage& message)
{
    if (!connected) {
        return false;
    }

    auto serialized = message.serialize();
    bool success = sendBytes(serialized.data(), serialized.size());

    if (success) {
        updateLastActivity();
    }

    return success;
}

std::unique_ptr<NetworkMessage> ClientConnection::receiveMessage()
{
    if (!connected) {
        return nullptr;
    }

    // First, read the header (6 bytes: 2 for type, 4 for size)
    auto header = receiveBytes(6);
    if (header.size() != 6) {
        return nullptr;
    }

    // Read message size from header
    uint32_t dataSize = header[2] | (static_cast<uint32_t>(header[3]) << 8) |
                        (static_cast<uint32_t>(header[4]) << 16) |
                        (static_cast<uint32_t>(header[5]) << 24);

    // Read the remaining data
    auto data = receiveBytes(dataSize);
    if (data.size() != dataSize) {
        return nullptr;
    }

    // Combine header and data for deserialization
    std::vector<uint8_t> fullMessage = header;
    fullMessage.insert(fullMessage.end(), data.begin(), data.end());

    updateLastActivity();

    return NetworkMessage::deserialize(fullMessage.data(), fullMessage.size());
}

void ClientConnection::disconnect()
{
    if (connected) {
        connected = false;
        if (socketFd >= 0) {
            close(socketFd);
            socketFd = -1;
        }
    }
}

void ClientConnection::updateLastActivity()
{
    auto now = std::chrono::steady_clock::now();
    auto duration = now.time_since_epoch();
    lastActivityTime = std::chrono::duration<double>(duration).count();
}

double ClientConnection::getTimeSinceLastActivity() const
{
    auto now = std::chrono::steady_clock::now();
    auto duration = now.time_since_epoch();
    double currentTime = std::chrono::duration<double>(duration).count();
    return currentTime - lastActivityTime;
}

bool ClientConnection::sendBytes(const uint8_t* data, size_t length)
{
    size_t totalSent = 0;

    while (totalSent < length) {
        ssize_t sent = send(socketFd, reinterpret_cast<const char*>(data + totalSent),
                            static_cast<int>(length - totalSent), 0);
        if (sent <= 0) {
            connected = false;
            return false;
        }
        totalSent += sent;
    }

    return true;
}

std::vector<uint8_t> ClientConnection::receiveBytes(size_t length)
{
    std::vector<uint8_t> buffer(length);
    size_t totalReceived = 0;

    while (totalReceived < length) {
        ssize_t received = recv(socketFd, reinterpret_cast<char*>(buffer.data() + totalReceived),
                                static_cast<int>(length - totalReceived), 0);
        if (received <= 0) {
            connected = false;
            return {};
        }
        totalReceived += received;
    }

    return buffer;
}

} // namespace networking
} // namespace fresh
