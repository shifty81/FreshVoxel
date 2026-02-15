#pragma once
#include <glm/glm.hpp>

#include "Camera.h"

// Include the appropriate input manager header for the template implementation
#ifdef _WIN32
#include "input/Win32InputManager.h"
#else
#include "input/InputManager.h"
#endif

namespace fresh
{

class VoxelWorld;

/**
 * @brief Player controller with FPS movement and physics
 *
 * Handles player movement, jumping, collision detection with voxel terrain,
 * and camera positioning. Integrates with InputManager and VoxelWorld.
 */
class Player
{
public:
    Player();

    /**
     * @brief Update player state (physics, collision)
     * @param deltaTime Time since last frame in seconds
     */
    void update(float deltaTime);

    /**
     * @brief Process input to control player movement
     * @param input Input manager to read controls from
     * @param deltaTime Time since last frame
     */
    template<typename InputManagerT>
    void handleInput(const InputManagerT& input, float deltaTime);

    /**
     * @brief Process mouse movement for camera control
     * @param xOffset Mouse X movement
     * @param yOffset Mouse Y movement
     */
    void handleMouseMovement(float xOffset, float yOffset);

    // Camera access
    Camera& getCamera()
    {
        return camera;
    }
    const Camera& getCamera() const
    {
        return camera;
    }

    // Position
    glm::vec3 getPosition() const
    {
        return position;
    }
    void setPosition(const glm::vec3& pos);

    // World reference
    void setWorld(VoxelWorld* w)
    {
        world = w;
    }

    // State queries
    bool isPlayerGrounded() const
    {
        return isGrounded;
    }
    bool isPlayerSprinting() const
    {
        return isSprinting;
    }
    bool isPlayerCrouching() const
    {
        return isCrouching;
    }

    // Configuration
    void setWalkSpeed(float speed)
    {
        walkSpeed = speed;
    }
    void setSprintSpeed(float speed)
    {
        sprintSpeed = speed;
    }
    void setMouseSensitivity(float sens)
    {
        mouseSensitivity = sens;
    }
    
    // Free flight mode control
    void setFreeFlightMode(bool enabled)
    {
        freeFlightMode = enabled;
    }
    bool isFreeFlightMode() const
    {
        return freeFlightMode;
    }

    // Health and stamina system (for HUD integration)
    float getHealth() const { return m_health; }
    float getMaxHealth() const { return m_maxHealth; }
    float getStamina() const { return m_stamina; }
    float getMaxStamina() const { return m_maxStamina; }
    
    void setHealth(float health) { m_health = glm::clamp(health, 0.0f, m_maxHealth); }
    void setMaxHealth(float maxHealth) { m_maxHealth = maxHealth; }
    void setStamina(float stamina) { m_stamina = glm::clamp(stamina, 0.0f, m_maxStamina); }
    void setMaxStamina(float maxStamina) { m_maxStamina = maxStamina; }
    
    void takeDamage(float damage) { setHealth(m_health - damage); }
    void heal(float amount) { setHealth(m_health + amount); }
    void consumeStamina(float amount) { setStamina(m_stamina - amount); }
    void restoreStamina(float amount) { setStamina(m_stamina + amount); }

private:
    /**
     * @brief Apply gravity to player velocity
     */
    void applyGravity(float deltaTime);

    /**
     * @brief Move player in given direction with collision checking
     * @param direction Movement direction (normalized)
     * @param speed Movement speed
     * @param deltaTime Time delta
     */
    void handleMovement(const glm::vec3& direction, float speed, float deltaTime);

    /**
     * @brief Check if position would collide with terrain
     * @param newPos Position to check
     * @return true if collision detected
     */
    bool checkCollision(const glm::vec3& newPos);

    /**
     * @brief Check if player is on ground
     * @return true if grounded
     */
    bool checkGrounded();

    /**
     * @brief Check if there's ground ahead at a position (for crouch edge detection)
     * @param newPos Position to check
     * @return true if ground exists below this position
     */
    bool checkGroundAhead(const glm::vec3& newPos);

    Camera camera;
    VoxelWorld* world = nullptr;

    glm::vec3 position{0.0f, 80.0f, 0.0f};
    glm::vec3 velocity{0.0f};

