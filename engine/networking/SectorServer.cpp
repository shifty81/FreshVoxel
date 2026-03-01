#include "networking/SectorServer.h"

#include <algorithm>
#include <chrono>

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
    (void)deltaTime;
    // Sector-level entity tick: systems like physics and AI would
    // iterate over entityManager.getEntitiesWithComponent<>() here
    // once those components are registered per-sector.

    // Update last update time
    auto now = std::chrono::steady_clock::now();
    auto duration = now.time_since_epoch();
    lastUpdateTime = std::chrono::duration<double>(duration).count();
}

void SectorServer::broadcastToPlayers(const NetworkMessage& message)
{
    std::lock_guard<std::mutex> lock(playersMutex);
    // Actual socket delivery is handled by GameServer which owns
    // the ClientConnection objects. SectorServer tracks player IDs
    // so GameServer can look up the corresponding connections and
    // call sendMessage() on each. This method is a coordination point
    // that will be wired once GameServer exposes a sendToClient() API.
    (void)message;
}

void SectorServer::initializeSector()
{
    // Placeholder for sector initialization
    // Future: Initialize voxel world chunks for this sector region
}

} // namespace networking
} // namespace fresh
