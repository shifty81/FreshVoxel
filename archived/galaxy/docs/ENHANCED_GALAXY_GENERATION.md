# Enhanced Galaxy Generation Documentation

## Overview
The galaxy generation system has been significantly enhanced with improved asteroids, larger stations with docking arms, and a complete captain/trading system. This document describes all the new features and how to use them.

## Key Enhancements

### 1. Enhanced Asteroids (1.5x-2x Count)
- **Increased Density**: Asteroid density increased from 10.0 to 15.0 (50% more asteroids)
- **Shape Variety**: 5 distinct shapes with different characteristics
- **Aesthetic Stretching**: Each shape has unique stretch patterns for visual variety
- **Resource Diversity**: 7 resource types (Iron, Titanium, Naonite, Trinium, Xanion, Ogonite, Avorion)

#### Asteroid Shapes
1. **Sphere**: Uniform, spherical asteroids (no stretching)
2. **Cube**: Blocky asteroids with slight variations
3. **Triangular**: Flattened, wedge-shaped asteroids
4. **Elongated**: Stretched 2x-5x along one axis (needle or blade shapes)
5. **Irregular**: Random stretching in all dimensions

### 2. Enhanced Stations (4-5x Larger)
- **Size Increase**: Stations are now 4-5x larger (sizeMultiplier: 4.0 - 5.0)
- **More Types**: 10 station types (added 4 new types)
- **Design Variants**: 5 different build designs per station
- **Docking Arms**: 2-6 large docking arms per station
- **Services**: 8 different service types
- **Trading**: 9 commodity types for trade
- **Captain Hiring**: 1-4 captains available at certain stations

#### New Station Types
1. **MilitaryOutpost**: Military services, weapons, repairs
2. **RefuelingDepot**: Specialized fuel and basic supplies
3. **CargoHub**: Trading hub with high commodity variety
4. **ScienceLab**: Research upgrades and medical services

#### Station Services
1. **Trade**: Buy/sell commodities
2. **Repair**: Ship repair services
3. **Refuel**: Refueling services
4. **HireCaptain**: Hire captains for your fleet
5. **ShipPurchase**: Buy new ships
6. **Upgrade**: Ship upgrade services
7. **Medical**: Medical services for crew
8. **Information**: Information broker services

#### Commodity Types
1. **Food**: Essential supplies
2. **Water**: Life support
3. **Fuel**: Ship propulsion
4. **Ore**: Raw materials
5. **Metal**: Processed materials
6. **Electronics**: Advanced components
7. **Medicine**: Medical supplies
8. **Weapons**: Combat equipment
9. **Luxury**: High-value goods

### 3. Docking System
- **Docking Arms**: Large arms extending from stations (40-100 units long)
- **Multiple Bays**: Each arm has 1-4 docking bays
- **Radial Distribution**: Arms evenly distributed around station
- **Size Scaling**: Arm length scales with station size
- **Hailing**: Stations can be hailed when docked (GUI interaction)

### 4. Captain System
- **Captain Generation**: 1-4 captains per hiring station
- **Skill Levels**: 1-10 skill rating
- **Specialties**: Combat, Trade, Exploration, Mining, Diplomacy, Engineering
- **Hiring Cost**: Based on skill level (500-20,000 credits)
- **Passenger Bay**: Ships can store multiple captains
- **Captain Assignment**: One captain per ship, others wait in passenger bay
- **Fleet Management**: Transfer captains between ships via GUI

## API Reference

### GalaxySector.h

