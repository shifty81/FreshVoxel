#include "gameplay/Raft.h"
#include "gameplay/Player.h"
#include "voxel/VoxelWorld.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

namespace fresh
{

Raft::Raft()
    : m_position(0.0f)
    , m_velocity(0.0f)
    , m_rotation(0.0f)
    , m_world(nullptr)
    , m_mountedPlayer(nullptr)
    , m_isMounted(false)
    , m_isInWater(false)
    , m_width(2.0f)
    , m_length(3.0f)
    , m_height(0.5f)
    , m_moveSpeed(4.0f)
    , m_turnSpeed(90.0f)
    , m_drag(0.85f)
    , m_buoyancy(2.0f)
{
}

void Raft::initialize(const glm::vec3& position, VoxelWorld* world)
{
    m_position = position;
    m_world = world;
    m_isInWater = checkWaterBelow(position);
}

void Raft::update(float deltaTime)
{
    if (!m_world) {
        return;
    }
    
    // Check if still in water
    m_isInWater = checkWaterBelow(m_position);
    
    // Apply water physics
    if (m_isInWater) {
        applyWaterPhysics(deltaTime);
    }
    
    // Update position based on velocity
    m_position += m_velocity * deltaTime;
    
    // Keep raft at water surface
    if (m_isInWater) {
        // Find water level and float at surface
        int waterY = static_cast<int>(m_position.y);
        for (int y = waterY; y < waterY + 10; ++y) {
            WorldPos checkPos(static_cast<int>(m_position.x), y, static_cast<int>(m_position.z));
            Voxel* voxel = m_world->getVoxel(checkPos);
            if (voxel && voxel->type == VoxelType::Water) {
                m_position.y = static_cast<float>(y) + 0.8f; // Float slightly above water block
                break;
            }
        }
    }
    
    // Update mounted player position
    if (m_isMounted && m_mountedPlayer) {
        glm::vec3 playerPos = m_position;
        playerPos.y += m_height + 1.0f; // Position player standing on raft
        m_mountedPlayer->setPosition(playerPos);
    }
}

void Raft::setPosition(const glm::vec3& position)
{
    m_position = position;
    m_isInWater = checkWaterBelow(position);
}

void Raft::mount(Player* player)
{
    if (!player || m_isMounted) {
        return;
    }
    
    m_mountedPlayer = player;
    m_isMounted = true;
    
    // Disable player's normal physics while on raft
    if (m_mountedPlayer) {
        m_mountedPlayer->setFreeFlightMode(false);
    }
}

void Raft::dismount()
{
    if (!m_isMounted) {
        return;
    }
    
    // Re-enable player physics
    if (m_mountedPlayer) {
        // Place player slightly to the side of raft
        glm::vec3 dismountPos = m_position;
        dismountPos.x += m_width * 1.5f;
        dismountPos.y += m_height + 0.5f;
        m_mountedPlayer->setPosition(dismountPos);
    }
    
    m_mountedPlayer = nullptr;
    m_isMounted = false;
}

void Raft::handleInput(float forward, float strafe, float turn, float deltaTime)
{
    if (!m_isInWater || !m_isMounted) {
        return;
    }
    
    // Apply turning
    if (std::abs(turn) > 0.01f) {
        m_rotation += turn * m_turnSpeed * deltaTime;
        
        // Keep rotation in 0-360 range
        while (m_rotation >= 360.0f) m_rotation -= 360.0f;
        while (m_rotation < 0.0f) m_rotation += 360.0f;
    }
    
    // Calculate movement direction based on rotation
    float radians = glm::radians(m_rotation);
    glm::vec3 forwardDir(std::sin(radians), 0.0f, std::cos(radians));
    glm::vec3 rightDir(std::cos(radians), 0.0f, -std::sin(radians));
    
    // Apply forward/backward movement
    if (std::abs(forward) > 0.01f) {
        m_velocity += forwardDir * forward * m_moveSpeed * deltaTime;
    }
    
    // Apply strafing (though rafts typically don't strafe much)
    if (std::abs(strafe) > 0.01f) {
        m_velocity += rightDir * strafe * (m_moveSpeed * 0.5f) * deltaTime;
    }
}

bool Raft::canPlaceAt(const glm::vec3& position, VoxelWorld* world)
{
    if (!world) {
        return false;
    }
    
    // Check if there's water below
    WorldPos checkPos(
        static_cast<int>(position.x),
        static_cast<int>(position.y) - 1,
        static_cast<int>(position.z)
    );
    
    Voxel* voxel = world->getVoxel(checkPos);
    if (!voxel) {
        return false;
    }
    
    return voxel->type == VoxelType::Water;
}

bool Raft::checkWaterBelow(const glm::vec3& position)
{
    if (!m_world) {
        return false;
    }
    
    // Check block directly below raft
    WorldPos belowPos(
        static_cast<int>(position.x),
        static_cast<int>(position.y) - 1,
        static_cast<int>(position.z)
    );
    
    Voxel* voxel = m_world->getVoxel(belowPos);
    if (!voxel) {
        return false;
    }
    
    return voxel->type == VoxelType::Water;
}

void Raft::applyWaterPhysics(float deltaTime)
{
    // Apply drag to slow down raft
    m_velocity *= m_drag;
    
    // Apply small buoyancy adjustment to keep stable
    if (m_velocity.y < -0.1f) {
        m_velocity.y += m_buoyancy * deltaTime;
    } else if (m_velocity.y > 0.1f) {
        m_velocity.y -= m_buoyancy * deltaTime;
    } else {
        m_velocity.y *= 0.5f; // Dampen vertical movement
    }
    
    // Clamp maximum speed
    float maxSpeed = m_moveSpeed * 1.5f;
    float speed = glm::length(m_velocity);
    if (speed > maxSpeed) {
        m_velocity = glm::normalize(m_velocity) * maxSpeed;
    }
}

} // namespace fresh
