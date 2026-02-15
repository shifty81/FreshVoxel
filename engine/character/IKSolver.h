#pragma once
#include <glm/glm.hpp>

namespace fresh
{

/**
 * @brief Result of an IK solve operation
 */
struct IKSolution {
    glm::vec3 joint1Rotation; // First joint rotation (e.g., shoulder or hip)
    glm::vec3 joint2Rotation; // Second joint rotation (e.g., elbow or knee)
    glm::vec3 endEffectorRotation; // End effector rotation (e.g., foot or hand orientation)
    bool success;             // Whether solution was found
};

/**
 * @brief Two-bone Inverse Kinematics solver
 *
 * Solves for joint rotations to reach a target position with a two-bone chain
 * (e.g., shoulder-elbow-hand or hip-knee-foot).
 */
class TwoBoneIK
{
public:
    /**
     * @brief Solve two-bone IK for a limb
     *
     * @param origin Start position of the chain (e.g., shoulder/hip)
     * @param joint1Length Length of first bone (e.g., upper arm/thigh)
     * @param joint2Length Length of second bone (e.g., forearm/shin)
     * @param target Target position to reach (e.g., hand/foot)
     * @param poleVector Direction hint for joint bending (e.g., elbow/knee direction)
     * @return IK solution with joint rotations
     */
    static IKSolution solve(const glm::vec3& origin, float joint1Length, float joint2Length,
                            const glm::vec3& target, const glm::vec3& poleVector);

    /**
     * @brief Solve two-bone IK with constraints
     *
     * @param origin Start position of the chain
     * @param joint1Length Length of first bone
     * @param joint2Length Length of second bone
     * @param target Target position to reach
     * @param poleVector Direction hint for joint bending
     * @param maxAngle1 Maximum bend angle for first joint (radians)
     * @param maxAngle2 Maximum bend angle for second joint (radians)
     * @return IK solution with joint rotations
     */
    static IKSolution solveWithConstraints(const glm::vec3& origin, float joint1Length,
                                           float joint2Length, const glm::vec3& target,
                                           const glm::vec3& poleVector, float maxAngle1,
                                           float maxAngle2);

private:
    /**
     * @brief Calculate angle between three points using law of cosines
     */
    static float calculateAngle(float a, float b, float c);

    /**
     * @brief Clamp angle to valid range
     */
    static float clampAngle(float angle, float maxAngle);
};

/**
 * @brief Foot IK system for terrain adaptation
 *
 * Adjusts character feet to match terrain height and slope.
 */
class FootIK
{
public:
    FootIK();
    ~FootIK();

    /**
     * @brief Set IK parameters
     * @param hipToKneeLength Upper leg length
     * @param kneeToFootLength Lower leg length
     */
    void setLegLengths(float hipToKneeLength, float kneeToFootLength);

    /**
     * @brief Solve foot IK to place foot on terrain
     *
     * @param hipPosition Position of hip joint
     * @param targetFootPosition Target ground position for foot
     * @param surfaceNormal Normal of ground surface
     * @return IK solution for leg
     */
    IKSolution solveFoot(const glm::vec3& hipPosition, const glm::vec3& targetFootPosition,
                         const glm::vec3& surfaceNormal);

    /**
     * @brief Enable/disable foot IK
     */
    void setEnabled(bool enable)
    {
        this->enabled = enable;
    }
    bool isEnabled() const
    {
        return enabled;
    }

    /**
     * @brief Set maximum leg stretch distance
     */
    void setMaxStretch(float stretch)
    {
        this->maxStretch = stretch;
    }

private:
    float hipToKneeLength;
    float kneeToFootLength;
    float maxStretch;
    bool enabled;
};

/**
 * @brief Hand IK system for item holding
 *
 * Positions character hands to hold items and tools.
 */
class HandIK
{
public:
    HandIK();
    ~HandIK();

    /**
     * @brief Set IK parameters
     * @param shoulderToElbowLength Upper arm length
     * @param elbowToHandLength Lower arm length
     */
    void setArmLengths(float shoulderToElbowLength, float elbowToHandLength);

    /**
     * @brief Solve hand IK to reach target position
     *
     * @param shoulderPosition Position of shoulder joint
     * @param targetHandPosition Target position for hand
     * @param gripRotation Desired rotation of hand
     * @return IK solution for arm
     */
    IKSolution solveHand(const glm::vec3& shoulderPosition, const glm::vec3& targetHandPosition,
                         const glm::vec3& gripRotation);

    /**
     * @brief Enable/disable hand IK
     */
    void setEnabled(bool enable)
    {
        this->enabled = enable;
    }
    bool isEnabled() const
    {
        return enabled;
    }

private:
    float shoulderToElbowLength;
    float elbowToHandLength;
    bool enabled;
};

/**
 * @brief Look-at IK for head/eye tracking
 *
 * Rotates character head to look at a target.
 */
class LookAtIK
{
public:
    /**
     * @brief Calculate head rotation to look at target
     *
     * @param headPosition Current head position
     * @param targetPosition Position to look at
     * @param upVector Up direction
     * @return Rotation for head bone (Euler angles)
     */
    static glm::vec3 calculateLookRotation(const glm::vec3& headPosition,
                                           const glm::vec3& targetPosition,
                                           const glm::vec3& upVector = glm::vec3(0, 1, 0));

    /**
     * @brief Calculate head rotation with constraints
     *
     * @param headPosition Current head position
     * @param targetPosition Position to look at
     * @param upVector Up direction
     * @param maxYaw Maximum horizontal rotation (radians)
     * @param maxPitch Maximum vertical rotation (radians)
     * @return Constrained rotation for head bone
     */
    static glm::vec3 calculateLookRotationConstrained(const glm::vec3& headPosition,
                                                      const glm::vec3& targetPosition,
                                                      const glm::vec3& upVector, float maxYaw,
                                                      float maxPitch);
};

} // namespace fresh
