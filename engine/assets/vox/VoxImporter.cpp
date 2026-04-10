#include "assets/vox/VoxImporter.h"
#include "core/Logger.h"
#include "voxel/VoxelWorld.h"

namespace fresh
{
namespace vox
{

// ---------------------------------------------------------------------------
// Heuristic palette-colour → VoxelType mapping
// ---------------------------------------------------------------------------
VoxelType VoxImporter::paletteToVoxelType(const VoxPaletteColor& c)
{
    // Transparent or near-transparent → Glass / Water
    if (c.a < 64)  return VoxelType::Air;
    if (c.a < 200) return VoxelType::Glass;

    // Derive a simple hue approximation to map to a voxel type.
    // Full accuracy would require a per-project palette config; this
    // heuristic is good enough for preview/level-editing use cases.

    const int r = c.r, g = c.g, b = c.b;

    // Blue-dominant → Water / Ice
    if (b > r + 60 && b > g + 30) {
        if (b > 200 && r < 100) return VoxelType::Water;
        return VoxelType::Ice;
    }

    // Green-dominant → Grass / Leaves
    if (g > r + 40 && g > b + 30) {
        if (g > 160) return VoxelType::Grass;
        return VoxelType::Leaves;
    }

    // Sandy/orange → Sand / Sandstone
    if (r > 160 && g > 120 && b < 100) {
        if (r - b > 80) return VoxelType::Sand;
        return VoxelType::SandStone;
    }

    // Very dark → Obsidian / Coal
    if (r < 50 && g < 50 && b < 50) {
        return VoxelType::Obsidian;
    }

    // Brownish → Dirt / Wood
    if (r > g && r > b && g > b) {
        if (r > 160 && g > 100) return VoxelType::Dirt;
        return VoxelType::Wood;
    }

    // White / light grey → Snow / Stone
    if (r > 200 && g > 200 && b > 200) return VoxelType::Snow;
    if (r > 150 && g > 150 && b > 150) return VoxelType::Stone;

    // Reddish → Brick / RedSand
    if (r > 150 && g < 100 && b < 100) return VoxelType::Brick;

    // Default: stone
    return VoxelType::Stone;
}

// ---------------------------------------------------------------------------
// importIntoWorld — file path entry point
// ---------------------------------------------------------------------------
bool VoxImporter::importIntoWorld(const std::string& filePath,
                                  VoxelWorld*        world,
                                  const WorldPos&    origin,
                                  int                modelIdx) const
{
    if (!world) {
        Logger::getInstance().error("VoxImporter: null world pointer", "Vox");
        return false;
    }

    VoxFileParser parser;
    VoxFile vf = parser.parse(filePath);
    if (!vf.isValid) {
        Logger::getInstance().error(
            "VoxImporter: failed to parse '" + filePath + "'", "Vox");
        return false;
    }

    return importIntoWorld(vf, world, origin, modelIdx);
}

// ---------------------------------------------------------------------------
// importIntoWorld — VoxFile entry point
// ---------------------------------------------------------------------------
bool VoxImporter::importIntoWorld(const VoxFile&  voxFile,
                                  VoxelWorld*     world,
                                  const WorldPos& origin,
                                  int             modelIdx) const
{
    if (!world || !voxFile.isValid || voxFile.models.empty()) return false;

    // Determine which models to import
    const int first = (modelIdx < 0) ? 0 : modelIdx;
    const int last  = (modelIdx < 0) ? static_cast<int>(voxFile.models.size()) - 1
                                     : modelIdx;

    int voxelsPlaced = 0;

    for (int mi = first; mi <= last; ++mi) {
        if (mi < 0 || mi >= static_cast<int>(voxFile.models.size())) continue;
        const VoxModel& model = voxFile.models[static_cast<size_t>(mi)];

        for (const VoxVoxel& vv : model.voxels) {
            if (vv.colorIndex == 0) continue; // empty / reserved

            const VoxPaletteColor& color = voxFile.palette[vv.colorIndex];
            VoxelType type = paletteToVoxelType(color);
            if (type == VoxelType::Air) continue;

            WorldPos wp(
                origin.x + static_cast<int>(vv.x),
                origin.y + static_cast<int>(vv.z), // .vox is Y-up; engine is Y-up with Z=depth
                origin.z + static_cast<int>(vv.y)
            );

            world->setVoxel(wp, Voxel(type));
            ++voxelsPlaced;
        }
    }

    Logger::getInstance().info(
        "VoxImporter: placed " + std::to_string(voxelsPlaced) + " voxels", "Vox");
    return voxelsPlaced > 0;
}

} // namespace vox
} // namespace fresh
