#pragma once
#include <string>
#include <vector>

#include <glm/glm.hpp>

namespace fresh
{
namespace physics
{

/**
 * @brief Single particle in a backpack/item physics simulation
 *
 * Stores position, previous position for Verlet integration,
 * accumulated acceleration, mass, and a pinned flag.
 */
struct BackpackParticle {
    glm::vec3 position{0.0f};
    glm::vec3 previousPosition{0.0f};
    glm::vec3 acceleration{0.0f};
    float mass = 1.0f;
    bool pinned = false;
};

/**
 * @brief Distance constraint between two backpack particles
 */
struct BackpackConstraint {
    int particleA = 0;
    int particleB = 0;
    float restLength = 0.0f;
};

/**
 * @brief A single carried item (backpack, pouch, satchel, etc.) with physics
 *
 * Each item has an anchor particle (follows the character's bone) and a
 * body particle (the bag itself) connected by a distance constraint. The
 * body particle is free to swing under gravity and inertia.
 */
struct BackpackItem {
    std::string name;
    std::string anchorBone;           // Bone the item is strapped to
    glm::vec3 restOffset{0.0f};       // Rest position relative to anchor
    std::vector<BackpackParticle> particles; // [0] = anchor, [1] = body
    std::vector<BackpackConstraint> constraints;
};

/**
 * @brief Verlet-integration physics for items carried by a character
 *
 * Simulates the swinging and bouncing of backpacks, pouches, and other
 * carried items as the character moves. Each item has an anchor particle
 * that follows a character bone and a body particle that hangs freely.
 * Uses the same Verlet integration and constraint satisfaction approach
 * as ClothSimulation.
 */
class BackpackPhysics
{
public:
    BackpackPhysics() = default;
    ~BackpackPhysics() = default;

    /**
     * @brief Add a carried item to the simulation
     *
     * Creates an anchor particle at anchorPosition and a body particle at
     * anchorPosition + item.restOffset with a distance constraint between them.
     *
     * @param item Item definition (name, anchorBone, restOffset)
     * @param anchorPosition Initial world-space anchor position
     */
    void addItem(const BackpackItem& item, const glm::vec3& anchorPosition);

    /**
     * @brief Remove a carried item by name
     * @param name Name of the item to remove
     */
    void removeItem(const std::string& name);

    /**
     * @brief Update the anchor position of a carried item
     *
     * The anchor particle is teleported to the new position without
     * building up residual velocity. Other particles integrate freely.
     *
     * @param name Item name
     * @param anchorPosition New world-space anchor position
     */
    void setAnchorPosition(const std::string& name, const glm::vec3& anchorPosition);

    /**
     * @brief Advance all items by one time step
     * @param deltaTime Time step in seconds
     */
    void update(float deltaTime);

    /**
     * @brief Apply a force to all non-pinned particles in all items (e.g., wind)
     * @param force Force vector to apply
     */
    void applyForce(const glm::vec3& force);

    /**
     * @brief Apply a gravity force every update
     * @param gravity Gravity vector (e.g., {0, -9.81, 0})
     */
    void setGravity(const glm::vec3& gravity);

    /** @brief Get current gravity vector */
    [[nodiscard]] glm::vec3 getGravity() const;

    /** @brief Set velocity damping factor (0 = no damping, 1 = full stop) */
    void setDamping(float d);

    /** @brief Get current damping factor */
    [[nodiscard]] float getDamping() const;

    /** @brief Set constraint solver iterations per update */
    void setConstraintIterations(int n);

    /** @brief Get constraint solver iterations */
    [[nodiscard]] int getConstraintIterations() const;

    /** @brief Get number of currently tracked items */
    [[nodiscard]] int getItemCount() const;

    /**
     * @brief Get world-space anchor position for an item
     * @param name Item name
     * @return Anchor particle position (zero vector if not found)
     */
    [[nodiscard]] glm::vec3 getAnchorPosition(const std::string& name) const;

    /**
     * @brief Get world-space body (bag) position for an item
     * @param name Item name
     * @return Body particle position (zero vector if not found)
     */
    [[nodiscard]] glm::vec3 getBodyPosition(const std::string& name) const;

    /**
     * @brief Get read-only access to the particles of a specific item
     * @param name Item name
     * @return Pointer to particle vector, or nullptr if not found
     */
    [[nodiscard]] const std::vector<BackpackParticle>* getParticles(const std::string& name) const;

    /**
     * @brief Get read-only access to the constraints of a specific item
     * @param name Item name
     * @return Pointer to constraint vector, or nullptr if not found
     */
    [[nodiscard]] const std::vector<BackpackConstraint>* getConstraints(
        const std::string& name) const;

    /**
     * @brief Reset all items to their rest positions with zero velocity
     */
    void reset();

private:
    void verletIntegrate(BackpackItem& item, float dt);
    void satisfyConstraints(BackpackItem& item);

    BackpackItem* findItem(const std::string& name);
    const BackpackItem* findItem(const std::string& name) const;

    std::vector<BackpackItem> m_items;
    glm::vec3 m_gravity{0.0f, -9.81f, 0.0f};
    float m_damping = 0.02f;
    int m_constraintIterations = 5;
};

} // namespace physics
} // namespace fresh
