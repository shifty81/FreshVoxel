#include "generation/World2DGenerator.h"
#include "voxel/VoxelWorld.h"
#include "voxel/Chunk.h"
#include "core/Logger.h"
#include <algorithm>
#include <random>

namespace fresh {

World2DGenerator::World2DGenerator()
    : World2DGenerator(Settings{}) {
}

World2DGenerator::World2DGenerator(const Settings& settings)
    : settings_(settings) {
    
    // Initialize noise generators
    surfaceNoise_ = std::make_unique<NoiseGenerator>(static_cast<int>(settings_.seed));
    caveNoise_ = std::make_unique<NoiseGenerator>(static_cast<int>(settings_.seed + 1));
    oreNoise_ = std::make_unique<NoiseGenerator>(static_cast<int>(settings_.seed + 2));
    
    Logger::getInstance().info("Initialized 2D generator with style: " + 
                 std::to_string(static_cast<int>(settings_.style)), "World2DGenerator");
}

void World2DGenerator::setDimensions(int width, int height, int depth) {
    settings_.worldWidth = width;
    settings_.worldHeight = height;
    settings_.worldDepth = std::max(1, std::min(depth, 3)); // Clamp to 1-3
}

void World2DGenerator::setSeed(uint64_t seed) {
    settings_.seed = seed;
    surfaceNoise_ = std::make_unique<NoiseGenerator>(static_cast<int>(seed));
    caveNoise_ = std::make_unique<NoiseGenerator>(static_cast<int>(seed + 1));
    oreNoise_ = std::make_unique<NoiseGenerator>(static_cast<int>(seed + 2));
}

void World2DGenerator::generateChunk(Chunk& chunk, int chunkX, int chunkZ) {
    switch (settings_.style) {
        case Style::PLATFORMER:
            // For pure 2D platformer, chunkZ should typically be 0
            generatePlatformerSurface(chunk, chunkX);
            break;
            
        case Style::TOPDOWN:
            // Top-down Zelda-style uses both X and Z coordinates
            generateTopDownWorld(chunk, chunkX, chunkZ);
            break;
            
        case Style::METROIDVANIA:
            // Metroidvania uses room-based generation, not chunk-based
            Logger::getInstance().warning("Metroidvania style requires generateWorld(), not generateChunk()", 
                          "World2DGenerator");
            break;
            
        case Style::RUNNER:
            // Endless runner generates segments on demand
            Logger::getInstance().warning("Runner style uses EndlessRunnerGenerator", 
                          "World2DGenerator");
            break;
            
        case Style::PUZZLE:
            // Puzzle style uses room-based generation
            Logger::getInstance().warning("Puzzle style requires generateWorld(), not generateChunk()", 
                          "World2DGenerator");
            break;
    }
}

void World2DGenerator::generatePlatformerSurface(Chunk& chunk, int chunkX) {
    const int surfaceLevel = static_cast<int>(settings_.worldHeight * settings_.surfaceLevel);
    
    // Generate for each X position in chunk
    for (int localX = 0; localX < 16; localX++) {
        int worldX = chunkX * 16 + localX;
        
        // Get surface height using Perlin noise
        float noise = surfaceNoise_->perlin2D(worldX * 0.01f, 0.0f);
        int height = surfaceLevel + static_cast<int>(noise * settings_.surfaceVariation);
        
        // Clamp height to chunk bounds
        height = std::max(0, std::min(height, 255));
        
        // Fill from bottom to surface for all depth layers
        for (int z = 0; z < settings_.worldDepth; z++) {
            for (int y = 0; y < height; y++) {
                VoxelType type = getVoxelTypeForDepth(height - y);
                chunk.setVoxel(localX, y, z, Voxel(type));
            }
        }
    }
    
    // Generate additional features
    generateUnderground(chunk, chunkX, surfaceLevel);
    generateCaves(chunk, chunkX);
    generateOres(chunk, chunkX);
    generateSurfaceFeatures(chunk, chunkX);
    
    if (settings_.generateBackgroundWalls) {
        generateBackgroundWalls(chunk, chunkX);
    }
}

void World2DGenerator::generateTopDownWorld(Chunk& chunk, int chunkX, int chunkZ) {
    // Top-down Zelda-style world generation
    // World is viewed from above, Y is a single layer (like height 1)
    // X and Z form the playable ground plane
    
    const int groundLevel = 1; // Single layer at Y=1
    
    // Generate terrain for each XZ position in the chunk
    for (int localX = 0; localX < 16; localX++) {
        for (int localZ = 0; localZ < 16; localZ++) {
            int worldX = chunkX * 16 + localX;
            int worldZ = chunkZ * 16 + localZ;
            
            // Use Perlin noise to determine terrain type
            float terrainNoise = surfaceNoise_->perlin2D(worldX * 0.05f, worldZ * 0.05f);
            float detailNoise = surfaceNoise_->perlin2D(worldX * 0.1f, worldZ * 0.1f);
            
            // Base ground layer (grass or dirt)
            chunk.setVoxel(localX, 0, localZ, Voxel(VoxelType::Bedrock)); // Base layer
            
            // Determine ground type based on noise
            VoxelType groundType = VoxelType::Grass;
            if (terrainNoise > 0.6f) {
                groundType = VoxelType::Stone; // Rocky areas
            } else if (terrainNoise < -0.3f) {
                groundType = VoxelType::Sand; // Sandy areas
            } else if (terrainNoise < 0.0f) {
                groundType = VoxelType::Dirt; // Dirt patches
            }
            
            chunk.setVoxel(localX, groundLevel, localZ, Voxel(groundType));
            
            // Add obstacles/walls for dungeon-like areas
            if (detailNoise > 0.7f) {
                // Create walls/obstacles
                chunk.setVoxel(localX, groundLevel + 1, localZ, Voxel(VoxelType::Stone));
                if (detailNoise > 0.85f) {
                    // Taller walls
                    chunk.setVoxel(localX, groundLevel + 2, localZ, Voxel(VoxelType::Stone));
                }
            }
            
            // Add trees/decorations
            if (groundType == VoxelType::Grass && detailNoise > 0.4f && detailNoise < 0.5f) {
                // Place tree trunk
                chunk.setVoxel(localX, groundLevel + 1, localZ, Voxel(VoxelType::Wood));
                // Tree foliage on top
                if (detailNoise > 0.45f) {
                    chunk.setVoxel(localX, groundLevel + 2, localZ, Voxel(VoxelType::Leaves));
                }
            }
            
            // Add water areas
            float waterNoise = caveNoise_->perlin2D(worldX * 0.03f, worldZ * 0.03f);
            if (waterNoise < -0.5f) {
                // Remove ground to create water/holes
                chunk.setVoxel(localX, groundLevel, localZ, Voxel(VoxelType::Water));
            }
        }
    }
    
    // Generate dungeons/rooms if appropriate noise threshold
    // This could be expanded to create Zelda-style dungeons and rooms
}

void World2DGenerator::generateUnderground(Chunk& chunk, int chunkX, int surfaceHeight) {
    // Underground is already filled by generatePlatformerSurface
    // This method can add special underground layers or features
    (void)chunkX;       // Unused parameter - reserved for future chunk-specific logic
    (void)surfaceHeight; // Unused parameter - reserved for future surface-relative logic
    
    for (int localX = 0; localX < 16; localX++) {
        for (int z = 0; z < settings_.worldDepth; z++) {
            // Add bedrock at bottom
            chunk.setVoxel(localX, 0, z, Voxel(VoxelType::Bedrock));
            
            // Deep stone layer
            for (int y = 1; y < 20; y++) {
                if (chunk.getVoxel(localX, y, z) == VoxelType::Stone) {
                    // Keep as stone or could use a different type
                }
            }
        }
    }
}

void World2DGenerator::generateCaves(Chunk& chunk, int chunkX) {
    if (settings_.caveFrequency <= 0.0f) {
        return;
    }
    
    // Use cellular automata for organic cave generation
    std::mt19937 rng(static_cast<unsigned int>(settings_.seed + chunkX));
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    
    // Phase 1: Random initial state
    for (int localX = 0; localX < 16; localX++) {
        int worldX = chunkX * 16 + localX;
        
        for (int y = 20; y < 200; y++) {
            float noise = caveNoise_->perlin2D(worldX * 0.05f, y * 0.05f);
            
            if (noise > (1.0f - settings_.caveFrequency)) {
                for (int z = 0; z < settings_.worldDepth; z++) {
                    chunk.setVoxel(localX, y, z, Voxel(VoxelType::Air));
                }
            }
        }
    }
    
    // Phase 2: Apply cellular automata
    // Note: This is simplified - full implementation would need
    // access to neighboring chunks for proper edge handling
    for (int iteration = 0; iteration < settings_.caveIterations; iteration++) {
        auto chunkCopy = chunk;
        
        for (int localX = 1; localX < 15; localX++) {
            for (int y = 21; y < 199; y++) {
                int solidNeighbors = countSolidNeighbors(chunkCopy, localX, y, 0);
                
                // Birth/death rules
                for (int z = 0; z < settings_.worldDepth; z++) {
                    if (solidNeighbors >= 4) {
                        chunk.setVoxel(localX, y, z, Voxel(VoxelType::Stone));
                    } else {
                        chunk.setVoxel(localX, y, z, Voxel(VoxelType::Air));
                    }
                }
            }
        }
    }
}

void World2DGenerator::generateOres(Chunk& chunk, int chunkX) {
    if (settings_.oreFrequency <= 0.0f) {
        return;
    }
    
    std::mt19937 rng(static_cast<unsigned int>(settings_.seed + chunkX + 1000));
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    
    for (int localX = 0; localX < 16; localX++) {
        int worldX = chunkX * 16 + localX;
        
        for (int y = 1; y < 200; y++) {
            // Check if this is solid stone
            Voxel currentVoxel = chunk.getVoxel(localX, y, 0);
            if (currentVoxel != VoxelType::Stone) {
                continue;
            }
            
            // Use noise for ore placement
            float noise = oreNoise_->perlin2D(worldX * 0.1f, y * 0.1f);
            
            if (noise > (1.0f - settings_.oreFrequency)) {
                // Determine ore type based on depth
                VoxelType oreType;
                if (y < 30) {
                    oreType = VoxelType::Diamond;
                } else if (y < 60) {
                    oreType = VoxelType::Gold;
                } else if (y < 100) {
                    oreType = VoxelType::Iron;
                } else {
                    oreType = VoxelType::Coal;
                }
                
                // Place ore in all depth layers
                for (int z = 0; z < settings_.worldDepth; z++) {
                    chunk.setVoxel(localX, y, z, Voxel(oreType));
                }
            }
        }
    }
}

void World2DGenerator::generateSurfaceFeatures(Chunk& chunk, int chunkX) {
    std::mt19937 rng(static_cast<unsigned int>(settings_.seed + chunkX + 2000));
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    
    for (int localX = 0; localX < 16; localX++) {
        // Find surface
        int surfaceY = -1;
        for (int y = 255; y >= 0; y--) {
            if (chunk.getVoxel(localX, y, 0) != VoxelType::Air) {
                surfaceY = y;
                break;
            }
        }
        
        if (surfaceY < 0 || surfaceY > 250) {
            continue;
        }
        
        // Check if we should place a tree here
        if (dist(rng) < settings_.treeFrequency) {
            if (isValidTreePosition(chunk, localX, surfaceY + 1, 0)) {
                generateTree(chunk, localX, surfaceY + 1, 0);
            }
        }
    }
}

void World2DGenerator::generateBackgroundWalls(Chunk& chunk, int chunkX) {
    // Background walls fill in behind the playable area
    // This is useful for visual depth in 2D games
    (void)chunk;  // Unused parameter - reserved for future implementation
    (void)chunkX; // Unused parameter - reserved for future implementation
    
    // For now, this is a placeholder
    // Full implementation would create wall tiles behind solid blocks
}

VoxelType World2DGenerator::getVoxelTypeForDepth(int depthFromSurface) const {
    if (depthFromSurface == 0) {
        return VoxelType::Grass;
    } else if (depthFromSurface < 3) {
        return VoxelType::Dirt;
    } else {
        return VoxelType::Stone;
    }
}

int World2DGenerator::countSolidNeighbors(const Chunk& chunk, int x, int y, int z) const {
    int count = 0;
    
    // Check 8 neighbors in 2D (or 26 in 3D if worldDepth > 1)
    for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
            if (dx == 0 && dy == 0) continue;
            
            int nx = x + dx;
            int ny = y + dy;
            
            if (nx < 0 || nx >= 16 || ny < 0 || ny >= 256) {
                count++; // Treat out of bounds as solid
                continue;
            }
            
            const Voxel& voxel = chunk.getVoxel(nx, ny, z);
            if (voxel.type != VoxelType::Air) {
                count++;
            }
        }
    }
    
    return count;
}

