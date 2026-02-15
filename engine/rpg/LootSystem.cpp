#include "rpg/LootSystem.h"

namespace fresh
{
namespace rpg
{

LootTable::LootTable() {}

void LootTable::addDrop(const LootDrop& drop)
{
    drops.push_back(drop);
}

void LootTable::clear()
{
    drops.clear();
}

std::vector<LootDrop> LootTable::generateLoot(std::mt19937& rng) const
{
    std::vector<LootDrop> result;
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    for (const auto& drop : drops) {
        if (dist(rng) <= drop.dropChance) {
            result.push_back(drop);
        }
    }

    return result;
}

LootSystem::LootSystem()
{
    initializeDefaultLootTables();
}

void LootSystem::registerLootTable(const std::string& name, const LootTable& table)
{
    lootTables[name] = table;
}

const LootTable* LootSystem::getLootTable(const std::string& name) const
{
    auto it = lootTables.find(name);
    return (it != lootTables.end()) ? &it->second : nullptr;
}

std::vector<LootDrop> LootSystem::generateLoot(const std::string& tableName, uint32_t seed)
{
    const LootTable* table = getLootTable(tableName);
    if (!table) {
        return {};
    }

    std::mt19937 rng(seed);
    return table->generateLoot(rng);
}

std::vector<LootDrop> LootSystem::generateAsteroidLoot(uint32_t seed)
{
    return generateLoot("asteroid", seed);
}

std::vector<LootDrop> LootSystem::generateEnemyLoot(int enemyLevel, uint32_t seed)
{
    std::string tableName = "enemy_level_" + std::to_string(enemyLevel);
    return generateLoot(tableName, seed);
}

void LootSystem::initializeDefaultLootTables()
{
    // Asteroid loot table
    {
        LootTable table;
        table.addDrop({ResourceType::Iron, 50.0f, 0.8f});
        table.addDrop({ResourceType::Titanium, 30.0f, 0.5f});
        table.addDrop({ResourceType::Naonite, 20.0f, 0.3f});
        table.addDrop({ResourceType::Trinium, 10.0f, 0.1f});
        registerLootTable("asteroid", table);
    }

    // Enemy level 1 loot
    {
        LootTable table;
        table.addDrop({ResourceType::Credits, 100.0f, 1.0f});
        table.addDrop({ResourceType::Iron, 20.0f, 0.6f});
        table.addDrop({ResourceType::Energy, 50.0f, 0.4f});
        registerLootTable("enemy_level_1", table);
    }

    // Enemy level 5 loot
    {
        LootTable table;
        table.addDrop({ResourceType::Credits, 500.0f, 1.0f});
        table.addDrop({ResourceType::Titanium, 50.0f, 0.7f});
        table.addDrop({ResourceType::Naonite, 30.0f, 0.5f});
        table.addDrop({ResourceType::Energy, 100.0f, 0.6f});
        registerLootTable("enemy_level_5", table);
    }
}

} // namespace rpg
} // namespace fresh
