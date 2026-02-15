#pragma once

#include "voxel/VoxelTypes.h"

namespace fresh
{

/**
 * @brief Raycasting system for block interaction
 */
class RaycastSystem
{
public:
    RaycastSystem();
    ~RaycastSystem();

    /**
     * @brief Initialize raycasting system
     * @return true if successful
     */
    bool initialize();
};

} // namespace fresh
