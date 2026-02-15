#include "networking/SectorServer.h"

#include <algorithm>
#include <chrono>

#include "galaxy/GalaxyGenerator.h"

namespace fresh
{
namespace networking
{

SectorServer::SectorServer(int x, int y) : sectorX(x), sectorY(y), lastUpdateTime(0.0)
{
    initializeSector();
}

SectorServer::~SectorServer() {}

void SectorServer::addPlayer(uint32_t clientId)
{
    std::lock_guard<std::mutex> lock(playersMutex);
    if (std::find(playerIds.begin(), playerIds.end(), clientId) == playerIds.end()) {
        playerIds.push_back(clientId);
    }
}

void SectorServer::removePlayer(uint32_t clientId)
{
    std::lock_guard<std::mutex> lock(playersMutex);
    playerIds.erase(std::remove(playerIds.begin(), playerIds.end(), clientId), playerIds.end());
}

bool SectorServer::hasPlayer(uint32_t clientId) const
{
    std::lock_guard<std::mutex> lock(playersMutex);
    return std::find(playerIds.begin(), playerIds.end(), clientId) != playerIds.end();
}

int SectorServer::getPlayerCount() const
{
    std::lock_guard<std::mutex> lock(playersMutex);
    return static_cast<int>(playerIds.size());
}

void SectorServer::update(float deltaTime)
{
    (void)deltaTime; // Unused - placeholder for future implementation
    // Update entities in this sector
    // This would include physics, AI, etc.

    // Update last update time
    auto now = std::chrono::steady_clock::now();
    auto duration = now.time_since_epoch();
    lastUpdateTime = std::chrono::duration<double>(duration).count();
}

void SectorServer::broadcastToPlayers(const NetworkMessage& message)
{
    (void)message; // Unused - placeholder for future implementation
    std::lock_guard<std::mutex> lock(playersMutex);
    // In a real implementation, this would send the message to all connected clients
    // For now, this is a placeholder
}

void SectorServer::initializeSector()
{
    // Generate sector content
    galaxy::GalaxyGenerator generator;
    sector = generator.generateSector(sectorX, sectorY);

    // Create entities for sector objects (asteroids, stations, ships)
    // This would involve creating Entity objects with appropriate components
}

} // namespace networking
} // namespace fresh
