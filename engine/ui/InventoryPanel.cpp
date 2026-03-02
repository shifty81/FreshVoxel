#include "ui/InventoryPanel.h"
#include <algorithm>
#include <sstream>
#include <iomanip>

namespace fresh
{

InventoryPanel::InventoryPanel(int slotCount)
    : m_inventory(nullptr)
    , m_selectedSlot(-1)
    , m_hoveredSlot(-1)
    , m_visible(false)
    , m_dragDropEnabled(true)
    , m_slotCount(slotCount)
    , m_dragging(false)
    , m_dragSourceSlot(-1)
    , m_renderedSlotCount(0)
{
    m_slots.resize(slotCount);
    for (int i = 0; i < slotCount; ++i) {
        m_slots[i].slotIndex = i;
        m_slots[i].isEmpty = true;
    }
}

void InventoryPanel::update(float /*deltaTime*/)
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

    // Build display data for each slot so UI frameworks can render
    m_renderedSlotCount = 0;
    for (int i = 0; i < m_slotCount; ++i) {
        const auto& slot = m_slots[i];
        if (slot.isEmpty) {
            continue;
        }

        // Pre-compute display properties for this slot
        // These are consumed by platform-specific UI (Win32 native controls)
        std::string icon = getResourceIcon(slot.type);
        Color color = getResourceColor(slot.type);
        std::string name = getResourceName(slot.type);

        // Determine slot visual state for rendering
        bool isSelected = (i == m_selectedSlot);
        bool isHovered = (i == m_hoveredSlot);
        bool isDragSource = (m_dragging && i == m_dragSourceSlot);

        // Suppress unused variable warnings - these values are consumed
        // by platform-specific UI backends that override this method
        (void)icon;
        (void)color;
        (void)name;
        (void)isSelected;
        (void)isHovered;
        (void)isDragSource;

        ++m_renderedSlotCount;
    }
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
    if (isValidSlotIndex(slotIndex)) {
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
    auto& toSlotData = m_slots[toSlot];

    if (toSlotData.isEmpty) {
        // Move to empty slot
        toSlotData.type = fromSlotData.type;
        toSlotData.amount = transferAmount;
        toSlotData.isEmpty = false;

        fromSlotData.amount -= transferAmount;
        if (fromSlotData.amount <= 0.0f) {
            fromSlotData.isEmpty = true;
            fromSlotData.amount = 0.0f;
        }
    } else if (toSlotData.type == fromSlotData.type) {
        // Stack same item type
        toSlotData.amount += transferAmount;

        fromSlotData.amount -= transferAmount;
        if (fromSlotData.amount <= 0.0f) {
            fromSlotData.isEmpty = true;
            fromSlotData.amount = 0.0f;
        }
    } else if (transferAmount >= fromSlotData.amount) {
        // Swap different item types (only when transferring all)
        std::swap(fromSlotData.type, toSlotData.type);
        std::swap(fromSlotData.amount, toSlotData.amount);
    }

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
    // Raw materials
    case rpg::ResourceType::Wood:
        return "textures/icons/wood.png";
    case rpg::ResourceType::Stone:
        return "textures/icons/stone.png";
    case rpg::ResourceType::Dirt:
        return "textures/icons/dirt.png";
    case rpg::ResourceType::Sand:
        return "textures/icons/sand.png";
    case rpg::ResourceType::Coal:
        return "textures/icons/coal.png";
    case rpg::ResourceType::IronOre:
        return "textures/icons/iron_ore.png";
    case rpg::ResourceType::GoldOre:
        return "textures/icons/gold_ore.png";
    case rpg::ResourceType::Diamond:
        return "textures/icons/diamond.png";
    // Processed materials
    case rpg::ResourceType::Plank:
        return "textures/icons/plank.png";
    case rpg::ResourceType::Cobblestone:
        return "textures/icons/cobblestone.png";
    case rpg::ResourceType::IronIngot:
        return "textures/icons/iron_ingot.png";
    case rpg::ResourceType::GoldIngot:
        return "textures/icons/gold_ingot.png";
    case rpg::ResourceType::Glass:
        return "textures/icons/glass.png";
    case rpg::ResourceType::Brick:
        return "textures/icons/brick.png";
    // Organic
    case rpg::ResourceType::Leaves:
        return "textures/icons/leaves.png";
    case rpg::ResourceType::Seeds:
        return "textures/icons/seeds.png";
    case rpg::ResourceType::Wheat:
        return "textures/icons/wheat.png";
    case rpg::ResourceType::Apple:
        return "textures/icons/apple.png";
    case rpg::ResourceType::Leather:
        return "textures/icons/leather.png";
    case rpg::ResourceType::Feather:
        return "textures/icons/feather.png";
    // Special
    case rpg::ResourceType::Redstone:
        return "textures/icons/redstone.png";
    case rpg::ResourceType::Lapis:
        return "textures/icons/lapis.png";
    case rpg::ResourceType::Emerald:
        return "textures/icons/emerald.png";
    case rpg::ResourceType::Clay:
        return "textures/icons/clay.png";
    case rpg::ResourceType::Flint:
        return "textures/icons/flint.png";
    case rpg::ResourceType::Gunpowder:
        return "textures/icons/gunpowder.png";
    case rpg::ResourceType::String:
        return "textures/icons/string.png";
    case rpg::ResourceType::Bone:
        return "textures/icons/bone.png";
    // Advanced materials
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
    // Raw materials
    case rpg::ResourceType::Wood:
        return {139, 90, 43};   // Brown
    case rpg::ResourceType::Stone:
        return {128, 128, 128}; // Gray
    case rpg::ResourceType::Dirt:
        return {139, 69, 19};   // Saddle brown
    case rpg::ResourceType::Sand:
        return {238, 214, 175}; // Wheat
    case rpg::ResourceType::Coal:
        return {47, 47, 47};    // Dark gray
    case rpg::ResourceType::IronOre:
        return {150, 120, 100}; // Brownish gray
    case rpg::ResourceType::GoldOre:
        return {255, 215, 0};   // Gold
    case rpg::ResourceType::Diamond:
        return {185, 242, 255}; // Light cyan
    // Processed materials
    case rpg::ResourceType::Plank:
        return {205, 133, 63};  // Peru
    case rpg::ResourceType::Cobblestone:
        return {105, 105, 105}; // Dim gray
    case rpg::ResourceType::IronIngot:
        return {210, 210, 210}; // Light gray
    case rpg::ResourceType::GoldIngot:
        return {255, 215, 0};   // Gold
    case rpg::ResourceType::Glass:
        return {230, 240, 255}; // Alice blue
    case rpg::ResourceType::Brick:
        return {178, 34, 34};   // Fire brick
    // Organic
    case rpg::ResourceType::Leaves:
        return {34, 139, 34};   // Forest green
    case rpg::ResourceType::Seeds:
        return {154, 205, 50};  // Yellow green
    case rpg::ResourceType::Wheat:
        return {218, 165, 32};  // Goldenrod
    case rpg::ResourceType::Apple:
        return {255, 0, 0};     // Red
    case rpg::ResourceType::Leather:
        return {139, 90, 43};   // Brown
    case rpg::ResourceType::Feather:
        return {255, 250, 250}; // Snow
    // Special
    case rpg::ResourceType::Redstone:
        return {255, 0, 0};     // Red
    case rpg::ResourceType::Lapis:
        return {65, 105, 225};  // Royal blue
    case rpg::ResourceType::Emerald:
        return {0, 201, 87};    // Emerald
    case rpg::ResourceType::Clay:
        return {168, 168, 168}; // Silver
    case rpg::ResourceType::Flint:
        return {58, 58, 58};    // Dark gray
    case rpg::ResourceType::Gunpowder:
        return {30, 30, 30};    // Almost black
    case rpg::ResourceType::String:
        return {255, 255, 255}; // White
    case rpg::ResourceType::Bone:
        return {250, 250, 240}; // Ivory
    // Advanced materials
    case rpg::ResourceType::Iron:
        return {180, 180, 180}; // Silver gray
    case rpg::ResourceType::Titanium:
        return {190, 200, 210}; // Titanium white
    case rpg::ResourceType::Naonite:
        return {0, 255, 128};   // Neon green
    case rpg::ResourceType::Trinium:
        return {100, 200, 255}; // Sky blue
    case rpg::ResourceType::Xanion:
        return {180, 100, 255}; // Purple
    case rpg::ResourceType::Ogonite:
        return {255, 140, 0};   // Dark orange
    case rpg::ResourceType::Avorion:
        return {255, 255, 100}; // Bright yellow
    case rpg::ResourceType::Energy:
        return {0, 200, 255};   // Electric blue
    case rpg::ResourceType::Credits:
        return {255, 215, 0};   // Gold
    default:
        return {128, 128, 128}; // Gray
    }
}

