#pragma once

#include <memory>
#include <unordered_map>
#include "VoxelTool.h"

namespace fresh
{

class VoxelWorld;

/**
 * @brief Manages voxel interaction tools
 * 
 * Provides tool selection, switching, and management for the player.
 * Integrates with the existing VoxelInteraction system.
 */
class ToolManager
{
public:
    ToolManager();
    ~ToolManager();

    /**
     * @brief Initialize tool manager with world reference
     * @param world VoxelWorld instance
     * @return true if successful
     */
    bool initialize(VoxelWorld* world);

    /**
     * @brief Select a tool by type
     * @param type Tool type to select
     * @return true if tool exists and was selected
     */
    bool selectTool(VoxelToolType type);

    /**
     * @brief Get currently active tool
     * @return Pointer to active tool, or nullptr if none selected
     */
    VoxelTool* getActiveTool() const { return m_activeTool; }

    /**
     * @brief Get a specific tool by type
     * @param type Tool type
     * @return Pointer to tool, or nullptr if not found
     */
    VoxelTool* getTool(VoxelToolType type) const;

    /**
     * @brief Get Construction Hammer tool
     */
    ConstructionHammer* getConstructionHammer() const;

    /**
     * @brief Get Pickaxe tool
     */
    Pickaxe* getPickaxe() const;

    /**
     * @brief Get Rake tool
     */
    Rake* getRake() const;

    /**
     * @brief Use the active tool at a position
     * @param pos World position
     * @param voxelType Type of voxel (for construction)
     * @return true if successful
     */
    bool useActiveTool(const WorldPos& pos, VoxelType voxelType);

    /**
     * @brief Cycle to next tool
     */
    void cycleNextTool();

    /**
     * @brief Cycle to previous tool
     */
    void cyclePreviousTool();

    /**
     * @brief Get active tool type
     */
    VoxelToolType getActiveToolType() const;

private:
    void createTools();

private:
    VoxelWorld* m_world;
    std::unordered_map<VoxelToolType, std::unique_ptr<VoxelTool>> m_tools;
    VoxelTool* m_activeTool;
    VoxelToolType m_activeToolType;
};

} // namespace fresh
