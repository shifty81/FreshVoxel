#include "generation/HeightmapWorldGenerator.h"
#include "voxel/Chunk.h"
#include "core/Logger.h"
#include <algorithm>
#include <cmath>

// Forward declare stb_image functions
extern "C" {
    unsigned char* stbi_load(char const* filename, int* x, int* y, int* channels_in_file, int desired_channels);
    void stbi_image_free(void* retval_from_stbi_load);
    const char* stbi_failure_reason(void);
}

namespace fresh
{

// HeightmapData implementation
float HeightmapData::getHeight(int x, int z) const
{
    // Handle out of bounds
    if (x < 0 || x >= width || z < 0 || z >= height) {
        return 0.0f;
    }
    
    int index = z * width + x;
    if (index < 0 || index >= static_cast<int>(heights.size())) {
        return 0.0f;
    }
    
    return heights[index];
}

float HeightmapData::getHeightInterpolated(float x, float z) const
{
    // Clamp to valid range
    x = std::max(0.0f, std::min(x, static_cast<float>(width - 1)));
    z = std::max(0.0f, std::min(z, static_cast<float>(height - 1)));
    
    // Get integer coordinates
    int x0 = static_cast<int>(std::floor(x));
    int z0 = static_cast<int>(std::floor(z));
    int x1 = std::min(x0 + 1, width - 1);
    int z1 = std::min(z0 + 1, height - 1);
    
    // Get fractional parts
    float fx = x - x0;
    float fz = z - z0;
    
    // Get corner heights
    float h00 = getHeight(x0, z0);
    float h10 = getHeight(x1, z0);
    float h01 = getHeight(x0, z1);
    float h11 = getHeight(x1, z1);
    
    // Bilinear interpolation
    float h0 = h00 * (1.0f - fx) + h10 * fx;
    float h1 = h01 * (1.0f - fx) + h11 * fx;
    float h = h0 * (1.0f - fz) + h1 * fz;
    
    return h;
}

// HeightmapGeneratorParams implementation
void HeightmapGeneratorParams::setupDefaultLayers()
{
    layers.clear();
    
    // Underwater layers (0.0 - 0.4)
    layers.push_back({0.0f, 0.3f, VoxelType::Sand});
    layers.push_back({0.3f, 0.4f, VoxelType::Dirt});
    
    // Ground layers (0.4 - 0.7)
    layers.push_back({0.4f, 0.7f, VoxelType::Grass});
    
    // Mountain layers (0.7 - 1.0)
    layers.push_back({0.7f, 0.9f, VoxelType::Stone});
    layers.push_back({0.9f, 1.0f, VoxelType::Stone}); // Peak
}

// HeightmapWorldGenerator implementation
HeightmapWorldGenerator::HeightmapWorldGenerator()
    : m_heightmapLoaded(false)
{
    // Setup default parameters
    m_params.setupDefaultLayers();
}

HeightmapWorldGenerator::~HeightmapWorldGenerator() = default;

bool HeightmapWorldGenerator::initialize(const HeightmapGeneratorParams& params)
{
    m_params = params;
    
    if (m_params.layers.empty()) {
        m_params.setupDefaultLayers();
    }
    
    if (!m_params.heightmapPath.empty()) {
        return loadHeightmap(m_params.heightmapPath);
    }
    
    return false;
}

bool HeightmapWorldGenerator::loadHeightmap(const std::string& filepath)
{
    Logger::getInstance().info("Loading heightmap from: " + filepath, "HeightmapWorldGenerator");
    
    int width, height, channels;
    unsigned char* data = stbi_load(filepath.c_str(), &width, &height, &channels, 1); // Force grayscale
    
    if (!data) {
        Logger::getInstance().error("Failed to load heightmap: " + std::string(stbi_failure_reason()), 
                                   "HeightmapWorldGenerator");
        return false;
    }
    
    // Convert to normalized float values
    m_heightmapData.width = width;
    m_heightmapData.height = height;
    m_heightmapData.heights.resize(width * height);
    
    for (int i = 0; i < width * height; ++i) {
        m_heightmapData.heights[i] = static_cast<float>(data[i]) / 255.0f;
    }
    
    stbi_image_free(data);
    
    m_heightmapLoaded = true;
    
    Logger::getInstance().info("Heightmap loaded: " + std::to_string(width) + "x" + std::to_string(height), 
                              "HeightmapWorldGenerator");
    
    return true;
}

WorldGeneratorMetadata HeightmapWorldGenerator::getMetadata() const
{
    return {
        "Heightmap World Generator",       // name
        "heightmap_world_generator",       // id
        "Generates terrain from heightmap images", // description
        "1.0.0",                           // version
        "Fresh Engine",                    // author
        false,                             // is2D
        true,                              // is3D
        {"terrain", "heightmap", "image"}  // tags
    };
}

void HeightmapWorldGenerator::generateChunk(Chunk* chunk)
{
    if (!chunk) {
        return;
    }
    
    const ChunkPos& chunkPos = chunk->getPosition();
    generateChunkInternal(chunk, chunkPos);
}

int HeightmapWorldGenerator::getHeight(int x, int z) const
{
    return static_cast<int>(getTerrainHeight(x, z));
}

void HeightmapWorldGenerator::generateChunkInternal(Chunk* chunk, const ChunkPos& chunkPos)
{
    if (!chunk || !m_heightmapLoaded) {
        return;
    }
    
    const int chunkWorldX = chunkPos.x * CHUNK_SIZE;
    const int chunkWorldZ = chunkPos.z * CHUNK_SIZE;
    
    // Generate terrain for each column in the chunk
    for (int localX = 0; localX < CHUNK_SIZE; ++localX) {
        for (int localZ = 0; localZ < CHUNK_SIZE; ++localZ) {
            int worldX = chunkWorldX + localX;
            int worldZ = chunkWorldZ + localZ;
            
            // Get terrain height from heightmap
            float terrainHeight = getTerrainHeight(worldX, worldZ);
            int maxY = static_cast<int>(terrainHeight);
            
            // Calculate normalized height for layer selection
            float normalizedHeight = (terrainHeight - m_params.minHeight) / 
                                    (m_params.maxHeight - m_params.minHeight);
            normalizedHeight = std::max(0.0f, std::min(1.0f, normalizedHeight));
            
            // Fill column from bottom to terrain height
            for (int y = 0; y <= maxY && y < CHUNK_HEIGHT; ++y) {
                int depthFromSurface = maxY - y;
                VoxelType blockType = getBlockTypeForHeight(normalizedHeight, depthFromSurface);
                
                chunk->setVoxel(localX, y, localZ, Voxel(blockType));
            }
            
            // Fill rest with air
            for (int y = maxY + 1; y < CHUNK_HEIGHT; ++y) {
                chunk->setVoxel(localX, y, localZ, Voxel(VoxelType::Air));
            }
        }
    }
    
    chunk->markDirty();
}

float HeightmapWorldGenerator::getTerrainHeight(int worldX, int worldZ) const
{
    if (!m_heightmapLoaded) {
        return 0.0f;
    }
    
    // Map world coordinates to heightmap coordinates
    float hmX, hmZ;
    worldToHeightmapCoords(worldX, worldZ, hmX, hmZ);
    
    // Get interpolated height from heightmap
    float normalizedHeight = m_heightmapData.getHeightInterpolated(hmX, hmZ);
    
    // Scale to world height range
    float worldHeight = m_params.minHeight + 
                       (normalizedHeight * (m_params.maxHeight - m_params.minHeight) * m_params.verticalScale);
    
    return worldHeight;
}

void HeightmapWorldGenerator::setParams(const HeightmapGeneratorParams& params)
{
    bool needsReload = (params.heightmapPath != m_params.heightmapPath);
    
    m_params = params;
    
    if (m_params.layers.empty()) {
        m_params.setupDefaultLayers();
    }
    
    if (needsReload && !m_params.heightmapPath.empty()) {
        loadHeightmap(m_params.heightmapPath);
    }
}

VoxelType HeightmapWorldGenerator::getBlockTypeForHeight(float normalizedHeight, int depthFromSurface) const
{
    // Find appropriate layer based on height
    VoxelType surfaceType = VoxelType::Grass;
    
    for (const auto& layer : m_params.layers) {
        if (normalizedHeight >= layer.minHeight && normalizedHeight < layer.maxHeight) {
            surfaceType = layer.blockType;
            break;
        }
    }
    
    // Use surface block type at surface
    if (depthFromSurface == 0) {
        return surfaceType;
    }
    
    // Subsurface layers
    if (depthFromSurface <= 3) {
        // Top 3 layers: use dirt (or sand if in sandy area)
        if (surfaceType == VoxelType::Sand) {
            return VoxelType::Sand;
        }
        return VoxelType::Dirt;
    }
    
    // Deep underground: stone
    return VoxelType::Stone;
}

void HeightmapWorldGenerator::worldToHeightmapCoords(int worldX, int worldZ, float& outX, float& outZ) const
{
    // Apply horizontal scale
    float scaledX = static_cast<float>(worldX) / m_params.horizontalScale;
    float scaledZ = static_cast<float>(worldZ) / m_params.horizontalScale;
    
    // Handle wrapping/tiling
    if (m_params.repeatX && m_heightmapData.width > 0) {
        scaledX = std::fmod(scaledX, static_cast<float>(m_heightmapData.width));
        if (scaledX < 0) scaledX += m_heightmapData.width;
    }
    
    if (m_params.repeatZ && m_heightmapData.height > 0) {
        scaledZ = std::fmod(scaledZ, static_cast<float>(m_heightmapData.height));
        if (scaledZ < 0) scaledZ += m_heightmapData.height;
    }
    
    outX = scaledX;
    outZ = scaledZ;
}

} // namespace fresh
