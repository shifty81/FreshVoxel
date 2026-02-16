#include "ecs/Physics2DSystem.h"
#include "ecs/Transform2DComponent.h"
#include "ecs/RigidBody2DComponent.h"
#include "ecs/EntityManager.h"
#include <algorithm>

namespace fresh
{
namespace ecs
{

Physics2DSystem::Physics2DSystem(EntityManager* manager)
    : SystemBase(manager)
    , gravity_(0.0f, -9.81f)
    , fixedTimeStep_(1.0f / 60.0f)
    , accumulator_(0.0f)
{
}

void Physics2DSystem::initialize()
{
    // Physics system initialization
    accumulator_ = 0.0f;
}

void Physics2DSystem::update(float deltaTime)
{
    if (!enabled)
        return;

    // Fixed timestep physics for stability
    accumulator_ += deltaTime;
    
    while (accumulator_ >= fixedTimeStep_)
    {
        integrateVelocities(fixedTimeStep_);
        applyDrag(fixedTimeStep_);
        integratePositions(fixedTimeStep_);
        
        accumulator_ -= fixedTimeStep_;
    }
}

void Physics2DSystem::setGravity(const glm::vec2& gravity)
{
    gravity_ = gravity;
}

void Physics2DSystem::setFixedTimeStep(float timeStep)
{
    fixedTimeStep_ = std::max(0.001f, timeStep);
}

void Physics2DSystem::integrateVelocities(float deltaTime)
{
    // Get all entities with Transform2D and RigidBody2D
    for (const auto& entity : entityManager->getAllEntities())
    {
        auto* transform = entityManager->getComponent<Transform2DComponent>(entity);
        auto* rigidbody = entityManager->getComponent<RigidBody2DComponent>(entity);
        
        if (!transform || !rigidbody)
            continue;
            
        if (rigidbody->bodyType != RigidBody2DComponent::BodyType::Dynamic)
            continue;
        
        // Apply gravity
        if (rigidbody->useGravity)
        {
            glm::vec2 gravityForce = gravity_ * rigidbody->mass;
            gravityForce.x *= rigidbody->gravityScale.x;
            gravityForce.y *= rigidbody->gravityScale.y;
            rigidbody->addForce(gravityForce);
        }
        
        // Calculate acceleration from forces: F = ma, so a = F/m
        rigidbody->acceleration = rigidbody->force * rigidbody->inverseMass;
        
        // Integrate velocity: v = v0 + a*dt
        rigidbody->velocity += rigidbody->acceleration * deltaTime;
        
        // Apply angular acceleration
        if (!rigidbody->freezeRotation)
        {
            // Simplified: assume uniform moment of inertia
            float angularAcceleration = rigidbody->torque * rigidbody->inverseMass;
            rigidbody->angularVelocity += angularAcceleration * deltaTime;
        }
        else
        {
            rigidbody->angularVelocity = 0.0f;
        }
        
        // Clear forces for next frame
        rigidbody->clearForces();
    }
}

void Physics2DSystem::integratePositions(float deltaTime)
{
    // Update positions based on velocities
    for (const auto& entity : entityManager->getAllEntities())
    {
        auto* transform = entityManager->getComponent<Transform2DComponent>(entity);
        auto* rigidbody = entityManager->getComponent<RigidBody2DComponent>(entity);
        
        if (!transform || !rigidbody)
            continue;
            
        if (rigidbody->bodyType == RigidBody2DComponent::BodyType::Static)
            continue;
        
        // Integrate position: p = p0 + v*dt
        transform->position += rigidbody->velocity * deltaTime;
        
        // Integrate rotation
        if (!rigidbody->freezeRotation)
        {
            transform->rotation += rigidbody->angularVelocity * deltaTime;
        }
    }
}

void Physics2DSystem::applyDrag(float deltaTime)
{
    // Apply drag to slow down objects
    for (const auto& entity : entityManager->getAllEntities())
    {
        auto* rigidbody = entityManager->getComponent<RigidBody2DComponent>(entity);
        
        if (!rigidbody)
            continue;
            
        if (rigidbody->bodyType != RigidBody2DComponent::BodyType::Dynamic)
            continue;
        
        // Linear drag: v = v * (1 - drag * dt)
        float dragFactor = std::max(0.0f, 1.0f - rigidbody->drag * deltaTime);
        rigidbody->velocity *= dragFactor;
        
        // Angular drag
        float angularDragFactor = std::max(0.0f, 1.0f - rigidbody->angularDrag * deltaTime);
        rigidbody->angularVelocity *= angularDragFactor;
    }
}

} // namespace ecs
} // namespace fresh
