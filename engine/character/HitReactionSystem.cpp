#include "character/HitReactionSystem.h"

#include <glm/gtc/constants.hpp>
#include <algorithm>
#include <cmath>

namespace fresh
{

// Maximum rotation offset in radians (30 degrees)
static constexpr float kMaxRotation = glm::pi<float>() / 6.0f;

void HitReactionSystem::processHit(const HitInfo& hit)
{
    HitReaction reaction;
    reaction.intensity = 1.0f;
    reaction.force = hit.force;
    reaction.direction = hit.direction;
    reaction.targetBone = hit.targetBone;
    reaction.active = true;
    reaction.rotationOffset = computeReactionOffset(reaction);

    if (static_cast<int>(m_reactions.size()) >= m_maxReactions)
    {
        // Replace the reaction with the lowest intensity
        auto it = std::min_element(m_reactions.begin(), m_reactions.end(),
                                   [](const HitReaction& a, const HitReaction& b) {
                                       return a.intensity < b.intensity;
                                   });
        *it = reaction;
    }
    else
    {
        m_reactions.push_back(reaction);
    }
}

void HitReactionSystem::update(float deltaTime)
{
    for (auto& reaction : m_reactions)
    {
        if (!reaction.active)
            continue;

        reaction.intensity -= reaction.decayRate * deltaTime;
        if (reaction.intensity <= 0.0f)
        {
            reaction.intensity = 0.0f;
            reaction.active = false;
        }
        else
        {
            // Recompute offset with updated intensity
            reaction.rotationOffset = computeReactionOffset(reaction);
        }
    }

    removeExpiredReactions();
}

void HitReactionSystem::clear()
{
    m_reactions.clear();
}

bool HitReactionSystem::hasActiveReactions() const
{
    return std::any_of(m_reactions.begin(), m_reactions.end(),
                       [](const HitReaction& r) { return r.active; });
}

int HitReactionSystem::getActiveReactionCount() const
{
    return static_cast<int>(
        std::count_if(m_reactions.begin(), m_reactions.end(),
                      [](const HitReaction& r) { return r.active; }));
}

const std::vector<HitReaction>& HitReactionSystem::getReactions() const
{
    return m_reactions;
}

glm::vec3 HitReactionSystem::getBoneReactionOffset(const std::string& boneName) const
{
    glm::vec3 totalOffset{0.0f};

    for (const auto& reaction : m_reactions)
    {
        if (!reaction.active)
            continue;

        if (reaction.targetBone == boneName)
            totalOffset += reaction.rotationOffset;
    }

    return totalOffset;
}

void HitReactionSystem::setMaxReactions(int max)
{
    m_maxReactions = max;
}

int HitReactionSystem::getMaxReactions() const
{
    return m_maxReactions;
}

void HitReactionSystem::setGlobalIntensity(float intensity)
{
    m_globalIntensity = intensity;
}

float HitReactionSystem::getGlobalIntensity() const
{
    return m_globalIntensity;
}

HitDirection HitReactionSystem::computeHitDirection(const glm::vec3& characterForward,
                                                     const glm::vec3& hitDir)
{
    // Vertical component check first
    // hitDir is the direction the hit travels; Above/Below describe where it comes FROM
    float upDot = hitDir.y;
    if (upDot < -0.7f)
        return HitDirection::Above;
    if (upDot > 0.7f)
        return HitDirection::Below;

    // Horizontal direction relative to character forward (XZ plane)
    glm::vec3 forward = glm::normalize(glm::vec3(characterForward.x, 0.0f, characterForward.z));
    glm::vec3 hitHoriz = glm::normalize(glm::vec3(hitDir.x, 0.0f, hitDir.z));

    float forwardDot = glm::dot(forward, hitHoriz);

    // Right vector via cross product with world up
    glm::vec3 right = glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f));
    float rightDot = glm::dot(right, hitHoriz);

    if (std::abs(forwardDot) >= std::abs(rightDot))
    {
        return forwardDot >= 0.0f ? HitDirection::Front : HitDirection::Back;
    }
    else
    {
        // Positive rightDot means hit travels in the right direction (comes FROM left)
        return rightDot >= 0.0f ? HitDirection::Left : HitDirection::Right;
    }
}

glm::vec3 HitReactionSystem::computeReactionOffset(const HitReaction& reaction) const
{
    float scale = reaction.intensity * reaction.force * m_globalIntensity * kMaxRotation;
    glm::vec3 offset{0.0f};

    switch (reaction.direction)
    {
    case HitDirection::Front:
        // Hit from front: pitch backward (positive X rotation)
        offset.x = scale;
        break;
    case HitDirection::Back:
        // Hit from back: pitch forward (negative X rotation)
        offset.x = -scale;
        break;
    case HitDirection::Left:
        // Hit from left: roll right (positive Z rotation)
        offset.z = scale;
        break;
    case HitDirection::Right:
        // Hit from right: roll left (negative Z rotation)
        offset.z = -scale;
        break;
    case HitDirection::Above:
        // Hit from above: pitch forward (negative X rotation)
        offset.x = -scale;
        break;
    case HitDirection::Below:
        // Hit from below: pitch backward (positive X rotation)
        offset.x = scale;
        break;
    }

    return offset;
}

void HitReactionSystem::removeExpiredReactions()
{
    m_reactions.erase(
        std::remove_if(m_reactions.begin(), m_reactions.end(),
                       [](const HitReaction& r) { return !r.active; }),
        m_reactions.end());
}

} // namespace fresh
