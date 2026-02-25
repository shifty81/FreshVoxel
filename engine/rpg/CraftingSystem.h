#pragma once
#include <algorithm>
#include <map>
#include <string>
#include <vector>

#include "Inventory.h"

namespace fresh
{
namespace rpg
{

/**
 * @brief Categories of craftable items in the voxel game
 */
enum class ItemCategory { 
    Tool,           // Pickaxe, Axe, Shovel, Hoe
    Weapon,         // Sword, Bow, Arrow
    Armor,          // Helmet, Chestplate, Leggings, Boots
    Building,       // Crafting table, Furnace, Chest
    Material,       // Stick, Plank, processed materials
    Food,           // Bread, Cooked meat
    Utility,        // Bucket, Compass, Clock
    Decoration      // Paintings, flower pots
};

/**
 * @brief Tool/material tiers for crafting progression
 */
enum class MaterialTier {
    Wood = 0,
    Stone = 1,
    Iron = 2,
    Gold = 3,
    Diamond = 4
};

/**
 * @brief Represents a crafted item in the voxel game
 */
class CraftedItem
{
public:
    CraftedItem();
    CraftedItem(ItemCategory category, MaterialTier tier, const std::string& name);

    ItemCategory getCategory() const
    {
        return category;
    }
    
    // Alias for backward compatibility with code expecting SubsystemType
    ItemCategory getType() const
    {
        return category;
    }
    
    MaterialTier getTier() const
    {
        return tier;
    }
    
    int getLevel() const
    {
        return static_cast<int>(tier);
    }
    const std::string& getName() const
    {
        return name;
    }

    // Stats (durability, damage, speed, etc.)
    float getStatBonus(const std::string& statName) const;
    void setStatBonus(const std::string& statName, float value);

    // Cost (for reference, actual requirements stored in recipe)
    const std::map<ResourceType, float>& getCost() const
    {
        return cost;
    }
    void setCost(const std::map<ResourceType, float>& c)
    {
        cost = c;
    }
    
    // Stack size (most items stack to 64, tools/weapons don't stack)
    int getMaxStackSize() const { return maxStackSize; }
    void setMaxStackSize(int size) { maxStackSize = size; }
    
    // Durability (0 = infinite/doesn't apply)
    int getMaxDurability() const { return maxDurability; }
    void setMaxDurability(int durability) { maxDurability = durability; }

private:
    ItemCategory category;
    MaterialTier tier;
    std::string name;
    std::map<std::string, float> statBonuses;
    std::map<ResourceType, float> cost;
    int maxStackSize = 64;
    int maxDurability = 0;
};

// Type alias for backward compatibility
using SubsystemUpgrade = CraftedItem;
using SubsystemType = ItemCategory;

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

    // Recipe search/filter
    /**
     * @brief Search recipes by name (case-insensitive substring match)
     * @param query Search query string
     * @return Vector of matching recipe names
     */
    std::vector<std::string> searchRecipes(const std::string& query) const;

    /**
     * @brief Filter recipes by subsystem type
     * @param type Subsystem type to filter by
     * @return Vector of matching recipe names
     */
    std::vector<std::string> getRecipesByType(SubsystemType type) const;

    /**
     * @brief Get recipes that can be crafted with current inventory
     * @param inventory Player inventory to check against
     * @return Vector of craftable recipe names
     */
    std::vector<std::string> getCraftableRecipes(const Inventory& inventory) const;

    // Crafting
    bool canCraft(const std::string& recipeName, const Inventory& inventory) const;
    bool craft(const std::string& recipeName, Inventory& inventory, SubsystemUpgrade& outUpgrade);

private:
    std::map<std::string, CraftingRecipe> recipes;

    void initializeDefaultRecipes();
};

} // namespace rpg
} // namespace fresh
