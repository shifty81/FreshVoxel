#pragma once

#include "character/HumanoidSkeleton.h"

#include <glm/glm.hpp>
#include <string>
#include <vector>

namespace fresh
{

/**
 * @brief Single rigid body in a ragdoll simulation
 *
 * Stores position, previous position for Verlet integration,
 * accumulated acceleration, mass, and a pinned flag. Each body
 * corresponds to a bone in the character skeleton.
 */
struct RagdollBody {
    std::string boneName;
    glm::vec3 position{0.0f};
    glm::vec3 previousPosition{0.0f};
    glm::vec3 acceleration{0.0f};
    float mass = 1.0f;
    bool pinned = false;
};

/**
 * @brief Distance constraint between two ragdoll bodies
 *
 * Maintains a rest length between two bodies identified by index,
 * corresponding to a parent-child bone relationship.
 */
struct RagdollConstraint {
    int bodyA = 0;
    int bodyB = 0;
    float restLength = 0.0f;
};

/**
 * @brief Verlet-integration ragdoll physics for humanoid skeletons
 *
 * Creates a set of rigid bodies from the skeleton bone hierarchy and
 * connects them with distance constraints. Bodies are integrated using
 * Verlet integration with configurable damping and iterative constraint
 * satisfaction, following the same pattern as ClothSimulation.
 */
class RagdollSystem
{
public:
    RagdollSystem() = default;
    ~RagdollSystem() = default;

    /**
     * @brief Initialize the ragdoll from a humanoid skeleton
     *
     * Creates a body for each bone at its world position and a distance
     * constraint for each parent-child bone relationship.
     *
     * @param skeleton The humanoid skeleton to build the ragdoll from
     */
    void initialize(const HumanoidSkeleton& skeleton);

    /**
     * @brief Advance the simulation by one time step
     * @param deltaTime Time step in seconds
     */
    void update(float deltaTime);

    /**
     * @brief Activate ragdoll at a given world position
     *
     * Translates all bodies so the root body is at rootPosition and
     * resets previous positions to eliminate residual velocity.
     *
     * @param rootPosition World position for the root body
     */
    void activate(const glm::vec3& rootPosition);

    /**
     * @brief Deactivate the ragdoll simulation
     */
    void deactivate();

    /** @brief Check whether the ragdoll simulation is active */
    [[nodiscard]] bool isActive() const;

    /**
     * @brief Apply a force to all non-pinned bodies (e.g., gravity, wind)
     * @param force Force vector to apply
     */
    void applyForce(const glm::vec3& force);

    /**
     * @brief Apply an instant velocity change to a specific body
     *
     * Since Verlet integration derives velocity from the position delta,
     * the impulse is applied by directly offsetting the body's position.
     *
     * @param bodyIndex Index of the body to affect
     * @param impulse Impulse vector to apply
     */
    void applyImpulse(int bodyIndex, const glm::vec3& impulse);

    /**
     * @brief Apply an impulse to all bodies near a world-space point
     *
     * Bodies within the given radius receive an impulse scaled by
     * (1 - distance / radius) for a linear falloff.
     *
     * @param point World-space origin of the impulse
     * @param impulse Maximum impulse vector at the origin
     * @param radius Radius of effect
     */
    void applyImpulseAtPoint(const glm::vec3& point, const glm::vec3& impulse, float radius);

    /** @brief Get read-only access to all bodies */
    [[nodiscard]] const std::vector<RagdollBody>& getBodies() const;

    /** @brief Get read-only access to all constraints */
    [[nodiscard]] const std::vector<RagdollConstraint>& getConstraints() const;

    /**
     * @brief Find a body index by bone name
     * @param boneName Name of the bone to search for
     * @return Index into the bodies vector, or -1 if not found
     */
    [[nodiscard]] int getBodyIndex(const std::string& boneName) const;

    /**
     * @brief Get the world position of a body by bone name
     * @param boneName Name of the bone to look up
     * @return World position of the body (zero vector if not found)
     */
    [[nodiscard]] glm::vec3 getBodyPosition(const std::string& boneName) const;

    /** @brief Set velocity damping factor (0 = no damping, 1 = full damping) */
    void setDamping(float d);

    /** @brief Get current damping factor */
    [[nodiscard]] float getDamping() const;

    /** @brief Set number of constraint solver iterations per update */
    void setConstraintIterations(int n);

    /** @brief Get number of constraint solver iterations */
    [[nodiscard]] int getConstraintIterations() const;

    /** @brief Set gravity vector applied each update */
    void setGravity(const glm::vec3& g);

    /** @brief Get current gravity vector */
    [[nodiscard]] glm::vec3 getGravity() const;

private:
    void verletIntegrate(float dt);
    void satisfyConstraints();

    /**
     * @brief Recursively walk the bone hierarchy and create bodies/constraints
     * @param bone Current bone to process
     * @param parentIndex Index of the parent body (-1 for root)
     */
    void buildFromBone(const Bone* bone, int parentIndex);

    std::vector<RagdollBody> m_bodies;
    std::vector<RagdollConstraint> m_constraints;

    bool m_active = false;
    float m_damping = 0.02f;
    int m_constraintIterations = 8;
    glm::vec3 m_gravity{0.0f, -9.81f, 0.0f};
};

} // namespace fresh
