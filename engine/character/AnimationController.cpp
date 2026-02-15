#include "character/AnimationController.h"

#include <algorithm>

namespace fresh
{

AnimationController::AnimationController()
    : currentTime(0.0f), blending(false), blendTime(0.0f), blendDuration(0.0f)
{
}

AnimationController::~AnimationController() = default;

void AnimationController::addState(const AnimationState& state)
{
    states[state.name] = state;

    // Set as current state if it's the first one
    if (currentStateName.empty()) {
        currentStateName = state.name;
    }
}

void AnimationController::addTransition(const AnimationTransition& transition)
{
    transitions.push_back(transition);
}

void AnimationController::setState(const std::string& stateName)
{
    if (states.find(stateName) == states.end()) {
        return; // State doesn't exist
    }

    if (stateName == currentStateName) {
        return; // Already in this state
    }

    // Start blending to new state
    previousStateName = currentStateName;
    currentStateName = stateName;
    blending = true;
    blendTime = 0.0f;

    // Use default blend duration if not specified
    blendDuration = 0.3f;

    // Find if there's a specific transition with blend duration
    for (const auto& transition : transitions) {
        if (transition.fromState == previousStateName && transition.toState == stateName) {
            blendDuration = transition.blendDuration;
            break;
        }
    }

    // Reset animation time for new state
    currentTime = 0.0f;
}

void AnimationController::setParameter(const std::string& paramName, float value)
{
    parameters[paramName] = value;
}

float AnimationController::getParameter(const std::string& paramName) const
{
    auto it = parameters.find(paramName);
    if (it != parameters.end()) {
        return it->second;
    }
    return 0.0f;
}

void AnimationController::update(VoxelCharacter& character, float deltaTime)
{
    if (currentStateName.empty() || states.find(currentStateName) == states.end()) {
        return;
    }

    // Check for state transitions
    checkTransitions();

    // Update blend time if blending
    if (blending) {
        blendTime += deltaTime;
        if (blendTime >= blendDuration) {
            blending = false;
            blendTime = 0.0f;
        }
    }

    // Update animation time
    AnimationState& currentState = states[currentStateName];
    currentTime += deltaTime * currentState.speed;

    // Handle looping
    if (currentState.clip.isLooping() && currentTime >= currentState.clip.getDuration()) {
        currentTime = 0.0f;
    }

    // Apply animation
    applyAnimation(character);
}

void AnimationController::checkTransitions()
{
    for (const auto& transition : transitions) {
        if (transition.fromState != currentStateName) {
            continue;
        }

        // Check condition
        float paramValue = getParameter(transition.conditionParameter);
        bool conditionMet = false;

        if (transition.conditionGreaterThan) {
            conditionMet = paramValue > transition.conditionValue;
        } else {
            conditionMet = paramValue < transition.conditionValue;
        }

        if (conditionMet) {
            setState(transition.toState);
            break; // Only one transition per frame
        }
    }
}

void AnimationController::applyAnimation(VoxelCharacter& character)
{
    if (currentStateName.empty()) {
        return;
    }

    AnimationState& currentState = states[currentStateName];

    if (blending && !previousStateName.empty() && states.find(previousStateName) != states.end()) {
        // Blend between previous and current animation
        AnimationState& previousState = states[previousStateName];
        float blendFactor = blendTime / blendDuration;
        blendAnimations(character, previousState.clip, currentState.clip, blendFactor);
    } else {
        // Apply current animation only
        const AnimationClip& clip = currentState.clip;
        auto& skeleton = character.getSkeleton();

        for (size_t i = 0; i < skeleton.size(); ++i) {
            glm::vec3 rotation, position;
            if (clip.sampleBone(currentTime, static_cast<int>(i), rotation, position)) {
                character.setBoneRotation(static_cast<int>(i), rotation);
                // Note: Position offset not yet implemented in VoxelCharacter
            }
        }
    }
}

void AnimationController::blendAnimations(VoxelCharacter& character, const AnimationClip& fromClip,
                                          const AnimationClip& toClip, float blendFactor)
{
    auto& skeleton = character.getSkeleton();

    for (size_t i = 0; i < skeleton.size(); ++i) {
        glm::vec3 fromRotation, fromPosition;
        glm::vec3 toRotation, toPosition;

        bool hasFrom =
            fromClip.sampleBone(currentTime, static_cast<int>(i), fromRotation, fromPosition);
        bool hasTo = toClip.sampleBone(currentTime, static_cast<int>(i), toRotation, toPosition);

        if (hasFrom && hasTo) {
            // Blend both rotations
            glm::vec3 blendedRotation = glm::mix(fromRotation, toRotation, blendFactor);
            character.setBoneRotation(static_cast<int>(i), blendedRotation);
        } else if (hasFrom) {
            // Fade out from animation
            glm::vec3 blendedRotation = glm::mix(fromRotation, glm::vec3(0.0f), blendFactor);
            character.setBoneRotation(static_cast<int>(i), blendedRotation);
        } else if (hasTo) {
            // Fade in to animation
            glm::vec3 blendedRotation = glm::mix(glm::vec3(0.0f), toRotation, blendFactor);
            character.setBoneRotation(static_cast<int>(i), blendedRotation);
        }
    }
}

} // namespace fresh