#### Enums and Structures
```cpp
// Asteroid shapes
enum class AsteroidShape { 
    Sphere, Cube, Triangular, Elongated, Irregular 
};

// Enhanced asteroid data
struct AsteroidData {
    glm::vec3 position;
    float size;
    ResourceType resourceType;
    float resourceAmount;
    AsteroidShape shape;        // NEW
    glm::vec3 stretchFactor;    // NEW
};

// Station services
enum class StationService {
    Trade, Repair, Refuel, HireCaptain, 
    ShipPurchase, Upgrade, Medical, Information
};

// Commodities for trade
enum class CommodityType {
    Food, Water, Fuel, Ore, Metal, 
    Electronics, Medicine, Weapons, Luxury
};

// Captain data
struct CaptainData {
    std::string name;
    int skillLevel;         // 1-10
    std::string specialty;  // Combat, Trade, etc.
    int hiringCost;
};

// Docking arm configuration
struct DockingArm {
    glm::vec3 offset;       // Relative to station
    glm::vec3 direction;    // Direction arm extends
    float length;           // 40-100 units
    int dockingBays;        // 1-4 bays per arm
};

// Enhanced station data
struct StationData {
    glm::vec3 position;
    StationType type;
    std::string name;
    int level;
    float sizeMultiplier;                   // NEW: 4.0-5.0
    std::vector<DockingArm> dockingArms;    // NEW
    int designVariant;                      // NEW: 0-4
    std::vector<StationService> services;   // NEW
    std::vector<CommodityType> commodities; // NEW
    std::vector<CaptainData> availableCaptains;  // NEW
};

// Enhanced ship data
struct ShipData {
    glm::vec3 position;
    glm::vec3 velocity;
    ShipType type;
    std::string faction;
    bool isHostile;
    bool hasCaptain;                    // NEW
    CaptainData captain;                // NEW
    std::vector<CaptainData> passengers; // NEW
};
```

### GalaxyGenerator.h

#### Configuration
```cpp
// Set asteroid density (default: 15.0, increased from 10.0)
void setAsteroidDensity(float density);

// Set station chance (default: 0.1, i.e., 10%)
void setStationChance(float chance);
```

#### Generation
```cpp
// Generate a complete sector with all enhancements
std::unique_ptr<GalaxySector> generateSector(int x, int y);
```

## Usage Examples

### Example 1: Generate Enhanced Sector
```cpp
#include "galaxy/GalaxyGenerator.h"

GalaxyGenerator generator(12345);  // Seed for deterministic generation
auto sector = generator.generateSector(0, 0);

// Access enhanced asteroids
for (const auto& asteroid : sector->getAsteroids()) {
    std::cout << "Asteroid shape: " << (int)asteroid.shape << std::endl;
    std::cout << "Stretch: " << asteroid.stretchFactor.x << ", " 
              << asteroid.stretchFactor.y << ", " 
              << asteroid.stretchFactor.z << std::endl;
}

// Access enhanced stations
for (const auto& station : sector->getStations()) {
    std::cout << "Station: " << station.name << std::endl;
    std::cout << "Size multiplier: " << station.sizeMultiplier << "x" << std::endl;
    std::cout << "Docking arms: " << station.dockingArms.size() << std::endl;
    std::cout << "Services: " << station.services.size() << std::endl;
    std::cout << "Available captains: " << station.availableCaptains.size() << std::endl;
}
```

### Example 2: Render Asteroids with Shapes
```cpp
void renderAsteroid(const AsteroidData& asteroid) {
    // Scale base size by stretch factor
    glm::vec3 finalSize = asteroid.size * asteroid.stretchFactor;
    
    switch (asteroid.shape) {
    case AsteroidShape::Sphere:
        renderSphere(asteroid.position, finalSize);
        break;
    case AsteroidShape::Cube:
        renderCube(asteroid.position, finalSize);
        break;
    case AsteroidShape::Triangular:
        renderTriangularAsteroid(asteroid.position, finalSize);
        break;
    case AsteroidShape::Elongated:
        renderElongatedAsteroid(asteroid.position, finalSize);
        break;
    case AsteroidShape::Irregular:
        renderIrregularAsteroid(asteroid.position, finalSize);
        break;
    }
}
```

### Example 3: Station Interaction
```cpp
void interactWithStation(const StationData& station) {
    // Check available services
    bool canTrade = std::find(station.services.begin(), 
                             station.services.end(), 
                             StationService::Trade) != station.services.end();
    
    bool canHireCaptain = std::find(station.services.begin(), 
                                   station.services.end(), 
                                   StationService::HireCaptain) != station.services.end();
    
    if (canTrade) {
        // Show trading GUI with available commodities
        displayTradingMenu(station.commodities);
    }
    
    if (canHireCaptain) {
        // Show captain hiring GUI
        displayCaptainHiringMenu(station.availableCaptains);
    }
}
```

