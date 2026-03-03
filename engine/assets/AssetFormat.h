#pragma once
#include <cstdint>

namespace fresh::asset {

constexpr uint32_t ASSET_MAGIC = 0x46525348; // "FRSH"

enum class AssetType : uint16_t {
    Graph,
    World,
    Mesh,
    Material,
    VoxelSchema,
    EditorProfile,
};

struct AssetHeader {
    uint32_t magic = ASSET_MAGIC;
    uint16_t version = 1;
    AssetType type = AssetType::Graph;
    uint32_t size = 0;
    uint64_t hash = 0;
};

} // namespace fresh::asset