void World2DGenerator::generateTree(Chunk& chunk, int x, int y, int z) {
    // Simple tree: trunk + foliage
    const int trunkHeight = 4 + (std::rand() % 3);
    const int foliageRadius = 2;
    
    // Trunk
    for (int i = 0; i < trunkHeight; i++) {
        if (y + i < 256) {
            chunk.setVoxel(x, y + i, z, Voxel(VoxelType::Wood));
        }
    }
    
    // Foliage (circle of leaves)
    int foliageY = y + trunkHeight;
    for (int dx = -foliageRadius; dx <= foliageRadius; dx++) {
        for (int dy = -foliageRadius; dy <= foliageRadius; dy++) {
            if (dx * dx + dy * dy <= foliageRadius * foliageRadius) {
                int leafX = x + dx;
                int leafY = foliageY + dy;
                
                if (leafX >= 0 && leafX < 16 && leafY >= 0 && leafY < 256) {
                    // Don't overwrite trunk
                    if (chunk.getVoxel(leafX, leafY, z) == VoxelType::Air) {
                        chunk.setVoxel(leafX, leafY, z, Voxel(VoxelType::Leaves));
                    }
                }
            }
        }
    }
}

bool World2DGenerator::isValidTreePosition(const Chunk& chunk, int x, int y, int z) const {
    // Check if there's enough space above
    for (int i = 0; i < 8; i++) {
        if (y + i >= 256) return false;
        if (chunk.getVoxel(x, y + i, z) != VoxelType::Air) {
            return false;
        }
    }
    
    // Check if standing on grass
    if (y > 0 && chunk.getVoxel(x, y - 1, z) == VoxelType::Grass) {
        return true;
    }
    
    return false;
}

