#include "editor/TerraformingSystem.h"

#include <algorithm>
#include <cmath>
#include <iostream>

#include "voxel/Chunk.h"
#include "voxel/VoxelWorld.h"

namespace fresh
{

TerraformingSystem::TerraformingSystem(VoxelWorld* world)
    : m_world(world),
      m_currentTool(TerraformTool::SingleBlock),
      m_currentMode(TerraformMode::Place),
      m_selectedVoxelType(VoxelType::Stone),
      m_toolSize(1),
      m_inCommandGroup(false)
{
}

TerraformingSystem::~TerraformingSystem() {}

bool TerraformingSystem::initialize()
{
    std::cout << "Terraforming system initialized" << std::endl;
    return true;
}

void TerraformingSystem::setToolSize(int size)
{
    m_toolSize = std::clamp(size, 1, 10);
}

bool TerraformingSystem::applyTool(const WorldPos& pos)
{
    if (!m_world) {
        return false;
    }

    // Start a new operation
    m_currentOperation.clear();

    // Apply the selected tool
    switch (m_currentTool) {
    case TerraformTool::SingleBlock:
        applySingle(pos);
        break;
    case TerraformTool::Brush:
        applyBrush(pos);
        break;
    case TerraformTool::Sphere:
        applySphere(pos, false);
        break;
    case TerraformTool::FilledSphere:
        applySphere(pos, true);
        break;
    case TerraformTool::Cube:
        applyCube(pos, false);
        break;
    case TerraformTool::FilledCube:
        applyCube(pos, true);
        break;
    case TerraformTool::Flatten:
        applyFlatten(pos);
        break;
    case TerraformTool::Smooth:
        applySmooth(pos);
        break;
    case TerraformTool::Paint:
        applyPaint(pos);
        break;
    default:
        break;
    }

    // Add operation to undo stack if any changes were made
    if (!m_currentOperation.empty()) {
        m_undoStack.push_back(m_currentOperation);

        // Limit undo stack size
        if (m_undoStack.size() > MAX_UNDO_STACK_SIZE) {
            m_undoStack.erase(m_undoStack.begin());
        }

        // Clear redo stack when new operation is performed
        m_redoStack.clear();

        std::cout << "Applied " << m_currentOperation.size() << " voxel changes" << std::endl;
        return true;
    }

    return false;
}

bool TerraformingSystem::applyToolLine(const WorldPos& start, const WorldPos& end)
{
    if (!m_world) {
        return false;
    }

    m_currentOperation.clear();

    // Bresenham's line algorithm in 3D
    int dx = std::abs(end.x - start.x);
    int dy = std::abs(end.y - start.y);
    int dz = std::abs(end.z - start.z);

    int sx = start.x < end.x ? 1 : -1;
    int sy = start.y < end.y ? 1 : -1;
    int sz = start.z < end.z ? 1 : -1;

    int dm = std::max({dx, dy, dz});

    WorldPos current = start;

    int i = dm;
    dx *= 2;
    dy *= 2;
    dz *= 2;

    while (i-- >= 0) {
        // Apply tool at current position
        switch (m_currentTool) {
        case TerraformTool::SingleBlock:
            applySingle(current);
            break;
        case TerraformTool::Brush:
            applyBrush(current);
            break;
        default:
            applySingle(current);
            break;
        }

        // Move to next point
        if (dx >= dm) {
            current.x += sx;
            dx -= 2 * dm;
        }
        if (dy >= dm) {
            current.y += sy;
            dy -= 2 * dm;
        }
        if (dz >= dm) {
            current.z += sz;
            dz -= 2 * dm;
        }
        dx += 2 * std::abs(end.x - start.x);
        dy += 2 * std::abs(end.y - start.y);
        dz += 2 * std::abs(end.z - start.z);
    }

    if (!m_currentOperation.empty()) {
        m_undoStack.push_back(m_currentOperation);
        if (m_undoStack.size() > MAX_UNDO_STACK_SIZE) {
            m_undoStack.erase(m_undoStack.begin());
        }
        m_redoStack.clear();
        return true;
    }

    return false;
}

void TerraformingSystem::applySingle(const WorldPos& pos)
{
    Voxel* voxel = m_world->getVoxel(pos);
    if (!voxel) {
        return;
    }

    Voxel newVoxel;
    if (m_currentMode == TerraformMode::Place) {
        newVoxel = Voxel(m_selectedVoxelType);
    } else if (m_currentMode == TerraformMode::Remove) {
        newVoxel = Voxel(VoxelType::Air);
    } else { // Replace
        if (voxel->type != VoxelType::Air) {
            newVoxel = Voxel(m_selectedVoxelType);
        } else {
            return;
        }
    }

    if (voxel->type != newVoxel.type) {
        recordCommand(pos, *voxel, newVoxel);
        setVoxel(pos, newVoxel);
    }
}

void TerraformingSystem::applyBrush(const WorldPos& pos)
{
    int radius = m_toolSize;

    for (int y = -radius; y <= radius; ++y) {
        for (int z = -radius; z <= radius; ++z) {
            for (int x = -radius; x <= radius; ++x) {
                float dist = static_cast<float>(std::sqrt(x * x + y * y + z * z));
                if (dist <= radius) {
                    WorldPos targetPos(pos.x + x, pos.y + y, pos.z + z);
                    applySingle(targetPos);
                }
            }
        }
    }
}

void TerraformingSystem::applySphere(const WorldPos& pos, bool filled)
{
    int radius = m_toolSize;

    for (int y = -radius; y <= radius; ++y) {
        for (int z = -radius; z <= radius; ++z) {
            for (int x = -radius; x <= radius; ++x) {
                float dist = static_cast<float>(std::sqrt(x * x + y * y + z * z));

                bool shouldPlace =
                    filled ? (dist <= radius) : (dist >= radius - 0.5f && dist <= radius + 0.5f);

                if (shouldPlace) {
                    WorldPos targetPos(pos.x + x, pos.y + y, pos.z + z);
                    applySingle(targetPos);
                }
            }
        }
    }
}

void TerraformingSystem::applyCube(const WorldPos& pos, bool filled)
{
    int size = m_toolSize;

    for (int y = -size; y <= size; ++y) {
        for (int z = -size; z <= size; ++z) {
            for (int x = -size; x <= size; ++x) {
                bool isEdge =
                    (x == -size || x == size || y == -size || y == size || z == -size || z == size);

                if (filled || isEdge) {
                    WorldPos targetPos(pos.x + x, pos.y + y, pos.z + z);
                    applySingle(targetPos);
                }
            }
        }
    }
}

void TerraformingSystem::applyFlatten(const WorldPos& pos)
{
    int radius = m_toolSize;
    int targetHeight = pos.y;

    for (int z = -radius; z <= radius; ++z) {
        for (int x = -radius; x <= radius; ++x) {
            float dist = static_cast<float>(std::sqrt(x * x + z * z));
            if (dist <= radius) {
                // Set all blocks above target height to air
                for (int y = targetHeight + 1; y < CHUNK_HEIGHT; ++y) {
                    WorldPos targetPos(pos.x + x, y, pos.z + z);
                    Voxel* voxel = m_world->getVoxel(targetPos);
                    if (voxel && voxel->type != VoxelType::Air) {
                        Voxel newVoxel(VoxelType::Air);
                        recordCommand(targetPos, *voxel, newVoxel);
                        setVoxel(targetPos, newVoxel);
                    }
                }

                // Fill blocks below target height
                for (int y = 0; y <= targetHeight; ++y) {
                    WorldPos targetPos(pos.x + x, y, pos.z + z);
                    Voxel* voxel = m_world->getVoxel(targetPos);
                    if (voxel && voxel->type == VoxelType::Air) {
                        Voxel newVoxel(m_selectedVoxelType);
                        recordCommand(targetPos, *voxel, newVoxel);
                        setVoxel(targetPos, newVoxel);
                    }
                }
            }
        }
    }
}

void TerraformingSystem::applySmooth(const WorldPos& pos)
{
    int radius = m_toolSize;

    // Calculate average height in area
    int totalHeight = 0;
    int count = 0;

    for (int z = -radius; z <= radius; ++z) {
        for (int x = -radius; x <= radius; ++x) {
            // Find surface height at this position
            for (int y = CHUNK_HEIGHT - 1; y >= 0; --y) {
                WorldPos checkPos(pos.x + x, y, pos.z + z);
                Voxel* voxel = m_world->getVoxel(checkPos);
                if (voxel && voxel->isSolid()) {
                    totalHeight += y;
                    count++;
                    break;
                }
            }
        }
    }

    if (count == 0)
        return;

    int avgHeight = totalHeight / count;

    // Apply smoothing by moving towards average height
    for (int z = -radius; z <= radius; ++z) {
        for (int x = -radius; x <= radius; ++x) {
            float dist = static_cast<float>(std::sqrt(x * x + z * z));
            if (dist <= radius) {
                // Find current surface height
                int surfaceHeight = 0;
                for (int y = CHUNK_HEIGHT - 1; y >= 0; --y) {
                    WorldPos checkPos(pos.x + x, y, pos.z + z);
                    Voxel* voxel = m_world->getVoxel(checkPos);
                    if (voxel && voxel->isSolid()) {
                        surfaceHeight = y;
                        break;
                    }
                }

                // Smoothly adjust towards average
                int targetHeight = (surfaceHeight + avgHeight) / 2;

                if (targetHeight > surfaceHeight) {
                    // Add blocks
                    for (int y = surfaceHeight + 1; y <= targetHeight; ++y) {
                        WorldPos targetPos(pos.x + x, y, pos.z + z);
                        applySingle(targetPos);
                    }
                } else if (targetHeight < surfaceHeight) {
                    // Remove blocks
                    for (int y = surfaceHeight; y > targetHeight; --y) {
                        WorldPos targetPos(pos.x + x, y, pos.z + z);
                        Voxel* voxel = m_world->getVoxel(targetPos);
                        if (voxel && voxel->isSolid()) {
                            Voxel newVoxel(VoxelType::Air);
                            recordCommand(targetPos, *voxel, newVoxel);
                            setVoxel(targetPos, newVoxel);
                        }
                    }
                }
            }
        }
    }
}

void TerraformingSystem::applyPaint(const WorldPos& pos)
{
    int radius = m_toolSize;

    for (int y = -radius; y <= radius; ++y) {
        for (int z = -radius; z <= radius; ++z) {
            for (int x = -radius; x <= radius; ++x) {
                float dist = static_cast<float>(std::sqrt(x * x + y * y + z * z));
                if (dist <= radius) {
                    WorldPos targetPos(pos.x + x, pos.y + y, pos.z + z);
                    Voxel* voxel = m_world->getVoxel(targetPos);

                    if (voxel && voxel->type != VoxelType::Air &&
                        voxel->type != m_selectedVoxelType) {
                        Voxel newVoxel(m_selectedVoxelType);
                        recordCommand(targetPos, *voxel, newVoxel);
                        setVoxel(targetPos, newVoxel);
                    }
                }
            }
        }
    }
}

void TerraformingSystem::setVoxel(const WorldPos& pos, const Voxel& voxel)
{
    m_world->setVoxel(pos, voxel);
}

void TerraformingSystem::recordCommand(const WorldPos& pos, const Voxel& oldVoxel,
                                       const Voxel& newVoxel)
{
    TerraformCommand cmd;
    cmd.position = pos;
    cmd.oldVoxel = oldVoxel;
    cmd.newVoxel = newVoxel;
    m_currentOperation.push_back(cmd);
}

bool TerraformingSystem::undo()
{
    if (m_undoStack.empty()) {
        return false;
    }

    auto commands = m_undoStack.back();
    m_undoStack.pop_back();

    // Reverse the commands
    for (auto it = commands.rbegin(); it != commands.rend(); ++it) {
        m_world->setVoxel(it->position, it->oldVoxel);
    }

    m_redoStack.push_back(commands);

    std::cout << "Undid " << commands.size() << " voxel changes" << std::endl;
    return true;
}

bool TerraformingSystem::redo()
{
    if (m_redoStack.empty()) {
        return false;
    }

    auto commands = m_redoStack.back();
    m_redoStack.pop_back();

    // Reapply the commands
    for (const auto& cmd : commands) {
        m_world->setVoxel(cmd.position, cmd.newVoxel);
    }

    m_undoStack.push_back(commands);

    std::cout << "Redid " << commands.size() << " voxel changes" << std::endl;
    return true;
}

void TerraformingSystem::clearHistory()
{
    m_undoStack.clear();
    m_redoStack.clear();
    std::cout << "Cleared undo/redo history" << std::endl;
}

void TerraformingSystem::beginCommandGroup()
{
    if (m_inCommandGroup) {
        std::cerr << "Warning: beginCommandGroup() called while already in a command group" << std::endl;
        return;
    }
    
    m_inCommandGroup = true;
    m_commandGroup.clear();
    std::cout << "Started command group" << std::endl;
}

void TerraformingSystem::recordVoxelChange(const WorldPos& pos, const Voxel& oldVoxel, const Voxel& newVoxel)
{
    TerraformCommand cmd;
    cmd.position = pos;
    cmd.oldVoxel = oldVoxel;
    cmd.newVoxel = newVoxel;
    
    if (m_inCommandGroup) {
        // Add to current group
        m_commandGroup.push_back(cmd);
    } else {
        // Create a single-command operation
        std::vector<TerraformCommand> singleOp;
        singleOp.push_back(cmd);
        
        m_undoStack.push_back(singleOp);
        
        // Clear redo stack when new command is added
        m_redoStack.clear();
        
        // Limit undo stack size
        if (m_undoStack.size() > MAX_UNDO_STACK_SIZE) {
            m_undoStack.erase(m_undoStack.begin());
        }
    }
}

void TerraformingSystem::endCommandGroup()
{
    if (!m_inCommandGroup) {
        std::cerr << "Warning: endCommandGroup() called without matching beginCommandGroup()" << std::endl;
        return;
    }
    
    m_inCommandGroup = false;
    
    if (!m_commandGroup.empty()) {
        // Push the group to undo stack
        m_undoStack.push_back(m_commandGroup);
        
        // Clear redo stack when new command is added
        m_redoStack.clear();
        
        // Limit undo stack size
        if (m_undoStack.size() > MAX_UNDO_STACK_SIZE) {
            m_undoStack.erase(m_undoStack.begin());
        }
        
        std::cout << "Ended command group with " << m_commandGroup.size() << " commands" << std::endl;
    }
    
    m_commandGroup.clear();
}

} // namespace fresh
