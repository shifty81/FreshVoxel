#include "rpg/CraftingSystem.h"
#include <cctype>

namespace fresh
{
namespace rpg
{

CraftedItem::CraftedItem() : category(ItemCategory::Material), tier(MaterialTier::Wood), name("Unknown Item")
{
}

CraftedItem::CraftedItem(ItemCategory cat, MaterialTier t, const std::string& n)
    : category(cat), tier(t), name(n)
{
    // Set default durability based on tier for tools/weapons
    if (category == ItemCategory::Tool || category == ItemCategory::Weapon) {
        switch (tier) {
            case MaterialTier::Wood:    maxDurability = 60; break;
            case MaterialTier::Stone:   maxDurability = 132; break;
            case MaterialTier::Iron:    maxDurability = 251; break;
            case MaterialTier::Gold:    maxDurability = 33; break;
            case MaterialTier::Diamond: maxDurability = 1562; break;
        }
        maxStackSize = 1; // Tools/weapons don't stack
    }
    
    if (category == ItemCategory::Armor) {
        maxStackSize = 1; // Armor doesn't stack
    }
}

float CraftedItem::getStatBonus(const std::string& statName) const
{
    auto it = statBonuses.find(statName);
    return (it != statBonuses.end()) ? it->second : 0.0f;
}

void CraftedItem::setStatBonus(const std::string& statName, float value)
{
    statBonuses[statName] = value;
}

CraftingSystem::CraftingSystem()
{
    initializeDefaultRecipes();
}

void CraftingSystem::addRecipe(const CraftingRecipe& recipe)
{
    recipes[recipe.name] = recipe;
}

const CraftingRecipe* CraftingSystem::getRecipe(const std::string& name) const
{
    auto it = recipes.find(name);
    return (it != recipes.end()) ? &it->second : nullptr;
}

std::vector<std::string> CraftingSystem::getAllRecipeNames() const
{
    std::vector<std::string> names;
    names.reserve(recipes.size());
    for (const auto& pair : recipes) {
        names.push_back(pair.first);
    }
    return names;
}

bool CraftingSystem::canCraft(const std::string& recipeName, const Inventory& inventory) const
{
    const CraftingRecipe* recipe = getRecipe(recipeName);
    if (!recipe) {
        return false;
    }

    for (const auto& req : recipe->requirements) {
        if (!inventory.hasResource(req.first, req.second)) {
            return false;
        }
    }

    return true;
}

bool CraftingSystem::craft(const std::string& recipeName, Inventory& inventory,
                           SubsystemUpgrade& outUpgrade)
{
    const CraftingRecipe* recipe = getRecipe(recipeName);
    if (!recipe || !canCraft(recipeName, inventory)) {
        return false;
    }

    // Consume resources
    for (const auto& req : recipe->requirements) {
        if (!inventory.removeResource(req.first, req.second)) {
            // This shouldn't happen if canCraft passed, but check anyway
            return false;
        }
    }

    // Create the upgrade
    outUpgrade = recipe->result;
    return true;
}

std::vector<std::string> CraftingSystem::searchRecipes(const std::string& query) const
{
    std::vector<std::string> results;
    if (query.empty()) {
        return getAllRecipeNames();
    }

    // Convert query to lowercase for case-insensitive search
    std::string lowerQuery = query;
    std::transform(lowerQuery.begin(), lowerQuery.end(), lowerQuery.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    for (const auto& pair : recipes) {
        std::string lowerName = pair.first;
        std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(),
                       [](unsigned char c) { return std::tolower(c); });

        if (lowerName.find(lowerQuery) != std::string::npos) {
            results.push_back(pair.first);
        }
    }
    return results;
}

std::vector<std::string> CraftingSystem::getRecipesByType(SubsystemType type) const
{
    std::vector<std::string> results;
    for (const auto& pair : recipes) {
        if (pair.second.result.getType() == type) {
            results.push_back(pair.first);
        }
    }
    return results;
}

std::vector<std::string> CraftingSystem::getCraftableRecipes(const Inventory& inventory) const
{
    std::vector<std::string> results;
    for (const auto& pair : recipes) {
        if (canCraft(pair.first, inventory)) {
            results.push_back(pair.first);
        }
    }
    return results;
}

void CraftingSystem::initializeDefaultRecipes()
{
    // =====================
    // BASIC MATERIALS
    // =====================
    
    // Planks (from Wood)
    {
        CraftingRecipe recipe;
        recipe.name = "Planks";
        recipe.result = CraftedItem(ItemCategory::Material, MaterialTier::Wood, "Planks");
        recipe.result.setStatBonus("yield", 4.0f); // 1 wood = 4 planks
        recipe.requirements[ResourceType::Wood] = 1.0f;
        recipe.craftingTime = 0.0f; // Instant
        addRecipe(recipe);
    }
    
    // Sticks (from Planks)
    {
        CraftingRecipe recipe;
        recipe.name = "Sticks";
        recipe.result = CraftedItem(ItemCategory::Material, MaterialTier::Wood, "Sticks");
        recipe.result.setStatBonus("yield", 4.0f); // 2 planks = 4 sticks
        recipe.requirements[ResourceType::Plank] = 2.0f;
        recipe.craftingTime = 0.0f;
        addRecipe(recipe);
    }
    
    // =====================
    // WOODEN TOOLS
    // =====================
    
    // Wooden Pickaxe
    {
        CraftingRecipe recipe;
        recipe.name = "Wooden Pickaxe";
        recipe.result = CraftedItem(ItemCategory::Tool, MaterialTier::Wood, "Wooden Pickaxe");
        recipe.result.setStatBonus("miningSpeed", 2.0f);
        recipe.result.setStatBonus("miningLevel", 1.0f); // Can mine stone
        recipe.requirements[ResourceType::Plank] = 3.0f;
        recipe.requirements[ResourceType::Wood] = 2.0f; // Sticks substitute
        recipe.craftingTime = 1.0f;
        addRecipe(recipe);
    }
    
    // Wooden Axe
    {
        CraftingRecipe recipe;
        recipe.name = "Wooden Axe";
        recipe.result = CraftedItem(ItemCategory::Tool, MaterialTier::Wood, "Wooden Axe");
        recipe.result.setStatBonus("choppingSpeed", 2.0f);
        recipe.result.setStatBonus("damage", 4.0f);
        recipe.requirements[ResourceType::Plank] = 3.0f;
        recipe.requirements[ResourceType::Wood] = 2.0f;
        recipe.craftingTime = 1.0f;
        addRecipe(recipe);
    }
    
    // Wooden Shovel
    {
        CraftingRecipe recipe;
        recipe.name = "Wooden Shovel";
        recipe.result = CraftedItem(ItemCategory::Tool, MaterialTier::Wood, "Wooden Shovel");
        recipe.result.setStatBonus("diggingSpeed", 2.0f);
        recipe.requirements[ResourceType::Plank] = 1.0f;
        recipe.requirements[ResourceType::Wood] = 2.0f;
        recipe.craftingTime = 1.0f;
        addRecipe(recipe);
    }
    
    // Wooden Sword
    {
        CraftingRecipe recipe;
        recipe.name = "Wooden Sword";
        recipe.result = CraftedItem(ItemCategory::Weapon, MaterialTier::Wood, "Wooden Sword");
        recipe.result.setStatBonus("damage", 4.0f);
        recipe.result.setStatBonus("attackSpeed", 1.6f);
        recipe.requirements[ResourceType::Plank] = 2.0f;
        recipe.requirements[ResourceType::Wood] = 1.0f;
        recipe.craftingTime = 1.0f;
        addRecipe(recipe);
    }
    
    // =====================
    // STONE TOOLS
    // =====================
    
    // Stone Pickaxe
    {
        CraftingRecipe recipe;
        recipe.name = "Stone Pickaxe";
        recipe.result = CraftedItem(ItemCategory::Tool, MaterialTier::Stone, "Stone Pickaxe");
        recipe.result.setStatBonus("miningSpeed", 4.0f);
        recipe.result.setStatBonus("miningLevel", 2.0f); // Can mine iron
        recipe.requirements[ResourceType::Cobblestone] = 3.0f;
        recipe.requirements[ResourceType::Wood] = 2.0f;
        recipe.craftingTime = 1.0f;
        addRecipe(recipe);
    }
    
    // Stone Axe
    {
        CraftingRecipe recipe;
        recipe.name = "Stone Axe";
        recipe.result = CraftedItem(ItemCategory::Tool, MaterialTier::Stone, "Stone Axe");
        recipe.result.setStatBonus("choppingSpeed", 4.0f);
        recipe.result.setStatBonus("damage", 5.0f);
        recipe.requirements[ResourceType::Cobblestone] = 3.0f;
        recipe.requirements[ResourceType::Wood] = 2.0f;
        recipe.craftingTime = 1.0f;
        addRecipe(recipe);
    }
    
    // Stone Sword
    {
        CraftingRecipe recipe;
        recipe.name = "Stone Sword";
        recipe.result = CraftedItem(ItemCategory::Weapon, MaterialTier::Stone, "Stone Sword");
        recipe.result.setStatBonus("damage", 5.0f);
        recipe.result.setStatBonus("attackSpeed", 1.6f);
        recipe.requirements[ResourceType::Cobblestone] = 2.0f;
        recipe.requirements[ResourceType::Wood] = 1.0f;
        recipe.craftingTime = 1.0f;
        addRecipe(recipe);
    }
    
    // =====================
    // IRON TOOLS
    // =====================
    
    // Iron Pickaxe
    {
        CraftingRecipe recipe;
        recipe.name = "Iron Pickaxe";
        recipe.result = CraftedItem(ItemCategory::Tool, MaterialTier::Iron, "Iron Pickaxe");
        recipe.result.setStatBonus("miningSpeed", 6.0f);
        recipe.result.setStatBonus("miningLevel", 3.0f); // Can mine diamond
        recipe.requirements[ResourceType::IronIngot] = 3.0f;
        recipe.requirements[ResourceType::Wood] = 2.0f;
        recipe.craftingTime = 2.0f;
        addRecipe(recipe);
    }
    
    // Iron Sword
    {
        CraftingRecipe recipe;
        recipe.name = "Iron Sword";
        recipe.result = CraftedItem(ItemCategory::Weapon, MaterialTier::Iron, "Iron Sword");
        recipe.result.setStatBonus("damage", 6.0f);
        recipe.result.setStatBonus("attackSpeed", 1.6f);
        recipe.requirements[ResourceType::IronIngot] = 2.0f;
        recipe.requirements[ResourceType::Wood] = 1.0f;
        recipe.craftingTime = 2.0f;
        addRecipe(recipe);
    }
    
    // =====================
    // DIAMOND TOOLS
    // =====================
    
    // Diamond Pickaxe
    {
        CraftingRecipe recipe;
        recipe.name = "Diamond Pickaxe";
        recipe.result = CraftedItem(ItemCategory::Tool, MaterialTier::Diamond, "Diamond Pickaxe");
        recipe.result.setStatBonus("miningSpeed", 8.0f);
        recipe.result.setStatBonus("miningLevel", 4.0f); // Can mine obsidian
        recipe.requirements[ResourceType::Diamond] = 3.0f;
        recipe.requirements[ResourceType::Wood] = 2.0f;
        recipe.craftingTime = 3.0f;
        addRecipe(recipe);
    }
    
    // Diamond Sword
    {
        CraftingRecipe recipe;
        recipe.name = "Diamond Sword";
        recipe.result = CraftedItem(ItemCategory::Weapon, MaterialTier::Diamond, "Diamond Sword");
        recipe.result.setStatBonus("damage", 7.0f);
        recipe.result.setStatBonus("attackSpeed", 1.6f);
        recipe.requirements[ResourceType::Diamond] = 2.0f;
        recipe.requirements[ResourceType::Wood] = 1.0f;
        recipe.craftingTime = 3.0f;
        addRecipe(recipe);
    }
    
    // =====================
    // BUILDING/UTILITY
    // =====================
    
    // Crafting Table
    {
        CraftingRecipe recipe;
        recipe.name = "Crafting Table";
        recipe.result = CraftedItem(ItemCategory::Building, MaterialTier::Wood, "Crafting Table");
        recipe.result.setStatBonus("craftingSlots", 9.0f);
        recipe.requirements[ResourceType::Plank] = 4.0f;
        recipe.craftingTime = 1.0f;
        addRecipe(recipe);
    }
    
    // Furnace
    {
        CraftingRecipe recipe;
        recipe.name = "Furnace";
        recipe.result = CraftedItem(ItemCategory::Building, MaterialTier::Stone, "Furnace");
        recipe.result.setStatBonus("smeltingSpeed", 1.0f);
        recipe.requirements[ResourceType::Cobblestone] = 8.0f;
        recipe.craftingTime = 2.0f;
        addRecipe(recipe);
    }
    
    // Chest
    {
        CraftingRecipe recipe;
        recipe.name = "Chest";
        recipe.result = CraftedItem(ItemCategory::Building, MaterialTier::Wood, "Chest");
        recipe.result.setStatBonus("storageSlots", 27.0f);
        recipe.requirements[ResourceType::Plank] = 8.0f;
        recipe.craftingTime = 1.0f;
        addRecipe(recipe);
    }
    
    // Torch (yields 4)
    {
        CraftingRecipe recipe;
        recipe.name = "Torch";
        recipe.result = CraftedItem(ItemCategory::Utility, MaterialTier::Wood, "Torch");
        recipe.result.setStatBonus("yield", 4.0f);
        recipe.result.setStatBonus("lightLevel", 14.0f);
        recipe.requirements[ResourceType::Coal] = 1.0f;
        recipe.requirements[ResourceType::Wood] = 1.0f;
        recipe.craftingTime = 0.0f;
        addRecipe(recipe);
    }
    
    // =====================
    // FOOD
    // =====================
    
    // Bread
    {
        CraftingRecipe recipe;
        recipe.name = "Bread";
        recipe.result = CraftedItem(ItemCategory::Food, MaterialTier::Wood, "Bread");
        recipe.result.setStatBonus("hunger", 5.0f);
        recipe.result.setStatBonus("saturation", 6.0f);
        recipe.requirements[ResourceType::Wheat] = 3.0f;
        recipe.craftingTime = 0.0f;
        addRecipe(recipe);
    }
}

} // namespace rpg
} // namespace fresh
