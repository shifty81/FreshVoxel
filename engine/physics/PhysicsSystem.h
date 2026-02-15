#pragma once

namespace fresh
{

/**
 * @brief Basic physics system for player movement and collisions
 */
class PhysicsSystem
{
public:
    PhysicsSystem();
    ~PhysicsSystem();

    /**
     * @brief Initialize physics system
     * @return true if successful
     */
    bool initialize();

    /**
     * @brief Update physics simulation
     * @param deltaTime Time since last update
     */
    void update(float deltaTime);
};

} // namespace fresh
