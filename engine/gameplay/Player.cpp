#include "gameplay/Player.h"

#include <algorithm>

#include "voxel/VoxelTypes.h"
#include "voxel/VoxelWorld.h"

namespace fresh
{

Player::Player()
{
    // Camera starts at player eye height
    camera.setPosition(position + glm::vec3(0.0f, eyeHeight, 0.0f));
}

void Player::update(float deltaTime)
{
    if (!world)
        return;

    // Update stamina (regenerates when not sprinting)
    if (!isSprinting) {
        restoreStamina(20.0f * deltaTime); // Regenerate 20 stamina per second
    } else {
        consumeStamina(10.0f * deltaTime); // Consume 10 stamina per second when sprinting
    }

    // In free flight mode, no gravity or collision
    if (freeFlightMode) {
        // Apply velocity directly in free flight mode
        position = position + velocity * deltaTime;
        // Update camera position to player position
        camera.setPosition(position);
        return;
    }

    // Apply gravity (only in normal play mode)
    applyGravity(deltaTime);

    // Apply velocity
    glm::vec3 newPosition = position + velocity * deltaTime;

    // Check collision
    if (!checkCollision(newPosition)) {
        position = newPosition;
    } else {
        // Stop vertical movement if we hit something
        velocity.y = 0.0f;
    }

    // Update grounded state
    isGrounded = checkGrounded();

    // Update camera position to follow player
    float currentEyeHeight = isCrouching ? eyeHeight * 0.8f : eyeHeight;
    camera.setPosition(position + glm::vec3(0.0f, currentEyeHeight, 0.0f));
}

void Player::handleMouseMovement(float xOffset, float yOffset)
{
    camera.processMouseMovement(xOffset * mouseSensitivity, -yOffset * mouseSensitivity);
}

void Player::setPosition(const glm::vec3& pos)
{
    position = pos;
    camera.setPosition(pos + glm::vec3(0.0f, eyeHeight, 0.0f));
}

void Player::applyGravity(float deltaTime)
{
    if (!isGrounded) {
        velocity.y -= gravity * deltaTime;
        // Terminal velocity
        velocity.y = (std::max)(velocity.y, -50.0f);
    } else {
        // Small downward velocity when grounded to stick to terrain
        velocity.y = -0.1f;
    }
}

void Player::handleMovement(const glm::vec3& direction, float speed, float deltaTime)
{
    glm::vec3 movement = direction * speed * deltaTime;

    // When crouching, check for edges to prevent falling off
    if (isCrouching && !freeFlightMode) {
        // Calculate the new position if we were to move
        glm::vec3 potentialNewPos = position + movement;
        
        // Check if there's ground at the new position
        if (!checkGroundAhead(potentialNewPos)) {
            // No ground ahead - prevent movement in this direction
            return;
        }
    }

    // Try to move in X
    glm::vec3 newPos = position + glm::vec3(movement.x, 0.0f, 0.0f);
    if (!checkCollision(newPos)) {
        position.x = newPos.x;
    }

    // Try to move in Z
    newPos = position + glm::vec3(0.0f, 0.0f, movement.z);
    if (!checkCollision(newPos)) {
        position.z = newPos.z;
    }
}

bool Player::checkCollision(const glm::vec3& newPos)
{
    if (!world)
        return false;

    // Check player bounding box (simplified cylinder as box)
    float checkHeight = isCrouching ? crouchHeight : height;

    // Sample points around player cylinder
    int samples = 8;
    for (int i = 0; i < samples; ++i) {
        float angle = (2.0f * 3.14159f * i) / samples;
        float x = newPos.x + radius * cos(angle);
        float z = newPos.z + radius * sin(angle);

        // Check from feet to head
        for (float y = newPos.y; y < newPos.y + checkHeight; y += 0.5f) {
            int blockX = static_cast<int>(std::floor(x));
            int blockY = static_cast<int>(std::floor(y));
            int blockZ = static_cast<int>(std::floor(z));

            Voxel* voxelPtr = world->getVoxel(WorldPos(blockX, blockY, blockZ));
            VoxelType voxel = voxelPtr ? voxelPtr->type : VoxelType::Air;
            if (voxel != VoxelType::Air && voxel != VoxelType::Water) {
                return true; // Collision detected
            }
        }
    }

    return false;
}

bool Player::checkGrounded()
{
    if (!world)
        return false;

    // Check slightly below player feet
    float checkY = position.y - 0.1f;

    // Sample points around player cylinder
    int samples = 4;
    for (int i = 0; i < samples; ++i) {
        float angle = (2.0f * 3.14159f * i) / samples;
        float x = position.x + (radius * 0.7f) * cos(angle);
        float z = position.z + (radius * 0.7f) * sin(angle);

        int blockX = static_cast<int>(std::floor(x));
        int blockY = static_cast<int>(std::floor(checkY));
        int blockZ = static_cast<int>(std::floor(z));

        Voxel* voxelPtr = world->getVoxel(WorldPos(blockX, blockY, blockZ));
        VoxelType voxel = voxelPtr ? voxelPtr->type : VoxelType::Air;
        if (voxel != VoxelType::Air && voxel != VoxelType::Water) {
            return true; // Ground detected
        }
    }

    return false;
}

bool Player::checkGroundAhead(const glm::vec3& newPos)
{
    if (!world)
        return false;

    // Check slightly below the new position to see if there's ground
    // This prevents the player from walking off edges while crouching
    float checkY = newPos.y - 0.5f; // Check a bit below feet

    // Sample points around player cylinder at the new position
    int samples = 8;
    for (int i = 0; i < samples; ++i) {
        float angle = (2.0f * 3.14159f * i) / samples;
        float x = newPos.x + (radius * 0.8f) * cos(angle);
        float z = newPos.z + (radius * 0.8f) * sin(angle);

        int blockX = static_cast<int>(std::floor(x));
        int blockY = static_cast<int>(std::floor(checkY));
        int blockZ = static_cast<int>(std::floor(z));

        Voxel* voxelPtr = world->getVoxel(WorldPos(blockX, blockY, blockZ));
        VoxelType voxel = voxelPtr ? voxelPtr->type : VoxelType::Air;
        if (voxel != VoxelType::Air && voxel != VoxelType::Water) {
            return true; // Ground detected ahead
        }
    }

    return false; // No ground ahead - would fall off edge
}

} // namespace fresh
