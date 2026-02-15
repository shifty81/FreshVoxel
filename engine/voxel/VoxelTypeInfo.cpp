#include "voxel/VoxelTypeInfo.h"

namespace fresh
{

const char* VoxelTypeInfo::getName(VoxelType type)
{
    switch (type) {
    case VoxelType::Air: return "Air";
    case VoxelType::Stone: return "Stone";
    case VoxelType::Dirt: return "Dirt";
    case VoxelType::Grass: return "Grass";
    case VoxelType::Sand: return "Sand";
    case VoxelType::Water: return "Water";
    case VoxelType::Wood: return "Wood";
    case VoxelType::Leaves: return "Leaves";
    case VoxelType::Bedrock: return "Bedrock";
    case VoxelType::Snow: return "Snow";
    case VoxelType::Ice: return "Ice";
    case VoxelType::Cobblestone: return "Cobblestone";
    case VoxelType::Planks: return "Planks";
    case VoxelType::Glass: return "Glass";
    case VoxelType::Brick: return "Brick";
    case VoxelType::Gravel: return "Gravel";
    case VoxelType::Coal: return "Coal Block";
    case VoxelType::Iron: return "Iron Block";
    case VoxelType::Gold: return "Gold Block";
    case VoxelType::Diamond: return "Diamond Block";
    case VoxelType::Obsidian: return "Obsidian";
    case VoxelType::Clay: return "Clay";
    case VoxelType::Mud: return "Mud";
    case VoxelType::SandStone: return "Sandstone";
    case VoxelType::RedSand: return "Red Sand";
    case VoxelType::RedSandStone: return "Red Sandstone";
    case VoxelType::CoalOre: return "Coal Ore";
    case VoxelType::IronOre: return "Iron Ore";
    case VoxelType::GoldOre: return "Gold Ore";
    case VoxelType::DiamondOre: return "Diamond Ore";
    case VoxelType::EmeraldOre: return "Emerald Ore";
    case VoxelType::StoneBrick: return "Stone Brick";
    case VoxelType::MossyStoneBrick: return "Mossy Stone Brick";
    case VoxelType::CrackedStoneBrick: return "Cracked Stone Brick";
    case VoxelType::ChiseledStoneBrick: return "Chiseled Stone Brick";
    case VoxelType::OakWood: return "Oak Wood";
    case VoxelType::BirchWood: return "Birch Wood";
    case VoxelType::SpruceWood: return "Spruce Wood";
    case VoxelType::JungleWood: return "Jungle Wood";
    case VoxelType::Wool: return "Wool";
    case VoxelType::Concrete: return "Concrete";
    case VoxelType::Terracotta: return "Terracotta";
    case VoxelType::Torch: return "Torch";
    case VoxelType::Ladder: return "Ladder";
    case VoxelType::Door: return "Door";
    default: return "Unknown";
    }
}

glm::vec3 VoxelTypeInfo::getColor(VoxelType type)
{
    switch (type) {
    case VoxelType::Air: return {0, 0, 0};
    case VoxelType::Stone: return {128, 128, 128};
    case VoxelType::Dirt: return {139, 69, 19};
    case VoxelType::Grass: return {34, 139, 34};
    case VoxelType::Sand: return {238, 214, 175};
    case VoxelType::Water: return {30, 144, 255};
    case VoxelType::Wood: return {139, 90, 43};
    case VoxelType::Leaves: return {34, 139, 34};
    case VoxelType::Bedrock: return {50, 50, 50};
    case VoxelType::Snow: return {255, 250, 250};
    case VoxelType::Ice: return {175, 225, 255};
    case VoxelType::Cobblestone: return {128, 128, 128};
    case VoxelType::Planks: return {139, 90, 43};
    case VoxelType::Glass: return {200, 230, 255};
    case VoxelType::Brick: return {150, 75, 50};
    case VoxelType::Gravel: return {136, 140, 141};
    case VoxelType::Coal: return {50, 50, 50};
    case VoxelType::Iron: return {167, 167, 173};
    case VoxelType::Gold: return {255, 215, 0};
    case VoxelType::Diamond: return {185, 242, 255};
    case VoxelType::Obsidian: return {16, 16, 30};
    case VoxelType::Clay: return {164, 168, 184};
    case VoxelType::Mud: return {92, 64, 51};
    case VoxelType::SandStone: return {222, 207, 163};
    case VoxelType::RedSand: return {169, 88, 32};
    case VoxelType::RedSandStone: return {185, 101, 54};
    case VoxelType::CoalOre: return {52, 52, 52};
    case VoxelType::IronOre: return {150, 134, 123};
    case VoxelType::GoldOre: return {206, 174, 78};
    case VoxelType::DiamondOre: return {109, 184, 181};
    case VoxelType::EmeraldOre: return {82, 171, 98};
    case VoxelType::StoneBrick: return {123, 123, 123};
    case VoxelType::MossyStoneBrick: return {100, 118, 100};
    case VoxelType::CrackedStoneBrick: return {115, 115, 115};
    case VoxelType::ChiseledStoneBrick: return {120, 120, 120};
    case VoxelType::OakWood: return {150, 110, 70};
    case VoxelType::BirchWood: return {217, 210, 195};
    case VoxelType::SpruceWood: return {74, 56, 41};
    case VoxelType::JungleWood: return {120, 85, 60};
    case VoxelType::Wool: return {220, 220, 220};
    case VoxelType::Concrete: return {188, 188, 188};
    case VoxelType::Terracotta: return {152, 94, 67};
    case VoxelType::Torch: return {255, 200, 100};
    case VoxelType::Ladder: return {139, 90, 43};
    case VoxelType::Door: return {139, 90, 43};
    default: return {128, 128, 128}; // Gray
    }
}

VoxelMaterial VoxelTypeInfo::getMaterial(VoxelType type)
{
    switch (type) {
    case VoxelType::Stone:
    case VoxelType::Cobblestone:
    case VoxelType::StoneBrick:
    case VoxelType::MossyStoneBrick:
    case VoxelType::CrackedStoneBrick:
    case VoxelType::ChiseledStoneBrick:
    case VoxelType::Brick:
    case VoxelType::SandStone:
    case VoxelType::RedSandStone:
        return VoxelMaterial::Stone;

    case VoxelType::CoalOre:
    case VoxelType::CoalOre:
    case VoxelType::IronOre:
    case VoxelType::GoldOre:
    case VoxelType::DiamondOre:
    case VoxelType::EmeraldOre:
    case VoxelType::Iron:
    case VoxelType::Gold:
    case VoxelType::Diamond:
    case VoxelType::Coal:
    case VoxelType::Obsidian:
        return VoxelMaterial::Metal;

    case VoxelType::Wood:
    case VoxelType::Planks:
    case VoxelType::OakWood:
    case VoxelType::BirchWood:
    case VoxelType::SpruceWood:
    case VoxelType::JungleWood:
        return VoxelMaterial::Wood;

    case VoxelType::Glass:
        return VoxelMaterial::Glass;

    case VoxelType::Water:
        return VoxelMaterial::Liquid;

    case VoxelType::Leaves:
    case VoxelType::Ladder:
        return VoxelMaterial::Plant;

    case VoxelType::Bedrock:
        return VoxelMaterial::Special;

    default:
        return VoxelMaterial::Stone;
    }
}

float VoxelTypeInfo::getHardness(VoxelType type)
{
    switch (type) {
    case VoxelType::Bedrock:
        return 100.0f; // Unbreakable in normal gameplay
    case VoxelType::Obsidian:
        return 50.0f;
    case VoxelType::Diamond:
    case VoxelType::DiamondOre:
        return 15.0f;
    case VoxelType::Iron:
    case VoxelType::IronOre:
        return 5.0f;
    case VoxelType::Stone:
    case VoxelType::Cobblestone:
    case VoxelType::StoneBrick:
        return 3.0f;
    case VoxelType::Wood:
    case VoxelType::OakWood:
    case VoxelType::BirchWood:
    case VoxelType::SpruceWood:
    case VoxelType::JungleWood:
    case VoxelType::Planks:
        return 2.0f;
    case VoxelType::Dirt:
    case VoxelType::Grass:
    case VoxelType::Sand:
    case VoxelType::Gravel:
    case VoxelType::Clay:
    case VoxelType::Mud:
        return 0.5f;
    case VoxelType::Glass:
        return 0.3f;
    case VoxelType::Leaves:
        return 0.2f;
    case VoxelType::Bedrock:
        return -1.0f; // Unbreakable
    default:
        return 1.0f;
    }
}

bool VoxelTypeInfo::isTransparent(VoxelType type)
{
    return type == VoxelType::Air || 
           type == VoxelType::Water || 
           type == VoxelType::Glass ||
           type == VoxelType::Ice ||
           type == VoxelType::Leaves;
}

bool VoxelTypeInfo::isOpaque(VoxelType type)
{
    return !isTransparent(type) && type != VoxelType::Air;
}

bool VoxelTypeInfo::isSolid(VoxelType type)
{
    return type != VoxelType::Air;
}

bool VoxelTypeInfo::isLiquid(VoxelType type)
{
    return type == VoxelType::Water;
}

bool VoxelTypeInfo::isLightSource(VoxelType type)
{
    return type == VoxelType::Torch;
}

uint8_t VoxelTypeInfo::getLightEmission(VoxelType type)
{
    if (type == VoxelType::Torch) {
        return 15; // Maximum light
    }
    return 0;
}

bool VoxelTypeInfo::isMineable(VoxelType type)
{
    return type != VoxelType::Air && type != VoxelType::Bedrock;
}

glm::ivec2 VoxelTypeInfo::getTextureCoords(VoxelType type, int faceIndex)
{
    // This is a simplified texture coordinate system
    // In a real implementation, this would map to a texture atlas
    // For now, return basic coordinates based on type
    int typeIndex = static_cast<int>(type);
    
    // Simple layout: each voxel type gets a row, faces are columns
    int row = typeIndex / 16;
    int col = (typeIndex % 16 + faceIndex) % 16;
    
    return glm::ivec2(col, row);
}

bool VoxelTypeInfo::isFlammable(VoxelType type)
{
    return type == VoxelType::Wood ||
           type == VoxelType::Planks ||
           type == VoxelType::Leaves ||
           type == VoxelType::OakWood ||
           type == VoxelType::BirchWood ||
           type == VoxelType::SpruceWood ||
           type == VoxelType::JungleWood ||
           type == VoxelType::Wool;
}

int VoxelTypeInfo::getToolRequirement(VoxelType type)
{
    VoxelMaterial material = getMaterial(type);
    
    switch (material) {
    case VoxelMaterial::Stone:
        if (type == VoxelType::Obsidian || type == VoxelType::Bedrock) {
            return 4; // Diamond required
        }
        return 1; // Wood pickaxe minimum
    
    case VoxelMaterial::Metal:
        if (type == VoxelType::Gold || type == VoxelType::Diamond) {
            return 3; // Iron required
        }
        return 2; // Stone required
    
    case VoxelMaterial::Wood:
    case VoxelMaterial::Dirt:
    case VoxelMaterial::Plant:
        return 0; // Hand minable
    
    case VoxelMaterial::Glass:
        return 0; // Hand minable but tool helps
    
    case VoxelMaterial::Special:
        if (type == VoxelType::Bedrock) {
            return 100; // Cannot be mined
        }
        return 0;
    
    default:
        return 0;
    }
}

float VoxelTypeInfo::getBlastResistance(VoxelType type)
{
    switch (type) {
    case VoxelType::Bedrock:
        return 3600000.0f; // Effectively indestructible
    case VoxelType::Obsidian:
        return 1200.0f;
    case VoxelType::Stone:
    case VoxelType::Cobblestone:
    case VoxelType::StoneBrick:
        return 6.0f;
    case VoxelType::Brick:
        return 6.0f;
    case VoxelType::Wood:
    case VoxelType::Planks:
        return 3.0f;
    case VoxelType::Dirt:
    case VoxelType::Sand:
    case VoxelType::Gravel:
        return 0.5f;
    case VoxelType::Glass:
        return 0.3f;
    default:
        return 1.0f;
    }
}

bool VoxelTypeInfo::isRenewable(VoxelType type)
{
    return type == VoxelType::Wood ||
           type == VoxelType::Leaves ||
           type == VoxelType::OakWood ||
           type == VoxelType::BirchWood ||
           type == VoxelType::SpruceWood ||
           type == VoxelType::JungleWood ||
           type == VoxelType::Dirt ||
           type == VoxelType::Grass;
}

VoxelType VoxelTypeInfo::getDropType(VoxelType type, VoxelType usedTool)
{
    (void)usedTool; // Reserved for future use with silk touch
    
    // Some blocks drop themselves, others drop different items
    switch (type) {
    case VoxelType::Grass:
        return VoxelType::Dirt;
    case VoxelType::Stone:
        return VoxelType::Cobblestone;
    case VoxelType::CoalOre:
        return VoxelType::Coal;
    case VoxelType::IronOre:
        return VoxelType::Iron;
    case VoxelType::GoldOre:
        return VoxelType::Gold;
    case VoxelType::DiamondOre:
        return VoxelType::Diamond;
    case VoxelType::Glass:
        // Glass breaks into nothing without silk touch
        return VoxelType::Air;
    case VoxelType::Leaves:
        // Leaves have a chance to drop saplings (simplified to nothing here)
        return VoxelType::Air;
    default:
        // Most blocks drop themselves
        return type;
    }
}

int VoxelTypeInfo::getDropQuantity(VoxelType type)
{
    // Most blocks drop 1 item
    // Some could drop multiple (e.g., redstone ore drops 4-5 redstone)
    (void)type;
    return 1;
}

} // namespace fresh
