#include "ui/InventoryPanel.h"
#include <algorithm>
#include <sstream>
#include <iomanip>

namespace fresh
{

InventoryPanel::InventoryPanel(int slotCount)
    : m_inventory(nullptr)
    , m_selectedSlot(-1)
    , m_visible(false)
    , m_dragDropEnabled(true)
    , m_slotCount(slotCount)
    , m_dragging(false)
    , m_dragSourceSlot(-1)
{
    m_slots.resize(slotCount);
    for (int i = 0; i < slotCount; ++i) {
        m_slots[i].slotIndex = i;
        m_slots[i].isEmpty = true;
    }
}

void InventoryPanel::update(float deltaTime)
{
    if (!m_visible || !m_inventory) {
        return;
    }

    // Refresh slots to match inventory state
    refreshSlots();
}

void InventoryPanel::render()
{
    if (!m_visible) {
        return;
    }

    // Note: Actual rendering implementation would use Win32 native controls
    // This is a placeholder for the rendering logic structure
    // In a real implementation, this would:
    // 1. Create/update a Win32 ListView control with icon view
    // 2. Populate items with resource icons and amounts
    // 3. Handle selection and drag-drop events
    // 4. Display capacity bar at bottom
}

void InventoryPanel::setInventory(rpg::Inventory* inventory)
{
    m_inventory = inventory;
    if (m_inventory) {
        refreshSlots();
    }
}

void InventoryPanel::setVisible(bool visible)
{
    m_visible = visible;
    if (visible && m_inventory) {
        refreshSlots();
    }
}

void InventoryPanel::setSelectedSlot(int slotIndex)
{
    if (slotIndex >= 0 && slotIndex < m_slotCount) {
        m_selectedSlot = slotIndex;
    } else {
        m_selectedSlot = -1;
    }
}

std::string InventoryPanel::getCapacityString() const
{
    if (!m_inventory) {
        return "0 / 0";
    }

    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1);
    oss << m_inventory->getTotalUsed() << " / " << m_inventory->getCapacity();
    return oss.str();
}

void InventoryPanel::transferItem(int fromSlot, int toSlot, float amount)
{
    if (!m_inventory || fromSlot < 0 || fromSlot >= m_slotCount ||
        toSlot < 0 || toSlot >= m_slotCount || fromSlot == toSlot) {
        return;
    }

    auto& fromSlotData = m_slots[fromSlot];
    if (fromSlotData.isEmpty) {
        return;
    }

    float transferAmount = (amount <= 0.0f) ? fromSlotData.amount : std::min(amount, fromSlotData.amount);

    // This is a simplified implementation
    // In a real implementation, you would need to handle:
    // 1. Stacking items of the same type
    // 2. Swapping different items
    // 3. Partial transfers
    // For now, we just note that this needs implementation

    refreshSlots();
}

void InventoryPanel::dropItem(int slotIndex, float amount)
{
    if (!m_inventory || slotIndex < 0 || slotIndex >= m_slotCount) {
        return;
    }

    auto& slot = m_slots[slotIndex];
    if (slot.isEmpty) {
        return;
    }

    float dropAmount = (amount <= 0.0f) ? slot.amount : std::min(amount, slot.amount);

    // Remove from inventory
    if (m_inventory->removeResource(slot.type, dropAmount)) {
        // Call drop callback if set
        if (m_onItemDropped) {
            m_onItemDropped(slot.type, dropAmount);
        }
        refreshSlots();
    }
}

void InventoryPanel::sortByType()
{
    if (!m_inventory) {
        return;
    }

    // Sort slots by resource type
    std::sort(m_slots.begin(), m_slots.end(), 
        [](const InventorySlot& a, const InventorySlot& b) {
            if (a.isEmpty != b.isEmpty) {
                return !a.isEmpty; // Non-empty slots first
            }
            if (a.isEmpty) {
                return false; // Both empty, maintain order
            }
            return static_cast<int>(a.type) < static_cast<int>(b.type);
        });

    // Update slot indices
    for (int i = 0; i < static_cast<int>(m_slots.size()); ++i) {
        m_slots[i].slotIndex = i;
    }
}

void InventoryPanel::sortByAmount()
{
    if (!m_inventory) {
        return;
    }

    // Sort slots by amount (descending)
    std::sort(m_slots.begin(), m_slots.end(),
        [](const InventorySlot& a, const InventorySlot& b) {
            if (a.isEmpty != b.isEmpty) {
                return !a.isEmpty; // Non-empty slots first
            }
            if (a.isEmpty) {
                return false; // Both empty, maintain order
            }
            return a.amount > b.amount; // Larger amounts first
        });

    // Update slot indices
    for (int i = 0; i < static_cast<int>(m_slots.size()); ++i) {
        m_slots[i].slotIndex = i;
    }
}

void InventoryPanel::refreshSlots()
{
    if (!m_inventory) {
        // Clear all slots
        for (auto& slot : m_slots) {
            slot.isEmpty = true;
            slot.amount = 0.0f;
        }
        return;
    }

    // Get all resources from inventory
    const auto& resources = m_inventory->getAllResources();
    
    // Reset all slots to empty
    for (auto& slot : m_slots) {
        slot.isEmpty = true;
        slot.amount = 0.0f;
    }

    // Fill slots with resources
    int slotIndex = 0;
    for (const auto& [type, amount] : resources) {
        if (slotIndex >= m_slotCount) {
            break; // No more slots available
        }

        m_slots[slotIndex].type = type;
        m_slots[slotIndex].amount = amount;
        m_slots[slotIndex].isEmpty = false;
        ++slotIndex;
    }
}

std::string InventoryPanel::getResourceIcon(rpg::ResourceType type) const
{
    // Return path to icon for each resource type
    // These paths should match your asset structure
    switch (type) {
    case rpg::ResourceType::Iron:
        return "textures/icons/iron.png";
    case rpg::ResourceType::Titanium:
        return "textures/icons/titanium.png";
    case rpg::ResourceType::Naonite:
        return "textures/icons/naonite.png";
    case rpg::ResourceType::Trinium:
        return "textures/icons/trinium.png";
    case rpg::ResourceType::Xanion:
        return "textures/icons/xanion.png";
    case rpg::ResourceType::Ogonite:
        return "textures/icons/ogonite.png";
    case rpg::ResourceType::Avorion:
        return "textures/icons/avorion.png";
    case rpg::ResourceType::Energy:
        return "textures/icons/energy.png";
    case rpg::ResourceType::Credits:
        return "textures/icons/credits.png";
    default:
        return "textures/icons/unknown.png";
    }
}

InventoryPanel::Color InventoryPanel::getResourceColor(rpg::ResourceType type) const
{
    // Return color for each resource type (for UI highlighting/backgrounds)
    switch (type) {
    case rpg::ResourceType::Iron:
        return {128, 128, 128}; // Gray
    case rpg::ResourceType::Titanium:
        return {176, 196, 222}; // Light steel blue
    case rpg::ResourceType::Naonite:
        return {144, 238, 144}; // Light green
    case rpg::ResourceType::Trinium:
        return {135, 206, 235}; // Sky blue
    case rpg::ResourceType::Xanion:
        return {255, 215, 0};   // Gold
    case rpg::ResourceType::Ogonite:
        return {255, 140, 0};   // Dark orange
    case rpg::ResourceType::Avorion:
        return {138, 43, 226};  // Blue violet
    case rpg::ResourceType::Energy:
        return {255, 255, 0};   // Yellow
    case rpg::ResourceType::Credits:
        return {50, 205, 50};   // Lime green
    default:
        return {128, 128, 128}; // Gray
    }
}

} // namespace fresh
