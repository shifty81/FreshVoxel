#pragma once
#include <glm/glm.hpp>
#include <string>

#include "ecs/IComponent.h"

namespace fresh
{
namespace ecs
{

/**
 * @brief Component for material properties
 *
 * Defines how a surface appears, including shader, color, textures,
 * and material parameters like metallic and smoothness.
 */
class MaterialComponent : public IComponent
{
public:
    MaterialComponent();
    virtual ~MaterialComponent() = default;

    COMPONENT_TYPE(MaterialComponent)

    // Shader type
    enum class ShaderType
    {
        Standard = 0,
        Unlit,
        Water,
        Transparent,
        Custom
    };

    ShaderType shaderType;

    // Color
    glm::vec4 color; // RGBA

    // Texture
    std::string texturePath;

    // Material properties
    float metallic;   // 0.0 = dielectric, 1.0 = metallic
    float smoothness; // 0.0 = rough, 1.0 = smooth
    float emission;   // Emission intensity
};

} // namespace ecs
} // namespace fresh
