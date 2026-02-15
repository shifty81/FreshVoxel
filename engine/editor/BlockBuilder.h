#pragma once
#include <memory>
#include <vector>

#include <glm/glm.hpp>

#include "voxelship/VoxelBlock.h"

namespace fresh
{
namespace editor
{

/**
 * @brief Block stretching mode for interactive editing
 */
enum class StretchMode {
    None,
    Corner,     // Stretch from a corner
    Edge,       // Stretch from an edge
    Face        // Stretch from a face
};

/**
 * @brief Builder mode tool for creating and modifying voxel blocks
 * 
 * Supports interactive block stretching with structural integrity constraints.
 * Allows grabbing corners, edges, or faces to stretch blocks for ship/station building.
 */
class BlockBuilder
{
public:
    BlockBuilder();
    ~BlockBuilder() = default;

    // Builder mode control
    void enable();
    void disable();
    bool isEnabled() const { return enabled; }

    // Block management
    void selectBlock(voxelship::VoxelBlock* block);
    void deselectBlock();
    voxelship::VoxelBlock* getSelectedBlock() const { return selectedBlock; }

    // Interactive stretching
    bool startStretch(const glm::vec3& rayOrigin, const glm::vec3& rayDirection);
    bool updateStretch(const glm::vec3& rayOrigin, const glm::vec3& rayDirection);
    bool endStretch();
    void cancelStretch();

    // Stretch mode
    StretchMode getStretchMode() const { return stretchMode; }
    void setStretchMode(StretchMode mode) { stretchMode = mode; }

    // Configuration
    void setMaxStretchRatio(float ratio) { maxStretchRatio = ratio; }
    float getMaxStretchRatio() const { return maxStretchRatio; }
    
    void setSnapToGrid(bool snap) { snapToGrid = snap; }
    bool isSnapToGrid() const { return snapToGrid; }
    
    void setGridSize(float size) { gridSize = size; }
    float getGridSize() const { return gridSize; }

    // Procedural generation integration
    void applyRandomStretch(voxelship::VoxelBlock& block, float minRatio = 0.5f, 
                           float maxRatio = 3.0f, int seed = 0);
    
    // Validation
    bool isStretchValid(const voxelship::VoxelBlock& block, const glm::vec3& newSize) const;
    float calculateStretchRatio(const glm::vec3& originalSize, 
                               const glm::vec3& newSize) const;

private:
    bool enabled;
    voxelship::VoxelBlock* selectedBlock;
    StretchMode stretchMode;
    
    // Stretching state
    bool isStretching;
    int stretchCornerIndex;
    glm::vec3 stretchStartPos;
    glm::vec3 originalBlockSize;
    glm::vec3 originalBlockPos;
    
    // Configuration
    float maxStretchRatio;
    bool snapToGrid;
    float gridSize;
    
    // Helper methods
    int findClosestCorner(const voxelship::VoxelBlock& block, 
                         const glm::vec3& point) const;
    glm::vec3 snapToGridPos(const glm::vec3& pos) const;
    bool rayIntersectBlock(const voxelship::VoxelBlock& block,
                          const glm::vec3& rayOrigin,
                          const glm::vec3& rayDirection,
                          glm::vec3& intersectionPoint) const;
};

} // namespace editor
} // namespace fresh
