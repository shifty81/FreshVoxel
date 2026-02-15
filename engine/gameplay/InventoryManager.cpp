#include "gameplay/InventoryManager.h"
#include "core/Logger.h"

namespace fresh
{

InventoryManager::InventoryManager(float capacity, int hotbarSize)
    : m_inventory(capacity)
    , m_inventoryPanel(40) // 40 inventory slots
    , m_hotbarPanel()
{
    // Initialize hotbar bindings to invalid type
    for (auto& binding : m_hotbarBindings) {
        binding = rpg::ResourceType::Iron; // Default to Iron
    }
}

bool InventoryManager::initialize()
{
    // Link inventory panel to inventory data
    m_inventoryPanel.setInventory(&m_inventory);

    // Initialize hotbar
    if (!m_hotbarPanel.initialize()) {
        LOG_ERROR_C("Failed to initialize hotbar panel", "InventoryManager");
        return false;
    }

    // Set up callbacks
    m_inventoryPanel.setOnItemDropped([this](rpg::ResourceType type, float amount) {
        if (m_onItemRemoved) {
            m_onItemRemoved(type, amount);
        }
    });

    m_inventoryPanel.setOnItemUsed([this](rpg::ResourceType type, float amount) {
        if (m_onItemUsed) {
            m_onItemUsed(type, amount);
        }
    });

    m_hotbarPanel.setSlotActivatedCallback([this](int slotIndex) {
        // When hotbar slot is activated, ensure it's selected
        selectHotbarSlot(slotIndex);
    });

    LOG_INFO_C("Inventory Manager initialized successfully", "InventoryManager");
    return true;
}

void InventoryManager::update(float deltaTime)
{
    // Update UI components
    m_inventoryPanel.update(deltaTime);

    // Refresh hotbar if inventory changed
    refreshHotbar();
}

void InventoryManager::render()
{
    // Render hotbar (always visible in play mode)
    m_hotbarPanel.render();

    // Render inventory panel (if visible)
    m_inventoryPanel.render();
}

bool InventoryManager::addItem(rpg::ResourceType type, float amount)
{
    if (amount <= 0.0f) {
        return false;
    }

    bool success = m_inventory.addResource(type, amount);

    if (success) {
        if (m_onItemAdded) {
            m_onItemAdded(type, amount);
        }
        LOG_INFO_C("Added " + std::to_string(amount) + "x " + 
                   rpg::getResourceName(type) + " to inventory", "InventoryManager");
    } else {
        if (m_inventory.isFull() && m_onInventoryFull) {
            m_onInventoryFull();
        }
        LOG_WARNING_C("Failed to add item - inventory full or invalid amount", "InventoryManager");
    }

    return success;
}

bool InventoryManager::removeItem(rpg::ResourceType type, float amount)
{
    if (amount <= 0.0f) {
        return false;
    }

    bool success = m_inventory.removeResource(type, amount);

    if (success) {
        if (m_onItemRemoved) {
            m_onItemRemoved(type, amount);
        }
        LOG_INFO_C("Removed " + std::to_string(amount) + "x " + 
                   rpg::getResourceName(type) + " from inventory", "InventoryManager");
    }

    return success;
}

bool InventoryManager::hasItem(rpg::ResourceType type, float amount) const
{
    return m_inventory.hasResource(type, amount);
}

float InventoryManager::getItemAmount(rpg::ResourceType type) const
{
    return m_inventory.getResourceAmount(type);
}

void InventoryManager::setInventoryVisible(bool visible)
{
    m_inventoryPanel.setVisible(visible);
}

void InventoryManager::toggleInventory()
{
    m_inventoryPanel.toggleVisibility();
}

void InventoryManager::setHotbarVisible(bool visible)
{
    m_hotbarPanel.setVisible(visible);
}

bool InventoryManager::bindToHotbar(int hotbarSlot, rpg::ResourceType resourceType)
{
    if (hotbarSlot < 0 || hotbarSlot >= 10) {
        return false;
    }

    m_hotbarBindings[hotbarSlot] = resourceType;
    refreshHotbar();

    LOG_INFO_C("Bound " + rpg::getResourceName(resourceType) + 
               " to hotbar slot " + std::to_string(hotbarSlot + 1), "InventoryManager");
    return true;
}

rpg::ResourceType InventoryManager::getHotbarBinding(int hotbarSlot) const
{
    if (hotbarSlot < 0 || hotbarSlot >= 10) {
        return rpg::ResourceType::Iron; // Default
    }
    return m_hotbarBindings[hotbarSlot];
}

bool InventoryManager::useSelectedHotbarItem(float amount)
{
    int selectedSlot = m_hotbarPanel.getSelectedSlot();
    if (selectedSlot < 0 || selectedSlot >= 10) {
        return false;
    }

    rpg::ResourceType type = m_hotbarBindings[selectedSlot];
    float useAmount = (amount <= 0.0f) ? 1.0f : amount;

    // Check if we have the item
    if (!hasItem(type, useAmount)) {
        LOG_WARNING_C("Cannot use item - not enough in inventory", "InventoryManager");
        return false;
    }

    // Remove from inventory
    if (removeItem(type, useAmount)) {
        if (m_onItemUsed) {
            m_onItemUsed(type, useAmount);
        }
        return true;
    }

    return false;
}

void InventoryManager::selectHotbarSlot(int slotIndex)
{
    m_hotbarPanel.setSelectedSlot(slotIndex);
}

int InventoryManager::getSelectedHotbarSlot() const
{
    return m_hotbarPanel.getSelectedSlot();
}

void InventoryManager::clearAll()
{
    m_inventory.clear();
    m_hotbarPanel.clearAllSlots();
    LOG_INFO_C("Cleared all inventory items", "InventoryManager");
}

void InventoryManager::refreshHotbar()
{
    // Update each hotbar slot with current inventory amounts
    for (int i = 0; i < 10; ++i) {
        rpg::ResourceType type = m_hotbarBindings[i];
        float amount = m_inventory.getResourceAmount(type);

        HotbarPanel::HotbarSlot slot;
        slot.isEmpty = (amount <= 0.0f);
        slot.itemName = rpg::getResourceName(type);
        slot.count = static_cast<int>(amount);
        // Note: VoxelType is not applicable for resources
        // This would need to be extended if hotbar also supports voxel blocks

        m_hotbarPanel.setSlot(i, slot);
    }
}

void InventoryManager::onHotbarItemDropped(rpg::ResourceType type, float amount)
{
    // Remove from inventory when dropped from hotbar
    removeItem(type, amount);
}

} // namespace fresh
