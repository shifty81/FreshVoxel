#include "ecs/RigidBody2DComponent.h"
#include <algorithm>

namespace fresh
{
namespace ecs
{

RigidBody2DComponent::RigidBody2DComponent()
    : bodyType(BodyType::Dynamic)
    , velocity(0.0f, 0.0f)
    , angularVelocity(0.0f)
    , acceleration(0.0f, 0.0f)
    , angularAcceleration(0.0f)
    , mass(1.0f)
    , inverseMass(1.0f)
    , drag(0.01f)
    , angularDrag(0.05f)
    , restitution(0.0f)
    , friction(0.3f)
    , useGravity(true)
    , freezeRotation(false)
    , gravityScale(1.0f, 1.0f)
    , force(0.0f, 0.0f)
    , torque(0.0f)
{
}

void RigidBody2DComponent::addForce(const glm::vec2& newForce)
{
    force += newForce;
}

void RigidBody2DComponent::addImpulse(const glm::vec2& impulse)
{
    if (bodyType != BodyType::Dynamic)
        return;
        
    velocity += impulse * inverseMass;
}

void RigidBody2DComponent::addTorque(float newTorque)
{
    torque += newTorque;
}

void RigidBody2DComponent::setMass(float newMass)
{
    mass = std::max(0.001f, newMass); // Prevent zero or negative mass
    
    if (bodyType == BodyType::Static)
    {
        inverseMass = 0.0f;
    }
    else
    {
        inverseMass = 1.0f / mass;
    }
}

void RigidBody2DComponent::clearForces()
{
    force = glm::vec2(0.0f);
    torque = 0.0f;
}

float RigidBody2DComponent::getKineticEnergy() const
{
    // KE = 0.5 * m * v^2 (linear) + 0.5 * I * ω^2 (rotational)
    // For 2D, we simplify rotational inertia
    float linearKE = 0.5f * mass * glm::dot(velocity, velocity);
    float rotationalKE = 0.5f * mass * angularVelocity * angularVelocity;
    return linearKE + rotationalKE;
}

} // namespace ecs
} // namespace fresh
