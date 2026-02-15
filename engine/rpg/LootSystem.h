#pragma once
#include <random>
#include <vector>

#include "CraftingSystem.h"
#include "Inventory.h"

namespace fresh
{
namespace rpg
{

/**
 * @brief Loot drop item
 */
struct LootDrop {
    ResourceType resource;
    float amount;
    float dropChance; // 0.0 to 1.0
};

/**
 * @brief Loot table for generating drops
 */
class LootTable
{
public:
    LootTable();

    void addDrop(const LootDrop& drop);
    void clear();

    // Generate loot drops based on probabilities
    std::vector<LootDrop> generateLoot(std::mt19937& rng) const;

private:
    std::vector<LootDrop> drops;
};

/**
 * @brief System for generating loot drops
 */
class LootSystem
{
public:
    LootSystem();

    // Register loot tables by name
    void registerLootTable(const std::string& name, const LootTable& table);
    const LootTable* getLootTable(const std::string& name) const;

    // Generate loot from a specific table
    std::vector<LootDrop> generateLoot(const std::string& tableName, uint32_t seed = 0);

    // Quick loot generation for common scenarios
    std::vector<LootDrop> generateAsteroidLoot(uint32_t seed = 0);
    std::vector<LootDrop> generateEnemyLoot(int enemyLevel, uint32_t seed = 0);

private:
    std::map<std::string, LootTable> lootTables;

    void initializeDefaultLootTables();
};

} // namespace rpg
} // namespace fresh
