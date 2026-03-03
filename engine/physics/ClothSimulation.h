#pragma once
#include <vector>

#include <glm/glm.hpp>

namespace fresh
{
namespace physics
{

/**
 * @brief Single particle in a cloth simulation
 *
 * Stores position, previous position for Verlet integration,
 * accumulated acceleration, mass, and a pinned flag.
 */
struct ClothParticle {
    glm::vec3 position{0.0f};
    glm::vec3 previousPosition{0.0f};
    glm::vec3 acceleration{0.0f};
    float mass = 1.0f;
    bool pinned = false;
};

/**
 * @brief Distance constraint between two cloth particles
 *
 * Maintains a rest length between two particles identified by index.
 */
struct ClothConstraint {
    int particleA = 0;
    int particleB = 0;
    float restLength = 0.0f;
};

/**
 * @brief Verlet-integration particle-based cloth simulation
 *
 * Creates a rectangular grid of particles connected by structural
 * (horizontal/vertical) and shear (diagonal) constraints. Particles
 * are integrated using Verlet integration with configurable damping
 * and iterative constraint satisfaction.
 */
class ClothSimulation
{
public:
    /**
     * @brief Construct a cloth simulation grid
     * @param width Number of particles along the X axis
     * @param height Number of particles along the Y axis
     * @param spacing Distance between adjacent particles
     */
    ClothSimulation(int width, int height, float spacing);
    ~ClothSimulation() = default;

    /**
     * @brief Advance the simulation by one time step
     * @param deltaTime Time step in seconds
     */
    void update(float deltaTime);

    /**
     * @brief Apply a force to all non-pinned particles (e.g., gravity, wind)
     * @param force Force vector to apply
     */
    void applyForce(const glm::vec3& force);

    /**
     * @brief Pin a particle so it remains stationary
     * @param index Particle index
     */
    void pinParticle(int index);

    /**
     * @brief Unpin a particle so it can move freely
     * @param index Particle index
     */
    void unpinParticle(int index);

    /**
     * @brief Set the position of a particle
     * @param index Particle index
     * @param pos New position
     */
    void setParticlePosition(int index, const glm::vec3& pos);

    /** @brief Get read-only access to all particles */
    [[nodiscard]] const std::vector<ClothParticle>& getParticles() const;

    /** @brief Get read-only access to all constraints */
    [[nodiscard]] const std::vector<ClothConstraint>& getConstraints() const;

    /** @brief Get grid width in particles */
    [[nodiscard]] int getWidth() const;

    /** @brief Get grid height in particles */
    [[nodiscard]] int getHeight() const;

    /**
     * @brief Convert grid coordinates to a linear particle index
     * @param x Column (0-based)
     * @param y Row (0-based)
     * @return Linear index into the particle array
     */
    [[nodiscard]] int getParticleIndex(int x, int y) const;

    /** @brief Set velocity damping factor (0 = no damping, 1 = full damping) */
    void setDamping(float d);

    /** @brief Get current damping factor */
    [[nodiscard]] float getDamping() const;

    /** @brief Set number of constraint solver iterations per update */
    void setConstraintIterations(int n);

    /** @brief Get number of constraint solver iterations */
    [[nodiscard]] int getConstraintIterations() const;

    /**
     * @brief Reset all particles to their initial positions and zero velocities
     */
    void reset();

private:
    void satisfyConstraints();
    void verletIntegrate(float dt);

    std::vector<ClothParticle> m_particles;
    std::vector<ClothConstraint> m_constraints;
    std::vector<glm::vec3> m_initialPositions;
    int m_width;
    int m_height;
    float m_spacing;
    float m_damping;
    int m_constraintIterations;
};

} // namespace physics
} // namespace fresh
