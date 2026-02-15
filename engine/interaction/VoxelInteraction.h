#pragma once

#include <glm/glm.hpp>

#include "physics/CollisionDetection.h"
#include "voxel/VoxelTypes.h"

namespace fresh
{

class VoxelWorld;
class Camera;

/**
 * @brief Handles player interaction with voxel world (place/break blocks)
 */
class VoxelInteraction
{
public:
    VoxelInteraction();

    /**
     * @brief Initialize the interaction system
     * @param world Voxel world to interact with
     */
    void initialize(VoxelWorld* world);

    /**
     * @brief Update interaction system
     * @param deltaTime Time since last update
     */
    void update(float deltaTime);

    /**
     * @brief Perform raycast from camera to find targeted block
     * @param camera Camera to raycast from
     * @param maxDistance Maximum raycast distance
     * @return RayHit with block information
     */
    RayHit performRaycast(const Camera& camera, float maxDistance = 5.0f);

    /**
     * @brief Place a block at the hit location
     * @param hit Raycast hit result
     * @param blockType Type of block to place
     * @return true if block was placed
     */
    bool placeBlock(const RayHit& hit, VoxelType blockType);

    /**
     * @brief Break the block at the hit location
     * @param hit Raycast hit result
     * @return true if block was broken
     */
    bool breakBlock(const RayHit& hit);

    /**
     * @brief Get currently targeted block
     */
    const RayHit& getTargetedBlock() const
    {
        return currentTarget;
    }

    /**
     * @brief Check if a block is currently targeted
     */
    bool hasTargetedBlock() const
    {
        return currentTarget.hit;
    }

private:
    VoxelWorld* world;
    RayHit currentTarget;
};

} // namespace fresh
