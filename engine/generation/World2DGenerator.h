#pragma once

#include "generation/TerrainGenerator.h"
#include "generation/NoiseGenerator.h"
#include <glm/glm.hpp>
#include <memory>
#include <vector>

namespace fresh {

/**
 * @brief 2D World Generator for side-scrolling platformer-style worlds
 * 
 * Generates 2D voxel worlds similar to Terraria or Starbound, where the world
 * has a fixed depth (typically 1-3 blocks) and extends horizontally and vertically.
 * Perfect for platformer, metroidvania, and endless runner game styles.
 */
class World2DGenerator {
public:
    /**
     * @brief Generation styles for 2D worlds
     */
    enum class Style {
        PLATFORMER,     ///< Terraria-style horizontal exploration
        TOPDOWN,        ///< Zelda-style top-down exploration
        METROIDVANIA,   ///< Connected rooms requiring abilities to progress
        RUNNER,         ///< Endless runner with procedural segments
        PUZZLE          ///< Room-based puzzle platformer
    };

    /**
     * @brief Settings for 2D world generation
     */
    struct Settings {
        Style style = Style::PLATFORMER;
        int worldWidth = 1024;      ///< Width in blocks
        int worldHeight = 256;      ///< Height in blocks
        int worldDepth = 1;         ///< Depth in blocks (Z-axis), typically 1-3
        uint64_t seed = 0;          ///< Random seed
        
        // Terrain settings
        float surfaceLevel = 0.5f;  ///< Relative surface height (0-1)
        float surfaceVariation = 30.0f; ///< Height variation
        
        // Cave settings
        float caveFrequency = 0.3f; ///< How many caves (0-1)
        int caveIterations = 5;     ///< Cellular automata iterations
        
        // Feature settings
        float oreFrequency = 0.1f;  ///< Ore deposit frequency
        float treeFrequency = 0.05f; ///< Surface tree frequency
        int dungeonCount = 2;       ///< Number of dungeons
        bool generateBackgroundWalls = true; ///< Generate background walls
    };

    /**
     * @brief Construct a 2D world generator with default settings
     */
    World2DGenerator();
    
    /**
     * @brief Construct a 2D world generator
     * @param settings Generation settings
     */
    explicit World2DGenerator(const Settings& settings);

    /**
     * @brief Set generation style
     */
    void setStyle(Style style) { settings_.style = style; }

    /**
     * @brief Set world dimensions
     */
    void setDimensions(int width, int height, int depth);

    /**
     * @brief Set random seed
     */
    void setSeed(uint64_t seed);

    /**
     * @brief Generate a 2D world chunk
     * @param chunk Chunk to fill
     * @param chunkX Chunk X coordinate (in world space: chunkX * 16)
     * @param chunkZ Chunk Z coordinate (should be 0 for pure 2D)
     */
    void generateChunk(Chunk& chunk, int chunkX, int chunkZ);

    /**
     * @brief Generate entire 2D world at once
     * @param world World to generate into
     * @note Use this for small 2D worlds that fit in memory
     */
    void generateWorld(class VoxelWorld& world);

    /**
     * @brief Get current settings
     */
    const Settings& getSettings() const { return settings_; }

private:
    Settings settings_;
    std::unique_ptr<NoiseGenerator> surfaceNoise_;
    std::unique_ptr<NoiseGenerator> caveNoise_;
    std::unique_ptr<NoiseGenerator> oreNoise_;

    /**
     * @brief Generate surface terrain for platformer style
     */
    void generatePlatformerSurface(Chunk& chunk, int chunkX);

    /**
     * @brief Generate top-down world (Zelda-style)
     */
    void generateTopDownWorld(Chunk& chunk, int chunkX, int chunkZ);

    /**
     * @brief Generate underground layers
     */
    void generateUnderground(Chunk& chunk, int chunkX, int surfaceHeight);

