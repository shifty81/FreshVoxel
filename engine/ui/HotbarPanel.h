#pragma once

#include <functional>
#include <array>
#include <string>

#include "voxel/VoxelTypes.h"

namespace fresh
{

/**
 * @brief Hotbar Panel for Play Mode
 *
 * Displays a horizontal bar of 10 slots (1-0) at the bottom middle of the screen.
 * Each slot can be linked to an inventory item or tool.
 * Used during play mode for quick access to items and tools.
 * 
 * @note On Windows with FRESH_WIN32_UI defined, this class serves as a data model
 * and API compatibility layer. The actual rendering is handled by Win32HUD in EditorManager.
 * On non-Windows platforms, this class would need to implement its own rendering.
 */
class HotbarPanel
{
public:
    static constexpr int HOTBAR_SIZE = 10; // Slots 1-0 (0 is slot 10)
    
    struct HotbarSlot {
        bool isEmpty = true;
        VoxelType voxelType = VoxelType::Air;
        std::string itemName;
        int count = 0;
    };
    
    using SlotActivatedCallback = std::function<void(int slotIndex)>;

    HotbarPanel();
    ~HotbarPanel();

    /**
     * @brief Initialize the hotbar
     * @return true if successful
     */
    bool initialize();

    /**
     * @brief Render the hotbar UI
     * Only renders in play mode
     */
    void render();

    /**
     * @brief Set panel visibility
     * @param visible true to show, false to hide
     */
    void setVisible(bool visible)
    {
        m_visible = visible;
    }

    /**
     * @brief Check if panel is visible
     * @return true if visible
     */
    bool isVisible() const
    {
        return m_visible;
    }

    /**
     * @brief Set the currently selected slot
     * @param slotIndex Slot index (0-9, where 9 is the '0' key)
     */
    void setSelectedSlot(int slotIndex);

    /**
     * @brief Get the currently selected slot
     * @return Slot index (0-9)
     */
    int getSelectedSlot() const
    {
        return m_selectedSlot;
    }

    /**
     * @brief Set the contents of a hotbar slot
     * @param slotIndex Slot index (0-9)
     * @param slot Slot data
     */
    void setSlot(int slotIndex, const HotbarSlot& slot);

    /**
     * @brief Get the contents of a hotbar slot
     * @param slotIndex Slot index (0-9)
     * @return Slot data
     */
    const HotbarSlot& getSlot(int slotIndex) const;

    /**
     * @brief Clear a hotbar slot
     * @param slotIndex Slot index (0-9)
     */
    void clearSlot(int slotIndex);

    /**
     * @brief Clear all hotbar slots
     */
    void clearAllSlots();

    /**
     * @brief Set callback for slot activation (when user presses 1-0)
     */
    void setSlotActivatedCallback(SlotActivatedCallback callback)
    {
        m_slotActivatedCallback = callback;
    }

    /**
     * @brief Handle keyboard input for hotbar slot selection
     * @param key GLFW key code
     */
    void handleKeyPress(int key);

private:
    void renderSlot(int slotIndex, const HotbarSlot& slot, bool isSelected);
    const char* getVoxelTypeName(VoxelType type) const;

private:
    bool m_visible;
    int m_selectedSlot; // 0-9, where 9 is the '0' key
    std::array<HotbarSlot, HOTBAR_SIZE> m_slots;
    SlotActivatedCallback m_slotActivatedCallback;
};

} // namespace fresh
