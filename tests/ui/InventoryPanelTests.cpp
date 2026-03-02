/**
 * @file InventoryPanelTests.cpp
 * @brief Tests for InventoryPanel tooltip and hover functionality
 */

#include <gtest/gtest.h>
#include "ui/InventoryPanel.h"
#include "rpg/Inventory.h"

using namespace fresh;

class InventoryPanelTooltipTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        panel = std::make_unique<InventoryPanel>(10);
        inventory = std::make_unique<rpg::Inventory>(1000.0f);
        panel->setInventory(inventory.get());
        panel->setVisible(true);

        // Add some items to the inventory
        inventory->addResource(rpg::ResourceType::Iron, 50.0f);
        inventory->addResource(rpg::ResourceType::Avorion, 10.0f);
        inventory->addResource(rpg::ResourceType::Credits, 500.0f);

        // Force a refresh so slots are populated
        panel->update(0.0f);
    }

    void TearDown() override
    {
        panel.reset();
        inventory.reset();
    }

    std::unique_ptr<InventoryPanel> panel;
    std::unique_ptr<rpg::Inventory> inventory;
};

// ============================================================================
// Tooltip Basic Tests
// ============================================================================

TEST_F(InventoryPanelTooltipTest, HoveredSlot_DefaultIsNone)
{
    EXPECT_EQ(-1, panel->getHoveredSlot());
}

TEST_F(InventoryPanelTooltipTest, HoverSlot_SetsHoveredSlot)
{
    panel->hoverSlot(0);
    EXPECT_EQ(0, panel->getHoveredSlot());
}

TEST_F(InventoryPanelTooltipTest, HoverSlot_InvalidIndex_SetsNone)
{
    panel->hoverSlot(0);
    panel->hoverSlot(-1);
    EXPECT_EQ(-1, panel->getHoveredSlot());
}

TEST_F(InventoryPanelTooltipTest, HoverSlot_OutOfRange_SetsNone)
{
    panel->hoverSlot(100);
    EXPECT_EQ(-1, panel->getHoveredSlot());
}

TEST_F(InventoryPanelTooltipTest, TooltipVisible_WhenHoveringNonEmptySlot)
{
    panel->hoverSlot(0); // Iron is in slot 0
    EXPECT_TRUE(panel->isTooltipVisible());
}

TEST_F(InventoryPanelTooltipTest, TooltipNotVisible_WhenHoveringEmptySlot)
{
    panel->hoverSlot(9); // Slot 9 should be empty (only 3 items)
    EXPECT_FALSE(panel->isTooltipVisible());
}

TEST_F(InventoryPanelTooltipTest, TooltipNotVisible_WhenNotHovering)
{
    EXPECT_FALSE(panel->isTooltipVisible());
}

// ============================================================================
// Tooltip Content Tests
// ============================================================================

TEST_F(InventoryPanelTooltipTest, GetTooltipForSlot_Iron_HasCorrectName)
{
    auto tooltip = panel->getTooltipForSlot(0); // Iron in slot 0
    EXPECT_EQ("Iron", tooltip.name);
}

TEST_F(InventoryPanelTooltipTest, GetTooltipForSlot_Iron_HasDescription)
{
    auto tooltip = panel->getTooltipForSlot(0);
    EXPECT_FALSE(tooltip.description.empty());
}

TEST_F(InventoryPanelTooltipTest, GetTooltipForSlot_Iron_HasRareRarity)
{
    auto tooltip = panel->getTooltipForSlot(0);
    EXPECT_EQ("Rare", tooltip.rarity);
}

TEST_F(InventoryPanelTooltipTest, GetTooltipForSlot_Avorion_HasLegendaryRarity)
{
    auto tooltip = panel->getTooltipForSlot(1); // Avorion in slot 1
    EXPECT_EQ("Legendary", tooltip.rarity);
}

TEST_F(InventoryPanelTooltipTest, GetTooltipForSlot_EmptySlot_HasEmptyName)
{
    auto tooltip = panel->getTooltipForSlot(9); // Empty slot
    EXPECT_TRUE(tooltip.name.empty());
}

TEST_F(InventoryPanelTooltipTest, GetTooltipForSlot_InvalidIndex_HasEmptyName)
{
    auto tooltip = panel->getTooltipForSlot(-1);
    EXPECT_TRUE(tooltip.name.empty());

    auto tooltip2 = panel->getTooltipForSlot(100);
    EXPECT_TRUE(tooltip2.name.empty());
}
