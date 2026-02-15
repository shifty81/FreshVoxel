#pragma once

#include <vector>
#include <unordered_set>
#include <glm/glm.hpp>
#include "voxel/VoxelTypes.h"

namespace fresh
{

/**
 * @brief Represents a single voxel position in world space
 */
struct VoxelPosition {
    int x, y, z;

    VoxelPosition() : x(0), y(0), z(0) {}
    VoxelPosition(int _x, int _y, int _z) : x(_x), y(_y), z(_z) {}
    VoxelPosition(const WorldPos& pos) : x(pos.x), y(pos.y), z(pos.z) {}

    bool operator==(const VoxelPosition& other) const noexcept
    {
        return x == other.x && y == other.y && z == other.z;
    }

    bool operator!=(const VoxelPosition& other) const noexcept
    {
        return !(*this == other);
    }

    WorldPos toWorldPos() const
    {
        return WorldPos(x, y, z);
    }
};

/**
 * @brief Represents a selection of voxels with their types
 */
struct VoxelSelection {
    std::vector<VoxelPosition> positions;
    std::vector<VoxelType> types;
    glm::ivec3 boundsMin;
    glm::ivec3 boundsMax;

    void clear()
    {
        positions.clear();
        types.clear();
        boundsMin = glm::ivec3(0);
        boundsMax = glm::ivec3(0);
    }

    bool isEmpty() const
    {
        return positions.empty();
    }

    size_t size() const
    {
        return positions.size();
    }
};

/**
 * @brief Manages voxel selection for the editor
 * 
 * Handles box selection, manipulation, and clipboard operations for voxels.
 * This is a critical component for Cut/Copy/Paste functionality.
 */
class SelectionManager
{
public:
    SelectionManager();
    ~SelectionManager();

    // Selection operations
    /**
     * @brief Start a new box selection
     * @param worldPos Starting position in world space
     */
    void startSelection(const glm::vec3& worldPos);

    /**
     * @brief Update the selection region as mouse drags
     * @param worldPos Current position in world space
     */
    void updateSelection(const glm::vec3& worldPos);

    /**
     * @brief Finalize the selection and build the list of selected voxels
     * @param world Pointer to voxel world to query voxel types
     */
    void finalizeSelection(class VoxelWorld* world);

    /**
     * @brief Clear the current selection
     */
    void clearSelection();

    // Query
    /**
     * @brief Check if there is an active selection
     * @return true if selection is active or has selected voxels
     */
    bool hasSelection() const;

    /**
     * @brief Check if currently in selection mode (dragging)
     * @return true if actively selecting
     */
    bool isSelecting() const
    {
        return m_isSelecting;
    }

    /**
     * @brief Get the number of selected voxels
     * @return Count of selected voxels
     */
    size_t getSelectionSize() const;

    /**
     * @brief Get the list of selected voxel positions
     * @return Vector of voxel positions
     */
    const std::vector<VoxelPosition>& getSelectedVoxels() const;

    /**
     * @brief Get the selection bounds for rendering
     * @param min Output minimum corner
     * @param max Output maximum corner
     * @return true if selection exists
     */
    bool getSelectionBounds(glm::ivec3& min, glm::ivec3& max) const;

    // Manipulation
    /**
     * @brief Delete all selected voxels
     * @param world Pointer to voxel world to modify
     */
    void deleteSelected(VoxelWorld* world);

    /**
     * @brief Move selected voxels by a delta
     * @param delta Movement vector
     * @param world Pointer to voxel world to modify
     */
    void moveSelection(const glm::ivec3& delta, VoxelWorld* world);

    // Clipboard operations
    /**
     * @brief Copy selected voxels to clipboard
     * @param world Pointer to voxel world to query voxel data
     */
    void copyToClipboard(VoxelWorld* world);

    /**
     * @brief Cut selected voxels (copy and delete)
     * @param world Pointer to voxel world to modify
     */
    void cutToClipboard(VoxelWorld* world);

    /**
     * @brief Check if clipboard has data
     * @return true if clipboard is not empty
     */
    bool hasClipboardData() const;

    /**
     * @brief Get clipboard data for preview
     * @return Const reference to clipboard selection
     */
    const VoxelSelection& getClipboardData() const
    {
        return m_clipboard;
    }

    /**
     * @brief Paste voxels from clipboard at specified position
     * @param pastePos Position to paste at (bottom-left corner)
     * @param world Pointer to voxel world to modify
     */
    void pasteFromClipboard(const glm::ivec3& pastePos, VoxelWorld* world);

    /**
     * @brief Set terraforming system for undo/redo integration
     * @param system Pointer to terraforming system (can be nullptr to disable)
     */
    void setTerraformingSystem(class TerraformingSystem* system)
    {
        m_terraformingSystem = system;
    }

    // Paste preview functionality
    /**
     * @brief Enable paste preview mode
     * @param pastePos Position where paste will occur
     */
    void enablePastePreview(const glm::ivec3& pastePos);

    /**
     * @brief Update paste preview position
     * @param pastePos New position for preview
     */
    void updatePastePreview(const glm::ivec3& pastePos);

    /**
     * @brief Disable paste preview mode
     */
    void disablePastePreview();

    /**
     * @brief Check if paste preview is active
     * @return true if preview is showing
     */
    bool isPastePreviewActive() const
    {
        return m_pastePreviewActive;
    }

    /**
     * @brief Get paste preview data for rendering
     * @param positions Output vector of preview positions
     * @param types Output vector of preview voxel types
     * @return true if preview is active and has data
     */
    bool getPastePreviewData(std::vector<VoxelPosition>& positions,
                             std::vector<VoxelType>& types) const;

    /**
     * @brief Get paste preview bounds for rendering
     * @param min Output minimum corner
     * @param max Output maximum corner
     * @return true if preview is active
     */
    bool getPastePreviewBounds(glm::ivec3& min, glm::ivec3& max) const;

private:
    /**
     * @brief Build selection from box defined by start and end points
     * @param world Pointer to voxel world to query voxels
     */
    void buildBoxSelection(VoxelWorld* world);

    /**
     * @brief Calculate bounds from current selection
     */
    void calculateBounds();

private:
    // Current selection state
    bool m_isSelecting;
    glm::vec3 m_selectionStart;
    glm::vec3 m_selectionEnd;
    
    // Selected voxels
    VoxelSelection m_selection;
    
    // Clipboard data
    VoxelSelection m_clipboard;
    
    // Undo/redo integration
    class TerraformingSystem* m_terraformingSystem;
    
    // Paste preview state
    bool m_pastePreviewActive;
    glm::ivec3 m_pastePreviewPosition;
};

} // namespace fresh

// Hash function for VoxelPosition to use in unordered_set
namespace std
{
template <>
struct hash<fresh::VoxelPosition> {
    std::size_t operator()(const fresh::VoxelPosition& pos) const noexcept
    {
        std::size_t seed = 0;
        seed ^= std::hash<int>{}(pos.x) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        seed ^= std::hash<int>{}(pos.y) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        seed ^= std::hash<int>{}(pos.z) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        return seed;
    }
};
} // namespace std
