#pragma once
#include <glm/glm.hpp>
#include "ecs/IComponent.h"

namespace fresh
{
namespace ecs
{

/**
 * @brief Component for 2D physics body
 *
 * Contains physical properties for 2D physics simulation including
 * velocity, forces, mass, and constraints.
 */
class RigidBody2DComponent : public IComponent
{
public:
    /**
     * @brief Type of physics body
     */
    enum class BodyType
    {
        Static,     ///< Immovable object (infinite mass)
        Kinematic,  ///< Moves but not affected by forces
        Dynamic     ///< Full physics simulation
    };

    RigidBody2DComponent();
    virtual ~RigidBody2DComponent() = default;

    COMPONENT_TYPE(RigidBody2DComponent)

    // Body type
    BodyType bodyType;

    // Motion properties
    glm::vec2 velocity;             ///< Linear velocity (units/second)
    float angularVelocity;          ///< Angular velocity (radians/second)
    glm::vec2 acceleration;         ///< Current acceleration
    float angularAcceleration;      ///< Angular acceleration
    
    // Physical properties
    float mass;                     ///< Mass of the body (kg)
    float inverseMass;              ///< 1/mass for optimization (0 for static)
    float drag;                     ///< Linear drag/air resistance (0-1)
    float angularDrag;              ///< Angular drag (0-1)
    float restitution;              ///< Bounciness (0 = no bounce, 1 = perfect bounce)
    float friction;                 ///< Surface friction (0 = no friction, 1 = high friction)
    
    // Constraints
    bool useGravity;                ///< Apply gravity to this body?
    bool freezeRotation;            ///< Prevent rotation?
    glm::vec2 gravityScale;         ///< Gravity multiplier (per axis)
    
    // Forces (cleared each frame)
    glm::vec2 force;                ///< Accumulated force
    float torque;                   ///< Accumulated torque

    /**
     * @brief Apply a force to the body
     */
    void addForce(const glm::vec2& force);

    /**
     * @brief Apply an impulse (instant velocity change)
     */
    void addImpulse(const glm::vec2& impulse);

    /**
     * @brief Apply torque
     */
    void addTorque(float torque);

    /**
     * @brief Set mass and recalculate inverse mass
     */
    void setMass(float newMass);

    /**
     * @brief Clear all accumulated forces
     */
    void clearForces();

    /**
     * @brief Get kinetic energy
     */
    float getKineticEnergy() const;
};

} // namespace ecs
} // namespace fresh
