#include "ecs/Transform2DComponent.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_transform_2d.hpp>
#include <cmath>

namespace fresh
{
namespace ecs
{

Transform2DComponent::Transform2DComponent()
    : position(0.0f, 0.0f)
    , rotation(0.0f)
    , scale(1.0f, 1.0f)
    , layer(0)
    , depth(0.0f)
{
}

Transform2DComponent::Transform2DComponent(const glm::vec2& pos, float rot, const glm::vec2& scl)
    : position(pos)
    , rotation(rot)
    , scale(scl)
    , layer(0)
    , depth(0.0f)
{
}

float Transform2DComponent::getRotationDegrees() const
{
    return glm::degrees(rotation);
}

void Transform2DComponent::setRotationDegrees(float degrees)
{
    rotation = glm::radians(degrees);
}

glm::mat3 Transform2DComponent::getTransformMatrix() const
{
    // Build 2D transformation matrix: T * R * S
    glm::mat3 transform(1.0f);
    
    // Apply translation
    transform = glm::translate(transform, position);
    
    // Apply rotation
    transform = glm::rotate(transform, rotation);
    
    // Apply scale
    transform = glm::scale(transform, scale);
    
    return transform;
}

glm::vec2 Transform2DComponent::getForward() const
{
    // Forward in 2D is the direction the entity is facing
    float cos_r = std::cos(rotation);
    float sin_r = std::sin(rotation);
    return glm::vec2(cos_r, sin_r);
}

glm::vec2 Transform2DComponent::getRight() const
{
    // Right is perpendicular to forward (90 degrees clockwise)
    float cos_r = std::cos(rotation);
    float sin_r = std::sin(rotation);
    return glm::vec2(sin_r, -cos_r);
}

glm::vec2 Transform2DComponent::transformPoint(const glm::vec2& localPoint) const
{
    glm::mat3 transform = getTransformMatrix();
    glm::vec3 point(localPoint.x, localPoint.y, 1.0f);
    glm::vec3 result = transform * point;
    return glm::vec2(result.x, result.y);
}

glm::vec2 Transform2DComponent::inverseTransformPoint(const glm::vec2& worldPoint) const
{
    glm::mat3 transform = glm::inverse(getTransformMatrix());
    glm::vec3 point(worldPoint.x, worldPoint.y, 1.0f);
    glm::vec3 result = transform * point;
    return glm::vec2(result.x, result.y);
}

} // namespace ecs
} // namespace fresh
