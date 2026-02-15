#include "physics/PhysicsSystemEnhanced.h"

#include <glm/gtx/quaternion.hpp>

#include "physics/PhysicsComponent.h"
#include "voxelship/VoxelStructureComponent.h"

namespace fresh
{
namespace physics
{

PhysicsSystem::PhysicsSystem(ecs::EntityManager* manager)
    : ecs::SystemBase(manager), gravity(0.0f, -9.81f, 0.0f)
{
}

void PhysicsSystem::initialize()
{
    // Initialize physics system
}

void PhysicsSystem::shutdown()
{
    // Cleanup physics system
}

void PhysicsSystem::update(float deltaTime)
{
    if (!enabled || deltaTime <= 0.0f) {
        return;
    }

    // Get all entities with physics components
    auto entities = entityManager->getEntitiesWithComponent<PhysicsComponent>();

    // Apply forces and integrate motion
    for (auto entity : entities) {
        auto* physics = entityManager->getComponent<PhysicsComponent>(entity);
        if (!physics || physics->isStatic) {
            continue;
        }

        // Apply gravity
        if (physics->useGravity) {
            physics->addForce(gravity * physics->mass);
        }

        // Integrate linear motion
        integrateLinearMotion(physics, deltaTime);

        // Integrate rotational motion
        integrateRotationalMotion(physics, deltaTime);

        // Apply drag
        applyDrag(physics, deltaTime);

        // Clear accumulated forces for next frame
        physics->clearForces();
    }

    // Handle collisions
    handleCollisions();
}

void PhysicsSystem::integrateLinearMotion(PhysicsComponent* physics, float dt)
{
    if (physics->mass > 0.0f) {
        // F = ma => a = F/m
        physics->acceleration = physics->force / physics->mass;

        // Semi-implicit Euler integration
        physics->velocity += physics->acceleration * dt;
        physics->position += physics->velocity * dt;
    }
}

void PhysicsSystem::integrateRotationalMotion(PhysicsComponent* physics, float dt)
{
    if (physics->mass > 0.0f) {
        // Compute angular acceleration: α = I^-1 * τ
        glm::mat3 worldInverseInertia = physics->getWorldInverseInertiaTensor();
        physics->angularAcceleration = worldInverseInertia * physics->torque;

        // Update angular velocity
        physics->angularVelocity += physics->angularAcceleration * dt;

        // Update rotation using quaternion integration
        // dq/dt = 0.5 * ω * q
        glm::quat omegaQuat(0.0f, physics->angularVelocity.x, physics->angularVelocity.y,
                            physics->angularVelocity.z);
        glm::quat dq = 0.5f * omegaQuat * physics->rotation;
        physics->rotation += dq * dt;
        physics->rotation = glm::normalize(physics->rotation);
    }
}

void PhysicsSystem::applyDrag(PhysicsComponent* physics, float dt)
{
    // Linear drag: F_drag = -k * v
    physics->velocity *= (1.0f - physics->drag * dt);

    // Angular drag: τ_drag = -k * ω
    physics->angularVelocity *= (1.0f - physics->angularDrag * dt);
}

void PhysicsSystem::handleCollisions()
{
    // Get all entities with both physics and voxel structure components
    auto entities =
        entityManager
            ->getEntitiesWithComponents<PhysicsComponent, voxelship::VoxelStructureComponent>();

    // Simple O(n²) collision detection
    // TODO: Optimize with spatial partitioning (octree, grid, or sweep-and-prune) for better
    // scalability
    for (size_t i = 0; i < entities.size(); ++i) {
        auto entityA = entities[i];
        auto* physicsA = entityManager->getComponent<PhysicsComponent>(entityA);
        auto* structureA = entityManager->getComponent<voxelship::VoxelStructureComponent>(entityA);

        if (!physicsA || !structureA)
            continue;

        for (size_t j = i + 1; j < entities.size(); ++j) {
            auto entityB = entities[j];
            auto* physicsB = entityManager->getComponent<PhysicsComponent>(entityB);
            auto* structureB =
                entityManager->getComponent<voxelship::VoxelStructureComponent>(entityB);

            if (!physicsB || !structureB)
                continue;

            // Check collision
            if (structureA->checkCollision(*structureB, physicsA->position, physicsB->position)) {
                // Calculate collision normal (simplified - from A to B)
                glm::vec3 normal = glm::normalize(physicsB->position - physicsA->position);

                // Calculate collision point (simplified - midpoint)
                glm::vec3 collisionPoint = (physicsA->position + physicsB->position) * 0.5f;

                // Apply elastic collision response
                applyElasticCollision(physicsA, physicsB, normal, collisionPoint);
            }
        }
    }
}

void PhysicsSystem::applyElasticCollision(PhysicsComponent* a, PhysicsComponent* b,
                                          const glm::vec3& collisionNormal,
                                          const glm::vec3& collisionPoint)
{
    // Skip if either object is static
    if (a->isStatic && b->isStatic) {
        return;
    }

    // Calculate relative velocity at collision point
    glm::vec3 rA = collisionPoint - a->position;
    glm::vec3 rB = collisionPoint - b->position;

    glm::vec3 vA = a->velocity + glm::cross(a->angularVelocity, rA);
    glm::vec3 vB = b->velocity + glm::cross(b->angularVelocity, rB);
    glm::vec3 relativeVelocity = vA - vB;

    // Calculate relative velocity along normal
    float velocityAlongNormal = glm::dot(relativeVelocity, collisionNormal);

    // Objects moving apart - no collision response needed
    if (velocityAlongNormal > 0) {
        return;
    }

    // Calculate restitution (coefficient of restitution)
    float e = (a->restitution + b->restitution) * 0.5f;

    // Calculate impulse scalar
    float invMassA = a->isStatic ? 0.0f : (1.0f / a->mass);
    float invMassB = b->isStatic ? 0.0f : (1.0f / b->mass);

    glm::mat3 invInertiaA = a->isStatic ? glm::mat3(0.0f) : a->getWorldInverseInertiaTensor();
    glm::mat3 invInertiaB = b->isStatic ? glm::mat3(0.0f) : b->getWorldInverseInertiaTensor();

    glm::vec3 rAcrossN = glm::cross(rA, collisionNormal);
    glm::vec3 rBcrossN = glm::cross(rB, collisionNormal);

    float impulseScalar = -(1.0f + e) * velocityAlongNormal;
    impulseScalar /= invMassA + invMassB +
                     glm::dot(collisionNormal, glm::cross(invInertiaA * rAcrossN, rA)) +
                     glm::dot(collisionNormal, glm::cross(invInertiaB * rBcrossN, rB));

    // Apply impulse
    glm::vec3 impulse = impulseScalar * collisionNormal;

    if (!a->isStatic) {
        a->velocity += impulse * invMassA;
        a->angularVelocity += invInertiaA * glm::cross(rA, impulse);
    }

    if (!b->isStatic) {
        b->velocity -= impulse * invMassB;
        b->angularVelocity -= invInertiaB * glm::cross(rB, impulse);
    }
}

} // namespace physics
} // namespace fresh
