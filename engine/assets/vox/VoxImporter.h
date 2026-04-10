#pragma once

// ===========================================================================
// VoxImporter — Converts a parsed VoxFile into engine voxel data
//
// Two placement modes:
//   1. importIntoWorld()   — places voxels directly into a VoxelWorld region
//   2. importAsStructure() — creates a VoxelStructureComponent (entity-based)
// ===========================================================================

#include "assets/vox/VoxFileParser.h"
#include "voxel/VoxelTypes.h"

#include <string>

namespace fresh
{

class VoxelWorld;

namespace vox
{

/**
 * @brief Converts parsed .vox data into engine voxel data.
 */
class VoxImporter
{
public:
    VoxImporter()  = default;
    ~VoxImporter() = default;

    /**
     * Load a .vox file and place its voxels directly into the world.
     *
     * @param filePath  Path to the .vox file.
     * @param world     Target VoxelWorld.
     * @param origin    World-space position of the (0,0,0) voxel in the model.
     * @param modelIdx  Which model to import (0 = first; -1 = merge all).
     * @return true on success.
     */
    bool importIntoWorld(const std::string& filePath,
                         VoxelWorld*        world,
                         const WorldPos&    origin,
                         int                modelIdx = 0) const;

    /**
     * Import from an already-parsed VoxFile.
     */
    bool importIntoWorld(const VoxFile&   voxFile,
                         VoxelWorld*      world,
                         const WorldPos&  origin,
                         int              modelIdx = 0) const;

private:
    /// Map a 1-based palette index to a VoxelType understood by the engine.
    static VoxelType paletteToVoxelType(const VoxPaletteColor& color);
};

} // namespace vox
} // namespace fresh
