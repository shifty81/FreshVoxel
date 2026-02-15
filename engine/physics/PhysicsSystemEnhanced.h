#pragma once
#include <glm/glm.hpp>

#include "ecs/SystemBase.h"
#include "PhysicsComponent.h"

namespace fresh
{
namespace physics
{

/**
 * @brief System that handles Newtonian physics simulation
 *
 * Processes entities with PhysicsComponent, applying forces, acceleration,
 * velocity, drag, and collision detection/response.
 */
class PhysicsSystem : public ecs::SystemBase
{
public:
    explicit PhysicsSystem(ecs::EntityManager* manager);
    virtual ~PhysicsSystem() = default;

    void update(float deltaTime) override;
    void initialize() override;
    void shutdown() override;

    // Global physics settings
    void setGravity(const glm::vec3& g)
    {
        gravity = g;
    }
    glm::vec3 getGravity() const
    {
        return gravity;
    }

    // Collision detection and response
    void handleCollisions();

    // Apply elastic collision response between two physics objects
    void applyElasticCollision(PhysicsComponent* a, PhysicsComponent* b,
                               const glm::vec3& collisionNormal, const glm::vec3& collisionPoint);

private:
    glm::vec3 gravity;

    void integrateLinearMotion(PhysicsComponent* physics, float dt);
    void integrateRotationalMotion(PhysicsComponent* physics, float dt);
    void applyDrag(PhysicsComponent* physics, float dt);
};

} // namespace physics
} // namespace fresh
