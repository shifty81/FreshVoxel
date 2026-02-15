#pragma once
#include <map>
#include <memory>
#include <string>

#include "character/AnimationClip.h"
#include "character/VoxelCharacter.h"

namespace fresh
{

/**
 * @brief Animation state in a state machine
 */
struct AnimationState {
    std::string name;
    AnimationClip clip;
    float speed = 1.0f; // Playback speed multiplier
};

/**
 * @brief Transition between animation states
 */
struct AnimationTransition {
    std::string fromState;
    std::string toState;
    float blendDuration = 0.3f; // Duration of blend in seconds

    // Simple condition based on parameter value
    std::string conditionParameter;
    float conditionValue = 0.0f;
    bool conditionGreaterThan = true; // true: >, false: <
};

/**
 * @brief Controller for managing character animations
 *
 * Handles animation playback, blending, and state machine transitions.
 */
class AnimationController
{
public:
    AnimationController();
    ~AnimationController();

    /**
     * @brief Add an animation state
     * @param state Animation state to add
     */
    void addState(const AnimationState& state);

    /**
     * @brief Add a transition between states
     * @param transition Transition to add
     */
    void addTransition(const AnimationTransition& transition);

    /**
     * @brief Set the current animation state
     * @param stateName Name of state to activate
     */
    void setState(const std::string& stateName);

    /**
     * @brief Set a parameter value (for transitions)
     * @param paramName Parameter name
     * @param value Parameter value
     */
    void setParameter(const std::string& paramName, float value);

    /**
     * @brief Get a parameter value
     * @param paramName Parameter name
     * @return Parameter value, or 0.0f if not found
     */
    float getParameter(const std::string& paramName) const;

    /**
     * @brief Update animation and apply to character
     * @param character Character to animate
     * @param deltaTime Time since last update
     */
    void update(VoxelCharacter& character, float deltaTime);

    /**
     * @brief Get current animation time
     */
    float getCurrentTime() const
    {
        return currentTime;
    }

    /**
     * @brief Get current state name
     */
    const std::string& getCurrentState() const
    {
        return currentStateName;
    }

    /**
     * @brief Check if currently blending between animations
     */
    bool isBlending() const
    {
        return blending;
    }

private:
    /**
     * @brief Check if any transitions should occur
     */
    void checkTransitions();

    /**
     * @brief Apply current animation to character
     * @param character Character to animate
     */
    void applyAnimation(VoxelCharacter& character);

    /**
     * @brief Blend between two animations
     * @param character Character to animate
     * @param fromClip Source animation
     * @param toClip Target animation
     * @param blendFactor Blend factor (0-1)
     */
    void blendAnimations(VoxelCharacter& character, const AnimationClip& fromClip,
                         const AnimationClip& toClip, float blendFactor);

    std::map<std::string, AnimationState> states;
    std::vector<AnimationTransition> transitions;
    std::map<std::string, float> parameters;

    std::string currentStateName;
    std::string previousStateName;
    float currentTime;

    bool blending;
    float blendTime;
    float blendDuration;
};

} // namespace fresh
