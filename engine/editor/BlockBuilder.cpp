#include "editor/BlockBuilder.h"

#include <algorithm>
#include <cmath>
#include <random>

namespace fresh
{
namespace editor
{

BlockBuilder::BlockBuilder()
    : enabled(false),
      selectedBlock(nullptr),
      stretchMode(StretchMode::Corner),
      isStretching(false),
      stretchCornerIndex(0),
      stretchStartPos(0.0f),
      originalBlockSize(1.0f),
      originalBlockPos(0.0f),
      maxStretchRatio(5.0f),
      snapToGrid(true),
      gridSize(0.5f)
{
}

void BlockBuilder::enable()
{
    enabled = true;
}

void BlockBuilder::disable()
{
    enabled = false;
    if (isStretching) {
        cancelStretch();
    }
}

void BlockBuilder::selectBlock(voxelship::VoxelBlock* block)
{
    if (isStretching) {
        cancelStretch();
    }
    selectedBlock = block;
}

void BlockBuilder::deselectBlock()
{
    if (isStretching) {
        cancelStretch();
    }
    selectedBlock = nullptr;
}

bool BlockBuilder::startStretch(const glm::vec3& rayOrigin, const glm::vec3& rayDirection)
{
    if (!enabled || !selectedBlock) {
        return false;
    }

    glm::vec3 intersectionPoint;
    if (!rayIntersectBlock(*selectedBlock, rayOrigin, rayDirection, intersectionPoint)) {
        return false;
    }

    // Find the closest corner to the intersection point
    stretchCornerIndex = findClosestCorner(*selectedBlock, intersectionPoint);
    
    // Store original state
    originalBlockSize = selectedBlock->getSize();
    originalBlockPos = selectedBlock->getPosition();
    stretchStartPos = intersectionPoint;
    
    isStretching = true;
    return true;
}

bool BlockBuilder::updateStretch(const glm::vec3& rayOrigin, const glm::vec3& rayDirection)
{
    if (!isStretching || !selectedBlock) {
        return false;
    }

    // Calculate intersection with a plane perpendicular to view direction
    // For simplicity, we'll use the current mouse position directly
    glm::vec3 currentPos = rayOrigin + rayDirection * glm::length(stretchStartPos - rayOrigin);
    
    if (snapToGrid) {
        currentPos = snapToGridPos(currentPos);
    }

    // Calculate delta from start
    glm::vec3 delta = currentPos - stretchStartPos;
    
    // Apply stretch to the block
    return selectedBlock->stretchCorner(stretchCornerIndex, delta);
}

bool BlockBuilder::endStretch()
{
    if (!isStretching) {
        return false;
    }

    // Check if the final stretch is structurally sound
    if (!selectedBlock->isStructurallySound()) {
        // Revert to original if not structurally sound
        selectedBlock->setSize(originalBlockSize);
        selectedBlock->setPosition(originalBlockPos);
        isStretching = false;
        return false;
    }

    isStretching = false;
    return true;
}

void BlockBuilder::cancelStretch()
{
    if (!isStretching || !selectedBlock) {
        return;
    }

    // Revert block to original state
    selectedBlock->setSize(originalBlockSize);
    selectedBlock->setPosition(originalBlockPos);
    
    isStretching = false;
}

void BlockBuilder::applyRandomStretch(voxelship::VoxelBlock& block, float minRatio, 
                                     float maxRatio, int seed)
{
    std::mt19937 rng(seed);
    std::uniform_real_distribution<float> ratioDist(minRatio, maxRatio);
    
    // Limit max ratio by material properties
    float materialLimit = block.getMaterial().maxStretchRatio;
    maxRatio = std::min(maxRatio, materialLimit);
    
    // Apply random stretch in each dimension
    glm::vec3 currentSize = block.getSize();
    glm::vec3 stretchFactors(
        ratioDist(rng),
        ratioDist(rng),
        ratioDist(rng)
    );
    
    // Ensure at least one dimension doesn't exceed the material limit
    float maxFactor = std::max(std::max(stretchFactors.x, stretchFactors.y), stretchFactors.z);
    if (maxFactor > materialLimit) {
        float scale = materialLimit / maxFactor;
        stretchFactors *= scale;
    }
    
    glm::vec3 newSize = currentSize * stretchFactors;
    
    // Apply if valid
    if (isStretchValid(block, newSize)) {
        block.stretch(newSize);
    }
}

bool BlockBuilder::isStretchValid(const voxelship::VoxelBlock& block, 
                                 const glm::vec3& newSize) const
{
    return block.canStretch(newSize);
}

float BlockBuilder::calculateStretchRatio(const glm::vec3& originalSize, 
                                         const glm::vec3& newSize) const
{
    float maxRatio = 0.0f;
    
    if (originalSize.x > 0.001f) {
        maxRatio = std::max(maxRatio, newSize.x / originalSize.x);
    }
    if (originalSize.y > 0.001f) {
        maxRatio = std::max(maxRatio, newSize.y / originalSize.y);
    }
    if (originalSize.z > 0.001f) {
        maxRatio = std::max(maxRatio, newSize.z / originalSize.z);
    }
    
    return maxRatio;
}

int BlockBuilder::findClosestCorner(const voxelship::VoxelBlock& block, 
                                   const glm::vec3& point) const
{
    int closestCorner = 0;
    float minDistance = std::numeric_limits<float>::max();
    
    // Check all 8 corners
    for (int i = 0; i < 8; ++i) {
        glm::vec3 corner = block.getCorner(i);
        float distance = glm::distance(corner, point);
        
        if (distance < minDistance) {
            minDistance = distance;
            closestCorner = i;
        }
    }
    
    return closestCorner;
}

glm::vec3 BlockBuilder::snapToGridPos(const glm::vec3& pos) const
{
    return glm::vec3(
        std::round(pos.x / gridSize) * gridSize,
        std::round(pos.y / gridSize) * gridSize,
        std::round(pos.z / gridSize) * gridSize
    );
}

bool BlockBuilder::rayIntersectBlock(const voxelship::VoxelBlock& block,
                                    const glm::vec3& rayOrigin,
                                    const glm::vec3& rayDirection,
                                    glm::vec3& intersectionPoint) const
{
    glm::vec3 min, max;
    block.getBounds(min, max);
    
    // Ray-AABB intersection test
    glm::vec3 invDir = 1.0f / rayDirection;
    glm::vec3 t0 = (min - rayOrigin) * invDir;
    glm::vec3 t1 = (max - rayOrigin) * invDir;
    
    glm::vec3 tMin = glm::min(t0, t1);
    glm::vec3 tMax = glm::max(t0, t1);
    
    float tEnter = std::max(std::max(tMin.x, tMin.y), tMin.z);
    float tExit = std::min(std::min(tMax.x, tMax.y), tMax.z);
    
    if (tEnter > tExit || tExit < 0.0f) {
        return false; // No intersection
    }
    
    // Calculate intersection point
    float t = (tEnter >= 0.0f) ? tEnter : tExit;
    intersectionPoint = rayOrigin + rayDirection * t;
    
    return true;
}

} // namespace editor
} // namespace fresh
