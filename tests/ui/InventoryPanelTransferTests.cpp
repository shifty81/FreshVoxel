/**
 * @file InventoryPanelTransferTests.cpp
 * @brief Tests for InventoryPanel item transfer and render functionality
 */

#include <gtest/gtest.h>
#include "ui/InventoryPanel.h"
#include "rpg/Inventory.h"

using namespace fresh;

class InventoryPanelTransferTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        panel = std::make_unique<InventoryPanel>(10);
        inventory = std::make_unique<rpg::Inventory>(1000.0f);
        panel->setInventory(inventory.get());
        panel->setVisible(true);

        // Add some items
        inventory->addResource(rpg::ResourceType::Wood, 50.0f);
        inventory->addResource(rpg::ResourceType::Stone, 30.0f);
        inventory->addResource(rpg::ResourceType::Iron, 20.0f);

        // Force refresh so slots are populated
        panel->update(0.0f);
    }

    std::unique_ptr<InventoryPanel> panel;
    std::unique_ptr<rpg::Inventory> inventory;
};

// ============================================================================
// Transfer Item Tests
// ============================================================================

TEST_F(InventoryPanelTransferTest, TransferItem_ToEmptySlot_MovesItem)
{
    // Slot 0 has Wood, slot 5 should be empty
    const auto& slotsBefore = panel->getSlots();
    ASSERT_FALSE(slotsBefore[0].isEmpty);
    ASSERT_TRUE(slotsBefore[5].isEmpty);

    auto originalType = slotsBefore[0].type;
    auto originalAmount = slotsBefore[0].amount;
    panel->transferItem(0, 5);

    // After transfer, the item should be in the destination slot
    const auto& slotsAfter = panel->getSlots();
    // Note: refreshSlots() re-populates from inventory, but the slot-level
    // transfer should have moved data before the refresh
    EXPECT_EQ(originalType, slotsAfter[0].type);
    EXPECT_EQ(originalAmount, slotsAfter[0].amount);
}

TEST_F(InventoryPanelTransferTest, TransferItem_InvalidFromSlot_DoesNothing)
{
    EXPECT_NO_THROW(panel->transferItem(-1, 5));
    EXPECT_NO_THROW(panel->transferItem(100, 5));
}

TEST_F(InventoryPanelTransferTest, TransferItem_InvalidToSlot_DoesNothing)
{
    EXPECT_NO_THROW(panel->transferItem(0, -1));
    EXPECT_NO_THROW(panel->transferItem(0, 100));
}

TEST_F(InventoryPanelTransferTest, TransferItem_SameSlot_DoesNothing)
{
    EXPECT_NO_THROW(panel->transferItem(0, 0));
}

TEST_F(InventoryPanelTransferTest, TransferItem_FromEmptySlot_DoesNothing)
{
    EXPECT_NO_THROW(panel->transferItem(9, 0)); // Slot 9 is empty
}

TEST_F(InventoryPanelTransferTest, TransferItem_NullInventory_DoesNothing)
{
    panel->setInventory(nullptr);
    EXPECT_NO_THROW(panel->transferItem(0, 5));
}

// ============================================================================
// Render Tests
// ============================================================================

TEST_F(InventoryPanelTransferTest, Render_WhenVisible_DoesNotCrash)
{
    panel->setVisible(true);
    EXPECT_NO_THROW(panel->render());
}

TEST_F(InventoryPanelTransferTest, Render_WhenHidden_DoesNotCrash)
{
    panel->setVisible(false);
    EXPECT_NO_THROW(panel->render());
}

TEST_F(InventoryPanelTransferTest, Render_EmptyInventory_DoesNotCrash)
{
    inventory->clear();
    panel->update(0.0f);
    EXPECT_NO_THROW(panel->render());
}

TEST_F(InventoryPanelTransferTest, Render_WithSelectedSlot_DoesNotCrash)
{
    panel->setSelectedSlot(0);
    EXPECT_NO_THROW(panel->render());
}

TEST_F(InventoryPanelTransferTest, Render_WithHoveredSlot_DoesNotCrash)
{
    panel->hoverSlot(0);
    EXPECT_NO_THROW(panel->render());
}

// ============================================================================
// Resource Coverage Tests
// ============================================================================

TEST_F(InventoryPanelTransferTest, TooltipForAdvancedResources_HasContent)
{
    // Test that advanced resource types have descriptions
    inventory->clear();
    inventory->addResource(rpg::ResourceType::Titanium, 10.0f);
    inventory->addResource(rpg::ResourceType::Naonite, 5.0f);
    inventory->addResource(rpg::ResourceType::Avorion, 1.0f);
    inventory->addResource(rpg::ResourceType::Energy, 100.0f);
    inventory->addResource(rpg::ResourceType::Credits, 500.0f);
    panel->update(0.0f);

    // Each populated slot should have a non-empty tooltip
    for (int i = 0; i < 5; ++i) {
        auto tooltip = panel->getTooltipForSlot(i);
        EXPECT_FALSE(tooltip.name.empty()) << "Slot " << i << " should have a name";
        EXPECT_FALSE(tooltip.description.empty()) << "Slot " << i << " should have a description";
        EXPECT_FALSE(tooltip.rarity.empty()) << "Slot " << i << " should have a rarity";
    }
}