### Example 4: Captain Management
```cpp
void hireCaptain(const CaptainData& captain, ShipData& playerShip) {
    // Check if player has enough credits
    if (playerCredits < captain.hiringCost) {
        return; // Can't afford
    }
    
    if (playerShip.hasCaptain) {
        // Move current captain to passenger bay
        playerShip.passengers.push_back(playerShip.captain);
    }
    
    // Assign new captain
    playerShip.captain = captain;
    playerShip.hasCaptain = true;
    playerCredits -= captain.hiringCost;
}

void transferCaptain(ShipData& fromShip, ShipData& toShip) {
    if (!fromShip.hasCaptain) return;
    
    if (toShip.hasCaptain) {
        // Move toShip's captain to passenger bay
        toShip.passengers.push_back(toShip.captain);
    }
    
    // Transfer captain
    toShip.captain = fromShip.captain;
    toShip.hasCaptain = true;
    
    // Check if fromShip has a replacement in passenger bay
    if (!fromShip.passengers.empty()) {
        fromShip.captain = fromShip.passengers.back();
        fromShip.passengers.pop_back();
    } else {
        fromShip.hasCaptain = false;
    }
}
```

### Example 5: Docking at Station
```cpp
bool attemptDocking(const glm::vec3& shipPosition, 
                    const StationData& station) {
    // Check proximity to docking arms
    for (const auto& arm : station.dockingArms) {
        glm::vec3 armEndPos = station.position + 
                             arm.offset + 
                             arm.direction * arm.length;
        
        float distance = glm::distance(shipPosition, armEndPos);
        
        if (distance < 10.0f) {  // Within docking range
            // Successful docking
            return true;
        }
    }
    
    return false;  // Not close enough to any docking arm
}
```

## Integration with Demo 1

The enhancements are automatically active in Demo 1 (Create New World). When you create a new world:

1. **Generate a Galaxy Sector**:
   ```cpp
   GalaxyGenerator generator(worldSeed);
   auto sector = generator.generateSector(0, 0);
   ```

2. **Populate World with Enhanced Content**:
   - 50% more asteroids with varied shapes
   - Larger stations (4-5x) with docking arms
   - Trading and captain systems active

3. **Player Interactions**:
   - Mine asteroids (notice shape variety)
   - Approach stations (see docking arms)
   - Dock at stations (hail for services)
   - Trade commodities
   - Hire captains for fleet

## Performance Impact

### Memory Usage
- **Per Asteroid**: +32 bytes (shape enum + stretch vector)
- **Per Station**: +variable (arms, services, commodities, captains)
- **Estimated**: ~10KB additional per sector

### Computation
- **Generation Time**: <5% increase (additional RNG calls)
- **Rendering**: Depends on asteroid shape complexity
- **Collision**: No change (bounding boxes still used)

## Configuration

### Adjusting Asteroid Density
```cpp
generator.setAsteroidDensity(20.0f);  // Even more asteroids (2x original)
generator.setAsteroidDensity(7.5f);   // Fewer asteroids (0.75x original)
```

### Adjusting Station Frequency
```cpp
generator.setStationChance(0.2f);  // 20% chance (more stations)
generator.setStationChance(0.05f); // 5% chance (fewer stations)
```

## Testing
Comprehensive tests are provided in:
- `tests/galaxy/GalaxyGeneratorEnhancedTest.cpp`

Run tests:
```bash
cd build
ctest --output-on-failure -R Galaxy
```

## Future Enhancements
- [ ] Dynamic station generation based on player actions
- [ ] Captain skill progression system
- [ ] Advanced trading economy simulation
- [ ] Multi-sector trading routes
- [ ] Station construction and upgrades
- [ ] Fleet management GUI
- [ ] Captain recruitment missions

## See Also
- [GalaxySector.h](../engine/galaxy/GalaxySector.h) - Data structures
- [GalaxyGenerator.h](../engine/galaxy/GalaxyGenerator.h) - Generation logic
- [BLOCK_STRETCHING.md](BLOCK_STRETCHING.md) - Block stretching feature
- [examples/README.md](../examples/README.md) - Demo scenarios