ItemTooltip InventoryPanel::getTooltipForSlot(int slotIndex) const
{
    if (!isValidSlotIndex(slotIndex)) {
        return ItemTooltip();
    }

    const auto& slot = m_slots[slotIndex];
    if (slot.isEmpty) {
        return ItemTooltip();
    }

    return ItemTooltip(
        getResourceName(slot.type),
        getResourceDescription(slot.type),
        getResourceRarity(slot.type)
    );
}

void InventoryPanel::hoverSlot(int slotIndex)
{
    if (isValidSlotIndex(slotIndex)) {
        m_hoveredSlot = slotIndex;
    } else {
        m_hoveredSlot = -1;
    }
}

bool InventoryPanel::isTooltipVisible() const
{
    if (!isValidSlotIndex(m_hoveredSlot)) {
        return false;
    }
    return !m_slots[m_hoveredSlot].isEmpty;
}

std::string InventoryPanel::getResourceName(rpg::ResourceType type) const
{
    // Use the centralized function from Inventory.h
    return rpg::getResourceName(type);
}

std::string InventoryPanel::getResourceDescription(rpg::ResourceType type) const
{
    switch (type) {
    // Raw materials
    case rpg::ResourceType::Wood:
        return "A common building material from trees.";
    case rpg::ResourceType::Stone:
        return "Solid rock, useful for tools and building.";
    case rpg::ResourceType::Dirt:
        return "Common soil found everywhere.";
    case rpg::ResourceType::Sand:
        return "Found near water, can be smelted into glass.";
    case rpg::ResourceType::Coal:
        return "Fuel source, burns longer than wood.";
    case rpg::ResourceType::IronOre:
        return "Iron ore that can be smelted into ingots.";
    case rpg::ResourceType::GoldOre:
        return "Precious gold ore for ingots and trading.";
    case rpg::ResourceType::Diamond:
        return "The hardest material, for the best tools.";
    // Processed materials
    case rpg::ResourceType::Plank:
        return "Processed wood planks for crafting.";
    case rpg::ResourceType::Cobblestone:
        return "Rough stone blocks, great for building.";
    case rpg::ResourceType::IronIngot:
        return "Refined iron for crafting tools and armor.";
    case rpg::ResourceType::GoldIngot:
        return "Refined gold for valuable items.";
    case rpg::ResourceType::Glass:
        return "Transparent block made from smelted sand.";
    case rpg::ResourceType::Brick:
        return "Hardened clay for decorative building.";
    // Organic
    case rpg::ResourceType::Leaves:
        return "Decorative foliage from trees.";
    case rpg::ResourceType::Seeds:
        return "Plant these to grow crops.";
    case rpg::ResourceType::Wheat:
        return "Harvested crop used to make bread.";
    case rpg::ResourceType::Apple:
        return "A nutritious fruit that restores health.";
    case rpg::ResourceType::Leather:
        return "Animal hide used for armor and books.";
    case rpg::ResourceType::Feather:
        return "Light feather used for arrows.";
    // Special
    case rpg::ResourceType::Redstone:
        return "Magical dust for mechanisms and contraptions.";
    case rpg::ResourceType::Lapis:
        return "Blue stone for enchanting and dyes.";
    case rpg::ResourceType::Emerald:
        return "Precious gem used for trading with villagers.";
    case rpg::ResourceType::Clay:
        return "Found near water, can be made into bricks.";
    case rpg::ResourceType::Flint:
        return "Sharp stone used to make arrows.";
    case rpg::ResourceType::Gunpowder:
        return "Explosive material dropped by enemies.";
    case rpg::ResourceType::String:
        return "Spider silk used for bows and fishing rods.";
    case rpg::ResourceType::Bone:
        return "Skeletal remains used for bone meal.";
    // Advanced materials
    case rpg::ResourceType::Iron:
        return "Refined iron metal for advanced crafting.";
    case rpg::ResourceType::Titanium:
        return "A rare, strong metal found deep underground.";
    case rpg::ResourceType::Naonite:
        return "An exotic green mineral with unique properties.";
    case rpg::ResourceType::Trinium:
        return "A lightweight exotic mineral prized for its strength.";
    case rpg::ResourceType::Xanion:
        return "A legendary purple mineral of immense power.";
    case rpg::ResourceType::Ogonite:
        return "A rare orange mineral found in extreme conditions.";
    case rpg::ResourceType::Avorion:
        return "The most legendary material in existence.";
    case rpg::ResourceType::Energy:
        return "Raw energy used to power systems and machines.";
    case rpg::ResourceType::Credits:
        return "Universal currency for trading and commerce.";
    default:
        return "An unidentified item.";
    }
}

std::string InventoryPanel::getResourceRarity(rpg::ResourceType type) const
{
    // Use the centralized rarity function from Inventory.h
    int rarity = rpg::getResourceRarity(type);
    switch (rarity) {
    case 0: return "Common";
    case 1: return "Uncommon";
    case 2: return "Rare";
    case 3: return "Epic";
    case 4: return "Legendary";
    default: return "Common";
    }
}

} // namespace fresh
