/**
 * @file CraftingPanelTests.cpp
 * @brief Tests for CraftingPanel crafting lifecycle and UI state
 */

#include <gtest/gtest.h>
#include "ui/CraftingPanel.h"
#include "rpg/CraftingSystem.h"
#include "rpg/Inventory.h"
#include "gameplay/InventoryManager.h"

using namespace fresh;

class CraftingPanelTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        panel = std::make_unique<CraftingPanel>();
        inventoryManager = std::make_unique<InventoryManager>(10000.0f);
        inventoryManager->initialize();
        craftingSystem = std::make_unique<rpg::CraftingSystem>();

        panel->initialize(inventoryManager.get());
        panel->setCraftingSystem(craftingSystem.get());
        panel->setVisible(true);
    }

    void TearDown() override
    {
        panel.reset();
        inventoryManager.reset();
        craftingSystem.reset();
    }

    std::unique_ptr<CraftingPanel> panel;
    std::unique_ptr<InventoryManager> inventoryManager;
    std::unique_ptr<rpg::CraftingSystem> craftingSystem;
};

// ============================================================================
// Initialization Tests
// ============================================================================

TEST_F(CraftingPanelTest, Initialize_WithValidManager_Succeeds)
{
    CraftingPanel p;
    InventoryManager mgr;
    mgr.initialize();
    EXPECT_TRUE(p.initialize(&mgr));
}

TEST_F(CraftingPanelTest, Initialize_WithNullManager_Fails)
{
    CraftingPanel p;
    EXPECT_FALSE(p.initialize(nullptr));
}

// ============================================================================
// Visibility Tests
// ============================================================================

TEST_F(CraftingPanelTest, DefaultVisibility_IsFalse)
{
    CraftingPanel p;
    EXPECT_FALSE(p.isVisible());
}

TEST_F(CraftingPanelTest, SetVisible_ChangesVisibility)
{
    panel->setVisible(false);
    EXPECT_FALSE(panel->isVisible());
    panel->setVisible(true);
    EXPECT_TRUE(panel->isVisible());
}

TEST_F(CraftingPanelTest, ToggleVisibility_TogglesState)
{
    panel->setVisible(true);
    panel->toggleVisibility();
    EXPECT_FALSE(panel->isVisible());
    panel->toggleVisibility();
    EXPECT_TRUE(panel->isVisible());
}

// ============================================================================
// Recipe Display Tests
// ============================================================================

TEST_F(CraftingPanelTest, GetAvailableRecipes_ReturnsRecipes)
{
    auto recipes = panel->getAvailableRecipes();
    EXPECT_FALSE(recipes.empty());
}

TEST_F(CraftingPanelTest, GetAvailableRecipes_ShowsCraftability)
{
    // Add resources to make Planks craftable
    inventoryManager->addItem(rpg::ResourceType::Wood, 10.0f);
    panel->update(0.0f);

    auto recipes = panel->getAvailableRecipes();
    bool foundCraftable = false;
    for (const auto& recipe : recipes) {
        if (recipe.name == "Planks") {
            EXPECT_TRUE(recipe.canCraft);
            foundCraftable = true;
        }
    }
    EXPECT_TRUE(foundCraftable);
}

TEST_F(CraftingPanelTest, SearchFilter_FiltersRecipes)
{
    panel->setSearchFilter("Pickaxe");
    auto recipes = panel->getAvailableRecipes();
    for (const auto& recipe : recipes) {
        EXPECT_NE(recipe.name.find("Pickaxe"), std::string::npos);
    }
}

// ============================================================================
// Crafting Lifecycle Tests
// ============================================================================

TEST_F(CraftingPanelTest, CraftSelectedRecipe_NoSelection_Fails)
{
    EXPECT_FALSE(panel->craftSelectedRecipe());
}

