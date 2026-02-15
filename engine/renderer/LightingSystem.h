#pragma once

#include <memory>
#include <string>
#include <vector>

#include <glm/glm.hpp>

namespace fresh
{

/**
 * @brief Light type enumeration
 */
enum class LightType {
    Directional, // Sun/Moon - infinite distance
    Point,       // Light bulb - radiates in all directions
    Spot         // Flashlight - cone of light
};

/**
 * @brief Base light class
 */
class Light
{
public:
    Light(LightType type) : type(type) {}
    virtual ~Light() = default;

    LightType type;
    glm::vec3 color{1.0f, 1.0f, 1.0f};
    float intensity = 1.0f;
    bool castShadows = true;
    bool enabled = true;

    // For soft shadows
    float shadowBias = 0.005f;
    int shadowMapSize = 1024;

    virtual glm::mat4 getViewMatrix() const = 0;
    virtual glm::mat4 getProjectionMatrix() const = 0;
};

/**
 * @brief Directional light (sun, moon)
 */
class DirectionalLight : public Light
{
public:
    DirectionalLight() : Light(LightType::Directional) {}

    glm::vec3 direction{0.0f, -1.0f, 0.0f}; // Direction light shines

    // Shadow map coverage
    float orthoSize = 50.0f;
    float nearPlane = 0.1f;
    float farPlane = 100.0f;

    glm::mat4 getViewMatrix() const override;
    glm::mat4 getProjectionMatrix() const override;
};

/**
 * @brief Point light (light bulb, torch)
 */
class PointLight : public Light
{
public:
    PointLight() : Light(LightType::Point) {}

    glm::vec3 position{0.0f, 0.0f, 0.0f};

    // Attenuation (falloff)
    float constant = 1.0f;
    float linear = 0.09f;
    float quadratic = 0.032f;
    float radius = 10.0f; // Maximum range

    glm::mat4 getViewMatrix() const override;
    glm::mat4 getProjectionMatrix() const override;

    // Calculate attenuation at distance
    float getAttenuation(float distance) const;
};

/**
 * @brief Spot light (flashlight, spotlight)
 */
class SpotLight : public Light
{
public:
    SpotLight() : Light(LightType::Spot) {}

    glm::vec3 position{0.0f, 0.0f, 0.0f};
    glm::vec3 direction{0.0f, -1.0f, 0.0f};

    // Cone angles (in radians)
    float innerConeAngle = glm::radians(12.5f);
    float outerConeAngle = glm::radians(17.5f);

    // Attenuation
    float constant = 1.0f;
    float linear = 0.09f;
    float quadratic = 0.032f;
    float radius = 20.0f;

    glm::mat4 getViewMatrix() const override;
    glm::mat4 getProjectionMatrix() const override;

    float getAttenuation(float distance) const;
    float getSpotFactor(const glm::vec3& lightToPixel) const;
};

/**
 * @brief Advanced lighting system with PBR support
 */
class LightingSystem
{
public:
    LightingSystem();
    ~LightingSystem();

    /**
     * @brief Initialize lighting system
     */
    bool initialize();

    /**
     * @brief Update lighting (for dynamic lights)
     */
    void update(float deltaTime);

    /**
     * @brief Add directional light
     */
    std::shared_ptr<DirectionalLight> addDirectionalLight();

    /**
     * @brief Add point light
     */
    std::shared_ptr<PointLight> addPointLight(const glm::vec3& position);

    /**
     * @brief Add spot light
     */
    std::shared_ptr<SpotLight> addSpotLight(const glm::vec3& position, const glm::vec3& direction);

    /**
     * @brief Remove light
     */
    void removeLight(std::shared_ptr<Light> light);

    /**
     * @brief Clear all lights
     */
    void clearLights();

    /**
     * @brief Get all lights
     */
    const std::vector<std::shared_ptr<Light>>& getLights() const
    {
        return lights;
    }

    /**
     * @brief Get directional lights only
     */
    std::vector<std::shared_ptr<DirectionalLight>> getDirectionalLights() const;

    /**
     * @brief Get point lights only
     */
    std::vector<std::shared_ptr<PointLight>> getPointLights() const;

    /**
     * @brief Get spot lights only
     */
    std::vector<std::shared_ptr<SpotLight>> getSpotLights() const;

    // PBR Environment
    /**
     * @brief Set ambient light color
     */
    void setAmbientLight(const glm::vec3& color, float intensity = 0.1f);

    /**
     * @brief Get ambient light color
     */
    glm::vec3 getAmbientColor() const
    {
        return ambientColor * ambientIntensity;
    }

    /**
     * @brief Enable/disable shadows globally
     */
    void setShadowsEnabled(bool enabled)
    {
        shadowsEnabled = enabled;
    }
    bool areShadowsEnabled() const
    {
        return shadowsEnabled;
    }

    /**
     * @brief Set shadow quality (map size)
     */
    void setShadowQuality(int size)
    {
        shadowMapSize = size;
    }
    int getShadowQuality() const
    {
        return shadowMapSize;
    }

    /**
     * @brief Enable/disable PBR
     */
    void setPBREnabled(bool enabled)
    {
        pbrEnabled = enabled;
    }
    bool isPBREnabled() const
    {
        return pbrEnabled;
    }

    /**
     * @brief Set IBL (Image-Based Lighting) textures
     */
    void setIBLTextures(const std::string& irradianceMap, const std::string& prefilteredMap,
                        const std::string& brdfLUT);

private:
    std::vector<std::shared_ptr<Light>> lights;

    // Ambient lighting
    glm::vec3 ambientColor{0.3f, 0.3f, 0.4f};
    float ambientIntensity = 0.1f;

    // Shadow settings
    bool shadowsEnabled = true;
    int shadowMapSize = 1024;

    // PBR settings
    bool pbrEnabled = true;

    // IBL textures for PBR
    std::string iblIrradianceMap;
    std::string iblPrefilteredMap;
    std::string iblBrdfLUT;
};

} // namespace fresh