void World2DGenerator::generateWorld(VoxelWorld& world) {
    (void)world; // Unused parameter - reserved for future implementation
    Logger::getInstance().info("Generating complete 2D world...", "World2DGenerator");
    
    // Calculate number of chunks needed
    int chunksNeeded = (settings_.worldWidth + 15) / 16;
    
    for (int chunkX = 0; chunkX < chunksNeeded; chunkX++) {
        // Generate chunk at (chunkX, 0) - all 2D chunks are at Z=0
        auto chunk = std::make_shared<Chunk>(ChunkPos(chunkX, 0));
        generateChunk(*chunk, chunkX, 0);
        
        // Add to world (assumes VoxelWorld has method to add chunks)
        // world.addChunk(chunkX, 0, chunk);
    }
    
    Logger::getInstance().info("2D world generation complete", "World2DGenerator");
}

// MetroidvaniaGenerator implementation
std::vector<MetroidvaniaGenerator::Room> MetroidvaniaGenerator::generate(int roomCount, uint64_t seed) {
    (void)seed; // Unused parameter - reserved for future randomization
    std::vector<Room> rooms;
    
    // Generate main path
    auto mainPath = generateMainPath(roomCount);
    rooms.insert(rooms.end(), mainPath.begin(), mainPath.end());
    
    // Add side areas
    auto sideAreas = generateSideAreas(mainPath);
    rooms.insert(rooms.end(), sideAreas.begin(), sideAreas.end());
    
    // Connect rooms
    connectRooms(rooms);
    
    return rooms;
}

