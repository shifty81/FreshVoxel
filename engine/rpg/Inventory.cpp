#include "rpg/Inventory.h"

namespace fresh
{
namespace rpg
{

Inventory::Inventory(float capacity) : capacity(capacity) {}

bool Inventory::addResource(ResourceType type, float amount)
{
    if (amount <= 0.0f) {
        return false;
    }

    float totalAfter = getTotalUsed() + amount;
    if (totalAfter > capacity) {
        return false;
    }

    resources[type] += amount;
    return true;
}

bool Inventory::removeResource(ResourceType type, float amount)
{
    if (amount <= 0.0f) {
        return false;
    }

    auto it = resources.find(type);
    if (it == resources.end() || it->second < amount) {
        return false;
    }

    it->second -= amount;
    if (it->second <= 0.0f) {
        resources.erase(it);
    }

    return true;
}

float Inventory::getResourceAmount(ResourceType type) const
{
    auto it = resources.find(type);
    return (it != resources.end()) ? it->second : 0.0f;
}

bool Inventory::hasResource(ResourceType type, float amount) const
{
    return getResourceAmount(type) >= amount;
}

float Inventory::getTotalUsed() const
{
    float total = 0.0f;
    for (const auto& pair : resources) {
        total += pair.second;
    }
    return total;
}

void Inventory::clear()
{
    resources.clear();
}

} // namespace rpg
} // namespace fresh
