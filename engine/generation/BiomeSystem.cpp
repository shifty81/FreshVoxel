#include "generation/BiomeSystem.h"
#include "generation/NoiseGenerator.h"
#include "core/Logger.h"
#include <cmath>
#include <algorithm>

namespace fresh
{

BiomeSystem::BiomeSystem()
    : m_seed(0)
    , m_temperatureScale(0.002f)    // Large scale for temperature patterns
    , m_humidityScale(0.0025f)      // Large scale for humidity patterns
    , m_temperatureOctaves(4)
    , m_humidityOctaves(4)
{
    setupBiomeProperties();
}

void BiomeSystem::initialize(uint64_t seed)
{
    m_seed = seed;
    setupBiomeProperties();
    LOG_INFO_C("BiomeSystem initialized with seed: " + std::to_string(seed), "BiomeSystem");
}

void BiomeSystem::setupBiomeProperties()
{
    // Cold biomes
    {
        auto& tundra = m_biomeProperties[static_cast<size_t>(BiomeType::Tundra)];
        tundra.type = BiomeType::Tundra;
        tundra.name = "Tundra";
        tundra.temperature = 0.1f;
        tundra.humidity = 0.3f;
        tundra.rainfall = 0.2f;
        tundra.heightVariation = 0.3f;
        tundra.heightOffset = 0.0f;
        tundra.roughness = 0.4f;
        tundra.surfaceBlock = VoxelType::Snow;
        tundra.subSurfaceBlock = VoxelType::Dirt;
        tundra.stoneBlock = VoxelType::Stone;
        tundra.mapColor = glm::vec3(0.8f, 0.9f, 1.0f);  // Light blue-white
        tundra.treeDensity = 0.01f;
        tundra.grassDensity = 0.2f;
        tundra.flowerDensity = 0.01f;
    }
    
    {
        auto& taiga = m_biomeProperties[static_cast<size_t>(BiomeType::Taiga)];
        taiga.type = BiomeType::Taiga;
        taiga.name = "Taiga";
        taiga.temperature = 0.2f;
        taiga.humidity = 0.5f;
        taiga.rainfall = 0.4f;
        taiga.heightVariation = 0.5f;
        taiga.heightOffset = 0.1f;
        taiga.roughness = 0.5f;
        taiga.surfaceBlock = VoxelType::Grass;
        taiga.subSurfaceBlock = VoxelType::Dirt;
        taiga.stoneBlock = VoxelType::Stone;
        taiga.mapColor = glm::vec3(0.3f, 0.5f, 0.3f);  // Dark green
        taiga.treeDensity = 0.3f;
        taiga.grassDensity = 0.4f;
        taiga.flowerDensity = 0.05f;
    }
    
    {
        auto& snowyMountains = m_biomeProperties[static_cast<size_t>(BiomeType::SnowyMountains)];
        snowyMountains.type = BiomeType::SnowyMountains;
        snowyMountains.name = "Snowy Mountains";
        snowyMountains.temperature = 0.0f;
        snowyMountains.humidity = 0.6f;
        snowyMountains.rainfall = 0.5f;
        snowyMountains.heightVariation = 1.0f;
        snowyMountains.heightOffset = 0.8f;
        snowyMountains.roughness = 0.9f;
        snowyMountains.surfaceBlock = VoxelType::Snow;
        snowyMountains.subSurfaceBlock = VoxelType::Stone;
        snowyMountains.stoneBlock = VoxelType::Stone;
        snowyMountains.mapColor = glm::vec3(1.0f, 1.0f, 1.0f);  // White
        snowyMountains.treeDensity = 0.0f;
        snowyMountains.grassDensity = 0.0f;
        snowyMountains.flowerDensity = 0.0f;
    }
    
    {
        auto& icePlains = m_biomeProperties[static_cast<size_t>(BiomeType::IcePlains)];
        icePlains.type = BiomeType::IcePlains;
        icePlains.name = "Ice Plains";
        icePlains.temperature = 0.05f;
        icePlains.humidity = 0.1f;
        icePlains.rainfall = 0.1f;
        icePlains.heightVariation = 0.2f;
        icePlains.heightOffset = 0.0f;
        icePlains.roughness = 0.2f;
        icePlains.surfaceBlock = VoxelType::Ice;
        icePlains.subSurfaceBlock = VoxelType::Snow;
        icePlains.stoneBlock = VoxelType::Stone;
        icePlains.mapColor = glm::vec3(0.7f, 0.9f, 1.0f);  // Ice blue
        icePlains.treeDensity = 0.0f;
        icePlains.grassDensity = 0.0f;
        icePlains.flowerDensity = 0.0f;
    }
    
    // Temperate biomes
    {
        auto& plains = m_biomeProperties[static_cast<size_t>(BiomeType::Plains)];
        plains.type = BiomeType::Plains;
        plains.name = "Plains";
        plains.temperature = 0.5f;
        plains.humidity = 0.3f;
        plains.rainfall = 0.4f;
        plains.heightVariation = 0.2f;
        plains.heightOffset = 0.0f;
        plains.roughness = 0.3f;
        plains.surfaceBlock = VoxelType::Grass;
        plains.subSurfaceBlock = VoxelType::Dirt;
        plains.stoneBlock = VoxelType::Stone;
        plains.mapColor = glm::vec3(0.5f, 0.8f, 0.3f);  // Grass green
        plains.treeDensity = 0.05f;
        plains.grassDensity = 0.7f;
        plains.flowerDensity = 0.2f;
    }
    
    {
        auto& forest = m_biomeProperties[static_cast<size_t>(BiomeType::Forest)];
        forest.type = BiomeType::Forest;
        forest.name = "Forest";
        forest.temperature = 0.5f;
        forest.humidity = 0.6f;
        forest.rainfall = 0.6f;
        forest.heightVariation = 0.4f;
        forest.heightOffset = 0.1f;
        forest.roughness = 0.5f;
        forest.surfaceBlock = VoxelType::Grass;
        forest.subSurfaceBlock = VoxelType::Dirt;
        forest.stoneBlock = VoxelType::Stone;
        forest.mapColor = glm::vec3(0.2f, 0.6f, 0.2f);  // Forest green
        forest.treeDensity = 0.4f;
        forest.grassDensity = 0.6f;
        forest.flowerDensity = 0.15f;
    }
    
    {
        auto& denseForest = m_biomeProperties[static_cast<size_t>(BiomeType::DenseForest)];
        denseForest.type = BiomeType::DenseForest;
        denseForest.name = "Dense Forest";
        denseForest.temperature = 0.55f;
        denseForest.humidity = 0.8f;
        denseForest.rainfall = 0.8f;
        denseForest.heightVariation = 0.5f;
        denseForest.heightOffset = 0.15f;
        denseForest.roughness = 0.6f;
        denseForest.surfaceBlock = VoxelType::Grass;
        denseForest.subSurfaceBlock = VoxelType::Dirt;
        denseForest.stoneBlock = VoxelType::Stone;
        denseForest.mapColor = glm::vec3(0.15f, 0.5f, 0.15f);  // Dark forest green
        denseForest.treeDensity = 0.7f;
        denseForest.grassDensity = 0.8f;
        denseForest.flowerDensity = 0.1f;
    }
    
    {
        auto& mountains = m_biomeProperties[static_cast<size_t>(BiomeType::Mountains)];
        mountains.type = BiomeType::Mountains;
        mountains.name = "Mountains";
        mountains.temperature = 0.4f;
        mountains.humidity = 0.4f;
        mountains.rainfall = 0.5f;
        mountains.heightVariation = 0.9f;
        mountains.heightOffset = 0.7f;
        mountains.roughness = 0.9f;
        mountains.surfaceBlock = VoxelType::Stone;
        mountains.subSurfaceBlock = VoxelType::Stone;
        mountains.stoneBlock = VoxelType::Stone;
        mountains.mapColor = glm::vec3(0.6f, 0.6f, 0.6f);  // Gray
        mountains.treeDensity = 0.1f;
        mountains.grassDensity = 0.2f;
        mountains.flowerDensity = 0.05f;
    }
    
    {
        auto& hills = m_biomeProperties[static_cast<size_t>(BiomeType::Hills)];
        hills.type = BiomeType::Hills;
        hills.name = "Hills";
        hills.temperature = 0.5f;
        hills.humidity = 0.5f;
        hills.rainfall = 0.5f;
        hills.heightVariation = 0.6f;
        hills.heightOffset = 0.3f;
        hills.roughness = 0.6f;
        hills.surfaceBlock = VoxelType::Grass;
        hills.subSurfaceBlock = VoxelType::Dirt;
        hills.stoneBlock = VoxelType::Stone;
        hills.mapColor = glm::vec3(0.4f, 0.7f, 0.3f);  // Light green
        hills.treeDensity = 0.2f;
        hills.grassDensity = 0.5f;
        hills.flowerDensity = 0.1f;
    }
    
    // Warm biomes
    {
        auto& desert = m_biomeProperties[static_cast<size_t>(BiomeType::Desert)];
        desert.type = BiomeType::Desert;
        desert.name = "Desert";
        desert.temperature = 0.8f;
        desert.humidity = 0.1f;
        desert.rainfall = 0.0f;
        desert.heightVariation = 0.3f;
        desert.heightOffset = 0.0f;
        desert.roughness = 0.4f;
        desert.surfaceBlock = VoxelType::Sand;
        desert.subSurfaceBlock = VoxelType::Sand;
        desert.stoneBlock = VoxelType::SandStone;
        desert.mapColor = glm::vec3(0.9f, 0.8f, 0.5f);  // Sand yellow
        desert.treeDensity = 0.0f;
        desert.grassDensity = 0.05f;
        desert.flowerDensity = 0.01f;
    }
    
    {
        auto& hotDesert = m_biomeProperties[static_cast<size_t>(BiomeType::HotDesert)];
        hotDesert.type = BiomeType::HotDesert;
        hotDesert.name = "Hot Desert";
        hotDesert.temperature = 1.0f;
        hotDesert.humidity = 0.0f;
        hotDesert.rainfall = 0.0f;
        hotDesert.heightVariation = 0.4f;
        hotDesert.heightOffset = 0.0f;
        hotDesert.roughness = 0.5f;
        hotDesert.surfaceBlock = VoxelType::RedSand;
        hotDesert.subSurfaceBlock = VoxelType::RedSand;
        hotDesert.stoneBlock = VoxelType::RedSandStone;
        hotDesert.mapColor = glm::vec3(1.0f, 0.6f, 0.3f);  // Red sand
        hotDesert.treeDensity = 0.0f;
        hotDesert.grassDensity = 0.0f;
        hotDesert.flowerDensity = 0.0f;
    }
    
    {
        auto& savanna = m_biomeProperties[static_cast<size_t>(BiomeType::Savanna)];
        savanna.type = BiomeType::Savanna;
        savanna.name = "Savanna";
        savanna.temperature = 0.75f;
        savanna.humidity = 0.3f;
        savanna.rainfall = 0.3f;
        savanna.heightVariation = 0.3f;
        savanna.heightOffset = 0.0f;
        savanna.roughness = 0.4f;
        savanna.surfaceBlock = VoxelType::Grass;
        savanna.subSurfaceBlock = VoxelType::Dirt;
        savanna.stoneBlock = VoxelType::Stone;
        savanna.mapColor = glm::vec3(0.8f, 0.7f, 0.4f);  // Dry grass
        savanna.treeDensity = 0.08f;
        savanna.grassDensity = 0.4f;
        savanna.flowerDensity = 0.05f;
    }
    
    {
        auto& jungle = m_biomeProperties[static_cast<size_t>(BiomeType::Jungle)];
        jungle.type = BiomeType::Jungle;
        jungle.name = "Jungle";
        jungle.temperature = 0.9f;
        jungle.humidity = 0.9f;
        jungle.rainfall = 1.0f;
        jungle.heightVariation = 0.5f;
        jungle.heightOffset = 0.1f;
        jungle.roughness = 0.7f;
        jungle.surfaceBlock = VoxelType::Grass;
        jungle.subSurfaceBlock = VoxelType::Dirt;
        jungle.stoneBlock = VoxelType::Stone;
        jungle.mapColor = glm::vec3(0.1f, 0.7f, 0.1f);  // Vibrant green
        jungle.treeDensity = 0.8f;
        jungle.grassDensity = 0.9f;
        jungle.flowerDensity = 0.3f;
    }
    
    {
        auto& swamp = m_biomeProperties[static_cast<size_t>(BiomeType::Swamp)];
        swamp.type = BiomeType::Swamp;
        swamp.name = "Swamp";
        swamp.temperature = 0.7f;
        swamp.humidity = 1.0f;
        swamp.rainfall = 0.9f;
        swamp.heightVariation = 0.1f;
        swamp.heightOffset = -0.2f;  // Lower than sea level
        swamp.roughness = 0.3f;
        swamp.surfaceBlock = VoxelType::Mud;
        swamp.subSurfaceBlock = VoxelType::Dirt;
        swamp.stoneBlock = VoxelType::Stone;
        swamp.mapColor = glm::vec3(0.3f, 0.5f, 0.3f);  // Murky green
        swamp.treeDensity = 0.3f;
        swamp.grassDensity = 0.7f;
        swamp.flowerDensity = 0.2f;
    }
    
    // Special biomes
    {
        auto& beach = m_biomeProperties[static_cast<size_t>(BiomeType::Beach)];
        beach.type = BiomeType::Beach;
        beach.name = "Beach";
        beach.temperature = 0.6f;
        beach.humidity = 0.5f;
        beach.rainfall = 0.3f;
        beach.heightVariation = 0.1f;
        beach.heightOffset = -0.05f;
        beach.roughness = 0.2f;
        beach.surfaceBlock = VoxelType::Sand;
        beach.subSurfaceBlock = VoxelType::Sand;
        beach.stoneBlock = VoxelType::Stone;
        beach.mapColor = glm::vec3(0.9f, 0.9f, 0.7f);  // Light sand
        beach.treeDensity = 0.0f;
        beach.grassDensity = 0.1f;
        beach.flowerDensity = 0.0f;
    }
    
    {
        auto& ocean = m_biomeProperties[static_cast<size_t>(BiomeType::Ocean)];
        ocean.type = BiomeType::Ocean;
        ocean.name = "Ocean";
        ocean.temperature = 0.5f;
        ocean.humidity = 1.0f;
        ocean.rainfall = 0.7f;
        ocean.heightVariation = 0.2f;
        ocean.heightOffset = -0.5f;
        ocean.roughness = 0.3f;
        ocean.surfaceBlock = VoxelType::Water;
        ocean.subSurfaceBlock = VoxelType::Sand;
        ocean.stoneBlock = VoxelType::Stone;
        ocean.mapColor = glm::vec3(0.2f, 0.4f, 0.8f);  // Blue
        ocean.treeDensity = 0.0f;
        ocean.grassDensity = 0.0f;
        ocean.flowerDensity = 0.0f;
    }
    
    {
        auto& deepOcean = m_biomeProperties[static_cast<size_t>(BiomeType::DeepOcean)];
        deepOcean.type = BiomeType::DeepOcean;
        deepOcean.name = "Deep Ocean";
        deepOcean.temperature = 0.4f;
        deepOcean.humidity = 1.0f;
        deepOcean.rainfall = 0.7f;
        deepOcean.heightVariation = 0.3f;
        deepOcean.heightOffset = -0.8f;
        deepOcean.roughness = 0.4f;
        deepOcean.surfaceBlock = VoxelType::Water;
        deepOcean.subSurfaceBlock = VoxelType::Sand;
        deepOcean.stoneBlock = VoxelType::Stone;
        deepOcean.mapColor = glm::vec3(0.1f, 0.2f, 0.6f);  // Dark blue
        deepOcean.treeDensity = 0.0f;
        deepOcean.grassDensity = 0.0f;
        deepOcean.flowerDensity = 0.0f;
    }
    
    {
        auto& river = m_biomeProperties[static_cast<size_t>(BiomeType::River)];
        river.type = BiomeType::River;
        river.name = "River";
        river.temperature = 0.5f;
        river.humidity = 1.0f;
        river.rainfall = 0.6f;
        river.heightVariation = 0.1f;
        river.heightOffset = -0.15f;
        river.roughness = 0.2f;
        river.surfaceBlock = VoxelType::Water;
        river.subSurfaceBlock = VoxelType::Sand;
        river.stoneBlock = VoxelType::Stone;
        river.mapColor = glm::vec3(0.3f, 0.5f, 0.9f);  // River blue
        river.treeDensity = 0.05f;
        river.grassDensity = 0.3f;
        river.flowerDensity = 0.1f;
    }
}

float BiomeSystem::generateTemperature(int worldX, int worldZ) const
{
    NoiseGenerator noise;
    
    // Base temperature from latitude (distance from equator)
    // This creates horizontal temperature bands
    float latitude = std::abs(worldZ * 0.0001f);  // Distance from "equator"
    float latitudeTemp = 1.0f - std::min(latitude, 1.0f);  // Warmer at equator
    
    // Add noise for variation
    float noiseValue = noise.fractalNoise2D(
        worldX * m_temperatureScale,
        worldZ * m_temperatureScale,
        static_cast<int>(m_temperatureOctaves),
        0.5f,  // persistence
        2.0f   // lacunarity
    );
    
    // Combine latitude and noise (latitude has more weight)
    float temperature = latitudeTemp * 0.7f + (noiseValue * 0.5f + 0.5f) * 0.3f;
    
    return std::clamp(temperature, 0.0f, 1.0f);
}

float BiomeSystem::generateHumidity(int worldX, int worldZ) const
{
    NoiseGenerator noise;
    
    // Humidity is purely noise-based (no latitude effect)
    float noiseValue = noise.fractalNoise2D(
        worldX * m_humidityScale,
        worldZ * m_humidityScale,
        static_cast<int>(m_humidityOctaves),
        0.5f,  // persistence
        2.0f   // lacunarity
    );
    
    // Normalize from [-1, 1] to [0, 1]
    float humidity = noiseValue * 0.5f + 0.5f;
    
    return std::clamp(humidity, 0.0f, 1.0f);
}

BiomeType BiomeSystem::selectBiome(float temperature, float humidity, float elevation) const
{
    // Special case: very low elevation = ocean/water
    if (elevation < 0.3f) {
        if (elevation < 0.15f) {
            return BiomeType::DeepOcean;
        }
        return BiomeType::Ocean;
    }
    
    // Beach at the edge of water
    if (elevation < 0.35f) {
        return BiomeType::Beach;
    }
    
    // Very high elevation = mountains
    if (elevation > 0.75f) {
        if (temperature < 0.3f) {
            return BiomeType::SnowyMountains;
        }
        return BiomeType::Mountains;
    }
    
    // Hills at medium-high elevation
    if (elevation > 0.6f) {
        return BiomeType::Hills;
    }
    
    // Biome selection based on temperature and humidity
    
    // Cold biomes (temp < 0.33)
    if (temperature < 0.33f) {
        if (humidity < 0.2f) {
            return BiomeType::IcePlains;
        } else if (humidity < 0.5f) {
            return BiomeType::Tundra;
        } else {
            return BiomeType::Taiga;
        }
    }
    
    // Temperate biomes (0.33 <= temp < 0.66)
    if (temperature < 0.66f) {
        if (humidity < 0.4f) {
            return BiomeType::Plains;
        } else if (humidity < 0.7f) {
            return BiomeType::Forest;
        } else {
            return BiomeType::DenseForest;
        }
    }
    
    // Warm biomes (temp >= 0.66)
    if (humidity < 0.15f) {
        return BiomeType::HotDesert;
    } else if (humidity < 0.4f) {
        if (temperature > 0.85f) {
            return BiomeType::Desert;
        }
        return BiomeType::Savanna;
    } else if (humidity < 0.7f) {
        return BiomeType::Savanna;
    } else if (humidity > 0.85f && elevation < 0.45f) {
        return BiomeType::Swamp;
    } else {
        return BiomeType::Jungle;
    }
}

BiomeType BiomeSystem::getBiomeAt(int worldX, int worldZ) const
{
    float temperature = getTemperature(worldX, worldZ);
    float humidity = getHumidity(worldX, worldZ);
    
    // Simple elevation calculation (could be improved with terrain generator)
    NoiseGenerator noise;
    float elevation = noise.fractalNoise2D(
        worldX * 0.001f,
        worldZ * 0.001f,
        4,
        0.5f,  // persistence
        2.0f   // lacunarity
    ) * 0.5f + 0.5f;
    
    return selectBiome(temperature, humidity, elevation);
}

const BiomeProperties& BiomeSystem::getBiomeProperties(BiomeType biome) const
{
    return m_biomeProperties[static_cast<size_t>(biome)];
}

float BiomeSystem::getTemperature(int worldX, int worldZ) const
{
    return generateTemperature(worldX, worldZ);
}

float BiomeSystem::getHumidity(int worldX, int worldZ) const
{
    return generateHumidity(worldX, worldZ);
}

BiomeProperties BiomeSystem::getBlendedProperties(int worldX, int worldZ, float blendRadius) const
{
    // Sample biomes in a grid around the position
    const int sampleCount = 5;  // 5x5 grid
    const float sampleSpacing = blendRadius / sampleCount;
    
    std::vector<std::pair<BiomeType, float>> samples;
    float totalWeight = 0.0f;
    
    for (int dx = -sampleCount/2; dx <= sampleCount/2; ++dx) {
        for (int dz = -sampleCount/2; dz <= sampleCount/2; ++dz) {
            int sampleX = worldX + static_cast<int>(dx * sampleSpacing);
            int sampleZ = worldZ + static_cast<int>(dz * sampleSpacing);
            
            BiomeType biome = getBiomeAt(sampleX, sampleZ);
            float distance = std::sqrt(static_cast<float>(dx*dx + dz*dz));
            float weight = 1.0f / (1.0f + distance);  // Closer = more weight
            
            samples.push_back({biome, weight});
            totalWeight += weight;
        }
    }
    
    // Blend properties
    BiomeProperties blended;
    blended.temperature = 0.0f;
    blended.humidity = 0.0f;
    blended.rainfall = 0.0f;
    blended.heightVariation = 0.0f;
    blended.heightOffset = 0.0f;
    blended.roughness = 0.0f;
    blended.treeDensity = 0.0f;
    blended.grassDensity = 0.0f;
    blended.flowerDensity = 0.0f;
    blended.mapColor = glm::vec3(0.0f);
    
    for (const auto& [biome, weight] : samples) {
        const auto& props = getBiomeProperties(biome);
        float normalizedWeight = weight / totalWeight;
        
        blended.temperature += props.temperature * normalizedWeight;
        blended.humidity += props.humidity * normalizedWeight;
        blended.rainfall += props.rainfall * normalizedWeight;
        blended.heightVariation += props.heightVariation * normalizedWeight;
        blended.heightOffset += props.heightOffset * normalizedWeight;
        blended.roughness += props.roughness * normalizedWeight;
        blended.treeDensity += props.treeDensity * normalizedWeight;
        blended.grassDensity += props.grassDensity * normalizedWeight;
        blended.flowerDensity += props.flowerDensity * normalizedWeight;
        blended.mapColor += props.mapColor * normalizedWeight;
    }
    
    // Use the dominant biome's block types
    BiomeType dominantBiome = getBiomeAt(worldX, worldZ);
    const auto& dominantProps = getBiomeProperties(dominantBiome);
    blended.type = dominantBiome;
    blended.name = dominantProps.name + " (Blended)";
    blended.surfaceBlock = dominantProps.surfaceBlock;
    blended.subSurfaceBlock = dominantProps.subSurfaceBlock;
    blended.stoneBlock = dominantProps.stoneBlock;
    
    return blended;
}

bool BiomeSystem::isInTransitionZone(int worldX, int worldZ, [[maybe_unused]] float threshold) const
{
    // Check if nearby positions have different biomes
    BiomeType center = getBiomeAt(worldX, worldZ);
    
    const int checkDistance = 16;  // Check 16 blocks away
    BiomeType north = getBiomeAt(worldX, worldZ - checkDistance);
    BiomeType south = getBiomeAt(worldX, worldZ + checkDistance);
    BiomeType east = getBiomeAt(worldX + checkDistance, worldZ);
    BiomeType west = getBiomeAt(worldX - checkDistance, worldZ);
    
    return (center != north || center != south || center != east || center != west);
}

std::vector<std::pair<BiomeType, float>> BiomeSystem::sampleBiomesInRadius(
    int worldX, int worldZ, float radius) const
{
    std::vector<std::pair<BiomeType, float>> result;
    
    const int samples = 8;  // Sample in 8 directions
    for (int i = 0; i < samples; ++i) {
        float angle = (i / static_cast<float>(samples)) * 2.0f * 3.14159265f;
        int sampleX = worldX + static_cast<int>(std::cos(angle) * radius);
        int sampleZ = worldZ + static_cast<int>(std::sin(angle) * radius);
        
        BiomeType biome = getBiomeAt(sampleX, sampleZ);
        result.push_back({biome, 1.0f / samples});  // Equal weight for now
    }
    
    return result;
}

} // namespace fresh
