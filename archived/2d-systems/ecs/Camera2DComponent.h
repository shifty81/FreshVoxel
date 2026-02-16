#pragma once
#include <glm/glm.hpp>
#include "ecs/IComponent.h"
#include "ecs/Entity.h"

namespace fresh
{
namespace ecs
{

/**
 * @brief Component for 2D camera
 *
 * Defines the view and projection for rendering 2D scenes.
 */
class Camera2DComponent : public IComponent
{
public:
    /**
     * @brief Camera projection mode
     */
    enum class ProjectionMode
    {
        Orthographic,   ///< Orthographic (no perspective)
        Perspective     ///< Perspective (for 2.5D effects)
    };

    Camera2DComponent();
    virtual ~Camera2DComponent() = default;

    COMPONENT_TYPE(Camera2DComponent)

    // Camera properties
    ProjectionMode projectionMode;
    float orthographicSize;     ///< Half-height of camera view (in world units)
    float aspectRatio;          ///< Width / Height ratio
    float nearClip;             ///< Near clipping plane
    float farClip;              ///< Far clipping plane
    
    // Camera settings
    bool isPrimary;             ///< Is this the main camera?
    int renderOrder;            ///< Order for multi-camera rendering
    glm::vec4 backgroundColor;  ///< Clear color
    
    // Follow target
    Entity followTarget;        ///< Entity to follow (invalid = don't follow)
    glm::vec2 followOffset;     ///< Offset from target position
    float followSmoothing;      ///< Smoothing factor (0 = instant, 1 = slow)
    
    // Bounds
    bool useBounds;             ///< Constrain camera to bounds?
    glm::vec2 boundsMin;        ///< Minimum camera position
    glm::vec2 boundsMax;        ///< Maximum camera position
    
    // Zoom
    float zoom;                 ///< Zoom level (1.0 = normal, 2.0 = 2x zoom)
    float minZoom;              ///< Minimum zoom
    float maxZoom;              ///< Maximum zoom

    /**
     * @brief Get view matrix
     */
    glm::mat4 getViewMatrix(const glm::vec2& position, float rotation) const;

    /**
     * @brief Get projection matrix
     */
    glm::mat4 getProjectionMatrix() const;

    /**
     * @brief Convert screen position to world position
     */
    glm::vec2 screenToWorld(const glm::vec2& screenPos, const glm::vec2& cameraPos, 
                           const glm::vec2& screenSize) const;

    /**
     * @brief Convert world position to screen position
     */
    glm::vec2 worldToScreen(const glm::vec2& worldPos, const glm::vec2& cameraPos,
                           const glm::vec2& screenSize) const;
};

} // namespace ecs
} // namespace fresh
