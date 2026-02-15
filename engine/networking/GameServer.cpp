#include "networking/GameServer.h"

#include <cstring>

#ifdef _WIN32
    #ifndef NOMINMAX
    #define NOMINMAX
    #endif
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    #define close closesocket
typedef int socklen_t;
#else
    #include <arpa/inet.h>
    #include <netinet/in.h>
    #include <sys/socket.h>

    #include <fcntl.h>
    #include <unistd.h>
#endif

namespace fresh
{
namespace networking
{

GameServer::GameServer(uint16_t port)
    : port(port), serverSocket(-1), running(false), maxClients(100), nextClientId(1)
{
}

GameServer::~GameServer()
{
    stop();
}

bool GameServer::start()
{
    if (running) {
        return true;
    }

    if (!createSocket()) {
        return false;
    }

    running = true;

    // Start accept thread
    acceptThread = std::thread(&GameServer::acceptConnectionsLoop, this);

    return true;
}

void GameServer::stop()
{
    if (!running) {
        return;
    }

    running = false;

    // Close all client connections
    {
        std::lock_guard<std::mutex> lock(clientsMutex);
        clients.clear();
    }

    // Close server socket
    closeSocket();

    // Wait for threads to finish
    if (acceptThread.joinable()) {
        acceptThread.join();
    }

    for (auto& thread : workerThreads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    workerThreads.clear();
}

void GameServer::update(float deltaTime)
{
    if (!running) {
        return;
    }

    // Update all sector servers
    std::lock_guard<std::mutex> lock(sectorsMutex);
    for (auto& pair : sectorServers) {
        pair.second->update(deltaTime);
    }
}

int GameServer::getConnectedClientCount() const
{
    std::lock_guard<std::mutex> lock(clientsMutex);
    return static_cast<int>(clients.size());
}

SectorServer* GameServer::getSectorServer(int x, int y)
{
    std::lock_guard<std::mutex> lock(sectorsMutex);
    auto key = std::make_pair(x, y);
    auto it = sectorServers.find(key);
    return (it != sectorServers.end()) ? it->second.get() : nullptr;
}

void GameServer::createSectorServer(int x, int y)
{
    std::lock_guard<std::mutex> lock(sectorsMutex);
    auto key = std::make_pair(x, y);
    if (sectorServers.find(key) == sectorServers.end()) {
        sectorServers[key] = std::make_unique<SectorServer>(x, y);
    }
}

void GameServer::broadcastMessage(const NetworkMessage& message)
{
    std::lock_guard<std::mutex> lock(clientsMutex);
    for (auto& pair : clients) {
        pair.second->sendMessage(message);
    }
}

void GameServer::broadcastToSector(int sectorX, int sectorY, const NetworkMessage& message)
{
    auto* sector = getSectorServer(sectorX, sectorY);
    if (sector) {
        sector->broadcastToPlayers(message);
    }
}

void GameServer::acceptConnectionsLoop()
{
    while (running) {
        struct sockaddr_in clientAddr;
        socklen_t clientAddrLen = sizeof(clientAddr);

        int clientSocket =
            static_cast<int>(accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen));

        if (clientSocket < 0) {
            continue;
        }

        // Check if we can accept more clients
        if (getConnectedClientCount() >= maxClients) {
            close(clientSocket);
            continue;
        }

        // Create client connection
        char addrBuffer[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(clientAddr.sin_addr), addrBuffer, INET_ADDRSTRLEN);
        std::string address = addrBuffer;
        auto client = std::make_unique<ClientConnection>(clientSocket, address);
        client->setClientId(nextClientId++);

        uint32_t clientId = client->getClientId();

        // Add to clients map
        {
            std::lock_guard<std::mutex> lock(clientsMutex);
            clients[clientId] = std::move(client);
        }

        // Start worker thread for this client
        workerThreads.emplace_back([this, clientId]() {
            ClientConnection* client = nullptr;
            {
                std::lock_guard<std::mutex> lock(clientsMutex);
                auto it = clients.find(clientId);
                if (it != clients.end()) {
                    client = it->second.get();
                }
            }

            if (client) {
                handleClientMessages(client);
            }
        });
    }
}

void GameServer::handleClientMessages(ClientConnection* client)
{
    while (running && client->isConnected()) {
        auto message = client->receiveMessage();
        if (message) {
            processMessage(client, std::move(message));
        } else {
            break;
        }
    }

    // Disconnect client
    disconnectClient(client->getClientId());
}

void GameServer::processMessage(ClientConnection* client, std::unique_ptr<NetworkMessage> message)
{
    (void)client; // Unused - placeholder for future implementation
    // Process different message types
    switch (message->getType()) {
    case MessageType::Connect:
        // Handle connection
        break;

    case MessageType::ChatMessage:
        // Broadcast chat message to all clients
        broadcastMessage(*message);
        break;

    case MessageType::SectorChange:
        // Handle sector change
        break;

    default:
        // Forward to appropriate sector server
        break;
    }
}

void GameServer::disconnectClient(uint32_t clientId)
{
    std::lock_guard<std::mutex> lock(clientsMutex);
    clients.erase(clientId);
}

bool GameServer::createSocket()
{
    serverSocket = static_cast<int>(socket(AF_INET, SOCK_STREAM, 0));
    if (serverSocket < 0) {
        return false;
    }

    // Set socket options
    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&opt),
               sizeof(opt));

    // Bind socket
    struct sockaddr_in serverAddr;
    std::memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        close(serverSocket);
        serverSocket = -1;
        return false;
    }

    // Listen for connections
    if (listen(serverSocket, 10) < 0) {
        close(serverSocket);
        serverSocket = -1;
        return false;
    }

    return true;
}

void GameServer::closeSocket()
{
    if (serverSocket >= 0) {
        close(serverSocket);
        serverSocket = -1;
    }
}

} // namespace networking
} // namespace fresh
