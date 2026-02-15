#include "ecs/Camera2DComponent.h"
#include <glm/gtc/matrix_transform.hpp>

namespace fresh
{
namespace ecs
{

Camera2DComponent::Camera2DComponent()
    : projectionMode(ProjectionMode::Orthographic)
    , orthographicSize(5.0f)
    , aspectRatio(16.0f / 9.0f)
    , nearClip(-1.0f)
    , farClip(1.0f)
    , isPrimary(true)
    , renderOrder(0)
    , backgroundColor(0.1f, 0.1f, 0.1f, 1.0f)
    , followOffset(0.0f, 0.0f)
    , followSmoothing(0.1f)
    , useBounds(false)
    , boundsMin(-100.0f, -100.0f)
    , boundsMax(100.0f, 100.0f)
    , zoom(1.0f)
    , minZoom(0.1f)
    , maxZoom(10.0f)
{
}

glm::mat4 Camera2DComponent::getViewMatrix(const glm::vec2& position, float rotation) const
{
    // Create 2D view matrix
    glm::mat4 view(1.0f);
    
    // Translate (negative because we move the world, not the camera)
    view = glm::translate(view, glm::vec3(-position.x, -position.y, 0.0f));
    
    // Rotate around Z axis
    view = glm::rotate(view, -rotation, glm::vec3(0.0f, 0.0f, 1.0f));
    
    return view;
}

glm::mat4 Camera2DComponent::getProjectionMatrix() const
{
    float effectiveSize = orthographicSize / zoom;
    float halfWidth = effectiveSize * aspectRatio;
    float halfHeight = effectiveSize;
    
    switch (projectionMode)
    {
        case ProjectionMode::Orthographic:
            return glm::ortho(-halfWidth, halfWidth, -halfHeight, halfHeight, nearClip, farClip);
            
        case ProjectionMode::Perspective:
            // For 2.5D effects
            return glm::perspective(glm::radians(60.0f), aspectRatio, nearClip, farClip);
    }
    
    return glm::mat4(1.0f);
}

glm::vec2 Camera2DComponent::screenToWorld(const glm::vec2& screenPos, const glm::vec2& cameraPos,
                                          const glm::vec2& screenSize) const
{
    // Convert screen coordinates (0,0 top-left) to NDC (-1 to 1)
    glm::vec2 ndc;
    ndc.x = (2.0f * screenPos.x) / screenSize.x - 1.0f;
    ndc.y = 1.0f - (2.0f * screenPos.y) / screenSize.y; // Flip Y
    
    // Apply camera size and zoom
    float effectiveSize = orthographicSize / zoom;
    float halfWidth = effectiveSize * aspectRatio;
    float halfHeight = effectiveSize;
    
    glm::vec2 world;
    world.x = ndc.x * halfWidth + cameraPos.x;
    world.y = ndc.y * halfHeight + cameraPos.y;
    
    return world;
}

glm::vec2 Camera2DComponent::worldToScreen(const glm::vec2& worldPos, const glm::vec2& cameraPos,
                                          const glm::vec2& screenSize) const
{
    // Apply camera size and zoom
    float effectiveSize = orthographicSize / zoom;
    float halfWidth = effectiveSize * aspectRatio;
    float halfHeight = effectiveSize;
    
    // Convert to camera-relative coordinates
    glm::vec2 relative = worldPos - cameraPos;
    
    // Convert to NDC
    glm::vec2 ndc;
    ndc.x = relative.x / halfWidth;
    ndc.y = relative.y / halfHeight;
    
    // Convert NDC to screen coordinates
    glm::vec2 screen;
    screen.x = (ndc.x + 1.0f) * 0.5f * screenSize.x;
    screen.y = (1.0f - ndc.y) * 0.5f * screenSize.y; // Flip Y
    
    return screen;
}

} // namespace ecs
} // namespace fresh
