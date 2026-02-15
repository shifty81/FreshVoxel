#pragma once
#include <atomic>
#include <map>
#include <memory>
#include <mutex>
#include <thread>

#include "../galaxy/GalaxySector.h"
#include "ClientConnection.h"
#include "SectorServer.h"

namespace fresh
{
namespace networking
{

/**
 * @brief Main server for handling multiplayer connections
 *
 * TCP-based client-server architecture with sector-based multiplayer
 * and multi-threaded sector handling for scalability.
 */
class GameServer
{
public:
    explicit GameServer(uint16_t port = 7777);
    ~GameServer();

    // Server lifecycle
    bool start();
    void stop();
    bool isRunning() const
    {
        return running;
    }

    // Update server (process messages, manage connections)
    void update(float deltaTime);

    // Configuration
    void setMaxClients(int max)
    {
        maxClients = max;
    }
    int getMaxClients() const
    {
        return maxClients;
    }
    int getConnectedClientCount() const;

    // Sector management
    SectorServer* getSectorServer(int x, int y);
    void createSectorServer(int x, int y);

    // Broadcasting
    void broadcastMessage(const NetworkMessage& message);
    void broadcastToSector(int sectorX, int sectorY, const NetworkMessage& message);

private:
    uint16_t port;
    int serverSocket;
    std::atomic<bool> running;
    int maxClients;
    uint32_t nextClientId;

    // Client management
    std::map<uint32_t, std::unique_ptr<ClientConnection>> clients;
    mutable std::mutex clientsMutex;

    // Sector servers (multi-threaded)
    std::map<std::pair<int, int>, std::unique_ptr<SectorServer>> sectorServers;
    mutable std::mutex sectorsMutex;

    // Threading
    std::thread acceptThread;
    std::vector<std::thread> workerThreads;

    // Internal methods
    void acceptConnectionsLoop();
    void handleClientMessages(ClientConnection* client);
    void processMessage(ClientConnection* client, std::unique_ptr<NetworkMessage> message);
    void disconnectClient(uint32_t clientId);

    // Socket operations
    bool createSocket();
    void closeSocket();
};

} // namespace networking
} // namespace fresh
