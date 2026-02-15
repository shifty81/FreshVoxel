#pragma once

#include "VoxelTypes.h"
#include <string>
#include <glm/glm.hpp>

namespace fresh
{

/**
 * @brief Voxel material properties
 */
enum class VoxelMaterial {
    Stone,      // Hard, requires pickaxe
    Dirt,       // Soft, diggable by hand
    Wood,       // Medium, requires axe
    Metal,      // Very hard, requires better tools
    Glass,      // Fragile, transparent
    Liquid,     // Flowing, non-solid
    Plant,      // Soft, cuttable
    Special     // Unique properties
};

/**
 * @brief Information about voxel types
 * 
 * Provides metadata for each voxel type including name, color,
 * material properties, and behavior characteristics.
 */
class VoxelTypeInfo
{
public:
    /**
     * @brief Get display name for voxel type
     * @param type Voxel type
     * @return Display name
     */
    static const char* getName(VoxelType type);

    /**
     * @brief Get base color for voxel type
     * @param type Voxel type
     * @return RGB color (0-255)
     */
    static glm::vec3 getColor(VoxelType type);

    /**
     * @brief Get material type
     * @param type Voxel type
     * @return Material classification
     */
    static VoxelMaterial getMaterial(VoxelType type);

    /**
     * @brief Get hardness value (mining time multiplier)
     * @param type Voxel type
     * @return Hardness (1.0 = normal, higher = harder)
     */
    static float getHardness(VoxelType type);

    /**
     * @brief Check if voxel is transparent
     * @param type Voxel type
     * @return True if transparent
     */
    static bool isTransparent(VoxelType type);

    /**
     * @brief Check if voxel is opaque
     * @param type Voxel type
     * @return True if opaque
     */
    static bool isOpaque(VoxelType type);

    /**
     * @brief Check if voxel is solid
     * @param type Voxel type
     * @return True if solid (not air)
     */
    static bool isSolid(VoxelType type);

    /**
     * @brief Check if voxel is liquid
     * @param type Voxel type
     * @return True if liquid
     */
    static bool isLiquid(VoxelType type);

    /**
     * @brief Check if voxel emits light
     * @param type Voxel type
     * @return True if emits light
     */
    static bool isLightSource(VoxelType type);

    /**
     * @brief Get light level emitted by voxel
     * @param type Voxel type
     * @return Light level (0-15)
     */
    static uint8_t getLightEmission(VoxelType type);

    /**
     * @brief Check if voxel is mineable
     * @param type Voxel type
     * @return True if can be mined/broken
     */
    static bool isMineable(VoxelType type);

    /**
     * @brief Get texture coordinates for voxel face
     * @param type Voxel type
     * @param faceIndex Face index (0-5: +X, -X, +Y, -Y, +Z, -Z)
     * @return Texture atlas coordinates (0-15)
     */
    static glm::ivec2 getTextureCoords(VoxelType type, int faceIndex);

    /**
     * @brief Check if voxel is flammable
     * @param type Voxel type
     * @return True if can catch fire
     */
    static bool isFlammable(VoxelType type);

    /**
     * @brief Get tool requirement level
     * @param type Voxel type
     * @return Tool level (0 = hand, 1 = wood, 2 = stone, 3 = iron, 4 = diamond)
     */
    static int getToolRequirement(VoxelType type);

    /**
     * @brief Get blast resistance
     * @param type Voxel type
     * @return Blast resistance value
     */
    static float getBlastResistance(VoxelType type);

    /**
     * @brief Check if voxel is renewable
     * @param type Voxel type
     * @return True if can be regrown/replenished
     */
    static bool isRenewable(VoxelType type);

    /**
     * @brief Get voxel drop when broken
     * @param type Voxel type broken
     * @param usedTool Tool used to break it
     * @return Voxel type that drops
     */
    static VoxelType getDropType(VoxelType type, VoxelType usedTool = VoxelType::Air);

    /**
     * @brief Get drop quantity multiplier
     * @param type Voxel type
     * @return Quantity multiplier (typically 1)
     */
    static int getDropQuantity(VoxelType type);
};

} // namespace fresh
