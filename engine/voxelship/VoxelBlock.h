#pragma once
#include <string>

#include <glm/glm.hpp>

namespace fresh
{
namespace voxelship
{

/**
 * @brief Material properties for voxel blocks
 */
struct VoxelMaterial {
    float mass;         // Mass per unit volume (kg/m³)
    float hardness;     // Resistance to damage
    float conductivity; // Thermal/electrical conductivity
    float maxStretchRatio; // Maximum stretch ratio allowed (structural integrity limit)
    std::string name;

    VoxelMaterial()
        : mass(1.0f), hardness(1.0f), conductivity(0.0f), maxStretchRatio(5.0f), name("Default")
    {
    }
};

/**
 * @brief Represents a single voxel block with position, size, and material
 *
 * Used for flexible ship and station construction with arbitrary-sized blocks.
 * Supports block stretching with structural integrity constraints.
 */
class VoxelBlock
{
public:
    VoxelBlock();
    VoxelBlock(const glm::vec3& pos, const glm::vec3& sz, const VoxelMaterial& mat);

    // Getters
    const glm::vec3& getPosition() const
    {
        return position;
    }
    const glm::vec3& getSize() const
    {
        return size;
    }
    const VoxelMaterial& getMaterial() const
    {
        return material;
    }
    float getMass() const
    {
        return material.mass * (size.x * size.y * size.z);
    }

    // Setters
    void setPosition(const glm::vec3& pos)
    {
        position = pos;
    }
    void setSize(const glm::vec3& sz)
    {
        size = sz;
    }
    void setMaterial(const VoxelMaterial& mat)
    {
        material = mat;
    }

    // Block stretching with structural integrity
    bool canStretch(const glm::vec3& newSize) const;
    bool stretch(const glm::vec3& newSize);
    bool stretchCorner(int cornerIndex, const glm::vec3& delta);
    glm::vec3 getCorner(int cornerIndex) const;
    float getStretchRatio() const;
    bool isStructurallySound() const;

    // Collision detection
    bool intersects(const VoxelBlock& other) const;
    bool contains(const glm::vec3& point) const;

    // Get bounding box
    void getBounds(glm::vec3& min, glm::vec3& max) const;

private:
    glm::vec3 position;     // Center position
    glm::vec3 size;         // Dimensions (width, height, depth)
    VoxelMaterial material; // Material properties
    glm::vec3 originalSize; // Original size before stretching (for integrity checks)

    float calculateStretchRatio(const glm::vec3& testSize) const;
};

} // namespace voxelship
} // namespace fresh
