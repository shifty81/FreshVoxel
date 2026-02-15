#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <string>
#include "voxel/VoxelTypes.h"

namespace fresh
{

/**
 * @brief Biome types based on temperature and humidity
 * 
 * Following a similar system to Minecraft's biome distribution:
 * - Temperature ranges from cold (0) to hot (1)
 * - Humidity ranges from dry (0) to wet (1)
 */
enum class BiomeType
{
    // Cold biomes (temp < 0.33)
    Tundra,          // Cold + Dry
    Taiga,           // Cold + Medium humidity
    SnowyMountains,  // Cold + Wet (high elevation)
    IcePlains,       // Cold + Very dry
    
    // Temperate biomes (0.33 <= temp < 0.66)
    Plains,          // Temperate + Dry
    Forest,          // Temperate + Medium humidity
    DenseForest,     // Temperate + Wet
    Mountains,       // Temperate + Dry (high elevation)
    Hills,           // Temperate + Medium (mid elevation)
    
    // Warm biomes (temp >= 0.66)
    Desert,          // Warm + Very dry
    HotDesert,       // Warm + Extremely dry
    Savanna,         // Warm + Dry
    Jungle,          // Warm + Wet
    Swamp,           // Warm + Very wet (low elevation)
    
    // Special biomes
    Beach,           // Transition between land and water
    Ocean,           // Deep water
    DeepOcean,       // Very deep water
    River,           // Flowing water through land
    
    // Count
    Count
};

/**
 * @brief Biome properties and characteristics
 */
struct BiomeProperties
{
    BiomeType type;
    std::string name;
    
    // Climate properties
    float temperature;      // 0.0 (freezing) to 1.0 (scorching)
    float humidity;         // 0.0 (arid) to 1.0 (humid)
    float rainfall;         // 0.0 (no rain) to 1.0 (constant rain)
    
    // Terrain properties
    float heightVariation;  // How much terrain height varies (0-1)
    float heightOffset;     // Base height adjustment (-1 to 1)
    float roughness;        // How rough/smooth the terrain is (0-1)
    
    // Surface blocks
    VoxelType surfaceBlock;      // Top layer block
    VoxelType subSurfaceBlock;   // Below surface block
    VoxelType stoneBlock;        // Deep underground block
    
    // Colors for visualization
    glm::vec3 mapColor;          // Color on biome map
    
    // Vegetation
    float treeDensity;           // Trees per chunk (0-1)
    float grassDensity;          // Grass coverage (0-1)
    float flowerDensity;         // Flower coverage (0-1)
    
    BiomeProperties()
        : type(BiomeType::Plains)
        , name("Unknown")
        , temperature(0.5f)
        , humidity(0.5f)
        , rainfall(0.5f)
        , heightVariation(0.5f)
        , heightOffset(0.0f)
        , roughness(0.5f)
        , surfaceBlock(VoxelType::Grass)
        , subSurfaceBlock(VoxelType::Dirt)
        , stoneBlock(VoxelType::Stone)
        , mapColor(0.5f, 1.0f, 0.5f)
        , treeDensity(0.1f)
        , grassDensity(0.5f)
        , flowerDensity(0.1f)
    {}
};

/**
 * @brief Advanced biome system with temperature and humidity maps
 * 
 * Implements Phase 8 (Weeks 4-5) from the roadmap:
 * - Temperature and humidity map generation
 * - 2D biome distribution table (temp × humidity)
 * - Smooth biome transitions
 */
class BiomeSystem
{
public:
    BiomeSystem();
    ~BiomeSystem() = default;
    
    /**
     * @brief Initialize the biome system with a seed
     */
    void initialize(uint64_t seed);
    
    /**
     * @brief Get biome type at world position
     * 
     * Uses temperature and humidity maps to determine biome
     */
    BiomeType getBiomeAt(int worldX, int worldZ) const;
    
    /**
     * @brief Get biome properties for a biome type
     */
    const BiomeProperties& getBiomeProperties(BiomeType biome) const;
    
    /**
     * @brief Get temperature at world position (0-1)
     */
    float getTemperature(int worldX, int worldZ) const;
    
    /**
     * @brief Get humidity at world position (0-1)
     */
    float getHumidity(int worldX, int worldZ) const;
    
    /**
     * @brief Get blended biome properties at a position
     * 
     * Smoothly blends properties from nearby biomes for transitions
     */
    BiomeProperties getBlendedProperties(int worldX, int worldZ, float blendRadius = 64.0f) const;
    
    /**
     * @brief Check if position is in a biome transition zone
     */
    bool isInTransitionZone(int worldX, int worldZ, float threshold = 0.1f) const;
    
    /**
     * @brief Get the seed used for biome generation
     */
    uint64_t getSeed() const { return m_seed; }
    
private:
    /**
     * @brief Setup default biome properties
     */
    void setupBiomeProperties();
    
    /**
     * @brief Determine biome from temperature and humidity
     */
    BiomeType selectBiome(float temperature, float humidity, float elevation) const;
    
    /**
     * @brief Generate temperature value using Perlin noise
     */
    float generateTemperature(int worldX, int worldZ) const;
    
    /**
     * @brief Generate humidity value using Perlin noise
     */
    float generateHumidity(int worldX, int worldZ) const;
    
    /**
     * @brief Sample biomes in a radius for blending
     */
    std::vector<std::pair<BiomeType, float>> sampleBiomesInRadius(
        int worldX, int worldZ, float radius) const;
    
    uint64_t m_seed;
    BiomeProperties m_biomeProperties[static_cast<size_t>(BiomeType::Count)];
    
    // Noise scales for temperature and humidity
    float m_temperatureScale;
    float m_humidityScale;
    float m_temperatureOctaves;
    float m_humidityOctaves;
};

} // namespace fresh
