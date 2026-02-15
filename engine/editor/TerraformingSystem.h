#pragma once

#include <memory>
#include <vector>

#include "voxel/VoxelTypes.h"

namespace fresh
{

class VoxelWorld;

/**
 * @brief Tool types for terraforming
 */
enum class TerraformTool {
    SingleBlock,  // Single block placement/removal
    Brush,        // Brush with radius
    Sphere,       // Hollow sphere
    FilledSphere, // Filled sphere
    Cube,         // Hollow cube
    FilledCube,   // Filled cube
    Line,         // Line between two points
    Flatten,      // Flatten terrain to height
    Smooth,       // Smooth terrain
    Paint         // Change block type without geometry change
};

/**
 * @brief Terraforming operation mode
 */
enum class TerraformMode {
    Place,  // Place blocks
    Remove, // Remove blocks
    Replace // Replace one type with another
};

/**
 * @brief Command pattern for undo/redo
 */
struct TerraformCommand {
    WorldPos position;
    Voxel oldVoxel;
    Voxel newVoxel;
};

/**
 * @brief Terraforming system for world editing
 *
 * Provides tools and functionality for modifying the voxel world,
 * including placement, removal, and advanced terraforming operations.
 */
class TerraformingSystem
{
public:
    TerraformingSystem(VoxelWorld* world);
    ~TerraformingSystem();

    /**
     * @brief Initialize terraforming system
     * @return true if successful
     */
    bool initialize();

    /**
     * @brief Set the current tool
     * @param tool Tool to use
     */
    void setTool(TerraformTool tool)
    {
        m_currentTool = tool;
    }

    /**
     * @brief Get current tool
     * @return Current tool
     */
    TerraformTool getTool() const
    {
        return m_currentTool;
    }

    /**
     * @brief Get current tool (alias for getTool)
     * @return Current tool
     */
    TerraformTool getCurrentTool() const
    {
        return m_currentTool;
    }

    /**
     * @brief Set the current mode
     * @param mode Mode to use
     */
    void setMode(TerraformMode mode)
    {
        m_currentMode = mode;
    }

    /**
     * @brief Get current mode
     * @return Current mode
     */
    TerraformMode getMode() const
    {
        return m_currentMode;
    }

    /**
     * @brief Get current mode (alias for getMode)
     * @return Current mode
     */
    TerraformMode getCurrentMode() const
    {
        return m_currentMode;
    }

    /**
     * @brief Set the voxel type to place
     * @param type Voxel type
     */
    void setVoxelType(VoxelType type)
    {
        m_selectedVoxelType = type;
    }

    /**
     * @brief Get selected voxel type
     * @return Selected voxel type
     */
    VoxelType getVoxelType() const
    {
        return m_selectedVoxelType;
    }

    /**
     * @brief Set tool size/radius
     * @param size Tool size (1-10)
     */
    void setToolSize(int size);

    /**
     * @brief Get tool size
     * @return Tool size
     */
    int getToolSize() const
    {
        return m_toolSize;
    }

    /**
     * @brief Apply current tool at position
     * @param pos World position
     * @return true if successful
     */
    bool applyTool(const WorldPos& pos);

    /**
     * @brief Apply tool between two positions (for line tool)
     * @param start Start position
     * @param end End position
     * @return true if successful
     */
    bool applyToolLine(const WorldPos& start, const WorldPos& end);

    /**
     * @brief Undo last operation
     * @return true if successful
     */
    bool undo();

    /**
     * @brief Redo last undone operation
     * @return true if successful
     */
    bool redo();

    /**
     * @brief Clear undo/redo history
     */
    void clearHistory();

    /**
     * @brief Check if undo is available
     * @return true if undo is available
     */
    bool canUndo() const
    {
        return !m_undoStack.empty();
    }

    /**
     * @brief Check if redo is available
     * @return true if redo is available
     */
    bool canRedo() const
    {
        return !m_redoStack.empty();
    }

    /**
     * @brief Get the number of commands in undo stack
     * @return Number of undo commands
     */
    size_t getUndoCount() const
    {
        return m_undoStack.size();
    }

    /**
     * @brief Begin a command group for multi-voxel operations
     * 
     * This allows external systems like SelectionManager to record
     * multiple voxel changes as a single undoable operation.
     */
    void beginCommandGroup();

    /**
     * @brief Record a single voxel change command
     * 
     * Can be called standalone or as part of a command group.
     * 
     * @param pos Position of voxel
     * @param oldVoxel Previous voxel state
     * @param newVoxel New voxel state
     */
    void recordVoxelChange(const WorldPos& pos, const Voxel& oldVoxel, const Voxel& newVoxel);

    /**
     * @brief End command group and commit to undo stack
     * 
     * All commands recorded since beginCommandGroup() are grouped
     * as a single undoable operation.
     */
    void endCommandGroup();

private:
    void applySingle(const WorldPos& pos);
    void applyBrush(const WorldPos& pos);
    void applySphere(const WorldPos& pos, bool filled);
    void applyCube(const WorldPos& pos, bool filled);
    void applyFlatten(const WorldPos& pos);
    void applySmooth(const WorldPos& pos);
    void applyPaint(const WorldPos& pos);

    void setVoxel(const WorldPos& pos, const Voxel& voxel);
    void recordCommand(const WorldPos& pos, const Voxel& oldVoxel, const Voxel& newVoxel);
    void executeCommands(std::vector<TerraformCommand>& commands);

private:
    VoxelWorld* m_world;
    TerraformTool m_currentTool;
    TerraformMode m_currentMode;
    VoxelType m_selectedVoxelType;
    int m_toolSize;

    std::vector<std::vector<TerraformCommand>> m_undoStack;
    std::vector<std::vector<TerraformCommand>> m_redoStack;
    std::vector<TerraformCommand> m_currentOperation;
    
    // Command grouping state
    bool m_inCommandGroup;
    std::vector<TerraformCommand> m_commandGroup;

    const size_t MAX_UNDO_STACK_SIZE = 100;
};

} // namespace fresh
