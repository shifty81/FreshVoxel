#pragma once

#include <glm/glm.hpp>
#include <string>
#include <vector>

namespace fresh
{

/**
 * @brief Describes the relative direction of a hit on a character
 */
enum class HitDirection {
    Front,
    Back,
    Left,
    Right,
    Above,
    Below
};

/**
 * @brief Information about an incoming hit
 *
 * Describes the world-space impact point, force direction and magnitude,
 * the computed relative direction, and the target bone closest to the hit.
 */
struct HitInfo {
    glm::vec3 hitPoint{0.0f};
    glm::vec3 hitDirection{0.0f};
    float force = 0.0f;
    HitDirection direction = HitDirection::Front;
    std::string targetBone;
};

/**
 * @brief An active hit reaction being applied to bones
 *
 * Stores the current rotation offset, intensity that decays over time,
 * and which bone is affected. The force of the original hit scales the
 * intensity of the reaction.
 */
struct HitReaction {
    glm::vec3 rotationOffset{0.0f};
    float intensity = 1.0f;
    float decayRate = 3.0f;
    float force = 1.0f;
    HitDirection direction = HitDirection::Front;
    std::string targetBone;
    bool active = true;
};

/**
 * @brief Procedural hit reaction system for character skeletons
 *
 * Processes incoming hits and produces decaying rotation offsets that
 * can be applied to skeleton bones for reactive animations. Multiple
 * reactions can play simultaneously up to a configurable maximum.
 */
class HitReactionSystem
{
public:
    HitReactionSystem() = default;
    ~HitReactionSystem() = default;

    /**
     * @brief Create a new hit reaction from hit info
     *
     * If the maximum number of reactions is reached, the oldest
     * (lowest intensity) reaction is replaced.
     *
     * @param hit Information about the incoming hit
     */
    void processHit(const HitInfo& hit);

    /**
     * @brief Update all active reactions
     *
     * Decays intensity over time and removes expired reactions.
     *
     * @param deltaTime Time step in seconds
     */
    void update(float deltaTime);

    /** @brief Remove all active reactions */
    void clear();

    /** @brief Check if any reactions are still playing */
    [[nodiscard]] bool hasActiveReactions() const;

    /** @brief Get the number of currently active reactions */
    [[nodiscard]] int getActiveReactionCount() const;

    /** @brief Get read-only access to all reactions */
    [[nodiscard]] const std::vector<HitReaction>& getReactions() const;

    /**
     * @brief Get total rotation offset for a bone from all active reactions
     *
     * Accumulates offsets from all reactions that affect the given bone.
     * Returns the full offset if the reaction targets the specified bone,
     * otherwise returns zero.
     *
     * @param boneName Name of the bone to query
     * @return Accumulated rotation offset vector
     */
    [[nodiscard]] glm::vec3 getBoneReactionOffset(const std::string& boneName) const;

    /** @brief Set maximum number of simultaneous reactions */
    void setMaxReactions(int max);

    /** @brief Get maximum number of simultaneous reactions */
    [[nodiscard]] int getMaxReactions() const;

    /** @brief Set global intensity multiplier for all reactions */
    void setGlobalIntensity(float intensity);

    /** @brief Get global intensity multiplier */
    [[nodiscard]] float getGlobalIntensity() const;

    /**
     * @brief Compute the relative hit direction from character orientation
     *
     * Uses dot products between the character's forward vector and the
     * incoming hit direction to determine which side was hit.
     *
     * @param characterForward Character's forward direction (normalized)
     * @param hitDir Incoming hit direction (normalized)
     * @return Relative HitDirection
     */
    [[nodiscard]] static HitDirection computeHitDirection(const glm::vec3& characterForward,
                                                          const glm::vec3& hitDir);

private:
    glm::vec3 computeReactionOffset(const HitReaction& reaction) const;
    void removeExpiredReactions();

    std::vector<HitReaction> m_reactions;
    int m_maxReactions = 8;
    float m_globalIntensity = 1.0f;
};

} // namespace fresh
