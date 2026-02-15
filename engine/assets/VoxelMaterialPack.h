#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "renderer/Material.h"
#include "renderer/Texture.h"
#include "voxel/VoxelTypes.h"

namespace fresh
{

/**
 * @brief Face-specific texture mapping for a voxel block
 */
struct VoxelFaceTextures {
    std::string top;
    std::string bottom;
    std::string north;
    std::string south;
    std::string east;
    std::string west;
    
    // If all faces use the same texture
    std::string all;
    
    bool hasAllFaces() const { return !all.empty(); }
};

/**
 * @brief Material definition for a voxel type
 */
struct VoxelMaterialDefinition {
    VoxelType voxelType;
    std::string name;
    
    // Texture paths (relative to pack root)
    VoxelFaceTextures textures;
    
    // Optional PBR material properties
    struct MaterialProperties {
        std::string normalMap;
        std::string metallicRoughnessMap;
        std::string aoMap;
        std::string emissiveMap;
        
        // Material parameters
        float metallic = 0.0f;
        float roughness = 0.5f;
        float ao = 1.0f;
        glm::vec3 emissive{0.0f, 0.0f, 0.0f};
        float emissiveStrength = 1.0f;
        
        // Transparency
        float alpha = 1.0f;
        bool alphaBlend = false;
        float alphaCutoffValue = 0.5f;
    } materialProps;
    
    // Texture atlas parameters
    bool includeInAtlas = true;
    int atlasIndex = -1; // Set during atlas generation
};

/**
 * @brief Voxel texture/material pack
 * 
 * Allows customization of voxel block textures and materials similar to
 * Minecraft resource packs. Supports texture atlases for optimized rendering.
 */
class VoxelMaterialPack
{
public:
    VoxelMaterialPack(const std::string& packPath);
    ~VoxelMaterialPack();
    
    /**
     * @brief Load the material pack from disk
     */
    bool load();
    
    /**
     * @brief Unload the material pack
     */
    void unload();
    
    /**
     * @brief Get pack metadata
     */
    const std::string& getName() const { return name; }
    const std::string& getVersion() const { return version; }
    const std::string& getAuthor() const { return author; }
    const std::string& getDescription() const { return description; }
    
    /**
     * @brief Check if pack is loaded
     */
    bool isLoaded() const { return loaded; }
    
    /**
     * @brief Get material definition for a voxel type
     */
    const VoxelMaterialDefinition* getMaterialDefinition(VoxelType type) const;
    
    /**
     * @brief Get all material definitions
     */
    const std::vector<VoxelMaterialDefinition>& getMaterialDefinitions() const 
    { 
        return materialDefinitions; 
    }
    
    /**
     * @brief Get texture for a specific voxel type and face
     */
    std::shared_ptr<Texture> getTexture(VoxelType type, const std::string& face = "all");
    
    /**
     * @brief Check if this pack overrides a specific voxel type
     */
    bool overridesVoxelType(VoxelType type) const;
    
    /**
     * @brief Generate texture atlas from all textures in pack
     * @param atlasSize Size of atlas texture (e.g., 512, 1024, 2048)
     * @param tileSize Size of each tile in the atlas (e.g., 16, 32, 64)
     * @return Generated atlas texture or nullptr on failure
     */
    std::shared_ptr<Texture> generateTextureAtlas(int atlasSize = 1024, int tileSize = 32);
    
    /**
     * @brief Get the texture atlas if generated
     */
    std::shared_ptr<Texture> getTextureAtlas() const { return textureAtlas; }
    
    /**
     * @brief Get UV coordinates for a voxel type/face in the atlas
     * @param type Voxel type
     * @param face Face name ("top", "bottom", "north", etc., or "all")
     * @return UV coordinates [minU, minV, maxU, maxV] or empty if not in atlas
     */
    std::vector<float> getAtlasUVs(VoxelType type, const std::string& face = "all") const;
    
    /**
     * @brief Get priority for pack ordering (higher = more important)
     */
    int getPriority() const { return priority; }
    
private:
    bool parseManifest(const std::string& manifestPath);
    VoxelType parseVoxelType(const std::string& typeStr) const;
    std::string voxelTypeToString(VoxelType type) const;
    
    std::string packPath;
    std::string name;
    std::string version;
    std::string author;
    std::string description;
    int priority = 0; // For pack ordering when multiple packs loaded
    
    std::vector<VoxelMaterialDefinition> materialDefinitions;
    std::map<VoxelType, size_t> voxelTypeToDefIndex; // Fast lookup
    
    // Texture cache
    std::map<std::string, std::shared_ptr<Texture>> textureCache;
    
    // Texture atlas
    std::shared_ptr<Texture> textureAtlas;
    struct AtlasEntry {
        int x, y;           // Position in atlas (in tiles)
        int tileSize;       // Size of tile
        float uvMin[2];     // UV min coordinates
        float uvMax[2];     // UV max coordinates
    };
    std::map<std::string, AtlasEntry> atlasMapping; // Key: "VoxelType:face"
    
    bool loaded = false;
};

/**
 * @brief Voxel material pack manager
 * 
 * Manages loading and applying voxel texture/material packs.
 * Supports multiple packs with priority-based overriding.
 */
class VoxelMaterialPackManager
{
public:
    static VoxelMaterialPackManager& getInstance();
    
    /**
     * @brief Initialize the manager
     */
    void initialize(const std::string& packDirectory);
    
    /**
     * @brief Shutdown and unload all packs
     */
    void shutdown();
    
    /**
     * @brief Scan for and load all material packs
     */
    void scanAndLoadPacks();
    
    /**
     * @brief Load a specific material pack
     */
    bool loadPack(const std::string& packPath);
    
    /**
     * @brief Unload a material pack by name
     */
    void unloadPack(const std::string& packName);
    
    /**
     * @brief Get all loaded packs (sorted by priority)
     */
    const std::vector<std::shared_ptr<VoxelMaterialPack>>& getLoadedPacks() const 
    { 
        return loadedPacks; 
    }
    
    /**
     * @brief Get material definition for a voxel type (from highest priority pack)
     */
    const VoxelMaterialDefinition* getMaterialDefinition(VoxelType type) const;
    
    /**
     * @brief Get texture for a voxel type and face (from highest priority pack)
     */
    std::shared_ptr<Texture> getTexture(VoxelType type, const std::string& face = "all");
    
    /**
     * @brief Create a template voxel material pack
     */
    static bool createPackTemplate(const std::string& outputPath);
    
    /**
     * @brief Validate a material pack
     */
    bool validatePack(const std::string& packPath) const;
    
    /**
     * @brief Print statistics
     */
    void printStats() const;
    
private:
    VoxelMaterialPackManager() = default;
    ~VoxelMaterialPackManager() = default;
    VoxelMaterialPackManager(const VoxelMaterialPackManager&) = delete;
    VoxelMaterialPackManager& operator=(const VoxelMaterialPackManager&) = delete;
    
    void sortPacksByPriority();
    
    std::string packDirectory;
    std::vector<std::shared_ptr<VoxelMaterialPack>> loadedPacks;
};

} // namespace fresh
