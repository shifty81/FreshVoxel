#pragma once
#include <glm/glm.hpp>

namespace fresh
{

/**
 * @brief Character environmental state
 *
 * Represents the current relationship between the character and their
 * environment. States are mutually exclusive; the system transitions
 * between them based on the EnvironmentQuery each update.
 */
enum class EnvironmentState {
    Grounded,  // Standing or walking on solid ground
    Airborne,  // In the air (jumped or fell)
    Climbing,  // Attached to a climbable surface (ladder, cliff face)
    Swimming,  // Fully or partially submerged in water
    Ducking    // Crouching / ducking (still grounded)
};

/**
 * @brief Snapshot of environment conditions queried each update
 *
 * The caller fills this from the voxel world and input state and
 * passes it to EnvironmentalInteraction::update().
 */
struct EnvironmentQuery {
    glm::vec3 position{0.0f};        // Character root position in world space
    glm::vec3 velocity{0.0f};        // Current character velocity
    bool isGrounded = false;         // Character is touching solid ground
    bool isInWater = false;          // Position is inside a water voxel
    bool hasClimbableSurface = false;// A climbable surface is in reach
    bool isDuckInput = false;        // Player is holding duck/crouch key
    bool isClimbInput = false;       // Player is pressing toward a climbable surface
};

/**
 * @brief Environmental interaction state machine for characters
 *
 * Handles transitions between Grounded, Airborne, Climbing, Swimming,
 * and Ducking states and provides modified velocity, height, and
 * buoyancy values for use by the character controller.
 *
 * Transition rules (evaluated in priority order each update):
 *  1. Swimming — if isInWater, regardless of other conditions
 *  2. Climbing — if hasClimbableSurface && isClimbInput && not swimming
 *  3. Ducking  — if isDuckInput && isGrounded && not climbing/swimming
 *  4. Grounded — if isGrounded && not ducking/climbing/swimming
 *  5. Airborne — otherwise
 */
class EnvironmentalInteraction
{
public:
    EnvironmentalInteraction() = default;
    ~EnvironmentalInteraction() = default;

    /**
     * @brief Update the interaction state from an environment query
     * @param query Current environment conditions
     * @param deltaTime Time step in seconds
     */
    void update(const EnvironmentQuery& query, float deltaTime);

    /** @brief Get the current environment state */
    [[nodiscard]] EnvironmentState getState() const;

    /** @brief Check whether the character is currently climbing */
    [[nodiscard]] bool isClimbing() const;

    /** @brief Check whether the character is currently swimming */
    [[nodiscard]] bool isSwimming() const;

    /** @brief Check whether the character is currently ducking */
    [[nodiscard]] bool isDucking() const;

    /** @brief Check whether the character is on the ground (Grounded or Ducking) */
    [[nodiscard]] bool isGrounded() const;

    /**
     * @brief Compute a modified velocity that respects the current state
     *
     * - Climbing: horizontal input projected onto the climbable surface
     *   (XZ ignored; vertical scaled by climbSpeed)
     * - Swimming: all axes scaled by swimSpeed fraction
     * - Ducking: horizontal scaled by duckSpeedMultiplier; vertical unchanged
     * - Grounded/Airborne: input velocity returned unchanged
     *
     * @param inputVelocity Desired velocity from character controller
     * @return Modified velocity
     */
    [[nodiscard]] glm::vec3 getModifiedVelocity(const glm::vec3& inputVelocity) const;

    /**
     * @brief Height multiplier for the character collision capsule
     *
     * Returns duckHeightMultiplier when Ducking, 1.0 otherwise.
     */
    [[nodiscard]] float getHeightMultiplier() const;

    /**
     * @brief Upward buoyancy force applied when Swimming
     *
     * Returns the configured buoyancy value when in Swimming state, 0 otherwise.
     */
    [[nodiscard]] float getBuoyancy() const;

    // ── Configuration ────────────────────────────────────────────────────────

    /** @brief Set climb movement speed (units/second along vertical axis) */
    void setClimbSpeed(float speed);

    /** @brief Get configured climb speed */
    [[nodiscard]] float getClimbSpeed() const;

    /** @brief Set swim speed as a fraction of normal speed (0–1) */
    void setSwimSpeed(float speed);

    /** @brief Get configured swim speed fraction */
    [[nodiscard]] float getSwimSpeed() const;

    /** @brief Set horizontal speed multiplier while ducking (0–1) */
    void setDuckSpeedMultiplier(float multiplier);

    /** @brief Get configured duck speed multiplier */
    [[nodiscard]] float getDuckSpeedMultiplier() const;

    /** @brief Set the height multiplier applied when ducking (0–1) */
    void setDuckHeightMultiplier(float multiplier);

    /** @brief Get configured duck height multiplier */
    [[nodiscard]] float getDuckHeightMultiplier() const;

    /** @brief Set the upward buoyancy force applied in water */
    void setBuoyancy(float buoyancy);

    /** @brief Get configured buoyancy force */
    [[nodiscard]] float getBuoyancyValue() const;

private:
    EnvironmentState m_state = EnvironmentState::Grounded;

    float m_climbSpeed = 3.0f;
    float m_swimSpeed = 0.6f;
    float m_duckSpeedMultiplier = 0.5f;
    float m_duckHeightMultiplier = 0.5f;
    float m_buoyancy = 9.81f; // Balances gravity at water surface by default
};

} // namespace fresh
