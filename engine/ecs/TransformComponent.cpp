#include "ecs/TransformComponent.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/quaternion.hpp>

namespace fresh
{
namespace ecs
{

TransformComponent::TransformComponent()
    : position(0.0f, 0.0f, 0.0f), rotation(glm::quat(1.0f, 0.0f, 0.0f, 0.0f)), scale(1.0f, 1.0f, 1.0f)
{
}

TransformComponent::TransformComponent(const glm::vec3& pos, const glm::quat& rot, const glm::vec3& scl)
    : position(pos), rotation(rot), scale(scl)
{
}

glm::vec3 TransformComponent::getEulerAngles() const
{
    // Extract euler angles in degrees
    return glm::degrees(glm::eulerAngles(rotation));
}

void TransformComponent::setEulerAngles(const glm::vec3& euler)
{
    // Convert degrees to radians and create quaternion
    glm::vec3 radians = glm::radians(euler);
    rotation = glm::quat(radians);
}

glm::mat4 TransformComponent::getTransformMatrix() const
{
    glm::mat4 matrix = glm::mat4(1.0f);
    matrix = glm::translate(matrix, position);
    matrix = matrix * glm::toMat4(rotation);
    matrix = glm::scale(matrix, scale);
    return matrix;
}

glm::vec3 TransformComponent::getForward() const
{
    return rotation * glm::vec3(0.0f, 0.0f, -1.0f);
}

glm::vec3 TransformComponent::getRight() const
{
    return rotation * glm::vec3(1.0f, 0.0f, 0.0f);
}

glm::vec3 TransformComponent::getUp() const
{
    return rotation * glm::vec3(0.0f, 1.0f, 0.0f);
}

} // namespace ecs
} // namespace fresh
