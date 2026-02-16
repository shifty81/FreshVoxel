#pragma once

#include "NoiseGenerator.h"
#include "voxel/VoxelTypes.h"

namespace fresh
{

class Chunk;
class VoxelWorld;

/**
 * @brief Generates terrain using procedural algorithms
 *
 * Uses noise functions to create natural-looking terrain with
 * different biomes, caves, and structures.
 * 
 * Current Implementation:
 * - Perlin noise-based heightmap generation
 * - Multi-octave fractal noise for detail
 * - Basic height-based biome selection
 * - 3D cave generation using noise
 * - Tree and foliage placement
 * 
 * Future Advanced Features (see ADVANCED_WORLD_GENERATION.md):
 * - Climate simulation (temperature, rainfall)
 * - Geological layering (rock types, ore distribution)
 * - Hydrological systems (rivers, lakes)
 * - Structure generation (ruins, dungeons)
 * - JSON-based modding support
 * - LOD rendering for distant terrain
 */
class TerrainGenerator
{
public:
    TerrainGenerator();
    explicit TerrainGenerator(int seed);
    ~TerrainGenerator();

    /**
     * @brief Generate terrain for a chunk
     * @param chunk Chunk to generate terrain for
     */
    void generateChunk(Chunk* chunk);

    /**
     * @brief Generate terrain and assets for a chunk
     * @param chunk Chunk to generate for
     * @param world VoxelWorld for asset placement (optional)
     */
    void generateChunkWithAssets(Chunk* chunk, VoxelWorld* world = nullptr);

    /**
     * @brief Set generation seed
     * @param seed Random seed
     */
    void setSeed(int seed);

    /**
     * @brief Get height at world position
     * 
     * Current Implementation:
     * Uses multi-octave fractal Brownian motion (fBm) to generate
     * natural-looking terrain height variations.
     * 
     * Future Enhancement (see ADVANCED_WORLD_GENERATION.md):
     * Could be enhanced with climate-based modifications:
     * - Temperature affects terrain roughness
     * - Rainfall influences erosion patterns
     * - Altitude affects temperature (lapse rate)
     * 
     * @param x World X coordinate
     * @param z World Z coordinate
     * @return Height value
     */
    [[nodiscard]] int getHeight(int x, int z) const;
    
    /**
     * @brief Check if position should be water (river or lake)
     * @param x World X coordinate
     * @param z World Z coordinate
     * @param height Terrain height at this position
     * @return True if this should be a water block
     */
    [[nodiscard]] bool isWaterway(int x, int z, int height) const;

private:
    VoxelType getBlockType(int x, int y, int z, int surfaceHeight) const;
    void generateTreesAndFoliage(Chunk* chunk, VoxelWorld* world);
    void generateWaterways(Chunk* chunk);

private:
    NoiseGenerator m_noiseGenerator;
    int m_seed;
};

} // namespace fresh