std::vector<MetroidvaniaGenerator::Room> MetroidvaniaGenerator::generateMainPath(int roomCount) {
    std::vector<Room> path;
    
    // Start room
    path.push_back(createRoom(RoomType::START, 0));
    
    // Generate rooms along main path
    for (int i = 1; i < roomCount; i++) {
        path.push_back(createRoom(RoomType::NORMAL, i));
    }
    
    return path;
}

std::vector<MetroidvaniaGenerator::Room> MetroidvaniaGenerator::generateSideAreas(
    const std::vector<Room>& mainPath) {
    (void)mainPath; // Unused parameter - reserved for future side area generation
    std::vector<Room> sideAreas;
    
    // Generate treasure and ability rooms off the main path
    // This is a placeholder implementation
    
    return sideAreas;
}

void MetroidvaniaGenerator::connectRooms(std::vector<Room>& rooms) {
    (void)rooms; // Unused parameter - reserved for future room connection logic
    // Create connections between adjacent rooms
    // This is a placeholder implementation
}

MetroidvaniaGenerator::Room MetroidvaniaGenerator::createRoom(RoomType type, int difficulty) {
    Room room;
    room.type = type;
    room.difficulty = difficulty;
    room.size = glm::ivec2(20 + std::rand() % 30, 15 + std::rand() % 20);
    room.requiredAbility = AbilityRequired::NONE;
    
    return room;
}

