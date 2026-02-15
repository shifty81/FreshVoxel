#pragma once

#include <vector>
#include <functional>
#include "rpg/Inventory.h"

namespace fresh
{

/**
 * @brief Item slot information for UI display
 */
struct InventorySlot {
    rpg::ResourceType type;
    float amount;
    bool isEmpty;
    int slotIndex;

    InventorySlot() 
        : type(rpg::ResourceType::Iron)
        , amount(0.0f)
        , isEmpty(true)
        , slotIndex(-1)
    {}
};

/**
 * @brief Inventory panel for displaying and managing player inventory
 * 
 * Provides a grid-based inventory UI with drag-and-drop support,
 * item tooltips, and split stack functionality. Designed to work
 * with the native Windows UI framework.
 */
class InventoryPanel
{
public:
    /**
     * @brief Create inventory panel
     * @param slotCount Number of inventory slots (default 40)
     */
    explicit InventoryPanel(int slotCount = 40);
    ~InventoryPanel() = default;

    /**
     * @brief Update inventory panel
     * @param deltaTime Time since last update
     */
    void update(float deltaTime);

    /**
     * @brief Render the inventory panel
     * @note Implementation will use Windows native controls
     */
    void render();

    /**
     * @brief Set the inventory to display
     * @param inventory Reference to inventory data
     */
    void setInventory(rpg::Inventory* inventory);

    /**
     * @brief Get currently displayed inventory
     * @return Pointer to inventory or nullptr
     */
    rpg::Inventory* getInventory() const { return m_inventory; }

    /**
     * @brief Show/hide the inventory panel
     * @param visible True to show, false to hide
     */
    void setVisible(bool visible);

    /**
     * @brief Check if panel is visible
     * @return True if visible
     */
    bool isVisible() const { return m_visible; }

    /**
     * @brief Toggle visibility
     */
    void toggleVisibility() { setVisible(!m_visible); }

    /**
     * @brief Set callback for when item is used
     * @param callback Function called when item is used (resourceType, amount)
     */
    void setOnItemUsed(std::function<void(rpg::ResourceType, float)> callback) {
        m_onItemUsed = callback;
    }

    /**
     * @brief Set callback for when item is dropped
     * @param callback Function called when item is dropped
     */
    void setOnItemDropped(std::function<void(rpg::ResourceType, float)> callback) {
        m_onItemDropped = callback;
    }

    /**
     * @brief Get all inventory slots for rendering
     * @return Vector of inventory slots
     */
    const std::vector<InventorySlot>& getSlots() const { return m_slots; }

    /**
     * @brief Get selected slot index
     * @return Selected slot index or -1 if none
     */
    int getSelectedSlot() const { return m_selectedSlot; }

    /**
     * @brief Set selected slot
     * @param slotIndex Slot to select
     */
    void setSelectedSlot(int slotIndex);

    /**
     * @brief Get capacity information
     * @return Formatted string "Used / Total"
     */
    std::string getCapacityString() const;

    /**
     * @brief Handle item transfer between slots
     * @param fromSlot Source slot index
     * @param toSlot Destination slot index
     * @param amount Amount to transfer (0 = all)
     */
    void transferItem(int fromSlot, int toSlot, float amount = 0.0f);

    /**
     * @brief Handle item drop from inventory
     * @param slotIndex Slot to drop from
     * @param amount Amount to drop (0 = all)
     */
    void dropItem(int slotIndex, float amount = 0.0f);

    /**
     * @brief Sort inventory by item type
     */
    void sortByType();

    /**
     * @brief Sort inventory by amount (descending)
     */
    void sortByAmount();

    /**
     * @brief Enable/disable drag and drop
     * @param enabled True to enable
     */
    void setDragDropEnabled(bool enabled) { m_dragDropEnabled = enabled; }

    /**
     * @brief Check if drag and drop is enabled
     * @return True if enabled
     */
    bool isDragDropEnabled() const { return m_dragDropEnabled; }

private:
    /**
     * @brief Refresh slot data from inventory
     */
    void refreshSlots();

    /**
     * @brief Get resource icon path
     * @param type Resource type
     * @return Icon file path
     */
    std::string getResourceIcon(rpg::ResourceType type) const;

    /**
     * @brief Get resource color for UI display
     * @param type Resource type
     * @return RGB color values (0-255)
     */
    struct Color {
        unsigned char r, g, b;
    };
    Color getResourceColor(rpg::ResourceType type) const;

private:
    rpg::Inventory* m_inventory;
    std::vector<InventorySlot> m_slots;
    int m_selectedSlot;
    bool m_visible;
    bool m_dragDropEnabled;
    int m_slotCount;

    // Drag and drop state
    bool m_dragging;
    int m_dragSourceSlot;
    
    // Callbacks
    std::function<void(rpg::ResourceType, float)> m_onItemUsed;
    std::function<void(rpg::ResourceType, float)> m_onItemDropped;
};

} // namespace fresh
