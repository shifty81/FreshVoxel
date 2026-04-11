#pragma once
#include <string>
#include <vector>

#include <glm/glm.hpp>

namespace fresh
{
namespace physics
{

/**
 * @brief Single particle in a hair strand
 *
 * Stores position, previous position for Verlet integration,
 * accumulated acceleration, mass, and a pinned flag. The root
 * particle of each strand is pinned to follow the character's head.
 */
struct HairParticle {
    glm::vec3 position{0.0f};
    glm::vec3 previousPosition{0.0f};
    glm::vec3 acceleration{0.0f};
    float mass = 0.1f;
    bool pinned = false;
};

/**
 * @brief A single hair strand composed of a chain of particles
 *
 * Particle 0 is the root (pinned to the character's head bone).
 * Each subsequent particle is connected to the previous one by a
 * distance constraint of length segmentLength.
 */
struct HairStrand {
    std::string name;
    std::vector<HairParticle> particles;
    float segmentLength = 0.2f;   // Rest length between consecutive particles
};

/**
 * @brief Strand-based Verlet-integration hair physics simulation
 *
 * Manages a collection of hair strands, each modelled as a chain of
 * particles connected by distance constraints. The root of every strand
 * is pinned and updated each frame to follow the character's head bone.
 * All other particles are integrated freely under gravity, wind, and
 * other applied forces. The same Verlet integration and iterative
 * constraint satisfaction used in ClothSimulation is applied here.
 */
class HairPhysics
{
public:
    HairPhysics() = default;
    ~HairPhysics() = default;

    /**
     * @brief Add a hair strand starting at rootPosition
     *
     * Particles are placed in a chain along the -Y axis with segmentLength
     * spacing. The root particle (index 0) is pinned.
     *
     * @param strandName Unique name for the strand
     * @param rootPosition World-space root position (head attachment point)
     * @param particleCount Number of particles in the strand (minimum 2)
     * @param segmentLength Rest distance between consecutive particles
     */
    void addStrand(const std::string& strandName, const glm::vec3& rootPosition,
                   int particleCount, float segmentLength);

    /**
     * @brief Remove a strand by name
     * @param strandName Name of the strand to remove
     */
    void removeStrand(const std::string& strandName);

    /**
     * @brief Update the root position of a strand to follow the character
     *
     * Teleports the root particle to rootPosition without accumulating
     * velocity.
     *
     * @param strandName Strand to update
     * @param rootPosition New world-space root position
     */
    void setRootPosition(const std::string& strandName, const glm::vec3& rootPosition);

    /**
     * @brief Advance all strands by one time step
     * @param deltaTime Time step in seconds
     */
    void update(float deltaTime);

    /**
     * @brief Apply a force to all non-pinned particles in all strands
     * @param force Force vector to apply (e.g., wind)
     */
    void applyForce(const glm::vec3& force);

    /** @brief Set gravity vector applied each update */
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

    /** @brief Get total number of strands */
    [[nodiscard]] int getStrandCount() const;

    /**
     * @brief Get read-only access to a strand by name
     * @param strandName Strand to retrieve
     * @return Pointer to strand, or nullptr if not found
     */
    [[nodiscard]] const HairStrand* getStrand(const std::string& strandName) const;

    /**
     * @brief Get read-only access to all strands
     */
    [[nodiscard]] const std::vector<HairStrand>& getStrands() const;

    /**
     * @brief Reset all strands to their initial chain layout with zero velocity
     *
     * Particles are redistributed along the -Y axis from the current root
     * position with zero velocity.
     */
    void reset();

private:
    void verletIntegrate(HairStrand& strand, float dt);
    void satisfyConstraints(HairStrand& strand);

    HairStrand* findStrand(const std::string& name);
    const HairStrand* findStrand(const std::string& name) const;

    std::vector<HairStrand> m_strands;
    glm::vec3 m_gravity{0.0f, -9.81f, 0.0f};
    float m_damping = 0.01f;
    int m_constraintIterations = 5;
};

} // namespace physics
} // namespace fresh
