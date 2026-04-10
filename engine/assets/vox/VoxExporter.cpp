#include "assets/vox/VoxExporter.h"
#include "core/Logger.h"
#include "voxel/VoxelWorld.h"

#include <algorithm>
#include <fstream>

namespace fresh
{
namespace vox
{

// ---------------------------------------------------------------------------
// VoxelType → palette colour
// ---------------------------------------------------------------------------
VoxPaletteColor VoxExporter::voxelTypeToColor(VoxelType type)
{
    switch (type) {
    case VoxelType::Grass:       return {0x33, 0xCC, 0x33, 0xFF};
    case VoxelType::Dirt:        return {0x99, 0x66, 0x33, 0xFF};
    case VoxelType::Stone:       return {0x80, 0x80, 0x80, 0xFF};
    case VoxelType::Sand:        return {0xCC, 0xCC, 0x66, 0xFF};
    case VoxelType::SandStone:   return {0xCC, 0xAA, 0x55, 0xFF};
    case VoxelType::Water:       return {0x33, 0x66, 0xFF, 0xA0};
    case VoxelType::Ice:         return {0xAA, 0xCC, 0xFF, 0xC0};
    case VoxelType::Glass:       return {0xCC, 0xEE, 0xFF, 0x80};
    case VoxelType::Wood:        return {0x66, 0x44, 0x22, 0xFF};
    case VoxelType::Leaves:      return {0x22, 0x88, 0x22, 0xD0};
    case VoxelType::Snow:        return {0xEE, 0xEE, 0xFF, 0xFF};
    case VoxelType::Bedrock:     return {0x22, 0x22, 0x22, 0xFF};
    case VoxelType::Brick:       return {0xAA, 0x44, 0x33, 0xFF};
    case VoxelType::Obsidian:    return {0x11, 0x00, 0x22, 0xFF};
    case VoxelType::Coal:        return {0x22, 0x22, 0x22, 0xFF};
    case VoxelType::Iron:        return {0xBB, 0xBB, 0xBB, 0xFF};
    case VoxelType::Gold:        return {0xFF, 0xCC, 0x00, 0xFF};
    case VoxelType::Diamond:     return {0x33, 0xFF, 0xFF, 0xFF};
    case VoxelType::RedSand:     return {0xCC, 0x55, 0x22, 0xFF};
    case VoxelType::Gravel:      return {0x99, 0x88, 0x77, 0xFF};
    default:                     return {0x88, 0x88, 0x88, 0xFF};
    }
}

// ---------------------------------------------------------------------------
// Binary write helpers
// ---------------------------------------------------------------------------
void VoxExporter::writeU32(std::vector<uint8_t>& buf, uint32_t v)
{
    buf.push_back(static_cast<uint8_t>(v        & 0xFF));
    buf.push_back(static_cast<uint8_t>((v >>  8) & 0xFF));
    buf.push_back(static_cast<uint8_t>((v >> 16) & 0xFF));
    buf.push_back(static_cast<uint8_t>((v >> 24) & 0xFF));
}

void VoxExporter::writeChunkHeader(std::vector<uint8_t>& buf,
                                   const char id[4],
                                   uint32_t selfBytes,
                                   uint32_t childBytes)
{
    buf.push_back(static_cast<uint8_t>(id[0]));
    buf.push_back(static_cast<uint8_t>(id[1]));
    buf.push_back(static_cast<uint8_t>(id[2]));
    buf.push_back(static_cast<uint8_t>(id[3]));
    writeU32(buf, selfBytes);
    writeU32(buf, childBytes);
}

// ---------------------------------------------------------------------------
// exportRegion
// ---------------------------------------------------------------------------
bool VoxExporter::exportRegion(const VoxelWorld* world,
                               const WorldPos&   minCorner,
                               const WorldPos&   maxCorner,
                               const std::string& filePath) const
{
    if (!world) {
        Logger::getInstance().error("VoxExporter: null world pointer", "Vox");
        return false;
    }

    const int sizeX = maxCorner.x - minCorner.x + 1;
    const int sizeY = maxCorner.y - minCorner.y + 1;
    const int sizeZ = maxCorner.z - minCorner.z + 1;

    if (sizeX <= 0 || sizeY <= 0 || sizeZ <= 0 ||
        sizeX > 256 || sizeY > 256 || sizeZ > 256)
    {
        Logger::getInstance().error(
            "VoxExporter: region dimensions out of .vox bounds (max 256³)", "Vox");
        return false;
    }

    // --- Collect voxels and build a compact 256-entry colour palette ---
    struct VoxEntry { uint8_t x, y, z, idx; };
    std::vector<VoxEntry> voxels;
    voxels.reserve(static_cast<size_t>(sizeX * sizeY * sizeZ) / 4);

    // palette[0] is unused in .vox; we fill indices 1–255 on demand
    std::array<VoxPaletteColor, 256> palette{};
    // Sentinel: unoccupied palette slot has alpha == 0
    int nextPaletteIdx = 1;

    // Map from VoxelType enum to palette index (0 = not yet assigned)
    constexpr int kTypeCount = static_cast<int>(VoxelType::Count);
    int typeToIdx[kTypeCount]{};

    for (int y = 0; y < sizeY && nextPaletteIdx <= 255; ++y) {
        for (int z = 0; z < sizeZ; ++z) {
            for (int x = 0; x < sizeX; ++x) {
                WorldPos wp(minCorner.x + x,
                            minCorner.y + y,
                            minCorner.z + z);

                const Voxel* vp =
                    const_cast<VoxelWorld*>(world)->getVoxel(wp);
                if (!vp || vp->type == VoxelType::Air) continue;

                const int ti = static_cast<int>(vp->type);
                if (ti < 0 || ti >= kTypeCount) continue;

                if (typeToIdx[ti] == 0) {
                    if (nextPaletteIdx > 255) continue;
                    typeToIdx[ti]          = nextPaletteIdx;
                    palette[nextPaletteIdx] = voxelTypeToColor(vp->type);
                    ++nextPaletteIdx;
                }

                // .vox uses Y as height; engine uses Y as height too,
                // but vox Z-axis = engine Z-axis.
                VoxEntry e;
                e.x   = static_cast<uint8_t>(x);
                // Axis mapping: MagicaVoxel uses Y-up with Z as forward depth.
                // The engine also uses Y-up but with Z as horizontal depth.
                // To preserve correct orientation on round-trip import/export:
                //   engine Y  →  .vox Z  (height axis stays the same)
                //   engine Z  →  .vox Y  (horizontal depth ↔ vox depth)
                e.z   = static_cast<uint8_t>(y); // vox Z = engine Y (height)
                e.y   = static_cast<uint8_t>(z); // vox Y = engine Z (depth)
                e.idx = static_cast<uint8_t>(typeToIdx[ti]);
                voxels.push_back(e);
            }
        }
    }

    // --- Serialise to .vox ---
    std::vector<uint8_t> buf;
    buf.reserve(64 + voxels.size() * 4 + 256 * 4);

    // File header: magic + version
    const uint8_t magic[4] = {'V','O','X',' '};
    buf.insert(buf.end(), magic, magic + 4);
    writeU32(buf, 150); // version

    // Build SIZE chunk (12 bytes self)
    std::vector<uint8_t> sizeChunk;
    writeU32(sizeChunk, static_cast<uint32_t>(sizeX));
    writeU32(sizeChunk, static_cast<uint32_t>(sizeZ)); // vox sizeY = engine sizeZ
    writeU32(sizeChunk, static_cast<uint32_t>(sizeY)); // vox sizeZ = engine sizeY

    // Build XYZI chunk
    std::vector<uint8_t> xyziChunk;
    writeU32(xyziChunk, static_cast<uint32_t>(voxels.size()));
    for (const auto& v : voxels) {
        xyziChunk.push_back(v.x);
        xyziChunk.push_back(v.y);
        xyziChunk.push_back(v.z);
        xyziChunk.push_back(v.idx);
    }

    // Build RGBA chunk (256 entries × 4 bytes)
    std::vector<uint8_t> rgbaChunk;
    for (int i = 1; i <= 255; ++i) {
        rgbaChunk.push_back(palette[i].r);
        rgbaChunk.push_back(palette[i].g);
        rgbaChunk.push_back(palette[i].b);
        rgbaChunk.push_back(palette[i].a);
    }
    // Entry 255 (unused) — pad to 256 entries
    rgbaChunk.push_back(0); rgbaChunk.push_back(0);
    rgbaChunk.push_back(0); rgbaChunk.push_back(0);

    // Total children byte count
    const uint32_t childrenBytes =
        12 + static_cast<uint32_t>(sizeChunk.size()) +
        12 + static_cast<uint32_t>(xyziChunk.size()) +
        12 + static_cast<uint32_t>(rgbaChunk.size());

    // Write MAIN chunk
    writeChunkHeader(buf, "MAIN", 0, childrenBytes);

    writeChunkHeader(buf, "SIZE",
                     static_cast<uint32_t>(sizeChunk.size()), 0);
    buf.insert(buf.end(), sizeChunk.begin(), sizeChunk.end());

    writeChunkHeader(buf, "XYZI",
                     static_cast<uint32_t>(xyziChunk.size()), 0);
    buf.insert(buf.end(), xyziChunk.begin(), xyziChunk.end());

    writeChunkHeader(buf, "RGBA",
                     static_cast<uint32_t>(rgbaChunk.size()), 0);
    buf.insert(buf.end(), rgbaChunk.begin(), rgbaChunk.end());

    // --- Write to file ---
    std::ofstream out(filePath, std::ios::binary | std::ios::trunc);
    if (!out.is_open()) {
        Logger::getInstance().error(
            "VoxExporter: cannot open output file '" + filePath + "'", "Vox");
        return false;
    }

    out.write(reinterpret_cast<const char*>(buf.data()),
              static_cast<std::streamsize>(buf.size()));
    if (!out) {
        Logger::getInstance().error(
            "VoxExporter: write error for '" + filePath + "'", "Vox");
        return false;
    }

    Logger::getInstance().info(
        "VoxExporter: exported " + std::to_string(voxels.size()) +
        " voxels to '" + filePath + "'", "Vox");
    return true;
}

} // namespace vox
} // namespace fresh
