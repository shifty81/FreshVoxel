#pragma once

// ===========================================================================
// VoxExporter — Write engine voxel world regions back to .vox format
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
 * @brief Writes a rectangular region of the VoxelWorld to a .vox file.
 *
 * The exported file is compatible with MagicaVoxel 0.99.7+ (version 150).
 */
class VoxExporter
{
public:
    VoxExporter()  = default;
    ~VoxExporter() = default;

    /**
     * Export a world region to disk as a .vox file.
     *
     * @param world      Source VoxelWorld.
     * @param minCorner  Minimum world-space corner of the region.
     * @param maxCorner  Maximum world-space corner of the region.
     * @param filePath   Output file path (will be overwritten if it exists).
     * @return true on success.
     */
    bool exportRegion(const VoxelWorld* world,
                      const WorldPos&   minCorner,
                      const WorldPos&   maxCorner,
                      const std::string& filePath) const;

private:
    /// Build the default MagicaVoxel palette entry for a VoxelType.
    static VoxPaletteColor voxelTypeToColor(VoxelType type);

    /// Write a 4-byte little-endian uint32 to a vector.
    static void writeU32(std::vector<uint8_t>& buf, uint32_t v);

    /// Write an ASCII chunk header (id + selfBytes + childBytes).
    static void writeChunkHeader(std::vector<uint8_t>& buf,
                                 const char id[4],
                                 uint32_t selfBytes,
                                 uint32_t childBytes);
};

} // namespace vox
} // namespace fresh
