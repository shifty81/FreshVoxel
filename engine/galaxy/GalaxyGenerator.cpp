#include "galaxy/GalaxyGenerator.h"

#include <algorithm>
#include <cmath>
#include <sstream>

namespace fresh
{
namespace galaxy
{

GalaxyGenerator::GalaxyGenerator(uint32_t seed)
    : baseSeed(seed), asteroidDensity(15.0f), stationChance(0.1f), shipDensity(2.0f)
{
    // Increased asteroid density from 10.0f to 15.0f (1.5x increase)
}

void GalaxyGenerator::setSeed(uint32_t seed)
{
    baseSeed = seed;
}

std::unique_ptr<GalaxySector> GalaxyGenerator::generateSector(int x, int y)
{
    auto sector = std::make_unique<GalaxySector>(x, y);

    // Create sector-specific RNG
    uint32_t sectorSeed = getSectorSeed(x, y);
    std::mt19937 rng(sectorSeed);

    // Generate sector content
    generateAsteroids(sector.get(), rng);
    generateStations(sector.get(), rng);
    generateShips(sector.get(), rng);

    return sector;
}

void GalaxyGenerator::generateAsteroids(GalaxySector* sector, std::mt19937& rng)
{
    std::uniform_real_distribution<float> posDist(-500.0f, 500.0f);
    std::uniform_real_distribution<float> sizeDist(5.0f, 50.0f);
    std::uniform_real_distribution<float> amountDist(100.0f, 1000.0f);
    std::uniform_int_distribution<int> countDist(5, 20);

    // Increased asteroid count by 1.5x - 2x
    int asteroidCount = static_cast<int>(asteroidDensity * countDist(rng) / 10.0f);

    for (int i = 0; i < asteroidCount; ++i) {
        AsteroidData asteroid;
        asteroid.position = glm::vec3(posDist(rng), posDist(rng), posDist(rng));
        asteroid.size = sizeDist(rng);
        asteroid.resourceType = getRandomResourceType(rng);
        asteroid.resourceAmount = amountDist(rng);
        
        // New: Add shape variation and stretching
        asteroid.shape = getRandomAsteroidShape(rng);
        asteroid.stretchFactor = generateStretchFactor(asteroid.shape, rng);

        sector->addAsteroid(asteroid);
    }
}

void GalaxyGenerator::generateStations(GalaxySector* sector, std::mt19937& rng)
{
    std::uniform_real_distribution<float> chanceDist(0.0f, 1.0f);

    if (chanceDist(rng) < stationChance) {
        std::uniform_real_distribution<float> posDist(-300.0f, 300.0f);
        std::uniform_int_distribution<int> levelDist(1, 5);
        std::uniform_real_distribution<float> sizeMult(4.0f, 5.0f); // 4-5x larger

        StationData station;
        station.position = glm::vec3(posDist(rng), posDist(rng), posDist(rng));
        station.type = getRandomStationType(rng);
        station.name = generateStationName(sector->getX(), sector->getY(), rng);
        station.level = levelDist(rng);
        station.sizeMultiplier = sizeMult(rng); // 4-5x size increase
        station.designVariant = getRandomDesignVariant(rng);
        
        // Generate docking arms for the station
        generateDockingArms(station, rng);
        
        // Populate station services, commodities, and available captains
        populateStationServices(station, rng);
        populateCommodities(station, rng);
        generateAvailableCaptains(station, rng);

        sector->addStation(station);
    }
}

void GalaxyGenerator::generateShips(GalaxySector* sector, std::mt19937& rng)
{
    std::uniform_real_distribution<float> posDist(-400.0f, 400.0f);
    std::uniform_real_distribution<float> velDist(-10.0f, 10.0f);
    std::uniform_real_distribution<float> hostileDist(0.0f, 1.0f);
    std::uniform_int_distribution<int> countDist(0, 5);

    int shipCount = static_cast<int>(shipDensity * countDist(rng) / 2.0f);

    for (int i = 0; i < shipCount; ++i) {
        ShipData ship;
        ship.position = glm::vec3(posDist(rng), posDist(rng), posDist(rng));
        ship.velocity = glm::vec3(velDist(rng), velDist(rng), velDist(rng));
        ship.type = getRandomShipType(rng);
        ship.faction = getRandomFaction(rng);
        ship.isHostile = hostileDist(rng) < 0.3f; // 30% chance of hostile

        sector->addShip(ship);
    }
}

uint32_t GalaxyGenerator::getSectorSeed(int x, int y) const
{
    // Combine sector coordinates with base seed using hash function
    uint32_t hash = baseSeed;
    hash ^= static_cast<uint32_t>(x) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
    hash ^= static_cast<uint32_t>(y) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
    return hash;
}

ResourceType GalaxyGenerator::getRandomResourceType(std::mt19937& rng)
{
    std::uniform_int_distribution<int> dist(0, 6);
    return static_cast<ResourceType>(dist(rng));
}

StationType GalaxyGenerator::getRandomStationType(std::mt19937& rng)
{
    std::uniform_int_distribution<int> dist(0, 9); // Updated to include new types
    return static_cast<StationType>(dist(rng));
}

ShipType GalaxyGenerator::getRandomShipType(std::mt19937& rng)
{
    std::uniform_int_distribution<int> dist(0, 6);
    return static_cast<ShipType>(dist(rng));
}

std::string GalaxyGenerator::generateStationName(int x, int y, std::mt19937& rng)
{
    std::vector<std::string> prefixes = {"Alpha",   "Beta", "Gamma", "Delta",
                                         "Epsilon", "Zeta", "Nova",  "Stellar"};
    std::vector<std::string> suffixes = {"Station", "Outpost",  "Haven", "Port",
                                         "Hub",     "Terminal", "Base"};

    std::uniform_int_distribution<size_t> prefixDist(0, prefixes.size() - 1);
    std::uniform_int_distribution<size_t> suffixDist(0, suffixes.size() - 1);

    std::ostringstream oss;
    oss << prefixes[prefixDist(rng)] << " " << suffixes[suffixDist(rng)] << " " << x << "-" << y;

    return oss.str();
}

std::string GalaxyGenerator::getRandomFaction(std::mt19937& rng)
{
    std::vector<std::string> factions = {"Independent", "Federation", "Empire",   "Republic",
                                         "Alliance",    "Pirates",    "Merchants"};

    std::uniform_int_distribution<size_t> dist(0, factions.size() - 1);
    return factions[dist(rng)];
}

AsteroidShape GalaxyGenerator::getRandomAsteroidShape(std::mt19937& rng)
{
    std::uniform_int_distribution<int> dist(0, 4);
    return static_cast<AsteroidShape>(dist(rng));
}

glm::vec3 GalaxyGenerator::generateStretchFactor(AsteroidShape shape, std::mt19937& rng)
{
    // Maximum stretch ratio for structural integrity (configurable per material)
    const float MAX_STRETCH_RATIO = 5.0f;
    
    std::uniform_real_distribution<float> stretchDist(0.5f, 2.5f);
    
    switch (shape) {
    case AsteroidShape::Sphere:
        // Spheres have uniform dimensions
        return glm::vec3(1.0f, 1.0f, 1.0f);
        
    case AsteroidShape::Cube:
        // Cubes have slight variations (limited stretch)
        return glm::vec3(
            std::min(stretchDist(rng) * 0.8f + 0.2f, MAX_STRETCH_RATIO),
            std::min(stretchDist(rng) * 0.8f + 0.2f, MAX_STRETCH_RATIO),
            std::min(stretchDist(rng) * 0.8f + 0.2f, MAX_STRETCH_RATIO)
        );
        
    case AsteroidShape::Triangular:
        // Triangular shapes are flatter in one dimension
        return glm::vec3(
            std::min(stretchDist(rng), MAX_STRETCH_RATIO),
            std::min(stretchDist(rng), MAX_STRETCH_RATIO),
            std::min(stretchDist(rng) * 0.4f, MAX_STRETCH_RATIO)
        );
        
    case AsteroidShape::Elongated:
        // Elongated asteroids are stretched along one axis
        {
            std::uniform_int_distribution<int> axisDist(0, 2);
            int stretchAxis = axisDist(rng);
            glm::vec3 stretch(1.0f, 1.0f, 1.0f);
            // 2x-5x stretch but capped at MAX_STRETCH_RATIO
            stretch[stretchAxis] = std::min(stretchDist(rng) * 2.0f, MAX_STRETCH_RATIO);
            return stretch;
        }
        
    case AsteroidShape::Irregular:
    default:
        // Irregular shapes have random stretching in all dimensions
        {
            glm::vec3 stretch(stretchDist(rng), stretchDist(rng), stretchDist(rng));
            // Ensure no dimension exceeds MAX_STRETCH_RATIO
            stretch.x = std::min(stretch.x, MAX_STRETCH_RATIO);
            stretch.y = std::min(stretch.y, MAX_STRETCH_RATIO);
            stretch.z = std::min(stretch.z, MAX_STRETCH_RATIO);
            return stretch;
        }
    }
}

void GalaxyGenerator::generateDockingArms(StationData& station, std::mt19937& rng)
{
    std::uniform_int_distribution<int> armCountDist(2, 6); // 2-6 docking arms per station
    std::uniform_real_distribution<float> lengthDist(40.0f, 100.0f);
    std::uniform_int_distribution<int> baysDist(1, 4);
    
    int armCount = armCountDist(rng);
    
    // Generate arms in a radial pattern around the station
    for (int i = 0; i < armCount; ++i) {
        DockingArm arm;
        
        // Position arms around the station in a roughly even distribution
        float angle = (2.0f * 3.14159f * i) / armCount;
        float verticalOffset = (i % 2 == 0) ? 0.3f : -0.3f;
        
        arm.direction = glm::normalize(glm::vec3(
            std::cos(angle),
            verticalOffset,
            std::sin(angle)
        ));
        
        arm.offset = arm.direction * (station.sizeMultiplier * 20.0f); // Offset from center
        arm.length = lengthDist(rng) * station.sizeMultiplier;
        arm.dockingBays = baysDist(rng);
        
        station.dockingArms.push_back(arm);
    }
}

int GalaxyGenerator::getRandomDesignVariant(std::mt19937& rng)
{
    std::uniform_int_distribution<int> dist(0, 4); // 5 different design variants
    return dist(rng);
}

void GalaxyGenerator::populateStationServices(StationData& station, std::mt19937& rng)
{
    (void)rng; // Parameter reserved for future randomization
    // All stations offer some basic services
    station.services.push_back(StationService::Refuel);
    station.services.push_back(StationService::Information);
    
    // Add services based on station type
    switch (station.type) {
    case StationType::TradingPost:
        station.services.push_back(StationService::Trade);
        station.services.push_back(StationService::HireCaptain);
        break;
    case StationType::RepairDock:
        station.services.push_back(StationService::Repair);
        station.services.push_back(StationService::Upgrade);
        break;
    case StationType::Shipyard:
        station.services.push_back(StationService::ShipPurchase);
        station.services.push_back(StationService::Repair);
        station.services.push_back(StationService::HireCaptain);
        break;
    case StationType::Factory:
        station.services.push_back(StationService::Trade);
        station.services.push_back(StationService::Upgrade);
        break;
    case StationType::MiningStation:
        station.services.push_back(StationService::Trade);
        break;
    case StationType::ResearchStation:
        station.services.push_back(StationService::Upgrade);
        station.services.push_back(StationService::HireCaptain);
        break;
    case StationType::MilitaryOutpost:
        station.services.push_back(StationService::Repair);
        station.services.push_back(StationService::Upgrade);
        station.services.push_back(StationService::HireCaptain);
        break;
    case StationType::RefuelingDepot:
        station.services.push_back(StationService::Trade);
        break;
    case StationType::CargoHub:
        station.services.push_back(StationService::Trade);
        break;
    case StationType::ScienceLab:
        station.services.push_back(StationService::Medical);
        station.services.push_back(StationService::Upgrade);
        break;
    }
    
    // Higher level stations offer more services
    if (station.level >= 3) {
        station.services.push_back(StationService::Medical);
    }
    if (station.level >= 4) {
        if (std::find(station.services.begin(), station.services.end(), StationService::Trade) 
            == station.services.end()) {
            station.services.push_back(StationService::Trade);
        }
    }
}

void GalaxyGenerator::populateCommodities(StationData& station, std::mt19937& rng)
{
    std::uniform_int_distribution<int> commodityCount(3, 7);
    std::uniform_int_distribution<int> commodityType(0, 8);
    
    int count = commodityCount(rng);
    
    // All trading stations have basic commodities
    station.commodities.push_back(CommodityType::Food);
    station.commodities.push_back(CommodityType::Water);
    station.commodities.push_back(CommodityType::Fuel);
    
    // Add additional commodities based on station type
    switch (station.type) {
    case StationType::TradingPost:
        station.commodities.push_back(CommodityType::Electronics);
        station.commodities.push_back(CommodityType::Luxury);
        break;
    case StationType::MiningStation:
        station.commodities.push_back(CommodityType::Ore);
        station.commodities.push_back(CommodityType::Metal);
        break;
    case StationType::Factory:
        station.commodities.push_back(CommodityType::Metal);
        station.commodities.push_back(CommodityType::Electronics);
        break;
    case StationType::MilitaryOutpost:
        station.commodities.push_back(CommodityType::Weapons);
        break;
    case StationType::ScienceLab:
        station.commodities.push_back(CommodityType::Medicine);
        break;
    default:
        // Add random commodities for other station types
        for (int i = 0; i < count - 3 && i < 4; ++i) {
            CommodityType commodity = static_cast<CommodityType>(commodityType(rng));
            // Avoid duplicates
            if (std::find(station.commodities.begin(), station.commodities.end(), commodity) 
                == station.commodities.end()) {
                station.commodities.push_back(commodity);
            }
        }
        break;
    }
}

void GalaxyGenerator::generateAvailableCaptains(StationData& station, std::mt19937& rng)
{
    // Check if station offers captain hiring service
    if (std::find(station.services.begin(), station.services.end(), StationService::HireCaptain) 
        == station.services.end()) {
        return; // Station doesn't hire captains
    }
    
    std::uniform_int_distribution<int> captainCount(1, 4); // 1-4 captains available
    std::uniform_int_distribution<int> skillLevel(1, 10);
    std::uniform_int_distribution<int> baseCost(500, 2000);
    
    int count = captainCount(rng);
    
    for (int i = 0; i < count; ++i) {
        CaptainData captain;
        captain.name = generateCaptainName(rng);
        captain.skillLevel = skillLevel(rng);
        captain.specialty = getRandomCaptainSpecialty(rng);
        // Higher skill = higher cost
        captain.hiringCost = baseCost(rng) * captain.skillLevel;
        
        station.availableCaptains.push_back(captain);
    }
}

std::string GalaxyGenerator::generateCaptainName(std::mt19937& rng)
{
    std::vector<std::string> firstNames = {
        "Alex", "Morgan", "Riley", "Jordan", "Casey", "Taylor", "Quinn", "Avery",
        "Blake", "Cameron", "Dakota", "Emery", "Harper", "Kai", "Lennon", "Micah"
    };
    
    std::vector<std::string> lastNames = {
        "Chen", "Rodriguez", "Singh", "Kowalski", "O'Brien", "Yamamoto", "Santos",
        "Müller", "Kim", "Patel", "Williams", "Anderson", "Novak", "Hassan"
    };
    
    std::uniform_int_distribution<size_t> firstDist(0, firstNames.size() - 1);
    std::uniform_int_distribution<size_t> lastDist(0, lastNames.size() - 1);
    
    return firstNames[firstDist(rng)] + " " + lastNames[lastDist(rng)];
}

std::string GalaxyGenerator::getRandomCaptainSpecialty(std::mt19937& rng)
{
    std::vector<std::string> specialties = {
        "Combat", "Trade", "Exploration", "Mining", "Diplomacy", "Engineering"
    };
    
    std::uniform_int_distribution<size_t> dist(0, specialties.size() - 1);
    return specialties[dist(rng)];
}

} // namespace galaxy
} // namespace fresh
