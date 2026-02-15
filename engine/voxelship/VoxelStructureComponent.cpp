#include "voxelship/VoxelStructureComponent.h"

#include <limits>

namespace fresh
{
namespace voxelship
{

VoxelStructureComponent::VoxelStructureComponent()
    : cacheValid(false), cachedTotalMass(0.0f), cachedCenterOfMass(0.0f)
{
}

void VoxelStructureComponent::addBlock(const VoxelBlock& block)
{
    blocks.push_back(block);
    invalidateCache();
}

void VoxelStructureComponent::removeBlock(size_t index)
{
    if (index < blocks.size()) {
        blocks.erase(blocks.begin() + index);
        invalidateCache();
    }
}

void VoxelStructureComponent::clearBlocks()
{
    blocks.clear();
    invalidateCache();
}

float VoxelStructureComponent::getTotalMass() const
{
    if (cacheValid) {
        return cachedTotalMass;
    }

    float totalMass = 0.0f;
    for (const auto& block : blocks) {
        totalMass += block.getMass();
    }

    cachedTotalMass = totalMass;
    return totalMass;
}

glm::vec3 VoxelStructureComponent::getCenterOfMass() const
{
    if (cacheValid) {
        return cachedCenterOfMass;
    }

    if (blocks.empty()) {
        cachedCenterOfMass = glm::vec3(0.0f);
        return cachedCenterOfMass;
    }

    glm::vec3 weightedSum(0.0f);
    float totalMass = 0.0f;

    for (const auto& block : blocks) {
        float mass = block.getMass();
        weightedSum += block.getPosition() * mass;
        totalMass += mass;
    }

    if (totalMass > 0.0f) {
        cachedCenterOfMass = weightedSum / totalMass;
    } else {
        cachedCenterOfMass = glm::vec3(0.0f);
    }

    return cachedCenterOfMass;
}

glm::mat3 VoxelStructureComponent::getInertiaTensor() const
{
    glm::mat3 inertia(0.0f);
    glm::vec3 com = getCenterOfMass();

    for (const auto& block : blocks) {
        float mass = block.getMass();
        glm::vec3 r = block.getPosition() - com;
        const glm::vec3& size = block.getSize();

        // Parallel axis theorem for a box
        float Ixx =
            mass * (size.y * size.y + size.z * size.z) / 12.0f + mass * (r.y * r.y + r.z * r.z);
        float Iyy =
            mass * (size.x * size.x + size.z * size.z) / 12.0f + mass * (r.x * r.x + r.z * r.z);
        float Izz =
            mass * (size.x * size.x + size.y * size.y) / 12.0f + mass * (r.x * r.x + r.y * r.y);
        float Ixy = -mass * r.x * r.y;
        float Ixz = -mass * r.x * r.z;
        float Iyz = -mass * r.y * r.z;

        inertia[0][0] += Ixx;
        inertia[1][1] += Iyy;
        inertia[2][2] += Izz;
        inertia[0][1] += Ixy;
        inertia[1][0] += Ixy;
        inertia[0][2] += Ixz;
        inertia[2][0] += Ixz;
        inertia[1][2] += Iyz;
        inertia[2][1] += Iyz;
    }

    return inertia;
}

bool VoxelStructureComponent::checkCollision(const VoxelStructureComponent& other,
                                             const glm::vec3& thisPos,
                                             const glm::vec3& otherPos) const
{
    glm::vec3 offset = otherPos - thisPos;

    // TODO: Optimize with spatial indexing for large structures
    for (const auto& thisBlock : blocks) {
        for (const auto& otherBlock : other.blocks) {
            // Check collision with offset position (avoid copying VoxelBlock)
            glm::vec3 offsetPos = otherBlock.getPosition() + offset;
            glm::vec3 otherMin, otherMax;
            otherMin = offsetPos - otherBlock.getSize() * 0.5f;
            otherMax = offsetPos + otherBlock.getSize() * 0.5f;

            glm::vec3 thisMin, thisMax;
            thisBlock.getBounds(thisMin, thisMax);

            // AABB intersection test
            if ((thisMin.x <= otherMax.x && thisMax.x >= otherMin.x) &&
                (thisMin.y <= otherMax.y && thisMax.y >= otherMin.y) &&
                (thisMin.z <= otherMax.z && thisMax.z >= otherMin.z)) {
                return true;
            }
        }
    }

    return false;
}

bool VoxelStructureComponent::checkCollision(const VoxelBlock& block, const glm::vec3& offset) const
{
    VoxelBlock offsetBlock = block;
    offsetBlock.setPosition(block.getPosition() + offset);

    for (const auto& myBlock : blocks) {
        if (myBlock.intersects(offsetBlock)) {
            return true;
        }
    }

    return false;
}

void VoxelStructureComponent::getBounds(glm::vec3& min, glm::vec3& max) const
{
    if (blocks.empty()) {
        min = max = glm::vec3(0.0f);
        return;
    }

    min = glm::vec3(std::numeric_limits<float>::max());
    max = glm::vec3(std::numeric_limits<float>::lowest());

    for (const auto& block : blocks) {
        glm::vec3 blockMin, blockMax;
        block.getBounds(blockMin, blockMax);

        min = glm::min(min, blockMin);
        max = glm::max(max, blockMax);
    }
}

void VoxelStructureComponent::updateCachedValues()
{
    // These methods update the cached values as side effects
    getTotalMass();
    getCenterOfMass();
    cacheValid = true;
}

void VoxelStructureComponent::invalidateCache()
{
    cacheValid = false;
}

} // namespace voxelship
} // namespace fresh
