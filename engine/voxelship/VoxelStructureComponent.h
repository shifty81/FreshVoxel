#pragma once
#include <vector>

#include <glm/glm.hpp>

#include "ecs/IComponent.h"
#include "VoxelBlock.h"

namespace fresh
{
namespace voxelship
{

/**
 * @brief Component containing voxel structure data for entities
 *
 * This component stores the voxel blocks that make up a ship or station,
 * and provides methods for calculating mass properties and collision detection.
 */
class VoxelStructureComponent : public ecs::IComponent
{
public:
    VoxelStructureComponent();
    virtual ~VoxelStructureComponent() = default;

    COMPONENT_TYPE(VoxelStructureComponent)

    // Block management
    void addBlock(const VoxelBlock& block);
    void removeBlock(size_t index);
    void clearBlocks();

    const std::vector<VoxelBlock>& getBlocks() const
    {
        return blocks;
    }
    size_t getBlockCount() const
    {
        return blocks.size();
    }
    const VoxelBlock& getBlock(size_t index) const
    {
        return blocks[index];
    }

    // Mass properties
    float getTotalMass() const;
    glm::vec3 getCenterOfMass() const;
    glm::mat3 getInertiaTensor() const;

    // Collision detection
    bool checkCollision(const VoxelStructureComponent& other, const glm::vec3& thisPos,
                        const glm::vec3& otherPos) const;
    bool checkCollision(const VoxelBlock& block, const glm::vec3& offset) const;

    // Bounding box
    void getBounds(glm::vec3& min, glm::vec3& max) const;

    // Update cached values (call after modifying blocks)
    void updateCachedValues();

private:
    std::vector<VoxelBlock> blocks;

    // Cached values for performance
    mutable bool cacheValid;
    mutable float cachedTotalMass;
    mutable glm::vec3 cachedCenterOfMass;

    void invalidateCache();
};

} // namespace voxelship
} // namespace fresh