// EndlessRunnerGenerator implementation
EndlessRunnerGenerator::Segment EndlessRunnerGenerator::generateNextSegment(float currentDifficulty) {
    Segment segment;
    segment.length = 50 + std::rand() % 50;
    segment.difficulty = currentDifficulty;
    
    // Generate obstacles
    int obstacleCount = static_cast<int>(currentDifficulty * 10);
    for (int i = 0; i < obstacleCount; i++) {
        segment.obstacles.push_back(generateObstacle(currentDifficulty));
    }
    
    // Generate platforms
    int platformCount = 3 + std::rand() % 5;
    for (int i = 0; i < platformCount; i++) {
        segment.platforms.push_back(generatePlatform());
    }
    
    // Generate collectibles
    int collectibleCount = 5 + std::rand() % 10;
    for (int i = 0; i < collectibleCount; i++) {
        segment.collectibles.push_back(generateCollectible());
    }
    
    return segment;
}

void EndlessRunnerGenerator::buildSegment(VoxelWorld& world, const Segment& segment, int startX) {
    (void)world;   // Unused parameter - reserved for future world building
    (void)segment; // Unused parameter - reserved for future segment placement
    (void)startX;  // Unused parameter - reserved for future positioning
    // Build floor
    for (int x = 0; x < segment.length; x++) {
        // world.setVoxel(startX + x, 0, 0, VoxelType::Stone);
    }
    
    // Place obstacles
    // Place platforms
    // Place collectibles
}

EndlessRunnerGenerator::Obstacle EndlessRunnerGenerator::generateObstacle(float difficulty) {
    Obstacle obstacle;
    obstacle.x = std::rand() % 50;
    obstacle.y = 1;
    obstacle.width = 1 + std::rand() % 2;
    obstacle.height = 2 + static_cast<int>(difficulty * 3);
    obstacle.type = VoxelType::Stone;
    return obstacle;
}

EndlessRunnerGenerator::Platform EndlessRunnerGenerator::generatePlatform() {
    Platform platform;
    platform.x = std::rand() % 50;
    platform.y = 3 + std::rand() % 5;
    platform.width = 3 + std::rand() % 4;
    platform.type = VoxelType::Stone;
    return platform;
}

EndlessRunnerGenerator::Collectible EndlessRunnerGenerator::generateCollectible() {
    Collectible collectible;
    collectible.x = std::rand() % 50;
    collectible.y = 2 + std::rand() % 6;
    return collectible;
}

} // namespace fresh
