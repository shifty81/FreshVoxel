#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "ecs/IComponent.h"

namespace fresh
{
namespace physics
{

/**
 * @brief Component for physics properties with Newtonian physics
 *
 * Supports linear and rotational motion with forces, acceleration, velocity,
 * drag, and collision response.
 */
class PhysicsComponent : public ecs::IComponent
{
public:
    PhysicsComponent();
    virtual ~PhysicsComponent() = default;

    COMPONENT_TYPE(PhysicsComponent)

    // Linear motion
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 acceleration;
    float mass;
    float drag; // Linear drag coefficient

    // Rotational motion
    glm::quat rotation;
    glm::vec3 angularVelocity;
    glm::vec3 angularAcceleration;
    glm::mat3 inertiaTensor;
    glm::mat3 inverseInertiaTensor;
    float angularDrag; // Rotational drag coefficient

    // Forces and torques (accumulated per frame)
    glm::vec3 force;
    glm::vec3 torque;

    // Collision properties
    float restitution; // Elasticity (0 = inelastic, 1 = perfectly elastic)
    float friction;    // Surface friction

    // Flags
    bool isStatic;   // If true, object doesn't move
    bool useGravity; // If true, gravity force is applied

    // Methods
    void addForce(const glm::vec3& f);
    void addForceAtPoint(const glm::vec3& f, const glm::vec3& point);
    void addTorque(const glm::vec3& t);
    void clearForces();

    // Compute inverse inertia tensor in world space
    glm::mat3 getWorldInverseInertiaTensor() const;
};

} // namespace physics
} // namespace fresh
