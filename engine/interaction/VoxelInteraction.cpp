#include "interaction/VoxelInteraction.h"

#include "core/Logger.h"
#include "gameplay/Camera.h"
#include "voxel/VoxelTypes.h"
#include "voxel/VoxelWorld.h"

namespace fresh
{

VoxelInteraction::VoxelInteraction() : world(nullptr) {}

void VoxelInteraction::initialize(VoxelWorld* w)
{
    world = w;
}

void VoxelInteraction::update(float deltaTime)
{
    // Update logic if needed
    (void)deltaTime;
}

RayHit VoxelInteraction::performRaycast(const Camera& camera, float maxDistance)
{
    if (!world) {
        return RayHit{};
    }

    // Create ray from camera position and direction
    Ray ray(camera.getPosition(), camera.getFront());

    // Perform DDA raycast against voxel world
    currentTarget = CollisionDetection::raycastVoxel(ray, world, maxDistance);

    return currentTarget;
}

bool VoxelInteraction::placeBlock(const RayHit& hit, VoxelType blockType)
{
    if (!world || !hit.hit) {
        return false;
    }

    // Place block on the face that was hit (outside the hit block)
    int placeX = hit.blockX + static_cast<int>(hit.normal.x);
    int placeY = hit.blockY + static_cast<int>(hit.normal.y);
    int placeZ = hit.blockZ + static_cast<int>(hit.normal.z);

    // Check if target position is valid
    WorldPos pos(placeX, placeY, placeZ);
    Voxel* existingVoxel = world->getVoxel(pos);

    if (!existingVoxel || existingVoxel->type != VoxelType::Air) {
        // Position is occupied or invalid
        return false;
    }

    // Place the block
    world->setVoxel(pos, Voxel{blockType});

    LOG_INFO_C("Block placed at (" + std::to_string(placeX) + ", " + std::to_string(placeY) + ", " +
                   std::to_string(placeZ) + ")",
               "VoxelInteraction");

    return true;
}

bool VoxelInteraction::breakBlock(const RayHit& hit)
{
    if (!world || !hit.hit) {
        return false;
    }

    // Get the block at hit position
    WorldPos pos(hit.blockX, hit.blockY, hit.blockZ);
    Voxel* voxel = world->getVoxel(pos);

    if (!voxel || voxel->type == VoxelType::Air) {
        // Nothing to break
        return false;
    }

    // Don't allow breaking bedrock
    if (voxel->type == VoxelType::Bedrock) {
        LOG_INFO_C("Cannot break bedrock", "VoxelInteraction");
        return false;
    }

    // Break the block (set to air)
    world->setVoxel(pos, Voxel{VoxelType::Air});

    LOG_INFO_C("Block broken at (" + std::to_string(hit.blockX) + ", " +
                   std::to_string(hit.blockY) + ", " + std::to_string(hit.blockZ) + ")",
               "VoxelInteraction");

    return true;
}

} // namespace fresh
