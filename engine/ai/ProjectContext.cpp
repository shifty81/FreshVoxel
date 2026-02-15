#include "ai/ProjectContext.h"

#include <sstream>

namespace fresh
{

ProjectContext::ProjectContext() = default;

std::string ProjectContext::getEngineDescription()
{
    return
        "You are an AI assistant for the Fresh Voxel Engine, a professional Windows game editor "
        "and development platform built with C++20. The engine features:\n"
        "- A Minecraft-inspired voxel world with 16x256x16 chunk system\n"
        "- DirectX 12 primary renderer (DX11 fallback, OpenGL optional)\n"
        "- Unreal Engine-style editor with native Win32 UI\n"
        "- Lua scripting via Sol2/LuaJIT for game logic\n"
        "- Entity-Component-System (ECS) architecture\n"
        "- Behavior tree AI system for NPCs\n"
        "- Procedural world generation with noise-based terrain\n"
        "- Physics with collision detection\n"
        "- Audio system using OpenAL\n"
        "- Day/night cycle, inventory, crafting, and trading systems\n"
        "- Multiple game modes: 3D Voxel, 2D Platformer, 2D Top-Down\n\n"
        "Your role is to help developers build games using this engine. Provide practical, "
        "actionable advice with code examples when appropriate.";
}

std::string ProjectContext::getLuaScriptingContext()
{
    return
        "\n\n## Lua Scripting API\n"
        "The engine uses Lua (via Sol2/LuaJIT) for game scripting. Key APIs:\n\n"
        "### Entity Management\n"
        "```lua\n"
        "-- Create and manage entities\n"
        "local entity = world:createEntity(\"MyNPC\")\n"
        "entity:setPosition(x, y, z)\n"
        "entity:setRotation(rx, ry, rz)\n"
        "local pos = entity:getPosition()\n"
        "```\n\n"
        "### Event System\n"
        "```lua\n"
        "-- Subscribe to events\n"
        "events:on(\"player_interact\", function(entity)\n"
        "    print(\"Player interacted with: \" .. entity:getName())\n"
        "end)\n"
        "events:emit(\"custom_event\", data)\n"
        "```\n\n"
        "### World Interaction\n"
        "```lua\n"
        "-- Voxel world manipulation\n"
        "world:setBlock(x, y, z, BlockType.Stone)\n"
        "local blockType = world:getBlock(x, y, z)\n"
        "world:removeBlock(x, y, z)\n"
        "```\n\n"
        "### Time and Updates\n"
        "```lua\n"
        "-- Called every frame\n"
        "function onUpdate(deltaTime)\n"
        "    -- Game logic here\n"
        "end\n\n"
        "-- Timer utilities\n"
        "timer:after(2.0, function() print(\"Delayed!\") end)\n"
        "timer:every(1.0, function() print(\"Repeating!\") end)\n"
        "```\n";
}

std::string ProjectContext::getVoxelBuildingContext()
{
    return
        "\n\n## Voxel Types and Building\n"
        "Available block types: Air, Stone, Dirt, Grass, Sand, Water, Wood, Leaves, "
        "OakLog, BirchLog, Cobblestone, Brick, Glass, IronOre, GoldOre, DiamondOre, "
        "Coal, Bedrock, Snow, Ice, Cactus, Obsidian, Lava, Gravel, Clay, Sandstone, "
        "Planks, CraftingTable, Furnace, Torch, Bookshelf, MossyCobblestone, TNT, "
        "Wool, Flower, TallGrass, Mushroom, Pumpkin, Melon, Fence, Door, Ladder.\n\n"
        "### Chunk System\n"
        "- Chunks are 16x256x16 blocks\n"
        "- Greedy meshing optimizes rendering\n"
        "- Face culling removes hidden faces\n"
        "- Infinite world via chunk streaming\n\n"
        "### Terrain Generation\n"
        "- Perlin/Simplex noise for heightmaps\n"
        "- Biome system for varied terrain\n"
        "- Cave generation using 3D noise\n"
        "- Structure placement (trees, ores)\n";
}

std::string ProjectContext::getEditorToolsContext()
{
    return
        "\n\n## Editor Tools\n"
        "### Terraforming Tools\n"
        "- **Single Block**: Place/remove individual blocks\n"
        "- **Brush**: Paint blocks in a radius\n"
        "- **Sphere/Filled Sphere**: Create spherical shapes\n"
        "- **Cube/Filled Cube**: Create box shapes\n"
        "- **Line**: Draw lines between points\n"
        "- **Flatten**: Level terrain to a specific height\n"
        "- **Smooth**: Blend terrain heights\n"
        "- **Paint**: Change block types without modifying shape\n\n"
        "### Construction Hammer Blueprints\n"
        "- Wall, Floor, Stairs, Arch, Pillar, Ramp, Bridge\n\n"
        "### Editor Panels\n"
        "- **Scene Hierarchy**: Entity tree view (World Outliner)\n"
        "- **Inspector**: Property editor for selected entities\n"
        "- **Content Browser**: Asset management\n"
        "- **Console**: Command input and log output\n"
        "- **Terraforming Panel**: Tool selection and configuration\n"
        "- **Viewport**: 3D world view with transform gizmo\n\n"
        "### Keyboard Shortcuts\n"
        "- W/A/S/D: Move camera\n"
        "- Shift: Sprint\n"
        "- Ctrl+Z/Y: Undo/Redo\n"
        "- F: Focus on selection\n"
        "- G: Toggle grid\n"
        "- Delete: Remove selected\n";
}

std::string ProjectContext::getNPCAndAIContext()
{
    return
        "\n\n## NPC and AI System\n"
        "### Behavior Trees\n"
        "NPCs use behavior trees for decision making:\n"
        "- **Sequence Node**: Execute children in order (AND logic)\n"
        "- **Selector Node**: Try children until one succeeds (OR logic)\n"
        "- **WanderBehavior**: Random movement within radius\n"
        "- **FollowBehavior**: Follow a target entity\n\n"
        "### NPC Types\n"
        "- Friendly: Passive NPCs that interact with players\n"
        "- Trader: Buy/sell items with inventory\n"
        "- Hostile: Attack players on sight\n"
        "- Patrol: Follow waypoints\n"
        "- Guard: Protect an area\n"
        "- Wander: Roam randomly\n\n"
        "### Creating NPCs in Lua\n"
        "```lua\n"
        "local npc = world:createNPC(\"Guard\")\n"
        "npc:setPosition(100, 65, 100)\n"
        "npc:setBehavior(\"patrol\")\n"
        "npc:setWaypoints({{100,65,100}, {120,65,100}, {120,65,120}})\n"
        "npc:setDialogue(\"greeting\", \"Welcome, traveler!\")\n"
        "```\n";
}

std::string ProjectContext::generateSystemPrompt(Topic topic) const
{
    std::ostringstream prompt;
    
    // Always include the core engine description
    prompt << getEngineDescription();
    
    // Add topic-specific context
    switch (topic) {
        case Topic::LuaScripting:
            prompt << getLuaScriptingContext();
            break;
        case Topic::VoxelBuilding:
            prompt << getVoxelBuildingContext();
            break;
        case Topic::EditorTools:
            prompt << getEditorToolsContext();
            break;
        case Topic::NPCAndAI:
            prompt << getNPCAndAIContext();
            break;
        case Topic::GameDesign:
            prompt << getVoxelBuildingContext();
            prompt << getNPCAndAIContext();
            prompt << getLuaScriptingContext();
            break;
        case Topic::General:
        default:
            // Include all context for general questions
            prompt << getLuaScriptingContext();
            prompt << getVoxelBuildingContext();
            prompt << getEditorToolsContext();
            prompt << getNPCAndAIContext();
            break;
    }
    
    return prompt.str();
}

std::vector<std::string> ProjectContext::getTopicNames()
{
    return {
        "General",
        "Lua Scripting",
        "Voxel Building",
        "Editor Tools",
        "NPC & AI",
        "Game Design"
    };
}

std::string ProjectContext::topicToString(Topic topic)
{
    switch (topic) {
        case Topic::General:      return "General";
        case Topic::LuaScripting: return "Lua Scripting";
        case Topic::VoxelBuilding:return "Voxel Building";
        case Topic::EditorTools:  return "Editor Tools";
        case Topic::NPCAndAI:     return "NPC & AI";
        case Topic::GameDesign:   return "Game Design";
        default:                  return "General";
    }
}

} // namespace fresh
