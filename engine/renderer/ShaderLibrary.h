#pragma once

#include <memory>
#include <string>

namespace fresh
{

class RenderShader;
class LightingSystem;

/**
 * @brief Cell shading (toon) shader generator and manager
 *
 * Generates GLSL shaders for cel/toon shading effects with
 * customizable levels and edge detection.
 */
class CellShadingShader
{
public:
    struct Settings {
        int shadingLevels = 4;                 // Number of discrete shading levels
        float edgeThreshold = 0.1f;            // Edge detection threshold
        float edgeWidth = 1.0f;                // Edge line width
        glm::vec3 edgeColor{0.0f, 0.0f, 0.0f}; // Edge color (usually black)
        bool useOutline = true;                // Draw outlines
        bool useRimLight = false;              // Add rim lighting
        glm::vec3 rimColor{1.0f, 1.0f, 1.0f};  // Rim light color
        float rimPower = 3.0f;                 // Rim light sharpness
        float rimIntensity = 1.0f;             // Rim light intensity
        bool useSpecular = false;              // Add specular highlights
        float specularSize = 0.1f;             // Specular highlight size
        float specularIntensity = 1.0f;        // Specular intensity
    };

    /**
     * @brief Generate cell shading vertex shader
     */
    static std::string generateVertexShader(const Settings& settings);

    /**
     * @brief Generate cell shading fragment shader
     */
    static std::string generateFragmentShader(const Settings& settings);

    /**
     * @brief Create cell shading shader program
     * @param settings Cell shading settings
     * @return Compiled shader program
     */
    static std::shared_ptr<RenderShader> createShader(const Settings& settings);

    /**
     * @brief Create default cell shading shader
     */
    static std::shared_ptr<RenderShader> createDefaultShader();

    /**
     * @brief Update shader uniforms with lighting
     * @param shader Shader to update
     * @param lightingSystem Lighting system to get light data from
     */
    static void updateLightingUniforms(RenderShader* shader, LightingSystem* lightingSystem);
};

/**
 * @brief Standard PBR shader generator
 */
class PBRShader
{
public:
    struct Settings {
        bool useIBL = true;              // Use image-based lighting
        bool useNormalMapping = true;    // Use normal maps
        bool useParallaxMapping = false; // Use parallax occlusion mapping
        bool useEmissive = true;         // Support emissive materials
        bool useAO = true;               // Use ambient occlusion maps
        int maxLights = 8;               // Maximum number of lights
    };

    /**
     * @brief Generate PBR vertex shader
     */
    static std::string generateVertexShader(const Settings& settings);

    /**
     * @brief Generate PBR fragment shader
     */
    static std::string generateFragmentShader(const Settings& settings);

    /**
     * @brief Create PBR shader program
     */
    static std::shared_ptr<RenderShader> createShader(const Settings& settings);

    /**
     * @brief Create default PBR shader
     */
    static std::shared_ptr<RenderShader> createDefaultShader();

    /**
     * @brief Update shader uniforms
     */
    static void updateMaterialUniforms(RenderShader* shader, class Material* material);
    static void updateLightingUniforms(RenderShader* shader, LightingSystem* lightingSystem);
};

/**
 * @brief Shader library for managing common shaders
 */
class ShaderLibrary
{
public:
    /**
     * @brief Get singleton instance
     */
    static ShaderLibrary& getInstance();

    /**
     * @brief Initialize shader library with default shaders
     */
    bool initialize();

    /**
     * @brief Get shader by name
     */
    std::shared_ptr<RenderShader> getShader(const std::string& name);

    /**
     * @brief Add shader to library
     */
    void addShader(const std::string& name, std::shared_ptr<RenderShader> shader);

    /**
     * @brief Reload all shaders (hot-reload)
     */
    void reloadAll();

    // Predefined shader names
    static constexpr const char* SHADER_PBR = "pbr";
    static constexpr const char* SHADER_CELL = "cell";
    static constexpr const char* SHADER_UNLIT = "unlit";
    static constexpr const char* SHADER_VOXEL = "voxel";
    static constexpr const char* SHADER_SKYBOX = "skybox";
    static constexpr const char* SHADER_WATER = "water";

private:
    ShaderLibrary() = default;
    ~ShaderLibrary() = default;

    std::unordered_map<std::string, std::shared_ptr<RenderShader>> shaders;
};

} // namespace fresh
