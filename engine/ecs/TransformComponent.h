#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "ecs/IComponent.h"

namespace fresh
{
namespace ecs
{

/**
 * @brief Component for object transform (position, rotation, scale)
 *
 * Represents the spatial properties of an entity in 3D space.
 * This is the most fundamental component for any visible or physical object.
 */
class TransformComponent : public IComponent
{
public:
    TransformComponent();
    TransformComponent(const glm::vec3& pos, const glm::quat& rot, const glm::vec3& scl);
    virtual ~TransformComponent() = default;

    COMPONENT_TYPE(TransformComponent)

    // Transform properties
    glm::vec3 position;
    glm::quat rotation;
    glm::vec3 scale;

    /**
     * @brief Get the euler angles representation of rotation (in degrees)
     */
    glm::vec3 getEulerAngles() const;

    /**
     * @brief Set rotation from euler angles (in degrees)
     */
    void setEulerAngles(const glm::vec3& euler);

    /**
     * @brief Get the transform matrix
     */
    glm::mat4 getTransformMatrix() const;

    /**
     * @brief Get the forward direction vector
     */
    glm::vec3 getForward() const;

    /**
     * @brief Get the right direction vector
     */
    glm::vec3 getRight() const;

    /**
     * @brief Get the up direction vector
     */
    glm::vec3 getUp() const;
};

} // namespace ecs
} // namespace fresh
