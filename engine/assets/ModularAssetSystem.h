#pragma once
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <glm/glm.hpp>

namespace fresh
{

class ResourceManager;
class VoxelWorld;
class TerrainGenerator;

/**
 * @brief Placement rule for world generation
 */
enum class PlacementRule {
    Random,    // Random placement in biome
    Clustered, // Grouped together
    Grid,      // Regular grid pattern
    NearWater, // Near water sources
    OnHills,   // On elevated terrain
    InCaves,   // Underground caves
    RiverSide  // Along rivers
};

/**
 * @brief Biome type for asset placement
 */
enum class BiomeType {
    Any, // Can place in any biome
    Forest,
    Plains,
    Desert,
    Mountains,
    Swamp,
    Tundra,
    Ocean,
    Cave
};

/**
 * @brief Asset metadata for world generation
 */
struct AssetMetadata {
    std::string name;
    std::string description;
    std::string modelPath;
    std::string texturePath;

    // Placement rules
    std::vector<BiomeType> allowedBiomes;
    PlacementRule placementRule = PlacementRule::Random;
    float spawnChance = 0.1f;  // 0.0 to 1.0
    float minDistance = 10.0f; // Min distance between instances
    int minGroupSize = 1;      // For clustered placement
    int maxGroupSize = 5;

    // Dimensions
    glm::vec3 size{1.0f};
    glm::vec3 offset{0.0f};

    // Rotation
    bool randomRotation = true;
    float rotationStep = 90.0f; // Degrees (90 = only cardinal directions)

    // World generation
    bool placeOnSurface = true;
    bool embedInTerrain = false; // Partially sink into terrain
    float embedDepth = 0.0f;

    // Tags for filtering
    std::vector<std::string> tags;

    // Custom properties
    std::map<std::string, std::string> customProperties;
};

/**
 * @brief Asset pack containing multiple related assets
 */
class AssetPack
{
public:
    AssetPack(const std::string& packPath);
    ~AssetPack();

    bool load();
    void unload();

    const std::string& getName() const
    {
        return name;
    }
    const std::string& getVersion() const
    {
        return version;
    }
    const std::vector<AssetMetadata>& getAssets() const
    {
        return assets;
    }

    bool isLoaded() const
    {
        return loaded;
    }

    // Get assets by filter
    std::vector<AssetMetadata> getAssetsByBiome(BiomeType biome) const;
    std::vector<AssetMetadata> getAssetsByTag(const std::string& tag) const;
    std::vector<AssetMetadata> getAssetsByRule(PlacementRule rule) const;

private:
    bool parseManifest(const std::string& manifestPath);

    std::string packPath;
    std::string name;
    std::string version;
    std::string author;
    std::vector<AssetMetadata> assets;
    bool loaded = false;
};

/**
 * @brief Instance of an asset placed in the world
 */
struct AssetInstance {
    std::string assetName;
    glm::vec3 position;
    glm::vec3 rotation; // Euler angles in degrees
    glm::vec3 scale{1.0f};
    BiomeType biome;
    void* userData = nullptr; // For custom data
};

/**
 * @brief Modular asset system with auto-detection
 *
 * Automatically discovers and loads asset packs from the assets directory.
 * Integrates with world generation to populate the world with diverse content.
 * Supports marketplace and custom asset packs with simple drop-in installation.
 */
class ModularAssetSystem
{
public:
    static ModularAssetSystem& getInstance();

    /**
     * @brief Initialize and scan for asset packs
     * @param assetDirectory Root directory containing asset packs
     */
    void initialize(const std::string& assetDirectory);

    /**
     * @brief Shutdown and unload all assets
     */
    void shutdown();

    /**
     * @brief Scan directory for asset packs and load them
     */
    void scanAndLoadAssetPacks();

    /**
     * @brief Manually load an asset pack
     */
    bool loadAssetPack(const std::string& packPath);

    /**
     * @brief Unload an asset pack
     */
    void unloadAssetPack(const std::string& packName);

    /**
     * @brief Get all loaded asset packs
     */
    const std::vector<std::shared_ptr<AssetPack>>& getLoadedPacks() const
    {
        return loadedPacks;
    }

    /**
     * @brief Get asset pack by name
     */
    AssetPack* getAssetPack(const std::string& name);

    /**
     * @brief Get all assets matching criteria
     */
    std::vector<AssetMetadata> queryAssets(BiomeType biome = BiomeType::Any,
                                           const std::string& tag = "",
                                           PlacementRule rule = PlacementRule::Random) const;

    /**
     * @brief Generate asset placements for a chunk
     * @param world VoxelWorld for terrain queries
     * @param chunkX Chunk X coordinate
     * @param chunkZ Chunk Z coordinate
     * @param seed Random seed for deterministic generation
     * @return List of assets to place in this chunk
     */
    std::vector<AssetInstance> generateAssetsForChunk(VoxelWorld* world, int chunkX, int chunkZ,
                                                      uint32_t seed);

    /**
     * @brief Place assets in world based on placement rules
     * @param instances Asset instances to place
     * @param world World to place assets in
     */
    void placeAssetsInWorld(const std::vector<AssetInstance>& instances, VoxelWorld* world);

    /**
     * @brief Create asset pack template
     * Generates a template manifest file for creating custom asset packs
     */
    static bool createAssetPackTemplate(const std::string& outputPath);

    /**
     * @brief Validate asset pack
     * Checks if asset pack is valid and all files exist
     */
    bool validateAssetPack(const std::string& packPath) const;

    /**
     * @brief Get statistics about loaded assets
     */
    void printStats() const;

    /**
     * @brief Register custom placement function
     * Allows game code to define custom placement logic
     */
    using PlacementFunction = std::function<std::vector<AssetInstance>(
        const AssetMetadata&, VoxelWorld*, int chunkX, int chunkZ, uint32_t seed)>;

    void registerPlacementFunction(PlacementRule rule, PlacementFunction func);

private:
    ModularAssetSystem() = default;
    ~ModularAssetSystem() = default;
    ModularAssetSystem(const ModularAssetSystem&) = delete;
    ModularAssetSystem& operator=(const ModularAssetSystem&) = delete;

    std::vector<AssetInstance> placeRandomly(const AssetMetadata& asset, VoxelWorld* world,
                                             int chunkX, int chunkZ, uint32_t seed);

    std::vector<AssetInstance> placeClustered(const AssetMetadata& asset, VoxelWorld* world,
                                              int chunkX, int chunkZ, uint32_t seed);

    std::vector<AssetInstance> placeGrid(const AssetMetadata& asset, VoxelWorld* world, int chunkX,
                                         int chunkZ, uint32_t seed);

    bool canPlaceAsset(const AssetMetadata& asset, const glm::vec3& position, VoxelWorld* world,
                       BiomeType biome);

    BiomeType determineBiome(VoxelWorld* world, const glm::vec3& position);

    std::string assetDirectory;
    std::vector<std::shared_ptr<AssetPack>> loadedPacks;
    std::map<PlacementRule, PlacementFunction> placementFunctions;

    // Spatial tracking to enforce minimum distance
    std::map<std::string, std::vector<glm::vec2>> placedAssetPositions;
};

} // namespace fresh
