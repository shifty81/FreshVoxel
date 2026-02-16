#pragma once

#include <string>

namespace fresh
{

/**
 * @brief 2D World style enumeration
 */
enum class WorldStyle2D {
    Platformer = 0,  // Side-scrolling (Terraria-like)
    TopDown = 1      // Overhead view (Zelda-like)
};

/**
 * @brief World size enumeration
 */
enum class WorldSize {
    Small = 0,    // 8 chunks radius
    Medium = 1,   // 16 chunks radius
    Large = 2,    // 24 chunks radius
    Huge = 3      // 32 chunks radius
};

/**
 * @brief Terrain type enumeration
 */
enum class TerrainType {
    Flat = 0,
    Hills = 1,
    Mountains = 2,
    Islands = 3
};

/**
 * @brief Biome type enumeration
 */
enum class BiomeType {
    Forest = 0,
    Desert = 1,
    Snow = 2,
    Jungle = 3,
    Mixed = 4
};

/**
 * @brief World creation parameters structure
 */
struct WorldCreationParams {
    std::string name;
    int seed;
    bool is3D;
    int gameStyle2D; // WorldStyle2D cast to int for backward compatibility
    WorldSize worldSize;
    TerrainType terrainType;
    BiomeType biomeType;

    WorldCreationParams()
        : name("New Project")
        , seed(0)
        , is3D(true)
        , gameStyle2D(0)
        , worldSize(WorldSize::Medium)
        , terrainType(TerrainType::Hills)
        , biomeType(BiomeType::Mixed)
    {}
};

} // namespace fresh