TEST_F(CraftingPanelTest, CraftSelectedRecipe_InsufficientResources_Fails)
{
    panel->setSelectedRecipe(0);
    EXPECT_FALSE(panel->craftSelectedRecipe());
}

TEST_F(CraftingPanelTest, CraftSelectedRecipe_WithResources_Succeeds)
{
    // Add enough resources for Planks (requires Wood 1)
    inventoryManager->addItem(rpg::ResourceType::Wood, 10.0f);
    panel->update(0.0f);

    // Find the index of "Planks" recipe
    auto recipeNames = craftingSystem->getAllRecipeNames();
    int planksIndex = -1;
    for (int i = 0; i < static_cast<int>(recipeNames.size()); ++i) {
        if (recipeNames[i] == "Planks") {
            planksIndex = i;
            break;
        }
    }
    ASSERT_GE(planksIndex, 0);

    panel->setSelectedRecipe(planksIndex);
    EXPECT_TRUE(panel->craftSelectedRecipe());
    EXPECT_TRUE(panel->isCrafting());
}

TEST_F(CraftingPanelTest, CraftingProgress_IncreasesOverTime)
{
    inventoryManager->addItem(rpg::ResourceType::Wood, 10.0f);
    panel->update(0.0f);

    // Find Wooden Pickaxe (has 1.0s crafting time)
    auto recipeNames = craftingSystem->getAllRecipeNames();
    int pickaxeIndex = -1;
    for (int i = 0; i < static_cast<int>(recipeNames.size()); ++i) {
        if (recipeNames[i] == "Wooden Pickaxe") {
            pickaxeIndex = i;
            break;
        }
    }
    
    if (pickaxeIndex >= 0) {
        inventoryManager->addItem(rpg::ResourceType::Plank, 10.0f);
        panel->setSelectedRecipe(pickaxeIndex);
        
        if (panel->craftSelectedRecipe()) {
            float initialProgress = panel->getCraftingProgress();
            panel->update(0.5f); // Half a second
            EXPECT_GT(panel->getCraftingProgress(), initialProgress);
        }
    }
}

TEST_F(CraftingPanelTest, CancelCrafting_StopsCrafting)
{
    inventoryManager->addItem(rpg::ResourceType::Wood, 10.0f);
    inventoryManager->addItem(rpg::ResourceType::Plank, 10.0f);
    panel->update(0.0f);

    auto recipeNames = craftingSystem->getAllRecipeNames();
    int pickaxeIndex = -1;
    for (int i = 0; i < static_cast<int>(recipeNames.size()); ++i) {
        if (recipeNames[i] == "Wooden Pickaxe") {
            pickaxeIndex = i;
            break;
        }
    }
    
    if (pickaxeIndex >= 0) {
        panel->setSelectedRecipe(pickaxeIndex);
        if (panel->craftSelectedRecipe()) {
            panel->update(0.2f);
            panel->cancelCrafting();
            EXPECT_FALSE(panel->isCrafting());
        }
    }
}

// ============================================================================
// Render Tests (no crash on render)
// ============================================================================

TEST_F(CraftingPanelTest, Render_WhenVisible_DoesNotCrash)
{
    panel->setVisible(true);
    EXPECT_NO_THROW(panel->render());
}

TEST_F(CraftingPanelTest, Render_WhenHidden_DoesNotCrash)
{
    panel->setVisible(false);
    EXPECT_NO_THROW(panel->render());
}

TEST_F(CraftingPanelTest, Render_WhileCrafting_DoesNotCrash)
{
    inventoryManager->addItem(rpg::ResourceType::Wood, 10.0f);
    panel->update(0.0f);

    auto recipeNames = craftingSystem->getAllRecipeNames();
    for (int i = 0; i < static_cast<int>(recipeNames.size()); ++i) {
        if (recipeNames[i] == "Planks") {
            panel->setSelectedRecipe(i);
            panel->craftSelectedRecipe();
            break;
        }
    }
    EXPECT_NO_THROW(panel->render());
}
