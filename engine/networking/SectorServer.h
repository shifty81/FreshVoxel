#pragma once
#include <memory>
#include <mutex>
#include <vector>

#include "../ecs/EntityManager.h"
#include "../galaxy/GalaxySector.h"
#include "ClientConnection.h"

namespace fresh
{
namespace networking
{

/**
 * @brief Manages a single sector on the server
 *
 * Handles entities, physics, and player interactions within a specific sector.
 * Each sector runs on its own thread for scalability.
 */
class SectorServer
{
public:
    SectorServer(int x, int y);
    ~SectorServer();

    // Sector info
    int getX() const
    {
        return sectorX;
    }
    int getY() const
    {
        return sectorY;
    }

    // Player management
    void addPlayer(uint32_t clientId);
    void removePlayer(uint32_t clientId);
    bool hasPlayer(uint32_t clientId) const;
    int getPlayerCount() const;

    // Update sector
    void update(float deltaTime);

    // Get sector data
    const galaxy::GalaxySector& getSector() const
    {
        return *sector;
    }
    ecs::EntityManager& getEntityManager()
    {
        return entityManager;
    }

    // Broadcasting
    void broadcastToPlayers(const NetworkMessage& message);

private:
    int sectorX;
    int sectorY;
    std::unique_ptr<galaxy::GalaxySector> sector;
    ecs::EntityManager entityManager;

    // Players in this sector
    std::vector<uint32_t> playerIds;
    mutable std::mutex playersMutex;

    // Sector state
    double lastUpdateTime;

    void initializeSector();
};

} // namespace networking
} // namespace fresh
