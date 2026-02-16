#pragma once
#include "ecs/SystemBase.h"
#include <glm/glm.hpp>

namespace fresh
{
namespace ecs
{

/**
 * @brief System for 2D physics simulation
 *
 * Updates RigidBody2D components with forces, gravity, and velocity.
 * Integrates position and rotation based on physics properties.
 */
class Physics2DSystem : public SystemBase
{
public:
    explicit Physics2DSystem(EntityManager* manager);
    virtual ~Physics2DSystem() = default;

    /**
     * @brief Initialize the physics system
     */
    void initialize() override;

    /**
     * @brief Update physics simulation
     */
    void update(float deltaTime) override;

    /**
     * @brief Set global gravity
     */
    void setGravity(const glm::vec2& gravity);

    /**
     * @brief Get global gravity
     */
    glm::vec2 getGravity() const { return gravity_; }

    /**
     * @brief Set physics time step
     */
    void setFixedTimeStep(float timeStep);

    /**
     * @brief Get physics time step
     */
    float getFixedTimeStep() const { return fixedTimeStep_; }

private:
    glm::vec2 gravity_;             ///< Global gravity vector (e.g., 0, -9.81)
    float fixedTimeStep_;           ///< Fixed physics timestep for stability
    float accumulator_;             ///< Time accumulator for fixed timestep

    /**
     * @brief Apply forces and integrate velocities
     */
    void integrateVelocities(float deltaTime);

    /**
     * @brief Integrate positions from velocities
     */
    void integratePositions(float deltaTime);

    /**
     * @brief Apply drag to velocities
     */
    void applyDrag(float deltaTime);
};

} // namespace ecs
} // namespace fresh
