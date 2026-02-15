#include "voxelship/VoxelBlock.h"

#include <algorithm>

namespace fresh
{
namespace voxelship
{

VoxelBlock::VoxelBlock() : position(0.0f), size(1.0f), material(), originalSize(1.0f) {}

VoxelBlock::VoxelBlock(const glm::vec3& pos, const glm::vec3& sz, const VoxelMaterial& mat)
    : position(pos), size(sz), material(mat), originalSize(sz)
{
}

bool VoxelBlock::intersects(const VoxelBlock& other) const
{
    glm::vec3 thisMin, thisMax, otherMin, otherMax;
    getBounds(thisMin, thisMax);
    other.getBounds(otherMin, otherMax);

    return (thisMin.x <= otherMax.x && thisMax.x >= otherMin.x) &&
           (thisMin.y <= otherMax.y && thisMax.y >= otherMin.y) &&
           (thisMin.z <= otherMax.z && thisMax.z >= otherMin.z);
}

bool VoxelBlock::contains(const glm::vec3& point) const
{
    glm::vec3 min, max;
    getBounds(min, max);

    return (point.x >= min.x && point.x <= max.x) && (point.y >= min.y && point.y <= max.y) &&
           (point.z >= min.z && point.z <= max.z);
}

void VoxelBlock::getBounds(glm::vec3& min, glm::vec3& max) const
{
    glm::vec3 halfSize = size * 0.5f;
    min = position - halfSize;
    max = position + halfSize;
}

bool VoxelBlock::canStretch(const glm::vec3& newSize) const
{
    // Check if the new size respects structural integrity limits
    float stretchRatio = calculateStretchRatio(newSize);
    return stretchRatio <= material.maxStretchRatio;
}

bool VoxelBlock::stretch(const glm::vec3& newSize)
{
    if (!canStretch(newSize)) {
        return false; // Cannot stretch beyond structural limits
    }
    
    size = newSize;
    return true;
}

bool VoxelBlock::stretchCorner(int cornerIndex, const glm::vec3& delta)
{
    // Get the current corner position
    glm::vec3 corner = getCorner(cornerIndex);
    
    // Apply delta to corner
    corner += delta;
    
    // Calculate opposite corner
    int oppositeIndex = 7 - cornerIndex; // Bitwise opposite corner
    glm::vec3 oppositeCorner = getCorner(oppositeIndex);
    
    // Calculate new center and size
    glm::vec3 newCenter = (corner + oppositeCorner) * 0.5f;
    glm::vec3 newSize = glm::abs(corner - oppositeCorner);
    
    // Check structural integrity
    if (!canStretch(newSize)) {
        return false;
    }
    
    // Apply changes
    position = newCenter;
    size = newSize;
    return true;
}

glm::vec3 VoxelBlock::getCorner(int cornerIndex) const
{
    glm::vec3 min, max;
    getBounds(min, max);
    
    // 8 corners of a box (binary encoding: bit0=x, bit1=y, bit2=z)
    return glm::vec3(
        (cornerIndex & 1) ? max.x : min.x,
        (cornerIndex & 2) ? max.y : min.y,
        (cornerIndex & 4) ? max.z : min.z
    );
}

float VoxelBlock::getStretchRatio() const
{
    return calculateStretchRatio(size);
}

bool VoxelBlock::isStructurallySound() const
{
    return getStretchRatio() <= material.maxStretchRatio;
}

float VoxelBlock::calculateStretchRatio(const glm::vec3& testSize) const
{
    // Calculate the maximum stretch ratio across all dimensions
    float maxRatio = 0.0f;
    
    if (originalSize.x > 0.001f) {
        maxRatio = std::max(maxRatio, testSize.x / originalSize.x);
    }
    if (originalSize.y > 0.001f) {
        maxRatio = std::max(maxRatio, testSize.y / originalSize.y);
    }
    if (originalSize.z > 0.001f) {
        maxRatio = std::max(maxRatio, testSize.z / originalSize.z);
    }
    
    return maxRatio;
}

} // namespace voxelship
} // namespace fresh
