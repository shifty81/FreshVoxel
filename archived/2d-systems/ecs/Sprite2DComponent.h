#pragma once
#include <glm/glm.hpp>
#include <string>
#include "ecs/IComponent.h"

namespace fresh
{
namespace ecs
{

/**
 * @brief Component for 2D sprite rendering
 *
 * Contains information needed to render a 2D sprite, including texture,
 * color tint, and sprite sheet information for animation.
 */
class Sprite2DComponent : public IComponent
{
public:
    Sprite2DComponent();
    explicit Sprite2DComponent(const std::string& texturePath);
    virtual ~Sprite2DComponent() = default;

    COMPONENT_TYPE(Sprite2DComponent)

    // Texture information
    std::string texturePath;        ///< Path to texture file
    unsigned int textureID;         ///< OpenGL/DirectX texture ID (managed by renderer)
    
    // Sprite dimensions
    glm::vec2 size;                 ///< Size in world units (default matches texture size)
    glm::vec2 pivot;                ///< Pivot point (0,0 = bottom-left, 0.5,0.5 = center)
    
    // Color and blending
    glm::vec4 color;                ///< Color tint (RGBA, default white)
    float alpha;                    ///< Alpha transparency (0.0 = transparent, 1.0 = opaque)
    
    // Sprite sheet support
    bool isSprite;                  ///< True if using sprite sheet
    glm::ivec2 spriteSheetSize;     ///< Grid size of sprite sheet (columns, rows)
    glm::ivec2 spriteIndex;         ///< Current sprite in sheet (column, row)
    
    // Rendering flags
    bool flipX;                     ///< Flip sprite horizontally
    bool flipY;                     ///< Flip sprite vertically
    bool visible;                   ///< Is sprite visible?

    /**
     * @brief Get UV coordinates for current sprite in sheet
     * @return UV min and max (min.x, min.y, max.x, max.y)
     */
    glm::vec4 getSpriteUVs() const;

    /**
     * @brief Set sprite from sheet by index
     */
    void setSprite(int column, int row);

    /**
     * @brief Set sprite sheet dimensions
     */
    void setSpriteSheet(int columns, int rows);
};

} // namespace ecs
} // namespace fresh
