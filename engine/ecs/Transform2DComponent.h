#pragma once
#include <glm/glm.hpp>
#include "ecs/IComponent.h"

namespace fresh
{
namespace ecs
{

/**
 * @brief Component for 2D object transform (position, rotation, scale)
 *
 * Represents the spatial properties of an entity in 2D space.
 * Unlike TransformComponent, this uses 2D vectors and a single rotation angle.
 */
class Transform2DComponent : public IComponent
{
public:
    Transform2DComponent();
    Transform2DComponent(const glm::vec2& pos, float rot = 0.0f, const glm::vec2& scl = glm::vec2(1.0f));
    virtual ~Transform2DComponent() = default;

    COMPONENT_TYPE(Transform2DComponent)

    // Transform properties
    glm::vec2 position;     ///< Position in 2D space (X, Y)
    float rotation;         ///< Rotation in radians (counter-clockwise)
    glm::vec2 scale;        ///< Scale factor (X, Y)
    
    // Layer for rendering order
    int layer;              ///< Rendering layer (higher = rendered on top)
    float depth;            ///< Depth within layer for sub-sorting

    /**
     * @brief Get the rotation in degrees
     */
    float getRotationDegrees() const;

    /**
     * @brief Set rotation from degrees
     */
    void setRotationDegrees(float degrees);

    /**
     * @brief Get the 2D transform matrix
     */
    glm::mat3 getTransformMatrix() const;

    /**
     * @brief Get the forward direction vector (in 2D)
     */
    glm::vec2 getForward() const;

    /**
     * @brief Get the right direction vector (in 2D)
     */
    glm::vec2 getRight() const;

    /**
     * @brief Transform a local point to world space
     */
    glm::vec2 transformPoint(const glm::vec2& localPoint) const;

    /**
     * @brief Transform a world point to local space
     */
    glm::vec2 inverseTransformPoint(const glm::vec2& worldPoint) const;
};

} // namespace ecs
} // namespace fresh