    // Player dimensions (capsule collision)
    float height = 1.8f;
    float eyeHeight = 1.6f;
    float radius = 0.3f;
    float crouchHeight = 1.4f;

    // Movement speeds (blocks per second)
    float walkSpeed = 4.3f;
    float sprintSpeed = 5.6f;
    float crouchSpeed = 2.0f;
    float jumpVelocity = 8.0f;
    float gravity = 20.0f;
    float mouseSensitivity = 10.0f; // Increased from 1.0 for better responsiveness

    // State
    bool isGrounded = false;
    bool isSprinting = false;
    bool isCrouching = false;
    bool freeFlightMode = true; // New: Enable free-flying camera by default
    
    // Health and stamina
    float m_health = 100.0f;
    float m_maxHealth = 100.0f;
    float m_stamina = 100.0f;
    float m_maxStamina = 100.0f;
};

// Template implementation must be in header
template<typename InputManagerT>
void Player::handleInput(const InputManagerT& input, float deltaTime)
{
    if (!world)
        return;

    // Free flight mode: 6DOF movement
    if (freeFlightMode) {
        // Get camera direction vectors (use full 3D vectors for free flight)
        glm::vec3 forward = camera.getFront();
        glm::vec3 right = camera.getRight();
        glm::vec3 up = camera.getUp();

        // Calculate movement direction
        glm::vec3 moveDirection{0.0f};

        if (input.isActionActive(InputAction::MoveForward)) {
            moveDirection += forward;
        }
        if (input.isActionActive(InputAction::MoveBackward)) {
            moveDirection -= forward;
        }
        if (input.isActionActive(InputAction::MoveRight)) {
            moveDirection += right;
        }
        if (input.isActionActive(InputAction::MoveLeft)) {
            moveDirection -= right;
        }
        
        // Space bar for upward movement in free flight
        if (input.isActionActive(InputAction::Jump)) {
            moveDirection += up;
        }
        // Crouch for downward movement in free flight
        if (input.isActionActive(InputAction::Crouch)) {
            moveDirection -= up;
        }

        // Normalize if moving
        if (glm::length(moveDirection) > 0.0f) {
            moveDirection = glm::normalize(moveDirection);
        }

        // Determine speed
        isSprinting = input.isActionActive(InputAction::Sprint);
        float speed = isSprinting ? sprintSpeed : walkSpeed;

        // Apply movement directly to velocity
        velocity = moveDirection * speed;
        
        return;
    }

    // Normal play mode: ground-based movement with physics
    // Get camera direction vectors (ignore Y for horizontal movement)
    glm::vec3 forward = glm::normalize(glm::vec3(camera.getFront().x, 0.0f, camera.getFront().z));
    glm::vec3 right = glm::normalize(glm::vec3(camera.getRight().x, 0.0f, camera.getRight().z));

    // Calculate movement direction
    glm::vec3 moveDirection{0.0f};

    if (input.isActionActive(InputAction::MoveForward)) {
        moveDirection += forward;
    }
    if (input.isActionActive(InputAction::MoveBackward)) {
        moveDirection -= forward;
    }
    if (input.isActionActive(InputAction::MoveRight)) {
        moveDirection += right;
    }
    if (input.isActionActive(InputAction::MoveLeft)) {
        moveDirection -= right;
    }

    // Normalize if moving diagonally
    if (glm::length(moveDirection) > 0.0f) {
        moveDirection = glm::normalize(moveDirection);
    }

    // Determine speed
    isSprinting = input.isActionActive(InputAction::Sprint) && !isCrouching;
    isCrouching = input.isActionActive(InputAction::Crouch);

    float speed = walkSpeed;
    if (isSprinting) {
        speed = sprintSpeed;
    } else if (isCrouching) {
        speed = crouchSpeed;
    }

    // Apply movement
    if (glm::length(moveDirection) > 0.0f) {
        handleMovement(moveDirection, speed, deltaTime);
    }

    // Jump
    if (input.isActionJustPressed(InputAction::Jump) && isGrounded && !isCrouching) {
        velocity.y = jumpVelocity;
        isGrounded = false;
    }
}

} // namespace fresh
