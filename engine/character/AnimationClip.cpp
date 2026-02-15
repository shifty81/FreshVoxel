#include "character/AnimationClip.h"

#include <algorithm>
#include <cmath>

namespace fresh
{

AnimationClip::AnimationClip() : name("Unnamed"), duration(0.0f), looping(false) {}

AnimationClip::AnimationClip(const std::string& clipName)
    : name(clipName), duration(0.0f), looping(false)
{
}

AnimationClip::~AnimationClip() = default;

void AnimationClip::addKeyframe(const AnimationKeyframe& keyframe)
{
    keyframes.push_back(keyframe);

    // Sort keyframes by time
    std::sort(
        keyframes.begin(), keyframes.end(),
        [](const AnimationKeyframe& a, const AnimationKeyframe& b) { return a.time < b.time; });

    // Update duration
    if (!keyframes.empty()) {
        duration = keyframes.back().time;
    }
}

bool AnimationClip::sampleBone(float time, int boneIndex, glm::vec3& outRotation,
                               glm::vec3& outPosition) const
{
    if (keyframes.empty()) {
        return false;
    }

    // Handle looping
    if (looping && duration > 0.0f) {
        time = std::fmod(time, duration);
    }

    // Clamp to animation range
    time = std::clamp(time, 0.0f, duration);

    // Find surrounding keyframes
    int prevIndex, nextIndex;
    if (!findKeyframes(time, prevIndex, nextIndex)) {
        return false;
    }

    const AnimationKeyframe& prevFrame = keyframes[prevIndex];
    const AnimationKeyframe& nextFrame = keyframes[nextIndex];

    // Check if bone exists in keyframes
    auto prevRotIt = prevFrame.boneRotations.find(boneIndex);
    auto nextRotIt = nextFrame.boneRotations.find(boneIndex);
    auto prevPosIt = prevFrame.bonePositions.find(boneIndex);
    auto nextPosIt = nextFrame.bonePositions.find(boneIndex);

    if (prevRotIt == prevFrame.boneRotations.end() && nextRotIt == nextFrame.boneRotations.end()) {
        return false; // Bone not in animation
    }

    // Calculate interpolation factor
    float t = 0.0f;
    if (nextFrame.time > prevFrame.time) {
        t = (time - prevFrame.time) / (nextFrame.time - prevFrame.time);
    }

    // Interpolate rotation
    if (prevRotIt != prevFrame.boneRotations.end() && nextRotIt != nextFrame.boneRotations.end()) {
        outRotation = interpolateRotation(prevRotIt->second, nextRotIt->second, t);
    } else if (prevRotIt != prevFrame.boneRotations.end()) {
        outRotation = prevRotIt->second;
    } else {
        outRotation = nextRotIt->second;
    }

    // Interpolate position
    if (prevPosIt != prevFrame.bonePositions.end() && nextPosIt != nextFrame.bonePositions.end()) {
        outPosition = glm::mix(prevPosIt->second, nextPosIt->second, t);
    } else if (prevPosIt != prevFrame.bonePositions.end()) {
        outPosition = prevPosIt->second;
    } else if (nextPosIt != nextFrame.bonePositions.end()) {
        outPosition = nextPosIt->second;
    } else {
        outPosition = glm::vec3(0.0f);
    }

    return true;
}

bool AnimationClip::findKeyframes(float time, int& outPrev, int& outNext) const
{
    if (keyframes.empty()) {
        return false;
    }

    // Before first keyframe
    if (time <= keyframes[0].time) {
        outPrev = outNext = 0;
        return true;
    }

    // After last keyframe
    if (time >= keyframes.back().time) {
        outPrev = outNext = static_cast<int>(keyframes.size()) - 1;
        return true;
    }

    // Find surrounding keyframes
    for (size_t i = 0; i < keyframes.size() - 1; ++i) {
        if (time >= keyframes[i].time && time < keyframes[i + 1].time) {
            outPrev = static_cast<int>(i);
            outNext = static_cast<int>(i + 1);
            return true;
        }
    }

    return false;
}

glm::vec3 AnimationClip::interpolateRotation(const glm::vec3& a, const glm::vec3& b, float t) const
{
    // Simple linear interpolation for Euler angles
    // Note: For production, should use quaternion slerp to avoid gimbal lock
    return glm::mix(a, b, t);
}

// Factory methods

AnimationClip AnimationClipFactory::createAnimation(AnimationType type)
{
    switch (type) {
    case AnimationType::Idle:
        return createIdleAnimation();
    case AnimationType::Walk:
        return createWalkAnimation();
    case AnimationType::Run:
        return createRunAnimation();
    case AnimationType::Jump:
        return createJumpAnimation();
    case AnimationType::Crouch:
        return createCrouchAnimation();
    default:
        return AnimationClip("Custom");
    }
}

AnimationClip AnimationClipFactory::createIdleAnimation()
{
    AnimationClip clip("Idle");
    clip.setLooping(true);

    const float swayAmount = 0.05f;

    // Keyframe 0: Start position (0.0s)
    {
        AnimationKeyframe frame;
        frame.time = 0.0f;
        frame.boneRotations[1] = glm::vec3(0.0f, 0.0f, 0.0f); // Spine
        frame.boneRotations[3] = glm::vec3(0.0f, 0.0f, 0.0f); // Head
        clip.addKeyframe(frame);
    }

    // Keyframe 1: Slight sway right (1.0s)
    {
        AnimationKeyframe frame;
        frame.time = 1.0f;
        frame.boneRotations[1] = glm::vec3(0.0f, swayAmount, 0.0f);
        frame.boneRotations[3] = glm::vec3(0.0f, -swayAmount * 0.5f, 0.0f);
        clip.addKeyframe(frame);
    }

    // Keyframe 2: Back to center (2.0s)
    {
        AnimationKeyframe frame;
        frame.time = 2.0f;
        frame.boneRotations[1] = glm::vec3(0.0f, 0.0f, 0.0f);
        frame.boneRotations[3] = glm::vec3(0.0f, 0.0f, 0.0f);
        clip.addKeyframe(frame);
    }

    // Keyframe 3: Slight sway left (3.0s)
    {
        AnimationKeyframe frame;
        frame.time = 3.0f;
        frame.boneRotations[1] = glm::vec3(0.0f, -swayAmount, 0.0f);
        frame.boneRotations[3] = glm::vec3(0.0f, swayAmount * 0.5f, 0.0f);
        clip.addKeyframe(frame);
    }

    // Keyframe 4: Back to start (4.0s)
    {
        AnimationKeyframe frame;
        frame.time = 4.0f;
        frame.boneRotations[1] = glm::vec3(0.0f, 0.0f, 0.0f);
        frame.boneRotations[3] = glm::vec3(0.0f, 0.0f, 0.0f);
        clip.addKeyframe(frame);
    }

    return clip;
}

AnimationClip AnimationClipFactory::createWalkAnimation()
{
    AnimationClip clip("Walk");
    clip.setLooping(true);

    const float legSwing = 0.6f;
    const float armSwing = 0.4f;

    // Keyframe 0: Left leg forward, right leg back (0.0s)
    {
        AnimationKeyframe frame;
        frame.time = 0.0f;
        frame.boneRotations[11] = glm::vec3(legSwing, 0.0f, 0.0f);  // LeftLeg forward
        frame.boneRotations[14] = glm::vec3(-legSwing, 0.0f, 0.0f); // RightLeg back
        frame.boneRotations[5] = glm::vec3(-armSwing, 0.0f, 0.0f);  // LeftArm back
        frame.boneRotations[8] = glm::vec3(armSwing, 0.0f, 0.0f);   // RightArm forward
        clip.addKeyframe(frame);
    }

    // Keyframe 1: Legs passing (0.25s)
    {
        AnimationKeyframe frame;
        frame.time = 0.25f;
        frame.boneRotations[11] = glm::vec3(0.0f, 0.0f, 0.0f);
        frame.boneRotations[14] = glm::vec3(0.0f, 0.0f, 0.0f);
        frame.boneRotations[5] = glm::vec3(0.0f, 0.0f, 0.0f);
        frame.boneRotations[8] = glm::vec3(0.0f, 0.0f, 0.0f);
        clip.addKeyframe(frame);
    }

    // Keyframe 2: Right leg forward, left leg back (0.5s)
    {
        AnimationKeyframe frame;
        frame.time = 0.5f;
        frame.boneRotations[11] = glm::vec3(-legSwing, 0.0f, 0.0f); // LeftLeg back
        frame.boneRotations[14] = glm::vec3(legSwing, 0.0f, 0.0f);  // RightLeg forward
        frame.boneRotations[5] = glm::vec3(armSwing, 0.0f, 0.0f);   // LeftArm forward
        frame.boneRotations[8] = glm::vec3(-armSwing, 0.0f, 0.0f);  // RightArm back
        clip.addKeyframe(frame);
    }

    // Keyframe 3: Legs passing again (0.75s)
    {
        AnimationKeyframe frame;
        frame.time = 0.75f;
        frame.boneRotations[11] = glm::vec3(0.0f, 0.0f, 0.0f);
        frame.boneRotations[14] = glm::vec3(0.0f, 0.0f, 0.0f);
        frame.boneRotations[5] = glm::vec3(0.0f, 0.0f, 0.0f);
        frame.boneRotations[8] = glm::vec3(0.0f, 0.0f, 0.0f);
        clip.addKeyframe(frame);
    }

    // Keyframe 4: Back to start (1.0s)
    {
        AnimationKeyframe frame;
        frame.time = 1.0f;
        frame.boneRotations[11] = glm::vec3(legSwing, 0.0f, 0.0f);
        frame.boneRotations[14] = glm::vec3(-legSwing, 0.0f, 0.0f);
        frame.boneRotations[5] = glm::vec3(-armSwing, 0.0f, 0.0f);
        frame.boneRotations[8] = glm::vec3(armSwing, 0.0f, 0.0f);
        clip.addKeyframe(frame);
    }

    return clip;
}

AnimationClip AnimationClipFactory::createRunAnimation()
{
    AnimationClip clip("Run");
    clip.setLooping(true);

    const float legSwing = 1.0f; // More than walk
    const float armSwing = 0.8f; // More than walk
    const float bodyLean = 0.2f; // Forward lean

    // Similar to walk but faster and more exaggerated
    // Keyframe 0: Left leg forward (0.0s)
    {
        AnimationKeyframe frame;
        frame.time = 0.0f;
        frame.boneRotations[1] = glm::vec3(bodyLean, 0.0f, 0.0f); // Spine lean
        frame.boneRotations[11] = glm::vec3(legSwing, 0.0f, 0.0f);
        frame.boneRotations[14] = glm::vec3(-legSwing, 0.0f, 0.0f);
        frame.boneRotations[5] = glm::vec3(-armSwing, 0.0f, 0.0f);
        frame.boneRotations[8] = glm::vec3(armSwing, 0.0f, 0.0f);
        clip.addKeyframe(frame);
    }

    // Keyframe 1: Right leg forward (0.3s) - faster than walk
    {
        AnimationKeyframe frame;
        frame.time = 0.3f;
        frame.boneRotations[1] = glm::vec3(bodyLean, 0.0f, 0.0f);
        frame.boneRotations[11] = glm::vec3(-legSwing, 0.0f, 0.0f);
        frame.boneRotations[14] = glm::vec3(legSwing, 0.0f, 0.0f);
        frame.boneRotations[5] = glm::vec3(armSwing, 0.0f, 0.0f);
        frame.boneRotations[8] = glm::vec3(-armSwing, 0.0f, 0.0f);
        clip.addKeyframe(frame);
    }

    // Keyframe 2: Back to start (0.6s)
    {
        AnimationKeyframe frame;
        frame.time = 0.6f;
        frame.boneRotations[1] = glm::vec3(bodyLean, 0.0f, 0.0f);
        frame.boneRotations[11] = glm::vec3(legSwing, 0.0f, 0.0f);
        frame.boneRotations[14] = glm::vec3(-legSwing, 0.0f, 0.0f);
        frame.boneRotations[5] = glm::vec3(-armSwing, 0.0f, 0.0f);
        frame.boneRotations[8] = glm::vec3(armSwing, 0.0f, 0.0f);
        clip.addKeyframe(frame);
    }

    return clip;
}

AnimationClip AnimationClipFactory::createJumpAnimation()
{
    AnimationClip clip("Jump");
    clip.setLooping(false);

    // Keyframe 0: Crouch (0.0s)
    {
        AnimationKeyframe frame;
        frame.time = 0.0f;
        frame.boneRotations[1] = glm::vec3(0.3f, 0.0f, 0.0f);   // Spine bend
        frame.boneRotations[11] = glm::vec3(-0.5f, 0.0f, 0.0f); // Legs bent
        frame.boneRotations[14] = glm::vec3(-0.5f, 0.0f, 0.0f);
        frame.bonePositions[0] = glm::vec3(0.0f, -1.0f, 0.0f); // Root lower
        clip.addKeyframe(frame);
    }

    // Keyframe 1: Launch (0.2s)
    {
        AnimationKeyframe frame;
        frame.time = 0.2f;
        frame.boneRotations[1] = glm::vec3(-0.2f, 0.0f, 0.0f); // Spine extend
        frame.boneRotations[11] = glm::vec3(0.2f, 0.0f, 0.0f); // Legs extend
        frame.boneRotations[14] = glm::vec3(0.2f, 0.0f, 0.0f);
        frame.boneRotations[5] = glm::vec3(0.0f, 0.0f, -0.5f); // Arms up
        frame.boneRotations[8] = glm::vec3(0.0f, 0.0f, 0.5f);
        frame.bonePositions[0] = glm::vec3(0.0f, 0.5f, 0.0f);
        clip.addKeyframe(frame);
    }

    // Keyframe 2: Peak (0.5s)
    {
        AnimationKeyframe frame;
        frame.time = 0.5f;
        frame.boneRotations[1] = glm::vec3(0.0f, 0.0f, 0.0f);
        frame.boneRotations[11] = glm::vec3(0.0f, 0.0f, 0.0f);
        frame.boneRotations[14] = glm::vec3(0.0f, 0.0f, 0.0f);
        frame.boneRotations[5] = glm::vec3(0.0f, 0.0f, -0.3f);
        frame.boneRotations[8] = glm::vec3(0.0f, 0.0f, 0.3f);
        frame.bonePositions[0] = glm::vec3(0.0f, 0.0f, 0.0f);
        clip.addKeyframe(frame);
    }

    // Keyframe 3: Landing prep (0.8s)
    {
        AnimationKeyframe frame;
        frame.time = 0.8f;
        frame.boneRotations[1] = glm::vec3(0.2f, 0.0f, 0.0f);
        frame.boneRotations[11] = glm::vec3(-0.3f, 0.0f, 0.0f);
        frame.boneRotations[14] = glm::vec3(-0.3f, 0.0f, 0.0f);
        frame.boneRotations[5] = glm::vec3(0.0f, 0.0f, -0.2f);
        frame.boneRotations[8] = glm::vec3(0.0f, 0.0f, 0.2f);
        frame.bonePositions[0] = glm::vec3(0.0f, -0.5f, 0.0f);
        clip.addKeyframe(frame);
    }

    // Keyframe 4: Landed (1.0s)
    {
        AnimationKeyframe frame;
        frame.time = 1.0f;
        frame.boneRotations[1] = glm::vec3(0.0f, 0.0f, 0.0f);
        frame.boneRotations[11] = glm::vec3(0.0f, 0.0f, 0.0f);
        frame.boneRotations[14] = glm::vec3(0.0f, 0.0f, 0.0f);
        frame.boneRotations[5] = glm::vec3(0.0f, 0.0f, 0.0f);
        frame.boneRotations[8] = glm::vec3(0.0f, 0.0f, 0.0f);
        frame.bonePositions[0] = glm::vec3(0.0f, 0.0f, 0.0f);
        clip.addKeyframe(frame);
    }

    return clip;
}

AnimationClip AnimationClipFactory::createCrouchAnimation()
{
    AnimationClip clip("Crouch");
    clip.setLooping(false);

    // Keyframe 0: Standing (0.0s)
    {
        AnimationKeyframe frame;
        frame.time = 0.0f;
        frame.boneRotations[1] = glm::vec3(0.0f, 0.0f, 0.0f);
        frame.bonePositions[0] = glm::vec3(0.0f, 0.0f, 0.0f);
        clip.addKeyframe(frame);
    }

    // Keyframe 1: Crouched (0.3s)
    {
        AnimationKeyframe frame;
        frame.time = 0.3f;
        frame.boneRotations[1] = glm::vec3(0.4f, 0.0f, 0.0f);   // Spine bend forward
        frame.boneRotations[11] = glm::vec3(-0.6f, 0.0f, 0.0f); // Legs bent
        frame.boneRotations[14] = glm::vec3(-0.6f, 0.0f, 0.0f);
        frame.bonePositions[0] = glm::vec3(0.0f, -1.5f, 0.0f); // Lower body
        clip.addKeyframe(frame);
    }

    return clip;
}

} // namespace fresh
