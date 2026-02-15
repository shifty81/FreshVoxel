#pragma once

#include "rpg/Inventory.h"
#include "ui/InventoryPanel.h"
#include "ui/HotbarPanel.h"
#include <memory>
#include <functional>

namespace fresh
{

/**
 * @brief Manages player inventory system and UI integration
 * 
 * Coordinates between the inventory data (rpg::Inventory),
 * the inventory UI panel, and the hotbar for quick access.
 * Handles item pickup, usage, dropping, and hotbar bindings.
 */
class InventoryManager
{
public:
    /**
     * @brief Create inventory manager with default capacity
     * @param capacity Inventory capacity (default 1000.0)
     * @param hotbarSize Number of hotbar slots (default 10)
     */
    explicit InventoryManager(float capacity = 1000.0f, int hotbarSize = 10);
    ~InventoryManager() = default;

    /**
     * @brief Initialize the inventory system
     * @return true if successful
     */
    bool initialize();

    /**
     * @brief Update inventory system
     * @param deltaTime Time since last update
     */
    void update(float deltaTime);

    /**
     * @brief Render inventory UI (if visible)
     */
    void render();

    // ========== Inventory Access ==========
    
    /**
     * @brief Get the player inventory
     * @return Reference to inventory
     */
    rpg::Inventory& getInventory() { return m_inventory; }
    const rpg::Inventory& getInventory() const { return m_inventory; }

    /**
     * @brief Get inventory panel for UI customization
     * @return Reference to inventory panel
     */
    InventoryPanel& getInventoryPanel() { return m_inventoryPanel; }
    const InventoryPanel& getInventoryPanel() const { return m_inventoryPanel; }

    /**
     * @brief Get hotbar panel
     * @return Reference to hotbar panel
     */
    HotbarPanel& getHotbarPanel() { return m_hotbarPanel; }
    const HotbarPanel& getHotbarPanel() const { return m_hotbarPanel; }

    // ========== Inventory Operations ==========

    /**
     * @brief Add item to inventory
     * @param type Resource type
     * @param amount Amount to add
     * @return true if added successfully
     */
    bool addItem(rpg::ResourceType type, float amount);

    /**
     * @brief Remove item from inventory
     * @param type Resource type
     * @param amount Amount to remove
     * @return true if removed successfully
     */
    bool removeItem(rpg::ResourceType type, float amount);

    /**
     * @brief Check if inventory has item
     * @param type Resource type
     * @param amount Amount to check
     * @return true if has enough
     */
    bool hasItem(rpg::ResourceType type, float amount) const;

    /**
     * @brief Get amount of item in inventory
     * @param type Resource type
     * @return Amount in inventory
     */
    float getItemAmount(rpg::ResourceType type) const;

    /**
     * @brief Check if inventory is full
     * @return true if full
     */
    bool isFull() const { return m_inventory.isFull(); }

    /**
     * @brief Get available space
     * @return Available space
     */
    float getAvailableSpace() const { return m_inventory.getAvailableSpace(); }

    // ========== UI Control ==========

    /**
     * @brief Show/hide inventory UI
     * @param visible True to show
     */
    void setInventoryVisible(bool visible);

    /**
     * @brief Check if inventory UI is visible
     * @return True if visible
     */
    bool isInventoryVisible() const { return m_inventoryPanel.isVisible(); }

    /**
     * @brief Toggle inventory UI visibility
     */
    void toggleInventory();

    /**
     * @brief Show/hide hotbar
     * @param visible True to show
     */
    void setHotbarVisible(bool visible);

    /**
     * @brief Check if hotbar is visible
     * @return True if visible
     */
    bool isHotbarVisible() const { return m_hotbarPanel.isVisible(); }

    // ========== Hotbar Integration ==========

    /**
     * @brief Bind inventory item to hotbar slot
     * @param hotbarSlot Hotbar slot index (0-9)
     * @param resourceType Resource type to bind
     * @return true if bound successfully
     */
    bool bindToHotbar(int hotbarSlot, rpg::ResourceType resourceType);

    /**
     * @brief Get resource type bound to hotbar slot
     * @param hotbarSlot Hotbar slot index
     * @return Resource type or empty if not bound
     */
    rpg::ResourceType getHotbarBinding(int hotbarSlot) const;

    /**
     * @brief Use item in selected hotbar slot
     * @param amount Amount to use (0 = 1 unit)
     * @return true if used successfully
     */
    bool useSelectedHotbarItem(float amount = 1.0f);

    /**
     * @brief Select hotbar slot
     * @param slotIndex Slot to select (0-9)
     */
    void selectHotbarSlot(int slotIndex);

    /**
     * @brief Get selected hotbar slot
     * @return Selected slot index
     */
    int getSelectedHotbarSlot() const;

    // ========== Event Callbacks ==========

    /**
     * @brief Set callback for when item is added
     * @param callback Function called when item is added
     */
    void setOnItemAdded(std::function<void(rpg::ResourceType, float)> callback) {
        m_onItemAdded = callback;
    }

    /**
     * @brief Set callback for when item is removed
     * @param callback Function called when item is removed
     */
    void setOnItemRemoved(std::function<void(rpg::ResourceType, float)> callback) {
        m_onItemRemoved = callback;
    }

    /**
     * @brief Set callback for when item is used from hotbar
     * @param callback Function called when item is used
     */
    void setOnItemUsed(std::function<void(rpg::ResourceType, float)> callback) {
        m_onItemUsed = callback;
    }

    /**
     * @brief Set callback for when inventory becomes full
     * @param callback Function called when inventory is full
     */
    void setOnInventoryFull(std::function<void()> callback) {
        m_onInventoryFull = callback;
    }

    // ========== Utility ==========

    /**
     * @brief Sort inventory by type
     */
    void sortByType() { m_inventoryPanel.sortByType(); }

    /**
     * @brief Sort inventory by amount
     */
    void sortByAmount() { m_inventoryPanel.sortByAmount(); }

    /**
     * @brief Clear all items from inventory
     */
    void clearAll();

private:
    /**
     * @brief Update hotbar display from inventory
     */
    void refreshHotbar();

    /**
     * @brief Handle item drop from hotbar
     */
    void onHotbarItemDropped(rpg::ResourceType type, float amount);

private:
    // Core inventory data
    rpg::Inventory m_inventory;

    // UI components
    InventoryPanel m_inventoryPanel;
    HotbarPanel m_hotbarPanel;

    // Hotbar bindings (slot index -> resource type)
    std::array<rpg::ResourceType, 10> m_hotbarBindings;

    // Event callbacks
    std::function<void(rpg::ResourceType, float)> m_onItemAdded;
    std::function<void(rpg::ResourceType, float)> m_onItemRemoved;
    std::function<void(rpg::ResourceType, float)> m_onItemUsed;
    std::function<void()> m_onInventoryFull;
};

} // namespace fresh
