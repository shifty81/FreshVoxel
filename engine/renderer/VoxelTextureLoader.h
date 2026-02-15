#pragma once

#include <string>
#include <unordered_map>

#include "renderer/TextureManager.h"
#include "voxel/VoxelTypes.h"

namespace fresh
{

/**
 * @brief Helper class for loading and managing voxel block textures
 *
 * This class provides a convenient interface for loading the placeholder
 * textures generated for each voxel type. It handles texture resolution
 * selection and face-specific textures (top, bottom, sides).
 */
class VoxelTextureLoader
{
public:
    /**
     * @brief Texture resolution options
     */
    enum class TextureResolution {
        Low = 16,    // 16x16
        Medium = 32, // 32x32
        High = 64    // 64x64
    };

    /**
     * @brief Face-specific texture identifiers
     */
    enum class BlockFace { Top, Bottom, North, South, East, West };

    /**
     * @brief Initialize the texture loader with specified resolution
     * @param resolution Texture resolution to use
     * @param basePath Base path to texture directory
     */
    VoxelTextureLoader(TextureResolution resolution = TextureResolution::Low,
                       const std::string& basePath = "textures/blocks");

    /**
     * @brief Load all voxel textures
     * @return true if all textures loaded successfully
     */
    bool loadAllTextures();

    /**
     * @brief Get texture for a specific voxel type and face
     * @param type Voxel type
     * @param face Block face (default: Top for single texture blocks)
     * @param useMaterialPacks If true, check material packs first (default: true)
     * @return Shared pointer to texture or nullptr if not found
     */
    std::shared_ptr<Texture> getTexture(VoxelType type, BlockFace face = BlockFace::Top, 
                                       bool useMaterialPacks = true);

    /**
     * @brief Check if a voxel type has face-specific textures
     * @param type Voxel type
     * @return true if the block has different textures per face
     */
    bool hasFaceSpecificTextures(VoxelType type) const;

    /**
     * @brief Get the current texture resolution
     */
    TextureResolution getResolution() const
    {
        return currentResolution;
    }

    /**
     * @brief Set texture resolution (requires reloading)
     * @param resolution New texture resolution
     */
    void setResolution(TextureResolution resolution);

private:
    std::string getTexturePath(const std::string& textureName) const;
    std::string getResolutionSuffix() const;

    TextureResolution currentResolution;
    std::string textureBasePath;

    // Texture cache: VoxelType -> Face -> Texture
    std::unordered_map<VoxelType, std::unordered_map<BlockFace, std::shared_ptr<Texture>>>
        textureCache;
};

/**
 * @brief Texture paths for each voxel type
 *
 * This provides the mapping between voxel types and their texture files.
 * For blocks with face-specific textures (like grass), different textures
 * are used for top, bottom, and sides.
 */
namespace VoxelTexturePaths
{
// Simple blocks (same texture all faces)
inline const char* STONE = "stone";
inline const char* DIRT = "dirt";
inline const char* SAND = "sand";
inline const char* WATER = "water";
inline const char* WOOD = "wood";
inline const char* LEAVES = "leaves";
inline const char* COBBLESTONE = "cobblestone";
inline const char* PLANKS = "planks";

// Grass block (face-specific)
inline const char* GRASS_TOP = "grass_top";
inline const char* GRASS_SIDE = "grass_side";

// Default fallback
inline const char* DEFAULT = "default";
} // namespace VoxelTexturePaths

} // namespace fresh

// Hash function for BlockFace enum to use in unordered_map
namespace std
{
template <>
struct hash<fresh::VoxelTextureLoader::BlockFace> {
    std::size_t operator()(const fresh::VoxelTextureLoader::BlockFace& face) const
    {
        return std::hash<int>{}(static_cast<int>(face));
    }
};
} // namespace std
