#include "ecs/Sprite2DComponent.h"

namespace fresh
{
namespace ecs
{

Sprite2DComponent::Sprite2DComponent()
    : textureID(0)
    , size(1.0f, 1.0f)
    , pivot(0.5f, 0.5f)
    , color(1.0f, 1.0f, 1.0f, 1.0f)
    , alpha(1.0f)
    , isSprite(false)
    , spriteSheetSize(1, 1)
    , spriteIndex(0, 0)
    , flipX(false)
    , flipY(false)
    , visible(true)
{
}

Sprite2DComponent::Sprite2DComponent(const std::string& texturePath)
    : texturePath(texturePath)
    , textureID(0)
    , size(1.0f, 1.0f)
    , pivot(0.5f, 0.5f)
    , color(1.0f, 1.0f, 1.0f, 1.0f)
    , alpha(1.0f)
    , isSprite(false)
    , spriteSheetSize(1, 1)
    , spriteIndex(0, 0)
    , flipX(false)
    , flipY(false)
    , visible(true)
{
}

glm::vec4 Sprite2DComponent::getSpriteUVs() const
{
    if (!isSprite || spriteSheetSize.x <= 0 || spriteSheetSize.y <= 0)
    {
        // Return full texture UVs
        return glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
    }

    // Calculate UV coordinates for sprite in sheet
    float spriteWidth = 1.0f / static_cast<float>(spriteSheetSize.x);
    float spriteHeight = 1.0f / static_cast<float>(spriteSheetSize.y);
    
    float minU = static_cast<float>(spriteIndex.x) * spriteWidth;
    float minV = static_cast<float>(spriteIndex.y) * spriteHeight;
    float maxU = minU + spriteWidth;
    float maxV = minV + spriteHeight;
    
    return glm::vec4(minU, minV, maxU, maxV);
}

void Sprite2DComponent::setSprite(int column, int row)
{
    spriteIndex.x = column;
    spriteIndex.y = row;
}

void Sprite2DComponent::setSpriteSheet(int columns, int rows)
{
    isSprite = true;
    spriteSheetSize.x = columns;
    spriteSheetSize.y = rows;
    spriteIndex = glm::ivec2(0, 0);
}

} // namespace ecs
} // namespace fresh
