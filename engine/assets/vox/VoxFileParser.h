#pragma once

// ===========================================================================
// MagicaVoxel .vox File Format Parser
//
// Supports:
//   - Version 150 (current MagicaVoxel release) and version 200 (nTRN/nGRP/nSHP)
//   - Chunks: MAIN, SIZE, XYZI, RGBA, nTRN, nGRP, nSHP
//   - Multi-model .vox scenes (multiple SIZE/XYZI pairs under nSHP nodes)
//
// Reference: https://github.com/ephtracy/voxel-model/blob/master/MagicaVoxel-file-format-vox.txt
// ===========================================================================

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace fresh
{
namespace vox
{

/// RGBA colour entry in the MagicaVoxel 256-colour palette
struct VoxPaletteColor
{
    uint8_t r, g, b, a;
};

/// A single voxel: position within the model + palette index (1-based)
struct VoxVoxel
{
    uint8_t x, y, z;
    uint8_t colorIndex; ///< 1–255; 0 is reserved/empty in the .vox format
};

/// One model within the .vox file (SIZE + XYZI pair)
struct VoxModel
{
    uint32_t sizeX = 0;
    uint32_t sizeY = 0;
    uint32_t sizeZ = 0;
    std::vector<VoxVoxel> voxels;
};

/// Full parsed .vox file
struct VoxFile
{
    uint32_t version = 0;
    std::vector<VoxModel> models;
    std::array<VoxPaletteColor, 256> palette{}; ///< Index 0 unused; indices 1-255 are valid
    bool isValid = false;
};

/// ===========================================================================
/// Parser
/// ===========================================================================
class VoxFileParser
{
public:
    VoxFileParser()  = default;
    ~VoxFileParser() = default;

    /**
     * Parse a .vox file from disk.
     * @param filePath  Absolute or relative path to the .vox file.
     * @return Parsed VoxFile.  Check isValid before using.
     */
    [[nodiscard]] VoxFile parse(const std::string& filePath) const;

    /**
     * Parse a .vox file from an in-memory buffer (e.g. resource pack).
     * @param data   Pointer to raw .vox bytes.
     * @param size   Size of buffer in bytes.
     * @return Parsed VoxFile.  Check isValid before using.
     */
    [[nodiscard]] VoxFile parseFromMemory(const uint8_t* data, size_t size) const;

private:
    struct ParseContext
    {
        const uint8_t* data = nullptr;
        size_t         size = 0;
        size_t         pos  = 0;

        bool canRead(size_t bytes) const noexcept { return pos + bytes <= size; }

        template <typename T>
        T read()
        {
            T value{};
            // NOLINTNEXTLINE(bugprone-sizeof-expression)
            if (canRead(sizeof(T))) {
                __builtin_memcpy(&value, data + pos, sizeof(T));
                pos += sizeof(T);
            }
            return value;
        }

        void skip(size_t bytes) noexcept { pos += bytes; }
    };

    static void parseDefaultPalette(VoxFile& vf);
    static void parseChunk(ParseContext& ctx, VoxFile& vf);
    static void parseMain(ParseContext& ctx, VoxFile& vf);
    static void parseSize(ParseContext& ctx, VoxFile& vf);
    static void parseXYZI(ParseContext& ctx, VoxFile& vf);
    static void parseRGBA(ParseContext& ctx, VoxFile& vf);

    // Version 200 scene graph (optional – read but data unused for basic import)
    static void parseNTRN(ParseContext& ctx);
    static void parseNGRP(ParseContext& ctx);
    static void parseNSHP(ParseContext& ctx);
};

} // namespace vox
} // namespace fresh
