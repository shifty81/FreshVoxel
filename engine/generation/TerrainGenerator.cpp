#include "generation/TerrainGenerator.h"

#include <cmath>

#include "voxel/Chunk.h"
#include "voxel/VoxelWorld.h"
#include "assets/ModularAssetSystem.h"
#include "generation/World2DGenerator.h"

namespace fresh
{

TerrainGenerator::TerrainGenerator() 
    : m_noiseGenerator(0), m_seed(0), m_2dStyle(0), m_use2DGenerator(false) {}

TerrainGenerator::TerrainGenerator(int seed) 
    : m_noiseGenerator(seed), m_seed(seed), m_2dStyle(0), m_use2DGenerator(false) {}

TerrainGenerator::~TerrainGenerator() {}

void TerrainGenerator::setSeed(int seed)
{
    m_seed = seed;
    m_noiseGenerator.setSeed(seed);
    
    // Also update 2D generator if it exists
    if (m_2dGenerator) {
        m_2dGenerator->setSeed(seed);
    }
}

void TerrainGenerator::set2DStyle(int style)
{
    m_2dStyle = style;
    m_use2DGenerator = true;
    
    // Create 2D generator with appropriate style
    World2DGenerator::Settings settings;
    settings.seed = m_seed;
    
    if (style == 0) {
        // Platformer/Terraria style
        settings.style = World2DGenerator::Style::PLATFORMER;
        settings.worldDepth = 1; // Single depth layer for side-scrolling
    } else {
        // Top-down/Zelda style
        settings.style = World2DGenerator::Style::TOPDOWN;
        settings.worldDepth = 1; // Single height layer for top-down
    }
    
    m_2dGenerator = std::make_unique<World2DGenerator>(settings);
}

int TerrainGenerator::getHeight(int x, int z) const
{
    // Generate terrain height using multi-octave Perlin noise (fractal Brownian motion)
    // Scale factor (0.01f): Controls terrain frequency - lower = larger features
    // Octaves (4): Number of noise layers combined - more = finer detail
    // Persistence (0.5f): Amplitude decay per octave - lower = smoother terrain
    // Lacunarity (2.0f): Frequency multiplier per octave - higher = more detail variation
    
    // FUTURE ENHANCEMENT (see ADVANCED_WORLD_GENERATION.md):
    // This could be enhanced with climate-based modifications:
    // - Apply temperature maps to modify terrain roughness
    // - Use rainfall to create erosion patterns
    // - Implement altitude-based temperature lapse rate
    
    float noiseValue = m_noiseGenerator.fractalNoise2D(x * 0.01f, z * 0.01f,
                                                       4,    // octaves
                                                       0.5f, // persistence
                                                       2.0f  // lacunarity
    );

    // Map noise from [-1, 1] to height range [40, 80]
    // This creates terrain between y=40 (valleys) and y=80 (peaks)
    int height = 40 + static_cast<int>((noiseValue + 1.0f) * 20.0f);

    return height;
}

VoxelType TerrainGenerator::getBlockType(int x, int y, int z, int surfaceHeight) const
{
    // Determine block type based on height and position
    // Creates natural terrain layers: grass/dirt on top, stone below, bedrock at bottom
    
    // FUTURE ENHANCEMENT (see ADVANCED_WORLD_GENERATION.md):
    // This could be enhanced with geological layering:
    // - Different rock types (sedimentary, metamorphic, igneous)
    // - Realistic ore distribution based on rock type
    // - Geological provinces with regional variations
    
    if (y > surfaceHeight) {
        return VoxelType::Air;
    } else if (y == surfaceHeight) {
        // Top layer
        if (surfaceHeight > 62) {
            return VoxelType::Grass;
        } else if (surfaceHeight > 58) {
            return VoxelType::Sand;
        } else {
            return VoxelType::Stone;
        }
    } else if (y >= surfaceHeight - 3) {
        // Few layers below surface
        if (surfaceHeight > 58) {
            return VoxelType::Dirt;
        } else {
            return VoxelType::Stone;
        }
    } else {
        // Deep underground
        // Add caves using 3D noise
        float caveNoise = m_noiseGenerator.perlin3D(x * 0.05f, y * 0.05f, z * 0.05f);

        if (caveNoise > 0.5f) {
            return VoxelType::Air;
        }

        return VoxelType::Stone;
    }
}

bool TerrainGenerator::isWaterway(int x, int z, int height) const
{
    // Generate rivers using noise-based path following
    // Rivers follow low-lying areas and create natural waterways
    
    // River noise - creates winding paths
    float riverNoise = m_noiseGenerator.perlin2D(x * 0.02f, z * 0.02f);
    
    // Lakes in low areas
    float lakeNoise = m_noiseGenerator.perlin2D(x * 0.005f + 1000.0f, z * 0.005f + 1000.0f);
    
    // Rivers form in narrow bands where noise is close to 0
    bool isRiver = (std::abs(riverNoise) < 0.08f && height < 65);
    
    // Lakes form in low, flat areas
    bool isLake = (lakeNoise > 0.6f && height < 63);
    
    return isRiver || isLake;
}

void TerrainGenerator::generateWaterways(Chunk* chunk)
{
    if (!chunk) {
        return;
    }
    
    const ChunkPos& chunkPos = chunk->getPosition();
    const int waterLevel = 62; // Sea level
    
    // Pass through chunk and add water where appropriate
    for (int localX = 0; localX < CHUNK_SIZE; ++localX) {
        for (int localZ = 0; localZ < CHUNK_SIZE; ++localZ) {
            int worldX = chunkPos.x * CHUNK_SIZE + localX;
            int worldZ = chunkPos.z * CHUNK_SIZE + localZ;
            
            int surfaceHeight = getHeight(worldX, worldZ);
            
            // Check if this should be a waterway
            if (isWaterway(worldX, worldZ, surfaceHeight)) {
                // Fill from surface to water level with water
                for (int y = surfaceHeight + 1; y <= waterLevel; ++y) {
                    if (y < CHUNK_HEIGHT) {
                        chunk->setVoxel(localX, y, localZ, Voxel(VoxelType::Water));
                    }
                }
            }
        }
    }
}

void TerrainGenerator::generateChunk(Chunk* chunk)
{
    if (!chunk) {
        return; // Gracefully handle null pointer
    }

    const ChunkPos& chunkPos = chunk->getPosition();

    // Use 2D generator if enabled
    if (m_use2DGenerator && m_2dGenerator) {
        m_2dGenerator->generateChunk(*chunk, chunkPos.x, chunkPos.z);
        chunk->markDirty();
        return;
    }

    // Otherwise use standard 3D generation
    // Generate terrain for each column in the chunk
    for (int localX = 0; localX < CHUNK_SIZE; ++localX) {
        for (int localZ = 0; localZ < CHUNK_SIZE; ++localZ) {
            // Convert to world coordinates
            int worldX = chunkPos.x * CHUNK_SIZE + localX;
            int worldZ = chunkPos.z * CHUNK_SIZE + localZ;

            // Get height at this position
            int surfaceHeight = getHeight(worldX, worldZ);

            // Generate vertical column
            for (int y = 0; y < CHUNK_HEIGHT; ++y) {
                VoxelType type = getBlockType(worldX, y, worldZ, surfaceHeight);
                chunk->setVoxel(localX, y, localZ, Voxel(type));
            }
        }
    }
    
    // Generate waterways (rivers and lakes)
    generateWaterways(chunk);

    chunk->markDirty();
}

void TerrainGenerator::generateChunkWithAssets(Chunk* chunk, VoxelWorld* world)
{
    // First generate the base terrain
    generateChunk(chunk);

    // If world is provided, generate and place assets
    if (world) {
        const ChunkPos& chunkPos = chunk->getPosition();
        
        // Generate trees and foliage directly for testing
        generateTreesAndFoliage(chunk, world);
        
        // Also try to use the modular asset system if asset packs are available
        auto& assetSystem = ModularAssetSystem::getInstance();
        auto assetInstances = assetSystem.generateAssetsForChunk(
            world, 
            chunkPos.x, 
            chunkPos.z, 
            static_cast<uint32_t>(m_seed)
        );

        // Place the generated assets in the world
        assetSystem.placeAssetsInWorld(assetInstances, world);
        
        // Mark chunk as dirty to regenerate mesh with new assets
        chunk->markDirty();
    }
}

void TerrainGenerator::generateTreesAndFoliage(Chunk* chunk, VoxelWorld* world)
{
    (void)world; // Parameter reserved for future cross-chunk tree generation
    
    if (!chunk) {
        return;
    }

    const ChunkPos& chunkPos = chunk->getPosition();
    
    // Use noise to determine tree placement positions
    // This creates natural-looking clusters of trees
    for (int localX = 0; localX < CHUNK_SIZE; ++localX) {
        for (int localZ = 0; localZ < CHUNK_SIZE; ++localZ) {
            // Convert to world coordinates
            int worldX = chunkPos.x * CHUNK_SIZE + localX;
            int worldZ = chunkPos.z * CHUNK_SIZE + localZ;
            
            // Use noise to determine if a tree should spawn here
            // Scale of 0.08 creates better clusters of trees (increased density)
            float treeNoise = m_noiseGenerator.perlin2D(worldX * 0.08f, worldZ * 0.08f);
            
            // Get secondary noise for tree variety
            float varietyNoise = m_noiseGenerator.perlin2D(worldX * 0.15f + 100.0f, worldZ * 0.15f + 100.0f);
            
            // Only spawn trees on elevated terrain (grass blocks) with certain noise values
            int surfaceHeight = getHeight(worldX, worldZ);
            
            // Check if surface is grass (good for trees)
            if (surfaceHeight > 62 && surfaceHeight < 78) {  // Increased upper bound for more coverage
                // Spawn trees with higher probability (lowered threshold from 0.3 to 0.2)
                if (treeNoise > 0.2f && treeNoise < 0.85f) {  // Increased range for more trees
                    // Generate a tree at this position with variety based on noise
                    int baseHeight = 5;
                    if (varietyNoise > 0.5f) {
                        baseHeight = 7;  // Taller trees
                    } else if (varietyNoise < -0.3f) {
                        baseHeight = 4;  // Shorter trees
                    }
                    
                    int treeHeight = baseHeight + static_cast<int>((treeNoise + 1.0f) * 2.0f); // Variable height
                    int trunkHeight = treeHeight - 3; // Leaves start 3 blocks from top
                    
                    // Place tree trunk (Wood blocks) - directly in chunk
                    for (int y = 1; y <= trunkHeight; ++y) {
                        int worldY = surfaceHeight + y;
                        if (worldY < CHUNK_HEIGHT) {
                            chunk->setVoxel(localX, worldY, localZ, Voxel(VoxelType::Wood));
                        }
                    }
                    
                    // Place leaves (foliage) in a spherical pattern around the top
                    int leavesStartY = surfaceHeight + trunkHeight - 1;
                    
                    // Vary leaf radius based on tree size
                    int leavesRadius = 2;
                    if (baseHeight >= 7) {
                        leavesRadius = 3;  // Larger canopy for tall trees
                    }
                    
                    for (int dy = 0; dy <= 4; ++dy) {
                        int worldY = leavesStartY + dy;
                        if (worldY >= CHUNK_HEIGHT) break;
                        
                        // Smaller radius at top and bottom of leaves
                        int currentRadius = leavesRadius;
                        if (dy == 0 || dy == 4) {
                            currentRadius = 1;
                        } else if (dy == 1 || dy == 3) {
                            currentRadius = leavesRadius - 1;
                        }
                        
                        for (int dx = -currentRadius; dx <= currentRadius; ++dx) {
                            for (int dz = -currentRadius; dz <= currentRadius; ++dz) {
                                // Skip the trunk position
                                if (dx == 0 && dz == 0 && dy < 3) {
                                    continue;
                                }
                                
                                // Create roughly spherical shape
                                float dist = std::sqrt(dx * dx + dz * dz + dy * dy * 0.5f);
                                if (dist <= leavesRadius + 0.5f) {
                                    int leafX = localX + dx;
                                    int leafZ = localZ + dz;
                                    
                                    // Only place leaves within chunk bounds
                                    if (leafX >= 0 && leafX < CHUNK_SIZE && 
                                        leafZ >= 0 && leafZ < CHUNK_SIZE) {
                                        chunk->setVoxel(leafX, worldY, leafZ, Voxel(VoxelType::Leaves));
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

} // namespace fresh
