#pragma once

#include <glm/glm.hpp>
#include "voxel/VoxelTypes.h"

namespace fresh
{

class VoxelWorld;
class Player;

/**
 * @brief Raft entity for water traversal
 * 
 * Allows player to traverse water bodies by mounting a raft.
 * Raft can only be placed on water blocks and provides
 * smooth water-based movement.
 */
class Raft
{
public:
    Raft();
    ~Raft() = default;

    /**
     * @brief Initialize raft at a position
     * @param position World position
     * @param world Voxel world reference
     */
    void initialize(const glm::vec3& position, VoxelWorld* world);

    /**
     * @brief Update raft physics and movement
     * @param deltaTime Time since last frame in seconds
     */
    void update(float deltaTime);

    /**
     * @brief Get raft position
     * @return World position
     */
    glm::vec3 getPosition() const { return m_position; }

    /**
     * @brief Set raft position
     * @param position New world position
     */
    void setPosition(const glm::vec3& position);

    /**
     * @brief Get raft rotation (yaw)
     * @return Rotation angle in degrees
     */
    float getRotation() const { return m_rotation; }

    /**
     * @brief Set raft rotation
     * @param rotation Rotation angle in degrees
     */
    void setRotation(float rotation) { m_rotation = rotation; }

    /**
     * @brief Check if raft is currently mounted by player
     * @return True if player is on raft
     */
    bool isMounted() const { return m_isMounted; }

    /**
     * @brief Mount player on raft
     * @param player Player to mount
     */
    void mount(Player* player);

    /**
     * @brief Dismount player from raft
     */
    void dismount();

    /**
     * @brief Handle input for raft movement (when mounted)
     * @param forward Forward/backward input (-1 to 1)
     * @param strafe Left/right input (-1 to 1)
     * @param turn Turn input (-1 to 1)
     * @param deltaTime Time delta
     */
    void handleInput(float forward, float strafe, float turn, float deltaTime);

    /**
     * @brief Check if raft can be placed at position
     * @param position World position to check
     * @param world Voxel world reference
     * @return True if position is valid for raft placement
     */
    static bool canPlaceAt(const glm::vec3& position, VoxelWorld* world);

    /**
     * @brief Get raft dimensions for collision
     * @param width Output width
     * @param length Output length
     * @param height Output height
     */
    void getDimensions(float& width, float& length, float& height) const
    {
        width = m_width;
        length = m_length;
        height = m_height;
    }

    /**
     * @brief Check if raft is in valid water
     * @return True if raft is floating on water
     */
    bool isInWater() const { return m_isInWater; }

private:
    /**
     * @brief Check if position is over water
     * @param position Position to check
     * @return True if over water block
     */
    bool checkWaterBelow(const glm::vec3& position);

    /**
     * @brief Apply water physics (buoyancy, drag)
     */
    void applyWaterPhysics(float deltaTime);

private:
    glm::vec3 m_position;
    glm::vec3 m_velocity;
    float m_rotation;          // Yaw angle in degrees
    
    VoxelWorld* m_world;
    Player* m_mountedPlayer;
    
    bool m_isMounted;
    bool m_isInWater;
    
    // Raft properties
    float m_width;             // Raft width (X)
    float m_length;            // Raft length (Z)
    float m_height;            // Raft height (Y)
    float m_moveSpeed;         // Movement speed
    float m_turnSpeed;         // Turning speed
    float m_drag;              // Water drag coefficient
    float m_buoyancy;          // Upward force in water
};

} // namespace fresh
