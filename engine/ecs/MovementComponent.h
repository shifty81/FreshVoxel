#pragma once
#include "IComponent.h"
#include <glm/glm.hpp>

namespace fresh
{
namespace ecs
{

/**
 * @brief Component for managing entity movement
 *
 * This component demonstrates Component-Based Architecture by:
 * - Separating movement logic from other behaviors
 * - Being independently testable
 * - Allowing different movement strategies per entity
 */
class MovementComponent : public IComponent
{
public:
    MovementComponent(float maxSpeed = 5.0f, float acceleration = 10.0f)
        : maxSpeed(maxSpeed), acceleration(acceleration), velocity(0.0f, 0.0f, 0.0f),
          desiredDirection(0.0f, 0.0f, 0.0f), canMove(true)
    {
    }

    COMPONENT_TYPE(MovementComponent)

    // Movement control
    void setDesiredDirection(const glm::vec3& direction)
    {
        desiredDirection = direction;
        if (glm::length(desiredDirection) > 0.0f) {
            desiredDirection = glm::normalize(desiredDirection);
        }
    }

    void update(float deltaTime)
    {
        if (!canMove) {
            velocity = glm::vec3(0.0f);
            return;
        }

        // Calculate target velocity
        glm::vec3 targetVelocity = desiredDirection * maxSpeed;

        // Smoothly interpolate to target velocity
        float t = glm::min(acceleration * deltaTime, 1.0f);
        velocity = glm::mix(velocity, targetVelocity, t);
    }

    void stop()
    {
        velocity = glm::vec3(0.0f);
        desiredDirection = glm::vec3(0.0f);
    }

    // Setters
    void setMaxSpeed(float speed)
    {
        maxSpeed = speed;
    }
    void setAcceleration(float accel)
    {
        acceleration = accel;
    }
    void setCanMove(bool enabled)
    {
        canMove = enabled;
        if (!canMove) {
            velocity = glm::vec3(0.0f);
        }
    }

    // Getters
    glm::vec3 getVelocity() const
    {
        return velocity;
    }
    float getMaxSpeed() const
    {
        return maxSpeed;
    }
    float getAcceleration() const
    {
        return acceleration;
    }
    glm::vec3 getDesiredDirection() const
    {
        return desiredDirection;
    }
    bool getCanMove() const
    {
        return canMove;
    }
    float getCurrentSpeed() const
    {
        return glm::length(velocity);
    }

private:
    float maxSpeed;
    float acceleration;
    glm::vec3 velocity;
    glm::vec3 desiredDirection;
    bool canMove;
};

} // namespace ecs
} // namespace fresh
