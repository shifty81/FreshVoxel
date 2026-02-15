#pragma once
#include <map>
#include <string>
#include <vector>

#include "Inventory.h"

namespace fresh
{
namespace rpg
{

/**
 * @brief Types of subsystem upgrades
 */
enum class SubsystemType { Shield, Weapon, Engine, Cargo, Generator, Computer };

/**
 * @brief Represents a ship/station subsystem upgrade
 */
class SubsystemUpgrade
{
public:
    SubsystemUpgrade();
    SubsystemUpgrade(SubsystemType type, int level, const std::string& name);

    SubsystemType getType() const
    {
        return type;
    }
    int getLevel() const
    {
        return level;
    }
    const std::string& getName() const
    {
        return name;
    }

    // Stats
    float getStatBonus(const std::string& statName) const;
    void setStatBonus(const std::string& statName, float value);

    // Cost
    const std::map<ResourceType, float>& getCost() const
    {
        return cost;
    }
    void setCost(const std::map<ResourceType, float>& c)
    {
        cost = c;
    }

private:
    SubsystemType type;
    int level;
    std::string name;
    std::map<std::string, float> statBonuses;
    std::map<ResourceType, float> cost;
};

/**
 * @brief Recipe for crafting upgrades
 */
struct CraftingRecipe {
    std::string name;
    SubsystemUpgrade result;
    std::map<ResourceType, float> requirements;
    float craftingTime; // in seconds
};

/**
 * @brief Handles crafting of upgrades
 */
class CraftingSystem
{
public:
    CraftingSystem();

    // Recipe management
    void addRecipe(const CraftingRecipe& recipe);
    const CraftingRecipe* getRecipe(const std::string& name) const;
    std::vector<std::string> getAllRecipeNames() const;

    // Crafting
    bool canCraft(const std::string& recipeName, const Inventory& inventory) const;
    bool craft(const std::string& recipeName, Inventory& inventory, SubsystemUpgrade& outUpgrade);

private:
    std::map<std::string, CraftingRecipe> recipes;

    void initializeDefaultRecipes();
};

} // namespace rpg
} // namespace fresh