    /**
     * @brief Generate caves using cellular automata
     */
    void generateCaves(Chunk& chunk, int chunkX);

    /**
     * @brief Place ore deposits
     */
    void generateOres(Chunk& chunk, int chunkX);

    /**
     * @brief Generate surface features (trees, rocks, etc.)
     */
    void generateSurfaceFeatures(Chunk& chunk, int chunkX);

    /**
     * @brief Generate background walls
     */
    void generateBackgroundWalls(Chunk& chunk, int chunkX);

    /**
     * @brief Get voxel type based on depth from surface
     */
    VoxelType getVoxelTypeForDepth(int depthFromSurface) const;

    /**
     * @brief Count solid neighbors for cellular automata
     */
    int countSolidNeighbors(const Chunk& chunk, int x, int y, int z) const;

    /**
     * @brief Generate a simple tree at position
     */
    void generateTree(Chunk& chunk, int x, int y, int z);

    /**
     * @brief Check if position is valid for tree
     */
    bool isValidTreePosition(const Chunk& chunk, int x, int y, int z) const;
};

/**
 * @brief Metroidvania-style room-based generator
 */
class MetroidvaniaGenerator {
public:
    /**
     * @brief Room types in metroidvania
     */
    enum class RoomType {
        START,          ///< Starting room
        NORMAL,         ///< Regular room
        TREASURE,       ///< Contains valuable items
        BOSS,           ///< Boss fight room
        SAVE_POINT,     ///< Checkpoint/save station
        ABILITY_UNLOCK  ///< Grants new ability
    };

    /**
     * @brief Abilities required to access rooms
     */
    enum class AbilityRequired {
        NONE,
        DOUBLE_JUMP,
        WALL_JUMP,
        DASH,
        SWIM,
        GRAPPLE
    };

    /**
     * @brief Room data structure
     */
    struct Room {
        glm::ivec2 position;
        glm::ivec2 size;
        RoomType type;
        AbilityRequired requiredAbility;
        std::vector<glm::ivec2> doorPositions;
        int difficulty;
    };

    /**
     * @brief Generate connected metroidvania-style rooms
     */
    std::vector<Room> generate(int roomCount, uint64_t seed);

private:
    std::vector<Room> generateMainPath(int roomCount);
    std::vector<Room> generateSideAreas(const std::vector<Room>& mainPath);
    void connectRooms(std::vector<Room>& rooms);
    Room createRoom(RoomType type, int difficulty);
};

/**
 * @brief Endless runner segment generator
 */
class EndlessRunnerGenerator {
public:
    /**
     * @brief Obstacle types
     */
    struct Obstacle {
        int x;              ///< X position in segment
        int y;              ///< Y position
        int width;          ///< Width in blocks
        int height;         ///< Height in blocks
        VoxelType type;     ///< Block type
    };

    /**
     * @brief Platform data
     */
    struct Platform {
        int x;              ///< X position in segment
        int y;              ///< Y position
        int width;          ///< Platform width
        VoxelType type;     ///< Block type
    };

    /**
     * @brief Collectible item
     */
    struct Collectible {
        int x;              ///< X position in segment
        int y;              ///< Y position
        // Item type would go here
    };

    /**
     * @brief Generated segment
     */
    struct Segment {
        int length;         ///< Segment length in blocks
        float difficulty;   ///< Current difficulty level
        std::vector<Obstacle> obstacles;
        std::vector<Platform> platforms;
        std::vector<Collectible> collectibles;
    };

    /**
     * @brief Generate next segment based on current difficulty
     */
    Segment generateNextSegment(float currentDifficulty);

    /**
     * @brief Build segment into world
     */
    void buildSegment(class VoxelWorld& world, const Segment& segment, int startX);

private:
    Obstacle generateObstacle(float difficulty);
    Platform generatePlatform();
    Collectible generateCollectible();
    float difficultyIncrease_ = 0.01f;
};

} // namespace fresh
