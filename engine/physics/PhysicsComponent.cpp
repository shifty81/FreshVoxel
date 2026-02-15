#include "physics/PhysicsComponent.h"

#include <glm/gtx/quaternion.hpp>

namespace fresh
{
namespace physics
{

PhysicsComponent::PhysicsComponent()
    : position(0.0f),
      velocity(0.0f),
      acceleration(0.0f),
      mass(1.0f),
      drag(0.1f),
      rotation(1.0f, 0.0f, 0.0f, 0.0f),
      angularVelocity(0.0f),
      angularAcceleration(0.0f),
      inertiaTensor(1.0f),
      inverseInertiaTensor(1.0f),
      angularDrag(0.1f),
      force(0.0f),
      torque(0.0f),
      restitution(0.5f),
      friction(0.5f),
      isStatic(false),
      useGravity(true)
{
}

void PhysicsComponent::addForce(const glm::vec3& f)
{
    force += f;
}

void PhysicsComponent::addForceAtPoint(const glm::vec3& f, const glm::vec3& point)
{
    force += f;
    // Calculate torque from force at point
    glm::vec3 r = point - position;
    torque += glm::cross(r, f);
}

void PhysicsComponent::addTorque(const glm::vec3& t)
{
    torque += t;
}

void PhysicsComponent::clearForces()
{
    force = glm::vec3(0.0f);
    torque = glm::vec3(0.0f);
}

glm::mat3 PhysicsComponent::getWorldInverseInertiaTensor() const
{
    // Convert quaternion to rotation matrix
    glm::mat3 R = glm::mat3_cast(rotation);
    // Transform inertia tensor to world space: R * I^-1 * R^T
    return R * inverseInertiaTensor * glm::transpose(R);
}

} // namespace physics
} // namespace fresh
