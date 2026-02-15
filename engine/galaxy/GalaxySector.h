#pragma once
#include <string>
#include <vector>

#include <glm/glm.hpp>

namespace fresh
{
namespace galaxy
{

/**
 * @brief Resource types found in asteroids
 */
enum class ResourceType { Iron, Titanium, Naonite, Trinium, Xanion, Ogonite, Avorion };

/**
 * @brief Shape types for asteroids
 */
enum class AsteroidShape { Sphere, Cube, Triangular, Elongated, Irregular };

/**
 * @brief Data structure for an asteroid
 */
struct AsteroidData {
    glm::vec3 position;
    float size;
    ResourceType resourceType;
    float resourceAmount;
    AsteroidShape shape;        // Shape of the asteroid
    glm::vec3 stretchFactor;    // Allows stretching in x, y, z directions (1.0 = no stretch)

    AsteroidData()
        : position(0.0f),
          size(1.0f),
          resourceType(ResourceType::Iron),
          resourceAmount(0.0f),
          shape(AsteroidShape::Sphere),
          stretchFactor(1.0f, 1.0f, 1.0f)
    {
    }
};

/**
 * @brief Station types
 */
enum class StationType {
    TradingPost,
    RepairDock,
    Shipyard,
    Factory,
    MiningStation,
    ResearchStation,
    MilitaryOutpost,
    RefuelingDepot,
    CargoHub,
    ScienceLab
};

/**
 * @brief Services offered by stations
 */
enum class StationService {
    Trade,          // Buy/sell commodities
    Repair,         // Ship repairs
    Refuel,         // Refueling services
    HireCaptain,    // Hire captains for fleet
    ShipPurchase,   // Buy new ships
    Upgrade,        // Ship upgrades
    Medical,        // Medical services
    Information     // Information broker
};

/**
 * @brief Commodity types for trade
 */
enum class CommodityType {
    Food,
    Water,
    Fuel,
    Ore,
    Metal,
    Electronics,
    Medicine,
    Weapons,
    Luxury
};

/**
 * @brief Captain data for hiring
 */
struct CaptainData {
    std::string name;
    int skillLevel;         // 1-10 skill rating
    std::string specialty;  // Combat, Trade, Exploration, Mining
    int hiringCost;
    
    CaptainData()
        : name("Captain"), skillLevel(1), specialty("General"), hiringCost(1000)
    {
    }
};

/**
 * @brief Docking arm configuration
 */
struct DockingArm {
    glm::vec3 offset;       // Position relative to station center
    glm::vec3 direction;    // Direction the arm extends
    float length;           // Length of the docking arm
    int dockingBays;        // Number of docking bays on this arm

    DockingArm()
        : offset(0.0f), direction(1.0f, 0.0f, 0.0f), length(50.0f), dockingBays(2)
    {
    }
};

/**
 * @brief Data structure for a station
 */
struct StationData {
    glm::vec3 position;
    StationType type;
    std::string name;
    int level;
    float sizeMultiplier;                   // 4-5x larger than before
    std::vector<DockingArm> dockingArms;    // Large docking arms
    int designVariant;                      // Different build designs (0-4)
    std::vector<StationService> services;   // Services offered by this station
    std::vector<CommodityType> commodities; // Commodities available for trade
    std::vector<CaptainData> availableCaptains; // Captains available for hire

    StationData()
        : position(0.0f),
          type(StationType::TradingPost),
          name("Station"),
          level(1),
          sizeMultiplier(4.5f),
          designVariant(0)
    {
    }
};

/**
 * @brief Ship types
 */
enum class ShipType { Fighter, Corvette, Frigate, Destroyer, Cruiser, Battleship, Carrier };

/**
 * @brief Data structure for a ship
 */
struct ShipData {
    glm::vec3 position;
    glm::vec3 velocity;
    ShipType type;
    std::string faction;
    bool isHostile;
    bool hasCaptain;                    // Whether ship has assigned captain
    CaptainData captain;                // Current captain (if hasCaptain is true)
    std::vector<CaptainData> passengers; // Passenger bay for captains awaiting transfer

    ShipData()
        : position(0.0f),
          velocity(0.0f),
          type(ShipType::Fighter),
          faction("Independent"),
          isHostile(false),
          hasCaptain(false)
    {
    }
};

/**
 * @brief Represents a sector in the galaxy
 */
class GalaxySector
{
public:
    GalaxySector();
    GalaxySector(int x, int y);

    // Getters
    int getX() const
    {
        return x;
    }
    int getY() const
    {
        return y;
    }
    const std::vector<AsteroidData>& getAsteroids() const
    {
        return asteroids;
    }
    const std::vector<StationData>& getStations() const
    {
        return stations;
    }
    const std::vector<ShipData>& getShips() const
    {
        return ships;
    }

    // Setters
    void addAsteroid(const AsteroidData& asteroid);
    void addStation(const StationData& station);
    void addShip(const ShipData& ship);

    // Clear sector data
    void clear();

private:
    int x;
    int y;
    std::vector<AsteroidData> asteroids;
    std::vector<StationData> stations;
    std::vector<ShipData> ships;
};

} // namespace galaxy
} // namespace fresh
