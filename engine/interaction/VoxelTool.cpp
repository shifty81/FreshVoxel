#include "interaction/VoxelTool.h"

#include <algorithm>
#include <cmath>

#include "core/Logger.h"
#include "voxel/VoxelWorld.h"
#include "voxel/Chunk.h"
#include "voxel/VoxelTypes.h"

namespace fresh
{

// =============================================================================
// VoxelTool Base Class
// =============================================================================

VoxelTool::VoxelTool(VoxelToolType type)
    : m_world(nullptr), m_type(type), m_size(1)
{
}

bool VoxelTool::initialize(VoxelWorld* world)
{
    m_world = world;
    return m_world != nullptr;
}

void VoxelTool::setSize(int size)
{
    m_size = std::clamp(size, 1, 10);
}

bool VoxelTool::useToolDirectional(const WorldPos& pos, const glm::vec3& direction, 
                                   VoxelType voxelType)
{
    (void)direction; // Unused parameter
    // Default implementation delegates to basic useTool
    return useTool(pos, voxelType);
}

// =============================================================================
// ConstructionHammer Implementation
// =============================================================================

ConstructionHammer::ConstructionHammer()
    : VoxelTool(VoxelToolType::ConstructionHammer),
      m_currentBlueprint(Blueprint::SingleBlock)
{
}

bool ConstructionHammer::useTool(const WorldPos& pos, VoxelType voxelType)
{
    if (!m_world) {
        return false;
    }

    // Use current blueprint
    return placeBlueprint(pos, voxelType);
}

bool ConstructionHammer::useToolDirectional(const WorldPos& pos, const glm::vec3& direction,
                                           VoxelType voxelType)
{
    (void)direction; // Unused parameter - future enhancement for oriented placement
    if (!m_world) {
        return false;
    }

    // For directional use, place blueprint oriented based on direction
    // For now, use standard placement
    return placeBlueprint(pos, voxelType);
}

bool ConstructionHammer::placeBlueprint(const WorldPos& pos, VoxelType voxelType)
{
    if (!m_world) {
        return false;
    }

    switch (m_currentBlueprint) {
    case Blueprint::SingleBlock:
        m_world->setVoxel(pos, Voxel(voxelType));
        return true;
    case Blueprint::Wall:
        placeWall(pos, voxelType);
        return true;
    case Blueprint::Foundation:
        placeFoundation(pos, voxelType);
        return true;
    case Blueprint::Column:
        placeColumn(pos, voxelType);
        return true;
    case Blueprint::Floor:
        placeFloor(pos, voxelType);
        return true;
    case Blueprint::Stairs:
        placeStairs(pos, voxelType);
        return true;
    case Blueprint::Arch:
        placeArch(pos, voxelType);
        return true;
    default:
        return false;
    }
}

void ConstructionHammer::placeWall(const WorldPos& pos, VoxelType voxelType)
{
    // Create a vertical wall (5 blocks high, 3 blocks wide)
    for (int y = 0; y < 5; ++y) {
        for (int x = -1; x <= 1; ++x) {
            WorldPos blockPos(pos.x + x, pos.y + y, pos.z);
            m_world->setVoxel(blockPos, Voxel(voxelType));
        }
    }
}

void ConstructionHammer::placeFoundation(const WorldPos& pos, VoxelType voxelType)
{
    // Create a 3x3 horizontal foundation
    for (int x = -1; x <= 1; ++x) {
        for (int z = -1; z <= 1; ++z) {
            WorldPos blockPos(pos.x + x, pos.y, pos.z + z);
            m_world->setVoxel(blockPos, Voxel(voxelType));
        }
    }
}

void ConstructionHammer::placeColumn(const WorldPos& pos, VoxelType voxelType)
{
    // Create a vertical column (8 blocks high)
    for (int y = 0; y < 8; ++y) {
        WorldPos blockPos(pos.x, pos.y + y, pos.z);
        m_world->setVoxel(blockPos, Voxel(voxelType));
    }
}

void ConstructionHammer::placeFloor(const WorldPos& pos, VoxelType voxelType)
{
    // Create a 5x5 flat floor
    for (int x = -2; x <= 2; ++x) {
        for (int z = -2; z <= 2; ++z) {
            WorldPos blockPos(pos.x + x, pos.y, pos.z + z);
            m_world->setVoxel(blockPos, Voxel(voxelType));
        }
    }
}

void ConstructionHammer::placeStairs(const WorldPos& pos, VoxelType voxelType)
{
    // Create a simple staircase going up in +Z direction
    for (int i = 0; i < 5; ++i) {
        WorldPos blockPos(pos.x, pos.y + i, pos.z + i);
        m_world->setVoxel(blockPos, Voxel(voxelType));
    }
}

void ConstructionHammer::placeArch(const WorldPos& pos, VoxelType voxelType)
{
    // Create a simple arch (two columns with top blocks)
    // Left column
    for (int y = 0; y < 4; ++y) {
        m_world->setVoxel(WorldPos(pos.x - 2, pos.y + y, pos.z), Voxel(voxelType));
    }
    // Right column
    for (int y = 0; y < 4; ++y) {
        m_world->setVoxel(WorldPos(pos.x + 2, pos.y + y, pos.z), Voxel(voxelType));
    }
    // Top
    for (int x = -2; x <= 2; ++x) {
        m_world->setVoxel(WorldPos(pos.x + x, pos.y + 4, pos.z), Voxel(voxelType));
    }
}

// =============================================================================
// Pickaxe Implementation
// =============================================================================

Pickaxe::Pickaxe()
    : VoxelTool(VoxelToolType::Pickaxe)
{
}

bool Pickaxe::useTool(const WorldPos& pos, VoxelType voxelType)
{
    (void)voxelType; // Pickaxe removes blocks, voxelType is ignored
    if (m_size == 1) {
        return removeVoxel(pos);
    } else {
        return removeArea(pos, m_size);
    }
}

bool Pickaxe::removeVoxel(const WorldPos& pos)
{
    if (!m_world) {
        return false;
    }

    // Get current voxel
    Voxel* voxelPtr = m_world->getVoxel(pos);
    if (!voxelPtr) {
        return false;
    }
    Voxel currentVoxel = *voxelPtr;
    
    // Don't remove air or bedrock
    if (currentVoxel.type == VoxelType::Air || currentVoxel.type == VoxelType::Bedrock) {
        return false;
    }

    // Remove the voxel (set to air)
    m_world->setVoxel(pos, Voxel(VoxelType::Air));
    return true;
}

bool Pickaxe::removeArea(const WorldPos& pos, int radius)
{
    if (!m_world) {
        return false;
    }

    bool removedAny = false;

    // Remove voxels in a sphere around the position
    for (int x = -radius; x <= radius; ++x) {
        for (int y = -radius; y <= radius; ++y) {
            for (int z = -radius; z <= radius; ++z) {
                // Check if position is within sphere
                float distSq = static_cast<float>(x * x + y * y + z * z);
                if (distSq <= static_cast<float>(radius * radius)) {
                    WorldPos blockPos(pos.x + x, pos.y + y, pos.z + z);
                    if (removeVoxel(blockPos)) {
                        removedAny = true;
                    }
                }
            }
        }
    }

    return removedAny;
}

// =============================================================================
// Rake Implementation
// =============================================================================

Rake::Rake()
    : VoxelTool(VoxelToolType::Rake)
{
}

bool Rake::useTool(const WorldPos& pos, VoxelType voxelType)
{
    (void)voxelType; // Unused parameter
    // Rake levels ground at current height
    return levelGround(pos);
}

bool Rake::flattenToHeight(const WorldPos& pos, int targetHeight, int radius)
{
    if (!m_world) {
        return false;
    }

    // Flatten terrain to target height in circular area
    for (int x = -radius; x <= radius; ++x) {
        for (int z = -radius; z <= radius; ++z) {
            // Check if within circular radius
            float distSq = static_cast<float>(x * x + z * z);
            if (distSq <= static_cast<float>(radius * radius)) {
                WorldPos columnPos(pos.x + x, 0, pos.z + z);

                // Fill from bottom to target height with dirt
                for (int y = 0; y <= targetHeight; ++y) {
                    WorldPos blockPos(columnPos.x, y, columnPos.z);
                    Voxel* voxelPtr = m_world->getVoxel(blockPos);
                    if (!voxelPtr) continue;
                    Voxel currentVoxel = *voxelPtr;
                    
                    if (y < targetHeight) {
                        // Fill below target height
                        if (currentVoxel.type == VoxelType::Air) {
                            m_world->setVoxel(blockPos, Voxel(VoxelType::Dirt));
                        }
                    } else if (y == targetHeight) {
                        // Top layer should be grass or current material
                        if (currentVoxel.type == VoxelType::Air) {
                            m_world->setVoxel(blockPos, Voxel(VoxelType::Grass));
                        }
                    }
                }

                // Remove blocks above target height
                for (int y = targetHeight + 1; y < CHUNK_HEIGHT; ++y) {
                    WorldPos blockPos(columnPos.x, y, columnPos.z);
                    Voxel* voxelPtr = m_world->getVoxel(blockPos);
                    if (!voxelPtr) break;
                    Voxel currentVoxel = *voxelPtr;
                    
                    // Stop at first air block (no need to continue up)
                    if (currentVoxel.type == VoxelType::Air) {
                        break;
                    }
                    
                    // Don't remove bedrock
                    if (currentVoxel.type != VoxelType::Bedrock) {
                        m_world->setVoxel(blockPos, Voxel(VoxelType::Air));
                    }
                }
            }
        }
    }

    return true;
}

bool Rake::smoothTerrain(const WorldPos& pos, int radius)
{
    if (!m_world) {
        return false;
    }

    // Calculate average height in the area
    int totalHeight = 0;
    int count = 0;

    for (int x = -radius; x <= radius; ++x) {
        for (int z = -radius; z <= radius; ++z) {
            float distSq = static_cast<float>(x * x + z * z);
            if (distSq <= static_cast<float>(radius * radius)) {
                WorldPos columnPos(pos.x + x, 0, pos.z + z);
                
                // Find surface height
                for (int y = CHUNK_HEIGHT - 1; y >= 0; --y) {
                    Voxel* voxelPtr = m_world->getVoxel(WorldPos(columnPos.x, y, columnPos.z));
                    if (!voxelPtr) continue;
                    Voxel voxel = *voxelPtr;
                    if (voxel.isSolid()) {
                        totalHeight += y;
                        count++;
                        break;
                    }
                }
            }
        }
    }

    if (count == 0) {
        return false;
    }

    // Calculate average and flatten to that height
    int averageHeight = totalHeight / count;
    return flattenToHeight(pos, averageHeight, radius);
}

bool Rake::levelGround(const WorldPos& pos)
{
    // Level ground to the height at the center position
    return flattenToHeight(pos, pos.y, m_size);
}

} // namespace fresh
