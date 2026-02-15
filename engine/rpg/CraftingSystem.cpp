#include "rpg/CraftingSystem.h"

namespace fresh
{
namespace rpg
{

SubsystemUpgrade::SubsystemUpgrade() : type(SubsystemType::Shield), level(1), name("Basic Upgrade")
{
}

SubsystemUpgrade::SubsystemUpgrade(SubsystemType type, int level, const std::string& name)
    : type(type), level(level), name(name)
{
}

float SubsystemUpgrade::getStatBonus(const std::string& statName) const
{
    auto it = statBonuses.find(statName);
    return (it != statBonuses.end()) ? it->second : 0.0f;
}

void SubsystemUpgrade::setStatBonus(const std::string& statName, float value)
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

void CraftingSystem::initializeDefaultRecipes()
{
    // Basic Shield
    {
        CraftingRecipe recipe;
        recipe.name = "Basic Shield";
        recipe.result = SubsystemUpgrade(SubsystemType::Shield, 1, "Basic Shield");
        recipe.result.setStatBonus("shieldCapacity", 100.0f);
        recipe.result.setStatBonus("shieldRecharge", 10.0f);
        recipe.requirements[ResourceType::Iron] = 50.0f;
        recipe.requirements[ResourceType::Titanium] = 20.0f;
        recipe.craftingTime = 30.0f;
        addRecipe(recipe);
    }

    // Advanced Shield
    {
        CraftingRecipe recipe;
        recipe.name = "Advanced Shield";
        recipe.result = SubsystemUpgrade(SubsystemType::Shield, 2, "Advanced Shield");
        recipe.result.setStatBonus("shieldCapacity", 250.0f);
        recipe.result.setStatBonus("shieldRecharge", 25.0f);
        recipe.requirements[ResourceType::Titanium] = 100.0f;
        recipe.requirements[ResourceType::Naonite] = 50.0f;
        recipe.craftingTime = 60.0f;
        addRecipe(recipe);
    }

    // Basic Weapon
    {
        CraftingRecipe recipe;
        recipe.name = "Basic Weapon";
        recipe.result = SubsystemUpgrade(SubsystemType::Weapon, 1, "Basic Laser");
        recipe.result.setStatBonus("damage", 25.0f);
        recipe.result.setStatBonus("fireRate", 2.0f);
        recipe.requirements[ResourceType::Iron] = 30.0f;
        recipe.requirements[ResourceType::Titanium] = 30.0f;
        recipe.craftingTime = 45.0f;
        addRecipe(recipe);
    }

    // Cargo Bay Expansion
    {
        CraftingRecipe recipe;
        recipe.name = "Cargo Expansion";
        recipe.result = SubsystemUpgrade(SubsystemType::Cargo, 1, "Cargo Bay Mk1");
        recipe.result.setStatBonus("cargoCapacity", 500.0f);
        recipe.requirements[ResourceType::Iron] = 100.0f;
        recipe.craftingTime = 40.0f;
        addRecipe(recipe);
    }
}

} // namespace rpg
} // namespace fresh
