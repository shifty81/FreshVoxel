#pragma once
#include <map>
#include <string>
#include <vector>

#include <glm/glm.hpp>

namespace fresh
{

/**
 * @brief Single keyframe in an animation
 */
struct AnimationKeyframe {
    float time;                             // Time in seconds from animation start
    std::map<int, glm::vec3> boneRotations; // Bone index -> rotation (Euler angles in radians)
    std::map<int, glm::vec3> bonePositions; // Bone index -> position offset
};

/**
 * @brief Animation clip containing keyframes
 */
class AnimationClip
{
public:
    AnimationClip();
    explicit AnimationClip(const std::string& clipName);
    ~AnimationClip();

    /**
     * @brief Add a keyframe to the animation
     * @param keyframe Keyframe to add
     */
    void addKeyframe(const AnimationKeyframe& keyframe);

    /**
     * @brief Sample animation at a specific time (with interpolation)
     * @param time Time in seconds
     * @param boneIndex Bone to sample
     * @param outRotation Output rotation
     * @param outPosition Output position
     * @return true if bone was found in animation
     */
    bool sampleBone(float time, int boneIndex, glm::vec3& outRotation,
                    glm::vec3& outPosition) const;

    /**
     * @brief Get animation duration in seconds
     */
    float getDuration() const
    {
        return duration;
    }

    /**
     * @brief Get animation name
     */
    const std::string& getName() const
    {
        return name;
    }

    /**
     * @brief Set whether animation should loop
     */
    void setLooping(bool loop)
    {
        looping = loop;
    }
    bool isLooping() const
    {
        return looping;
    }

private:
    /**
     * @brief Find two keyframes surrounding the given time
     * @param time Time to search for
     * @param outPrev Index of previous keyframe
     * @param outNext Index of next keyframe
     * @return true if keyframes were found
     */
    bool findKeyframes(float time, int& outPrev, int& outNext) const;

    /**
     * @brief Interpolate between two rotation values
     */
    glm::vec3 interpolateRotation(const glm::vec3& a, const glm::vec3& b, float t) const;

    std::string name;
    std::vector<AnimationKeyframe> keyframes;
    float duration;
    bool looping;
};

/**
 * @brief Pre-defined animation types
 */
enum class AnimationType { Idle, Walk, Run, Jump, Crouch, Attack, Block, Custom };

/**
 * @brief Factory for creating standard animation clips
 */
class AnimationClipFactory
{
public:
    /**
     * @brief Create a standard animation clip
     * @param type Type of animation to create
     * @return Animation clip
     */
    static AnimationClip createAnimation(AnimationType type);

    /**
     * @brief Create idle animation (gentle swaying)
     */
    static AnimationClip createIdleAnimation();

    /**
     * @brief Create walk animation (alternating leg movement)
     */
    static AnimationClip createWalkAnimation();

    /**
     * @brief Create run animation (faster walking with more arm swing)
     */
    static AnimationClip createRunAnimation();

    /**
     * @brief Create jump animation (crouch, jump, land)
     */
    static AnimationClip createJumpAnimation();

    /**
     * @brief Create crouch animation (lower body position)
     */
    static AnimationClip createCrouchAnimation();
};

} // namespace fresh
