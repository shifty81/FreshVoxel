#pragma once
#include <memory>
#include <random>

#include "GalaxySector.h"

namespace fresh
{
namespace galaxy
{

/**
 * @brief Generates galaxy sectors with deterministic, seed-based algorithms
 *
 * Provides procedural generation of asteroids, stations, and ships
 * with consistent results based on sector coordinates and seed.
 */
class GalaxyGenerator
{
public:
    explicit GalaxyGenerator(uint32_t seed = 12345);

    /**
     * @brief Generate a complete sector
     * @param x Sector X coordinate
     * @param y Sector Y coordinate
     * @return Generated sector with all objects
     */
    std::unique_ptr<GalaxySector> generateSector(int x, int y);

    /**
     * @brief Set the generation seed
     */
    void setSeed(uint32_t seed);
    uint32_t getSeed() const
    {
        return baseSeed;
    }

    // Configuration
    void setAsteroidDensity(float density)
    {
        asteroidDensity = density;
    }
    void setStationChance(float chance)
    {
        stationChance = chance;
    }
    void setShipDensity(float density)
    {
        shipDensity = density;
    }

private:
    uint32_t baseSeed;
    float asteroidDensity;
    float stationChance;
    float shipDensity;

    // Generation methods
    void generateAsteroids(GalaxySector* sector, std::mt19937& rng);
    void generateStations(GalaxySector* sector, std::mt19937& rng);
    void generateShips(GalaxySector* sector, std::mt19937& rng);

    // Helper methods
    uint32_t getSectorSeed(int x, int y) const;
    ResourceType getRandomResourceType(std::mt19937& rng);
    StationType getRandomStationType(std::mt19937& rng);
    ShipType getRandomShipType(std::mt19937& rng);
    std::string generateStationName(int x, int y, std::mt19937& rng);
    std::string getRandomFaction(std::mt19937& rng);
    
    // New helper methods for enhanced generation
    AsteroidShape getRandomAsteroidShape(std::mt19937& rng);
    glm::vec3 generateStretchFactor(AsteroidShape shape, std::mt19937& rng);
    void generateDockingArms(StationData& station, std::mt19937& rng);
    int getRandomDesignVariant(std::mt19937& rng);
    void populateStationServices(StationData& station, std::mt19937& rng);
    void populateCommodities(StationData& station, std::mt19937& rng);
    void generateAvailableCaptains(StationData& station, std::mt19937& rng);
    std::string generateCaptainName(std::mt19937& rng);
    std::string getRandomCaptainSpecialty(std::mt19937& rng);
};

} // namespace galaxy
} // namespace fresh
