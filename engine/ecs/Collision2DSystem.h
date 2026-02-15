#pragma once
#include "ecs/SystemBase.h"
#include <glm/glm.hpp>
#include <vector>
#include <functional>

namespace fresh
{
namespace ecs
{

// Forward declarations
class Transform2DComponent;
class Collider2DComponent;
class RigidBody2DComponent;

/**
 * @brief Collision information structure
 */
struct Collision2D
{
    Entity entityA;             ///< First entity
    Entity entityB;             ///< Second entity
    glm::vec2 point;            ///< Contact point
    glm::vec2 normal;           ///< Collision normal (from A to B)
    float penetration;          ///< Penetration depth
    bool isTrigger;             ///< True if either collider is a trigger
};

/**
 * @brief System for 2D collision detection and response
 *
 * Detects collisions between Collider2D components and resolves them
 * for RigidBody2D physics objects.
 */
class Collision2DSystem : public SystemBase
{
public:
    using CollisionCallback = std::function<void(const Collision2D&)>;

    explicit Collision2DSystem(EntityManager* manager);
    virtual ~Collision2DSystem() = default;

    /**
     * @brief Initialize collision system
     */
    void initialize() override;

    /**
     * @brief Update collision detection and resolution
     */
    void update(float deltaTime) override;

    /**
     * @brief Register a callback for collision events
     */
    void onCollision(CollisionCallback callback);

    /**
     * @brief Register a callback for trigger events
     */
    void onTrigger(CollisionCallback callback);

private:
    std::vector<CollisionCallback> collisionCallbacks_;
    std::vector<CollisionCallback> triggerCallbacks_;
    std::vector<Collision2D> currentCollisions_;

    /**
     * @brief Broad phase: find potential collision pairs using AABB
     */
    std::vector<std::pair<Entity, Entity>> broadPhase();

    /**
     * @brief Narrow phase: detailed collision detection
     */
    bool narrowPhase(Entity entityA, Entity entityB, Collision2D& collision);

    /**
     * @brief Resolve collision with physics response
     */
    void resolveCollision(const Collision2D& collision);

    /**
     * @brief Detect collision between two boxes
     */
    bool checkBoxBox(const Transform2DComponent* transformA, const Collider2DComponent* colliderA,
                     const Transform2DComponent* transformB, const Collider2DComponent* colliderB,
                     Collision2D& collision);

    /**
     * @brief Detect collision between two circles
     */
    bool checkCircleCircle(const Transform2DComponent* transformA, const Collider2DComponent* colliderA,
                          const Transform2DComponent* transformB, const Collider2DComponent* colliderB,
                          Collision2D& collision);

    /**
     * @brief Detect collision between box and circle
     */
    bool checkBoxCircle(const Transform2DComponent* transformA, const Collider2DComponent* colliderA,
                       const Transform2DComponent* transformB, const Collider2DComponent* colliderB,
                       Collision2D& collision);
};

} // namespace ecs
} // namespace fresh
