#pragma once

#include "generation/IWorldGenerator.h"
#include "voxel/VoxelTypes.h"
#include <string>
#include <vector>
#include <memory>

namespace fresh
{

/**
 * @brief Heightmap data loaded from an image file
 */
struct HeightmapData
{
    int width;
    int height;
    std::vector<float> heights;  // Normalized heights (0.0 - 1.0)
    
    /**
     * @brief Get height value at specific coordinates
     * @param x X coordinate
     * @param z Z coordinate
     * @return Height value (0.0 - 1.0) or 0.0 if out of bounds
     */
    float getHeight(int x, int z) const;
    
    /**
     * @brief Get interpolated height at fractional coordinates
     * @param x X coordinate (can be fractional)
     * @param z Z coordinate (can be fractional)
     * @return Bilinearly interpolated height value
     */
    float getHeightInterpolated(float x, float z) const;
};

/**
 * @brief Parameters for heightmap-based terrain generation
 */
struct HeightmapGeneratorParams
{
    std::string heightmapPath;     // Path to heightmap image file
    
    float minHeight = 0.0f;         // Minimum world height (in voxels)
    float maxHeight = 128.0f;       // Maximum world height (in voxels)
    float verticalScale = 1.0f;     // Scale factor for height values
    
    float horizontalScale = 1.0f;   // Scale factor for X/Z coordinates
                                     // (1.0 = 1 pixel = 1 voxel)
    
    bool repeatX = false;           // Wrap heightmap in X direction
    bool repeatZ = false;           // Wrap heightmap in Z direction
    
    // Block type mapping based on height
    struct HeightLayer {
        float minHeight;             // Minimum height for this layer (0.0-1.0)
        float maxHeight;             // Maximum height for this layer (0.0-1.0)
        VoxelType blockType;         // Block type to use
    };
    
    std::vector<HeightLayer> layers;  // Height-based layer definitions
    
    // Default layer setup
    void setupDefaultLayers();
};

/**
 * @brief World generator that creates terrain from heightmap images
 * 
 * Supports common image formats (PNG, JPG, BMP, TGA) through stb_image.
 * Can generate terrain at any scale and with configurable height ranges.
 * 
 * Features:
 * - Load heightmaps from image files (grayscale or RGB)
 * - Configurable vertical and horizontal scaling
 * - Height-based block type layers (grass at top, stone below, etc.)
 * - Optional heightmap wrapping/tiling
 * - Bilinear interpolation for smooth terrain
 */
class HeightmapWorldGenerator : public IWorldGenerator
{
public:
    HeightmapWorldGenerator();
    virtual ~HeightmapWorldGenerator();

    /**
     * @brief Initialize generator with heightmap parameters
     * @param params Heightmap generation parameters
     * @return true if heightmap loaded successfully
     */
    bool initialize(const HeightmapGeneratorParams& params);

    /**
     * @brief Load heightmap from image file
     * @param filepath Path to heightmap image
     * @return true if loaded successfully
     */
    bool loadHeightmap(const std::string& filepath);

    // IWorldGenerator interface implementation
    /**
     * @brief Get plugin metadata
     */
    WorldGeneratorMetadata getMetadata() const override;

    /**
     * @brief Generate a chunk based on the heightmap (IWorldGenerator interface)
     * @param chunk Chunk to generate
     */
    void generateChunk(Chunk* chunk) override;

    /**
     * @brief Set the generation seed
     * @param seed Random seed value
     */
    void setSeed([[maybe_unused]] int seed) override { /* Heightmap doesn't use seed */ }

    /**
     * @brief Get the height at a world position
     * @param x World X coordinate
     * @param z World Z coordinate
     * @return Height value (Y coordinate)
     */
    int getHeight(int x, int z) const override;

    /**
     * @brief Get the heightmap data
     */
    const HeightmapData& getHeightmapData() const { return m_heightmapData; }

    /**
     * @brief Get current generator parameters
     */
    const HeightmapGeneratorParams& getParams() const { return m_params; }
    
    /**
     * @brief Set generator parameters (requires reloading heightmap)
     */
    void setParams(const HeightmapGeneratorParams& params);

    /**
     * @brief Get terrain height at world coordinates
     * @param worldX World X coordinate
     * @param worldZ World Z coordinate
     * @return Height in voxels
     */
    float getTerrainHeight(int worldX, int worldZ) const;

    /**
     * @brief Check if heightmap is loaded
     */
    bool isHeightmapLoaded() const { return m_heightmapLoaded; }

private:
    /**
     * @brief Generate a chunk based on the heightmap (helper method with position parameter)
     * @param chunk Chunk to generate
     * @param chunkPos Chunk position
     */
    void generateChunkInternal(Chunk* chunk, const ChunkPos& chunkPos);

    /**
     * @brief Determine block type based on height and layer configuration
     * @param normalizedHeight Height value (0.0 - 1.0)
     * @param depthFromSurface Distance from surface (0 = surface)
     * @return Block type to use
     */
    VoxelType getBlockTypeForHeight(float normalizedHeight, int depthFromSurface) const;

    /**
     * @brief Map world coordinates to heightmap coordinates
     * @param worldX World X coordinate
     * @param worldZ World Z coordinate
     * @param outX Output heightmap X coordinate
     * @param outZ Output heightmap Z coordinate
     */
    void worldToHeightmapCoords(int worldX, int worldZ, float& outX, float& outZ) const;

    HeightmapGeneratorParams m_params;
    HeightmapData m_heightmapData;
    bool m_heightmapLoaded;
};

} // namespace fresh
