#pragma once

#include <string>
#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include "voxel/VoxelTypes.h"

namespace fresh
{

class VoxelWorld;
class Camera;

/**
 * @brief Tool types for voxel interaction
 */
enum class VoxelToolType {
    ConstructionHammer,  // Building tool with blueprints
    Pickaxe,             // Mining and removal tool
    Rake                 // Terrain leveling tool
};

/**
 * @brief Base class for voxel interaction tools
 * 
 * Implements the tool pattern for different voxel manipulation behaviors
 * inspired by Enshrouded's building and terraforming system.
 */
class VoxelTool
{
public:
    explicit VoxelTool(VoxelToolType type);
    virtual ~VoxelTool() = default;

    /**
     * @brief Initialize the tool
     * @param world Voxel world to interact with
     * @return true if successful
     */
    virtual bool initialize(VoxelWorld* world);

    /**
     * @brief Use the tool at a specific world position
     * @param pos World position to apply tool
     * @param voxelType Type of voxel to place (if applicable)
     * @return true if tool was successfully used
     */
    virtual bool useTool(const WorldPos& pos, VoxelType voxelType) = 0;

    /**
     * @brief Use the tool in a direction (for area operations)
     * @param pos Center position
     * @param direction Direction vector
     * @param voxelType Type of voxel to place
     * @return true if successful
     */
    virtual bool useToolDirectional(const WorldPos& pos, const glm::vec3& direction, 
                                    VoxelType voxelType);

    /**
     * @brief Get tool type
     */
    VoxelToolType getType() const { return m_type; }

    /**
     * @brief Get tool name
     */
    virtual std::string getName() const = 0;

    /**
     * @brief Get tool description
     */
    virtual std::string getDescription() const = 0;

    /**
     * @brief Set tool size/radius for area operations
     * @param size Tool size (1-10)
     */
    void setSize(int size);

    /**
     * @brief Get tool size
     */
    int getSize() const { return m_size; }

protected:
    VoxelWorld* m_world;
    VoxelToolType m_type;
    int m_size;  // Tool radius/size for area operations
};

/**
 * @brief Construction Hammer tool for building structures
 * 
 * Features:
 * - Pre-made blueprint shapes (walls, foundations, etc.)
 * - Single-block placement for detailed work
 * - Seamless material blending
 */
class ConstructionHammer : public VoxelTool
{
public:
    ConstructionHammer();

    bool useTool(const WorldPos& pos, VoxelType voxelType) override;
    bool useToolDirectional(const WorldPos& pos, const glm::vec3& direction, 
                           VoxelType voxelType) override;

    std::string getName() const override { return "Construction Hammer"; }
    std::string getDescription() const override { 
        return "Primary building tool with blueprints and single-block placement"; 
    }

    /**
     * @brief Blueprint types for quick construction
     */
    enum class Blueprint {
        SingleBlock,      // Single voxel placement
        Wall,             // Vertical wall (5 blocks high)
        Foundation,       // Horizontal foundation (3x3)
        Column,           // Vertical column (8 blocks high)
        Floor,            // Flat floor (5x5)
        Stairs,           // Staircase (5 blocks)
        Arch              // Simple arch structure
    };

    /**
     * @brief Set current blueprint
     */
    void setBlueprint(Blueprint blueprint) { m_currentBlueprint = blueprint; }

    /**
     * @brief Get current blueprint
     */
    Blueprint getBlueprint() const { return m_currentBlueprint; }

    /**
     * @brief Place a blueprint structure at position
     * @param pos Center position for blueprint
     * @param voxelType Material type
     * @return true if successful
     */
    bool placeBlueprint(const WorldPos& pos, VoxelType voxelType);

private:
    Blueprint m_currentBlueprint;

    // Blueprint placement helpers
    void placeWall(const WorldPos& pos, VoxelType voxelType);
    void placeFoundation(const WorldPos& pos, VoxelType voxelType);
    void placeColumn(const WorldPos& pos, VoxelType voxelType);
    void placeFloor(const WorldPos& pos, VoxelType voxelType);
    void placeStairs(const WorldPos& pos, VoxelType voxelType);
    void placeArch(const WorldPos& pos, VoxelType voxelType);
};

/**
 * @brief Pickaxe tool for mining and removal
 * 
 * Features:
 * - Remove terrain and blocks
 * - Creates rougher, rustic edges
 * - Can mine resources
 */
class Pickaxe : public VoxelTool
{
public:
    Pickaxe();

    bool useTool(const WorldPos& pos, VoxelType voxelType) override;

    std::string getName() const override { return "Pickaxe"; }
    std::string getDescription() const override { 
        return "Mining tool for removing terrain and blocks with rustic edges"; 
    }

    /**
     * @brief Remove voxel at position
     * @param pos Position to remove
     * @return true if successful
     */
    bool removeVoxel(const WorldPos& pos);

    /**
     * @brief Remove voxels in a sphere (for area mining)
     * @param pos Center position
     * @param radius Sphere radius
     * @return true if successful
     */
    bool removeArea(const WorldPos& pos, int radius);
};

/**
 * @brief Rake tool for terrain leveling
 * 
 * Features:
 * - Create flat surfaces for foundations
 * - Smooth terrain transitions
 * - Level ground to specific height
 */
class Rake : public VoxelTool
{
public:
    Rake();

    bool useTool(const WorldPos& pos, VoxelType voxelType) override;

    std::string getName() const override { return "Rake"; }
    std::string getDescription() const override { 
        return "Leveling tool for creating flat surfaces and smooth terrain"; 
    }

    /**
     * @brief Flatten terrain to a specific height
     * @param pos Center position
     * @param targetHeight Height to flatten to
     * @param radius Area radius
     * @return true if successful
     */
    bool flattenToHeight(const WorldPos& pos, int targetHeight, int radius);

    /**
     * @brief Smooth terrain in an area
     * @param pos Center position
     * @param radius Smoothing radius
     * @return true if successful
     */
    bool smoothTerrain(const WorldPos& pos, int radius);

    /**
     * @brief Level ground at current height
     * @param pos Center position
     * @return true if successful (uses tool size for radius)
     */
    bool levelGround(const WorldPos& pos);
};

} // namespace fresh
