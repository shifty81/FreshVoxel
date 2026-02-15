#include "character/IKSolver.h"

#include <algorithm>
#include <cmath>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

namespace fresh
{

// TwoBoneIK implementation

IKSolution TwoBoneIK::solve(const glm::vec3& origin, float joint1Length, float joint2Length,
                            const glm::vec3& target, const glm::vec3& poleVector)
{
    IKSolution solution;
    solution.success = false;
    solution.joint1Rotation = glm::vec3(0.0f);
    solution.joint2Rotation = glm::vec3(0.0f);
    solution.endEffectorRotation = glm::vec3(0.0f);

    // Calculate distance to target
    glm::vec3 toTarget = target - origin;
    float targetDistance = glm::length(toTarget);

    // Check if target is reachable
    float maxReach = joint1Length + joint2Length;
    if (targetDistance > maxReach || targetDistance < 0.001f) {
        // Target out of reach or too close
        return solution;
    }

    // Calculate angles using law of cosines
    float a = joint1Length;
    float b = joint2Length;
    float c = targetDistance;

    // Angle at first joint (elbow/knee)
    float angle2 = calculateAngle(a, c, b);

    // Angle at origin (shoulder/hip)
    float angle1 = calculateAngle(c, a, b);

    // Calculate direction to target
    glm::vec3 targetDir = glm::normalize(toTarget);

    // Calculate pole direction (for elbow/knee bending direction)
    glm::vec3 poleDir = glm::normalize(poleVector);

    // Calculate rotation axis (perpendicular to target direction and pole direction)
    // Note: Currently unused but reserved for future full 3D rotation calculation
    [[maybe_unused]] glm::vec3 rotationAxis = glm::normalize(glm::cross(targetDir, poleDir));

    // Apply angles to solution
    // Note: This is a simplified 2D solution projected into 3D
    // For production, would need full 3D rotation calculation
    solution.joint1Rotation.x = angle1;
    solution.joint2Rotation.x = angle2 - 3.14159265359f; // Subtract PI to bend in correct direction

    solution.success = true;
    return solution;
}

IKSolution TwoBoneIK::solveWithConstraints(const glm::vec3& origin, float joint1Length,
                                           float joint2Length, const glm::vec3& target,
                                           const glm::vec3& poleVector, float maxAngle1,
                                           float maxAngle2)
{
    IKSolution solution = solve(origin, joint1Length, joint2Length, target, poleVector);

    if (solution.success) {
        // Apply angle constraints
        solution.joint1Rotation.x = clampAngle(solution.joint1Rotation.x, maxAngle1);
        solution.joint2Rotation.x = clampAngle(solution.joint2Rotation.x, maxAngle2);
    }

    return solution;
}

float TwoBoneIK::calculateAngle(float a, float b, float c)
{
    // Law of cosines: c² = a² + b² - 2ab·cos(C)
    // Solve for angle C: cos(C) = (a² + b² - c²) / (2ab)

    float cosAngle = (a * a + b * b - c * c) / (2.0f * a * b);

    // Clamp to valid range for acos
    cosAngle = std::clamp(cosAngle, -1.0f, 1.0f);

    return std::acos(cosAngle);
}

float TwoBoneIK::clampAngle(float angle, float maxAngle)
{
    return std::clamp(angle, -maxAngle, maxAngle);
}

// FootIK implementation

FootIK::FootIK() : hipToKneeLength(2.0f), kneeToFootLength(2.0f), maxStretch(1.2f), enabled(true) {}

FootIK::~FootIK() = default;

void FootIK::setLegLengths(float hipToKnee, float kneeToFoot)
{
    this->hipToKneeLength = hipToKnee;
    this->kneeToFootLength = kneeToFoot;
}

IKSolution FootIK::solveFoot(const glm::vec3& hipPosition, const glm::vec3& targetFootPosition,
                             const glm::vec3& surfaceNormal)
{
    // Suppress unused parameter warning - surfaceNormal would be used for foot orientation
    (void)surfaceNormal;

    IKSolution solution;

    if (!enabled) {
        solution.success = false;
        return solution;
    }

    // Calculate distance to target
    glm::vec3 toTarget = targetFootPosition - hipPosition;
    float distance = glm::length(toTarget);

    // Check if within reach
    float maxReach = (hipToKneeLength + kneeToFootLength) * maxStretch;
    if (distance > maxReach) {
        // Clamp to max reach
        toTarget = glm::normalize(toTarget) * maxReach;
    }

    // Calculate pole vector (knee should bend forward)
    glm::vec3 poleVector = glm::vec3(0, 0, 1); // Forward direction

    // Solve two-bone IK
    solution = TwoBoneIK::solve(hipPosition, hipToKneeLength, kneeToFootLength,
                                hipPosition + toTarget, poleVector);

    // Adjust foot rotation to match surface normal
    // Calculate rotation to align foot with surface
    glm::vec3 up = glm::vec3(0, 1, 0);
    glm::vec3 right = glm::normalize(glm::cross(up, surfaceNormal));
    glm::vec3 forward = glm::normalize(glm::cross(surfaceNormal, right));
    
    // Convert to Euler angles (pitch and roll based on surface slope)
    float pitch = std::asin(-forward.y);
    float roll = std::atan2(right.y, surfaceNormal.y);
    solution.endEffectorRotation = glm::vec3(pitch, 0.0f, roll);

    return solution;
}

// HandIK implementation

HandIK::HandIK() : shoulderToElbowLength(1.5f), elbowToHandLength(1.5f), enabled(true) {}

HandIK::~HandIK() = default;

void HandIK::setArmLengths(float shoulderToElbow, float elbowToHand)
{
    this->shoulderToElbowLength = shoulderToElbow;
    this->elbowToHandLength = elbowToHand;
}

IKSolution HandIK::solveHand(const glm::vec3& shoulderPosition, const glm::vec3& targetHandPosition,
                             const glm::vec3& gripRotation)
{
    IKSolution solution;

    if (!enabled) {
        solution.success = false;
        return solution;
    }

    // Calculate pole vector (elbow should bend downward)
    glm::vec3 poleVector = glm::vec3(0, -1, 0); // Downward direction

    // Solve two-bone IK
    solution = TwoBoneIK::solve(shoulderPosition, shoulderToElbowLength, elbowToHandLength,
                                targetHandPosition, poleVector);

    // Apply grip rotation (would need hand bone orientation)
    (void)gripRotation; // Suppress unused parameter warning

    return solution;
}

// LookAtIK implementation

glm::vec3 LookAtIK::calculateLookRotation(const glm::vec3& headPosition,
                                          const glm::vec3& targetPosition,
                                          const glm::vec3& upVector)
{
    // Suppress unused parameter warning - upVector would be used for roll calculation
    (void)upVector;

    // Calculate direction to target
    glm::vec3 direction = glm::normalize(targetPosition - headPosition);

    // Calculate yaw (horizontal rotation)
    float yaw = std::atan2(direction.x, direction.z);

    // Calculate pitch (vertical rotation)
    float pitch = std::asin(-direction.y);

    // Return as Euler angles (pitch, yaw, roll)
    return glm::vec3(pitch, yaw, 0.0f);
}

glm::vec3 LookAtIK::calculateLookRotationConstrained(const glm::vec3& headPosition,
                                                     const glm::vec3& targetPosition,
                                                     const glm::vec3& upVector, float maxYaw,
                                                     float maxPitch)
{
    glm::vec3 rotation = calculateLookRotation(headPosition, targetPosition, upVector);

    // Apply constraints
    rotation.x = std::clamp(rotation.x, -maxPitch, maxPitch); // Pitch
    rotation.y = std::clamp(rotation.y, -maxYaw, maxYaw);     // Yaw

    return rotation;
}

} // namespace fresh
