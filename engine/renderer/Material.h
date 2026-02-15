#pragma once

#include <memory>
#include <string>

#include <glm/glm.hpp>

namespace fresh
{

class Texture;

/**
 * @brief PBR Material properties
 *
 * Supports Physically Based Rendering with metallic-roughness workflow.
 * Includes support for albedo, normal, metallic, roughness, and AO maps.
 */
class Material
{
public:
    Material(const std::string& name = "DefaultMaterial");
    ~Material() = default;

    // Material properties
    std::string name;

    // PBR parameters
    glm::vec3 albedo{1.0f, 1.0f, 1.0f};   // Base color
    float metallic = 0.0f;                // Metallic factor (0=dielectric, 1=metal)
    float roughness = 0.5f;               // Surface roughness (0=smooth, 1=rough)
    float ao = 1.0f;                      // Ambient occlusion
    glm::vec3 emissive{0.0f, 0.0f, 0.0f}; // Emissive color
    float emissiveStrength = 1.0f;

    // Alpha/transparency
    float alpha = 1.0f;            // Alpha transparency
    bool alphaBlend = false;       // Enable alpha blending
    bool alphaCutoff = false;      // Use alpha cutoff instead of blend
    float alphaCutoffValue = 0.5f; // Cutoff threshold

    // Cell shading (toon) parameters
    bool enableCellShading = false;                   // Enable cartoon/toon rendering
    int cellShadingLevels = 4;                        // Number of shading levels
    float cellShadingEdgeWidth = 0.1f;                // Edge outline width
    glm::vec3 cellShadingEdgeColor{0.0f, 0.0f, 0.0f}; // Edge color

    // Texture maps (will be loaded via TextureManager)
    std::shared_ptr<Texture> albedoMap;            // Diffuse/albedo texture
    std::shared_ptr<Texture> normalMap;            // Normal map for detail
    std::shared_ptr<Texture> metallicRoughnessMap; // Combined metallic/roughness
    std::shared_ptr<Texture> aoMap;                // Ambient occlusion map
    std::shared_ptr<Texture> emissiveMap;          // Emissive map

    // Texture coordinates
    glm::vec2 textureScale{1.0f, 1.0f};
    glm::vec2 textureOffset{0.0f, 0.0f};

    // Render state
    bool doubleSided = false; // Disable backface culling
    bool receiveShadows = true;
    bool castShadows = true;

    // Shader uniforms (will be set by renderer)
    int shaderId = -1; // Associated shader program

    /**
     * @brief Set albedo texture from path
     */
    void setAlbedoTexture(const std::string& path);

    /**
     * @brief Set normal map from path
     */
    void setNormalMap(const std::string& path);

    /**
     * @brief Set metallic-roughness map from path
     */
    void setMetallicRoughnessMap(const std::string& path);

    /**
     * @brief Set AO map from path
     */
    void setAOMap(const std::string& path);

    /**
     * @brief Set emissive map from path
     */
    void setEmissiveMap(const std::string& path);

    /**
     * @brief Enable/disable cell shading
     */
    void setCellShading(bool enabled, int levels = 4);

    /**
     * @brief Check if material uses any textures
     */
    bool hasTextures() const;

    /**
     * @brief Check if material is transparent
     */
    bool isTransparent() const
    {
        return alphaBlend || alpha < 1.0f;
    }
};

} // namespace fresh
