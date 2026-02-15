#pragma once

#include <cstdint>
#include <functional>

namespace fresh
{

/**
 * @brief Voxel type enumeration
 */
enum class VoxelType : uint8_t {
    Air = 0,
    Stone,
    Dirt,
    Grass,
    Sand,
    Water,
    Wood,
    Leaves,
    Bedrock,
    Snow,
    Ice,
    Cobblestone,
    Planks,
    Glass,
    Brick,
    Gravel,
    Coal,
    Iron,
    Gold,
    Diamond,
    Obsidian,
    // Natural blocks
    Clay,
    Mud,
    SandStone,
    RedSand,
    RedSandStone,
    // Ore blocks
    CoalOre,
    IronOre,
    GoldOre,
    DiamondOre,
    EmeraldOre,
    // Crafted blocks
    StoneBrick,
    MossyStoneBrick,
    CrackedStoneBrick,
    ChiseledStoneBrick,
    // Wood variations
    OakWood,
    BirchWood,
    SpruceWood,
    JungleWood,
    // Decorative
    Wool,
    Concrete,
    Terracotta,
    // Functional blocks
    Torch,
    Ladder,
    Door,
    // Add more types as needed
    Count
};

/**
 * @brief Voxel data structure
 */
struct Voxel {
    VoxelType type;
    uint8_t light; // Light level 0-15

    Voxel() : type(VoxelType::Air), light(0) {}
    explicit Voxel(VoxelType t) : type(t), light(0) {}

    [[nodiscard]] bool isOpaque() const noexcept
    {
        return type != VoxelType::Air && type != VoxelType::Water && type != VoxelType::Glass;
    }

    [[nodiscard]] bool isTransparent() const noexcept
    {
        return type == VoxelType::Water || type == VoxelType::Glass || type == VoxelType::Ice;
    }

    [[nodiscard]] bool isSolid() const noexcept
    {
        return type != VoxelType::Air;
    }

    bool operator==(const Voxel& other) const noexcept
    {
        return type == other.type;
    }

    bool operator!=(const Voxel& other) const noexcept
    {
        return type != other.type;
    }

    bool operator==(VoxelType voxelType) const noexcept
    {
        return type == voxelType;
    }

    bool operator!=(VoxelType voxelType) const noexcept
    {
        return type != voxelType;
    }
};

/**
 * @brief Chunk dimensions
 */
constexpr int CHUNK_SIZE = 16;
constexpr int CHUNK_HEIGHT = 256;
constexpr int CHUNK_VOLUME = CHUNK_SIZE * CHUNK_SIZE * CHUNK_HEIGHT;

/**
 * @brief World coordinates
 */
struct WorldPos {
    int x, y, z;

    WorldPos() : x(0), y(0), z(0) {}
    WorldPos(int _x, int _y, int _z) : x(_x), y(_y), z(_z) {}

    bool operator==(const WorldPos& other) const noexcept
    {
        return x == other.x && y == other.y && z == other.z;
    }
};

/**
 * @brief Chunk coordinates
 */
struct ChunkPos {
    int x, z;

    ChunkPos() : x(0), z(0) {}
    ChunkPos(int _x, int _z) : x(_x), z(_z) {}

    bool operator==(const ChunkPos& other) const noexcept
    {
        return x == other.x && z == other.z;
    }

    static ChunkPos fromWorldPos(const WorldPos& pos) noexcept
    {
        return ChunkPos(pos.x >= 0 ? pos.x / CHUNK_SIZE : (pos.x + 1) / CHUNK_SIZE - 1,
                        pos.z >= 0 ? pos.z / CHUNK_SIZE : (pos.z + 1) / CHUNK_SIZE - 1);
    }
};

} // namespace fresh

// Hash function for ChunkPos to use in unordered_map
namespace std
{
template <>
struct hash<fresh::ChunkPos> {
    std::size_t operator()(const fresh::ChunkPos& pos) const noexcept
    {
        // Use a better hash combining technique (Boost-style hash_combine)
        // This provides better distribution than simple XOR
        std::size_t seed = 0;
        seed ^= std::hash<int>{}(pos.x) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        seed ^= std::hash<int>{}(pos.z) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        return seed;
    }
};
} // namespace std
