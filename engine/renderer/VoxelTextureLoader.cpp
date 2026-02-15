#include "renderer/VoxelTextureLoader.h"

#include <sstream>

#include "core/Logger.h"

namespace fresh
{

VoxelTextureLoader::VoxelTextureLoader(TextureResolution resolution, const std::string& basePath)
    : currentResolution(resolution), textureBasePath(basePath)
{
}

std::string VoxelTextureLoader::getResolutionSuffix() const
{
    int size = static_cast<int>(currentResolution);
    std::stringstream ss;
    ss << "_" << size << "x" << size << ".png";
    return ss.str();
}

std::string VoxelTextureLoader::getTexturePath(const std::string& textureName) const
{
    return textureBasePath + "/" + textureName + getResolutionSuffix();
}

bool VoxelTextureLoader::loadAllTextures()
{
    auto& texManager = TextureManager::getInstance();
    bool allLoaded = true;

    // Load Stone
    {
        auto tex = texManager.loadTexture(getTexturePath(VoxelTexturePaths::STONE));
        if (tex) {
            textureCache[VoxelType::Stone][BlockFace::Top] = tex;
            textureCache[VoxelType::Stone][BlockFace::Bottom] = tex;
            textureCache[VoxelType::Stone][BlockFace::North] = tex;
            textureCache[VoxelType::Stone][BlockFace::South] = tex;
            textureCache[VoxelType::Stone][BlockFace::East] = tex;
            textureCache[VoxelType::Stone][BlockFace::West] = tex;
            LOG_INFO("Loaded stone texture");
        } else {
            LOG_ERROR("Failed to load stone texture");
            allLoaded = false;
        }
    }

    // Load Dirt
    {
        auto tex = texManager.loadTexture(getTexturePath(VoxelTexturePaths::DIRT));
        if (tex) {
            textureCache[VoxelType::Dirt][BlockFace::Top] = tex;
            textureCache[VoxelType::Dirt][BlockFace::Bottom] = tex;
            textureCache[VoxelType::Dirt][BlockFace::North] = tex;
            textureCache[VoxelType::Dirt][BlockFace::South] = tex;
            textureCache[VoxelType::Dirt][BlockFace::East] = tex;
            textureCache[VoxelType::Dirt][BlockFace::West] = tex;
            LOG_INFO("Loaded dirt texture");
        } else {
            LOG_ERROR("Failed to load dirt texture");
            allLoaded = false;
        }
    }

    // Load Grass (face-specific)
    {
        auto topTex = texManager.loadTexture(getTexturePath(VoxelTexturePaths::GRASS_TOP));
        auto sideTex = texManager.loadTexture(getTexturePath(VoxelTexturePaths::GRASS_SIDE));
        auto bottomTex = texManager.loadTexture(getTexturePath(VoxelTexturePaths::DIRT));

        if (topTex && sideTex && bottomTex) {
            textureCache[VoxelType::Grass][BlockFace::Top] = topTex;
            textureCache[VoxelType::Grass][BlockFace::Bottom] = bottomTex;
            textureCache[VoxelType::Grass][BlockFace::North] = sideTex;
            textureCache[VoxelType::Grass][BlockFace::South] = sideTex;
            textureCache[VoxelType::Grass][BlockFace::East] = sideTex;
            textureCache[VoxelType::Grass][BlockFace::West] = sideTex;
            LOG_INFO("Loaded grass textures");
        } else {
            LOG_ERROR("Failed to load grass textures");
            allLoaded = false;
        }
    }

    // Load Sand
    {
        auto tex = texManager.loadTexture(getTexturePath(VoxelTexturePaths::SAND));
        if (tex) {
            textureCache[VoxelType::Sand][BlockFace::Top] = tex;
            textureCache[VoxelType::Sand][BlockFace::Bottom] = tex;
            textureCache[VoxelType::Sand][BlockFace::North] = tex;
            textureCache[VoxelType::Sand][BlockFace::South] = tex;
            textureCache[VoxelType::Sand][BlockFace::East] = tex;
            textureCache[VoxelType::Sand][BlockFace::West] = tex;
            LOG_INFO("Loaded sand texture");
        } else {
            LOG_ERROR("Failed to load sand texture");
            allLoaded = false;
        }
    }

    // Load Water
    {
        auto tex = texManager.loadTexture(getTexturePath(VoxelTexturePaths::WATER));
        if (tex) {
            textureCache[VoxelType::Water][BlockFace::Top] = tex;
            textureCache[VoxelType::Water][BlockFace::Bottom] = tex;
            textureCache[VoxelType::Water][BlockFace::North] = tex;
            textureCache[VoxelType::Water][BlockFace::South] = tex;
            textureCache[VoxelType::Water][BlockFace::East] = tex;
            textureCache[VoxelType::Water][BlockFace::West] = tex;
            LOG_INFO("Loaded water texture");
        } else {
            LOG_ERROR("Failed to load water texture");
            allLoaded = false;
        }
    }

    // Load Wood
    {
        auto tex = texManager.loadTexture(getTexturePath(VoxelTexturePaths::WOOD));
        if (tex) {
            textureCache[VoxelType::Wood][BlockFace::Top] = tex;
            textureCache[VoxelType::Wood][BlockFace::Bottom] = tex;
            textureCache[VoxelType::Wood][BlockFace::North] = tex;
            textureCache[VoxelType::Wood][BlockFace::South] = tex;
            textureCache[VoxelType::Wood][BlockFace::East] = tex;
            textureCache[VoxelType::Wood][BlockFace::West] = tex;
            LOG_INFO("Loaded wood texture");
        } else {
            LOG_ERROR("Failed to load wood texture");
            allLoaded = false;
        }
    }

    // Load Leaves
    {
        auto tex = texManager.loadTexture(getTexturePath(VoxelTexturePaths::LEAVES));
        if (tex) {
            textureCache[VoxelType::Leaves][BlockFace::Top] = tex;
            textureCache[VoxelType::Leaves][BlockFace::Bottom] = tex;
            textureCache[VoxelType::Leaves][BlockFace::North] = tex;
            textureCache[VoxelType::Leaves][BlockFace::South] = tex;
            textureCache[VoxelType::Leaves][BlockFace::East] = tex;
            textureCache[VoxelType::Leaves][BlockFace::West] = tex;
            LOG_INFO("Loaded leaves texture");
        } else {
            LOG_ERROR("Failed to load leaves texture");
            allLoaded = false;
        }
    }

    if (allLoaded) {
        LOG_INFO("All voxel textures loaded successfully");
    } else {
        LOG_WARNING("Some voxel textures failed to load");
    }

    return allLoaded;
}

std::shared_ptr<Texture> VoxelTextureLoader::getTexture(VoxelType type, BlockFace face, 
                                                        bool useMaterialPacks)
{
    // First, try to get texture from material packs if requested
    if (useMaterialPacks) {
        // Check VoxelMaterialPackManager for texture
        // This would be implemented when material pack system is integrated
        // For now, fall through to default texture cache
    }

    // Use default texture cache
    auto typeIt = textureCache.find(type);
    if (typeIt == textureCache.end()) {
        LOG_WARNING_C("Texture not found for voxel type", "VoxelTextureLoader");
        return nullptr;
    }

    auto faceIt = typeIt->second.find(face);
    if (faceIt == typeIt->second.end()) {
        // Try to get any texture for this type (fallback)
        if (!typeIt->second.empty()) {
            return typeIt->second.begin()->second;
        }
        return nullptr;
    }

    return faceIt->second;
}

bool VoxelTextureLoader::hasFaceSpecificTextures(VoxelType type) const
{
    // Currently only grass has face-specific textures
    return type == VoxelType::Grass;
}

void VoxelTextureLoader::setResolution(TextureResolution resolution)
{
    if (resolution != currentResolution) {
        currentResolution = resolution;
        textureCache.clear();
        LOG_INFO_C("Texture resolution changed, cache cleared", "VoxelTextureLoader");
    }
}

} // namespace fresh
