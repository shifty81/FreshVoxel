#pragma once

#include <vector>
#include <unordered_set>
#include <queue>
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
 * @brief Selection mode determines how voxels are selected
 */
enum class SelectionMode {
    Box,    ///< Rectangular box selection (default)
    Brush,  ///< Paint-style selection with configurable radius
    Wand    ///< Select connected voxels of the same type (flood fill)
};

/**
 * @brief Manages voxel selection for the editor
 * 
 * Handles box, brush, and wand selection modes, manipulation, and clipboard
 * operations for voxels. This is a critical component for Cut/Copy/Paste
 * functionality.
 */
class SelectionManager
{
public:
    SelectionManager();
    ~SelectionManager();

    // Selection mode
    /**
     * @brief Set the selection mode
     * @param mode Selection mode to use
     */
    void setSelectionMode(SelectionMode mode) { m_selectionMode = mode; }

    /**
     * @brief Get the current selection mode
     * @return Current selection mode
     */
    SelectionMode getSelectionMode() const { return m_selectionMode; }

    /**
     * @brief Set brush radius for Brush selection mode
     * @param radius Brush radius in voxels (clamped to 1-16)
     */
    void setBrushRadius(int radius) { m_brushRadius = std::max(1, std::min(16, radius)); }

    /**
     * @brief Get the current brush radius
     * @return Brush radius in voxels
     */
    int getBrushRadius() const { return m_brushRadius; }

    /**
     * @brief Set maximum wand flood fill limit
     * @param limit Maximum number of voxels to select (clamped to 1-10000)
     */
    void setWandLimit(int limit) { m_wandLimit = std::max(1, std::min(10000, limit)); }

    /**
     * @brief Get the current wand selection limit
     * @return Maximum number of voxels for wand select
     */
    int getWandLimit() const { return m_wandLimit; }

    // ---------------------------------------------------------------------------
    // Viewport rubber-band (2-D screen-space box) selection
    // ---------------------------------------------------------------------------

    /**
     * @brief Begin a rubber-band selection drag in normalised viewport space.
     *
     * Call when the user starts a left-drag in the viewport without holding
     * right-mouse (right-mouse is reserved for fly camera).
     *
     * @param u  Normalised horizontal position of the drag start [0,1].
     * @param v  Normalised vertical   position of the drag start [0,1].
     */
    void beginRubberBand(float u, float v);

    /**
     * @brief Update the rubber-band rectangle as the mouse moves.
     *
     * @param u  Current normalised horizontal position [0,1].
     * @param v  Current normalised vertical   position [0,1].
     */
    void updateRubberBand(float u, float v);

    /**
     * @brief Finish the rubber-band selection and resolve it to world voxels.
     *
     * Performs a frustum-vs-AABB test for all loaded chunks using the
     * rectangular pyramid defined by the four screen-space corners.  For now
     * selects the entity/chunk whose axis-aligned bounding-box centroid
     * projects within the rectangle.
     *
     * @param world   VoxelWorld to query.
     * @param viewMat Camera view matrix (column-major, matches glm).
     * @param projMat Camera projection matrix.
     */
    void endRubberBand(class VoxelWorld* world,
                       const glm::mat4& viewMat,
                       const glm::mat4& projMat);

    /**
     * @brief Cancel an in-progress rubber-band selection without committing.
     */
    void cancelRubberBand();

    /**
     * @brief Check whether a rubber-band drag is in progress.
     * @return true while between beginRubberBand() and endRubberBand()/cancelRubberBand().
     */
    bool isRubberBandActive() const { return m_rubberBandActive; }

    /**
     * @brief Get the current rubber-band rectangle in normalised viewport space.
     *
     * Returns the axis-aligned rectangle regardless of drag direction.
     * @param uMin  Output: left edge [0,1].
     * @param vMin  Output: top  edge [0,1].
     * @param uMax  Output: right  edge [0,1].
     * @param vMax  Output: bottom edge [0,1].
     * @return true if a rubber-band is active.
     */
    bool getRubberBandRect(float& uMin, float& vMin, float& uMax, float& vMax) const;

    // Selection operations
    /**
     * @brief Start a new selection (mode-dependent)
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
     * @brief Build selection using brush (sphere) around a center point
     * @param world Pointer to voxel world to query voxels
     */
    void buildBrushSelection(VoxelWorld* world);

    /**
     * @brief Build selection using wand (flood fill of same voxel type)
     * @param world Pointer to voxel world to query voxels
     */
    void buildWandSelection(VoxelWorld* world);

    /**
     * @brief Calculate bounds from current selection
     */
    void calculateBounds();

private:
    // Current selection state
    bool m_isSelecting;
    glm::vec3 m_selectionStart;
    glm::vec3 m_selectionEnd;
    
    // Selection mode
    SelectionMode m_selectionMode;
    int m_brushRadius;
    int m_wandLimit;
    
    // Selected voxels
    VoxelSelection m_selection;
    
    // Clipboard data
    VoxelSelection m_clipboard;
    
    // Undo/redo integration
    class TerraformingSystem* m_terraformingSystem;
    
    // Paste preview state
    bool m_pastePreviewActive;
    glm::ivec3 m_pastePreviewPosition;

    // Rubber-band (viewport 2-D box) selection state
    bool  m_rubberBandActive = false;
    float m_rbStartU = 0.0f;
    float m_rbStartV = 0.0f;
    float m_rbCurU   = 0.0f;
    float m_rbCurV   = 0.0f;
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
