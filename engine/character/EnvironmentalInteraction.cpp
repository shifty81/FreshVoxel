#include "character/EnvironmentalInteraction.h"

#include <glm/glm.hpp>

namespace fresh
{

void EnvironmentalInteraction::update(const EnvironmentQuery& query, float /*deltaTime*/)
{
    // Evaluate transitions in priority order
    if (query.isInWater)
    {
        m_state = EnvironmentState::Swimming;
        return;
    }

    if (query.hasClimbableSurface && query.isClimbInput)
    {
        m_state = EnvironmentState::Climbing;
        return;
    }

    if (query.isGrounded && query.isDuckInput)
    {
        m_state = EnvironmentState::Ducking;
        return;
    }

    if (query.isGrounded)
    {
        m_state = EnvironmentState::Grounded;
        return;
    }

    m_state = EnvironmentState::Airborne;
}

EnvironmentState EnvironmentalInteraction::getState() const
{
    return m_state;
}

bool EnvironmentalInteraction::isClimbing() const
{
    return m_state == EnvironmentState::Climbing;
}

bool EnvironmentalInteraction::isSwimming() const
{
    return m_state == EnvironmentState::Swimming;
}

bool EnvironmentalInteraction::isDucking() const
{
    return m_state == EnvironmentState::Ducking;
}

bool EnvironmentalInteraction::isGrounded() const
{
    return m_state == EnvironmentState::Grounded || m_state == EnvironmentState::Ducking;
}

glm::vec3 EnvironmentalInteraction::getModifiedVelocity(const glm::vec3& inputVelocity) const
{
    switch (m_state)
    {
    case EnvironmentState::Climbing:
    {
        // While climbing: only vertical movement is applied, scaled by climb speed
        float verticalInput = inputVelocity.y;
        return glm::vec3(0.0f, verticalInput * m_climbSpeed, 0.0f);
    }
    case EnvironmentState::Swimming:
    {
        // All axes scaled to swim speed fraction
        return inputVelocity * m_swimSpeed;
    }
    case EnvironmentState::Ducking:
    {
        // Horizontal movement reduced; vertical pass-through
        return glm::vec3(inputVelocity.x * m_duckSpeedMultiplier,
                         inputVelocity.y,
                         inputVelocity.z * m_duckSpeedMultiplier);
    }
    default:
        // Grounded / Airborne: no modification
        return inputVelocity;
    }
}

float EnvironmentalInteraction::getHeightMultiplier() const
{
    if (m_state == EnvironmentState::Ducking)
        return m_duckHeightMultiplier;
    return 1.0f;
}

float EnvironmentalInteraction::getBuoyancy() const
{
    if (m_state == EnvironmentState::Swimming)
        return m_buoyancy;
    return 0.0f;
}

// ── Configuration ─────────────────────────────────────────────────────────────

void EnvironmentalInteraction::setClimbSpeed(float speed)
{
    m_climbSpeed = speed;
}

float EnvironmentalInteraction::getClimbSpeed() const
{
    return m_climbSpeed;
}

void EnvironmentalInteraction::setSwimSpeed(float speed)
{
    m_swimSpeed = speed;
}

float EnvironmentalInteraction::getSwimSpeed() const
{
    return m_swimSpeed;
}

void EnvironmentalInteraction::setDuckSpeedMultiplier(float multiplier)
{
    m_duckSpeedMultiplier = multiplier;
}

float EnvironmentalInteraction::getDuckSpeedMultiplier() const
{
    return m_duckSpeedMultiplier;
}

void EnvironmentalInteraction::setDuckHeightMultiplier(float multiplier)
{
    m_duckHeightMultiplier = multiplier;
}

float EnvironmentalInteraction::getDuckHeightMultiplier() const
{
    return m_duckHeightMultiplier;
}

void EnvironmentalInteraction::setBuoyancy(float buoyancy)
{
    m_buoyancy = buoyancy;
}

} // namespace fresh
