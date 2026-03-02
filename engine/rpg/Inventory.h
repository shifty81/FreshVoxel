#pragma once
#include <map>
#include <string>

namespace fresh
{
namespace rpg
{

/**
 * @brief Resource types available in the voxel game
 * 
 * These represent gatherable and craftable materials in a voxel world,
 * similar to Minecraft-style resource systems.
 */
enum class ResourceType {
    // Raw materials (gathered from world)
    Wood,       // From trees
    Stone,      // From stone blocks
    Dirt,       // From digging
    Sand,       // From beaches/deserts
    Coal,       // From mining
    IronOre,    // From mining
    GoldOre,    // From mining (rare)
    Diamond,    // From mining (very rare)
    
    // Processed materials
    Plank,      // Crafted from Wood
    Cobblestone,// From mining stone
    IronIngot,  // Smelted from IronOre
    GoldIngot,  // Smelted from GoldOre
    Glass,      // Smelted from Sand
    Brick,      // Smelted from Clay
    
    // Organic materials
    Leaves,     // From trees (decorative)
    Seeds,      // From breaking grass
    Wheat,      // Grown from Seeds
    Apple,      // From trees (food)
    Leather,    // From animals
    Feather,    // From birds
    
    // Special materials
    Redstone,   // For mechanisms
    Lapis,      // For enchanting/dye
    Emerald,    // For trading
    Clay,       // From riverbeds
    Flint,      // From gravel
    Gunpowder,  // From enemies
    String,     // From spiders
    Bone,       // From skeletons

    // Advanced materials (trading/loot)
    Iron,       // Refined iron
    Titanium,   // Rare metal
    Naonite,    // Exotic mineral
    Trinium,    // Exotic mineral
    Xanion,     // Exotic mineral
    Ogonite,    // Exotic mineral
    Avorion,    // Legendary material
    Energy,     // Energy resource
    Credits,    // Currency
    
    // Count for iteration
    COUNT
};

/**
 * @brief Get string name for resource type
 */
inline std::string getResourceName(ResourceType type)
{
    switch (type) {
    case ResourceType::Wood:        return "Wood";
    case ResourceType::Stone:       return "Stone";
    case ResourceType::Dirt:        return "Dirt";
    case ResourceType::Sand:        return "Sand";
    case ResourceType::Coal:        return "Coal";
    case ResourceType::IronOre:     return "Iron Ore";
    case ResourceType::GoldOre:     return "Gold Ore";
    case ResourceType::Diamond:     return "Diamond";
    case ResourceType::Plank:       return "Plank";
    case ResourceType::Cobblestone: return "Cobblestone";
    case ResourceType::IronIngot:   return "Iron Ingot";
    case ResourceType::GoldIngot:   return "Gold Ingot";
    case ResourceType::Glass:       return "Glass";
    case ResourceType::Brick:       return "Brick";
    case ResourceType::Leaves:      return "Leaves";
    case ResourceType::Seeds:       return "Seeds";
    case ResourceType::Wheat:       return "Wheat";
    case ResourceType::Apple:       return "Apple";
    case ResourceType::Leather:     return "Leather";
    case ResourceType::Feather:     return "Feather";
    case ResourceType::Redstone:    return "Redstone";
    case ResourceType::Lapis:       return "Lapis Lazuli";
    case ResourceType::Emerald:     return "Emerald";
    case ResourceType::Clay:        return "Clay";
    case ResourceType::Flint:       return "Flint";
    case ResourceType::Gunpowder:   return "Gunpowder";
    case ResourceType::String:      return "String";
    case ResourceType::Bone:        return "Bone";
    case ResourceType::Iron:        return "Iron";
    case ResourceType::Titanium:    return "Titanium";
    case ResourceType::Naonite:     return "Naonite";
    case ResourceType::Trinium:     return "Trinium";
    case ResourceType::Xanion:      return "Xanion";
    case ResourceType::Ogonite:     return "Ogonite";
    case ResourceType::Avorion:     return "Avorion";
    case ResourceType::Energy:      return "Energy";
    case ResourceType::Credits:     return "Credits";
    default:                        return "Unknown";
    }
}

/**
 * @brief Get rarity/tier of a resource (0=common, 1=uncommon, 2=rare, 3=epic, 4=legendary)
 */
inline int getResourceRarity(ResourceType type)
{
    switch (type) {
    case ResourceType::Wood:
    case ResourceType::Stone:
    case ResourceType::Dirt:
    case ResourceType::Sand:
    case ResourceType::Leaves:
    case ResourceType::Seeds:
        return 0; // Common
        
    case ResourceType::Coal:
    case ResourceType::Clay:
    case ResourceType::Flint:
    case ResourceType::Cobblestone:
    case ResourceType::Plank:
    case ResourceType::Wheat:
    case ResourceType::String:
    case ResourceType::Bone:
    case ResourceType::Feather:
        return 1; // Uncommon
        
    case ResourceType::IronOre:
    case ResourceType::IronIngot:
    case ResourceType::Glass:
    case ResourceType::Brick:
    case ResourceType::Leather:
    case ResourceType::Apple:
    case ResourceType::Redstone:
    case ResourceType::Lapis:
    case ResourceType::Gunpowder:
    case ResourceType::Iron:
    case ResourceType::Energy:
    case ResourceType::Credits:
        return 2; // Rare
        
    case ResourceType::GoldOre:
    case ResourceType::GoldIngot:
    case ResourceType::Emerald:
    case ResourceType::Titanium:
    case ResourceType::Naonite:
    case ResourceType::Trinium:
        return 3; // Epic
        
    case ResourceType::Diamond:
    case ResourceType::Xanion:
    case ResourceType::Ogonite:
    case ResourceType::Avorion:
        return 4; // Legendary
        
    default:
        return 0;
    }
}

/**
 * @brief Manages resource storage with capacity limits
 */
class Inventory
{
public:
    explicit Inventory(float capacity = 1000.0f);

    // Resource management
    bool addResource(ResourceType type, float amount);
    bool removeResource(ResourceType type, float amount);
    float getResourceAmount(ResourceType type) const;
    bool hasResource(ResourceType type, float amount) const;

    // Capacity management
    float getTotalUsed() const;
    float getCapacity() const
    {
        return capacity;
    }
    void setCapacity(float cap)
    {
        capacity = cap;
    }
    float getAvailableSpace() const
    {
        return capacity - getTotalUsed();
    }
    bool isFull() const
    {
        return getTotalUsed() >= capacity;
    }

    // Get all resources
    const std::map<ResourceType, float>& getAllResources() const
    {
        return resources;
    }

    // Clear all resources
    void clear();

private:
    std::map<ResourceType, float> resources;
    float capacity;
};

} // namespace rpg
} // namespace fresh
