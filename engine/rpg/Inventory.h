#pragma once
#include <map>
#include <string>

namespace fresh
{
namespace rpg
{

/**
 * @brief Resource types available in the game
 */
enum class ResourceType {
    Iron,
    Titanium,
    Naonite,
    Trinium,
    Xanion,
    Ogonite,
    Avorion,
    Energy,
    Credits
};

/**
 * @brief Get string name for resource type
 */
inline std::string getResourceName(ResourceType type)
{
    switch (type) {
    case ResourceType::Iron:
        return "Iron";
    case ResourceType::Titanium:
        return "Titanium";
    case ResourceType::Naonite:
        return "Naonite";
    case ResourceType::Trinium:
        return "Trinium";
    case ResourceType::Xanion:
        return "Xanion";
    case ResourceType::Ogonite:
        return "Ogonite";
    case ResourceType::Avorion:
        return "Avorion";
    case ResourceType::Energy:
        return "Energy";
    case ResourceType::Credits:
        return "Credits";
    default:
        return "Unknown";
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
