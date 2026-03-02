#include <gtest/gtest.h>
#include "rpg/CraftingSystem.h"
#include "rpg/Inventory.h"

using namespace fresh::rpg;

class CraftingSystemTest : public ::testing::Test {
protected:
    void SetUp() override {
        crafting = std::make_unique<CraftingSystem>();
        inventory = std::make_unique<Inventory>(10000.0f);
    }
    
    std::unique_ptr<CraftingSystem> crafting;
    std::unique_ptr<Inventory> inventory;
};

// CraftedItem Tests
TEST(CraftedItemTest, Constructor_Default_CreatesValidItem) {
    CraftedItem item;
    EXPECT_EQ(ItemCategory::Material, item.getType());
    EXPECT_EQ(0, item.getLevel());
    EXPECT_EQ("Unknown Item", item.getName());
}

TEST(CraftedItemTest, Constructor_WithParameters_SetsCorrectValues) {
    CraftedItem item(ItemCategory::Weapon, MaterialTier::Diamond, "Diamond Sword");
    EXPECT_EQ(ItemCategory::Weapon, item.getType());
    EXPECT_EQ(4, item.getLevel());
    EXPECT_EQ("Diamond Sword", item.getName());
}

TEST(CraftedItemTest, SetStatBonus_AndGet_ReturnsCorrectValue) {
    CraftedItem item;
    item.setStatBonus("damage", 50.0f);
    item.setStatBonus("fireRate", 2.5f);
    
    EXPECT_EQ(50.0f, item.getStatBonus("damage"));
    EXPECT_EQ(2.5f, item.getStatBonus("fireRate"));
}

TEST(CraftedItemTest, GetStatBonus_NonExistent_ReturnsZero) {
    CraftedItem item;
    EXPECT_EQ(0.0f, item.getStatBonus("nonexistent"));
}

TEST(CraftedItemTest, SetCost_AndGetCost_ReturnsCorrectMap) {
    CraftedItem item;
    std::map<ResourceType, float> cost;
    cost[ResourceType::Iron] = 100.0f;
    cost[ResourceType::Titanium] = 50.0f;
    
    item.setCost(cost);
    const auto& retrievedCost = item.getCost();
    
    EXPECT_EQ(2u, retrievedCost.size());
    EXPECT_EQ(100.0f, retrievedCost.at(ResourceType::Iron));
    EXPECT_EQ(50.0f, retrievedCost.at(ResourceType::Titanium));
}

// CraftingSystem Recipe Management Tests
TEST_F(CraftingSystemTest, Constructor_InitializesDefaultRecipes) {
    auto recipes = crafting->getAllRecipeNames();
    EXPECT_FALSE(recipes.empty());
    EXPECT_GE(recipes.size(), 4u); // At least 4 default recipes
}

TEST_F(CraftingSystemTest, GetRecipe_ExistingRecipe_ReturnsValid) {
    const CraftingRecipe* recipe = crafting->getRecipe("Wooden Pickaxe");
    ASSERT_NE(nullptr, recipe);
    EXPECT_EQ("Wooden Pickaxe", recipe->name);
    EXPECT_EQ(ItemCategory::Tool, recipe->result.getType());
}

TEST_F(CraftingSystemTest, GetRecipe_NonExistent_ReturnsNull) {
    const CraftingRecipe* recipe = crafting->getRecipe("NonExistent Recipe");
    EXPECT_EQ(nullptr, recipe);
}

TEST_F(CraftingSystemTest, GetAllRecipeNames_ReturnsAllRecipes) {
    auto recipes = crafting->getAllRecipeNames();
    EXPECT_FALSE(recipes.empty());
    
    // Check for known default recipes
    bool hasPlanks = false;
    bool hasWoodenPickaxe = false;
    bool hasWoodenSword = false;
    bool hasFurnace = false;
    
    for (const auto& name : recipes) {
        if (name == "Planks") hasPlanks = true;
        if (name == "Wooden Pickaxe") hasWoodenPickaxe = true;
        if (name == "Wooden Sword") hasWoodenSword = true;
        if (name == "Furnace") hasFurnace = true;
    }
    
    EXPECT_TRUE(hasPlanks);
    EXPECT_TRUE(hasWoodenPickaxe);
    EXPECT_TRUE(hasWoodenSword);
    EXPECT_TRUE(hasFurnace);
}

TEST_F(CraftingSystemTest, AddRecipe_CustomRecipe_CanBeRetrieved) {
    CraftingRecipe customRecipe;
    customRecipe.name = "Custom Tool";
    customRecipe.result = CraftedItem(ItemCategory::Tool, MaterialTier::Iron, "Custom Tool");
    customRecipe.requirements[ResourceType::IronIngot] = 50.0f;
    customRecipe.craftingTime = 20.0f;
    
    crafting->addRecipe(customRecipe);
    
    const CraftingRecipe* retrieved = crafting->getRecipe("Custom Tool");
    ASSERT_NE(nullptr, retrieved);
    EXPECT_EQ("Custom Tool", retrieved->name);
    EXPECT_EQ(ItemCategory::Tool, retrieved->result.getType());
}

// CanCraft Tests
TEST_F(CraftingSystemTest, CanCraft_SufficientResources_ReturnsTrue) {
    // Wooden Pickaxe requires: Plank 3, Wood 2
    inventory->addResource(ResourceType::Plank, 10.0f);
    inventory->addResource(ResourceType::Wood, 10.0f);
    
    EXPECT_TRUE(crafting->canCraft("Wooden Pickaxe", *inventory));
}

TEST_F(CraftingSystemTest, CanCraft_InsufficientResources_ReturnsFalse) {
    // Wooden Pickaxe requires: Plank 3, Wood 2
    inventory->addResource(ResourceType::Plank, 1.0f);
    inventory->addResource(ResourceType::Wood, 1.0f);
    
    EXPECT_FALSE(crafting->canCraft("Wooden Pickaxe", *inventory));
}

TEST_F(CraftingSystemTest, CanCraft_MissingOneResource_ReturnsFalse) {
    // Wooden Pickaxe requires: Plank 3, Wood 2
    inventory->addResource(ResourceType::Plank, 10.0f);
    // Missing Wood
    
    EXPECT_FALSE(crafting->canCraft("Wooden Pickaxe", *inventory));
}

TEST_F(CraftingSystemTest, CanCraft_ExactResources_ReturnsTrue) {
    // Wooden Pickaxe requires: Plank 3, Wood 2
    inventory->addResource(ResourceType::Plank, 3.0f);
    inventory->addResource(ResourceType::Wood, 2.0f);
    
    EXPECT_TRUE(crafting->canCraft("Wooden Pickaxe", *inventory));
}

TEST_F(CraftingSystemTest, CanCraft_NonExistentRecipe_ReturnsFalse) {
    inventory->addResource(ResourceType::Iron, 1000.0f);
    EXPECT_FALSE(crafting->canCraft("NonExistent", *inventory));
}

// Craft Tests
TEST_F(CraftingSystemTest, Craft_ValidRecipeAndResources_Succeeds) {
    // Wooden Pickaxe requires: Plank 3, Wood 2
    inventory->addResource(ResourceType::Plank, 10.0f);
    inventory->addResource(ResourceType::Wood, 10.0f);
    
    CraftedItem result;
    EXPECT_TRUE(crafting->craft("Wooden Pickaxe", *inventory, result));
    
    // Check resources were consumed
    EXPECT_EQ(7.0f, inventory->getResourceAmount(ResourceType::Plank));
    EXPECT_EQ(8.0f, inventory->getResourceAmount(ResourceType::Wood));
    
    // Check result
    EXPECT_EQ(ItemCategory::Tool, result.getType());
    EXPECT_EQ("Wooden Pickaxe", result.getName());
    EXPECT_EQ(2.0f, result.getStatBonus("miningSpeed"));
}

TEST_F(CraftingSystemTest, Craft_InsufficientResources_Fails) {
    // Wooden Pickaxe requires: Plank 3, Wood 2
    inventory->addResource(ResourceType::Plank, 1.0f);
    inventory->addResource(ResourceType::Wood, 1.0f);
    
    CraftedItem result;
    EXPECT_FALSE(crafting->craft("Wooden Pickaxe", *inventory, result));
    
    // Resources should not be consumed
    EXPECT_EQ(1.0f, inventory->getResourceAmount(ResourceType::Plank));
    EXPECT_EQ(1.0f, inventory->getResourceAmount(ResourceType::Wood));
}

TEST_F(CraftingSystemTest, Craft_NonExistentRecipe_Fails) {
    inventory->addResource(ResourceType::Iron, 1000.0f);
    
    CraftedItem result;
    EXPECT_FALSE(crafting->craft("NonExistent", *inventory, result));
}

TEST_F(CraftingSystemTest, Craft_MultipleTimes_ConsumesCorrectly) {
    // Planks requires: Wood 1
    inventory->addResource(ResourceType::Wood, 5.0f);
    
    CraftedItem result1, result2;
    
    EXPECT_TRUE(crafting->craft("Planks", *inventory, result1));
    EXPECT_EQ(4.0f, inventory->getResourceAmount(ResourceType::Wood));
    
    EXPECT_TRUE(crafting->craft("Planks", *inventory, result2));
    EXPECT_EQ(3.0f, inventory->getResourceAmount(ResourceType::Wood));
}

// Default Recipe Tests
TEST_F(CraftingSystemTest, DefaultRecipe_WoodenPickaxe_HasCorrectRequirements) {
    const CraftingRecipe* recipe = crafting->getRecipe("Wooden Pickaxe");
    ASSERT_NE(nullptr, recipe);
    
    EXPECT_EQ(2u, recipe->requirements.size());
    EXPECT_EQ(3.0f, recipe->requirements.at(ResourceType::Plank));
    EXPECT_EQ(2.0f, recipe->requirements.at(ResourceType::Wood));
    EXPECT_EQ(1.0f, recipe->craftingTime);
}

TEST_F(CraftingSystemTest, DefaultRecipe_IronPickaxe_HasCorrectRequirements) {
    const CraftingRecipe* recipe = crafting->getRecipe("Iron Pickaxe");
    ASSERT_NE(nullptr, recipe);
    
    EXPECT_EQ(2u, recipe->requirements.size());
    EXPECT_EQ(3.0f, recipe->requirements.at(ResourceType::IronIngot));
    EXPECT_EQ(2.0f, recipe->requirements.at(ResourceType::Wood));
    EXPECT_EQ(2.0f, recipe->craftingTime);
    EXPECT_EQ(6.0f, recipe->result.getStatBonus("miningSpeed"));
}

TEST_F(CraftingSystemTest, DefaultRecipe_WoodenSword_HasCorrectRequirements) {
    const CraftingRecipe* recipe = crafting->getRecipe("Wooden Sword");
    ASSERT_NE(nullptr, recipe);
    
    EXPECT_EQ(2u, recipe->requirements.size());
    EXPECT_EQ(2.0f, recipe->requirements.at(ResourceType::Plank));
    EXPECT_EQ(1.0f, recipe->requirements.at(ResourceType::Wood));
    EXPECT_EQ(1.0f, recipe->craftingTime);
    EXPECT_EQ(ItemCategory::Weapon, recipe->result.getType());
}

TEST_F(CraftingSystemTest, DefaultRecipe_Chest_HasCorrectRequirements) {
    const CraftingRecipe* recipe = crafting->getRecipe("Chest");
    ASSERT_NE(nullptr, recipe);
    
    EXPECT_EQ(1u, recipe->requirements.size());
    EXPECT_EQ(8.0f, recipe->requirements.at(ResourceType::Plank));
    EXPECT_EQ(1.0f, recipe->craftingTime);
    EXPECT_EQ(27.0f, recipe->result.getStatBonus("storageSlots"));
}

// Integration Tests
TEST_F(CraftingSystemTest, Integration_FullCraftingWorkflow_Succeeds) {
    // Start with empty inventory
    EXPECT_EQ(0.0f, inventory->getTotalUsed());
    
    // Gather resources
    inventory->addResource(ResourceType::Wood, 20.0f);
    inventory->addResource(ResourceType::Plank, 20.0f);
    inventory->addResource(ResourceType::Cobblestone, 10.0f);
    
    // Check what we can craft
    EXPECT_TRUE(crafting->canCraft("Wooden Pickaxe", *inventory));
    EXPECT_TRUE(crafting->canCraft("Stone Pickaxe", *inventory));
    EXPECT_TRUE(crafting->canCraft("Crafting Table", *inventory));
    EXPECT_FALSE(crafting->canCraft("Iron Pickaxe", *inventory)); // Missing IronIngot
    
    // Craft Wooden Pickaxe
    CraftedItem pickaxe;
    EXPECT_TRUE(crafting->craft("Wooden Pickaxe", *inventory, pickaxe));
    EXPECT_EQ(ItemCategory::Tool, pickaxe.getType());
    
    // Verify remaining resources
    EXPECT_EQ(18.0f, inventory->getResourceAmount(ResourceType::Wood));
    EXPECT_EQ(17.0f, inventory->getResourceAmount(ResourceType::Plank));
    
    // Craft Stone Sword
    CraftedItem sword;
    EXPECT_TRUE(crafting->craft("Stone Sword", *inventory, sword));
    EXPECT_EQ(ItemCategory::Weapon, sword.getType());
    
    // Verify resources consumed
    EXPECT_EQ(17.0f, inventory->getResourceAmount(ResourceType::Wood));
    EXPECT_EQ(8.0f, inventory->getResourceAmount(ResourceType::Cobblestone));
}

// Edge Cases
TEST_F(CraftingSystemTest, Craft_ExactResourceAmount_Succeeds) {
    // Planks requires: Wood 1
    inventory->addResource(ResourceType::Wood, 1.0f);
    
    CraftedItem result;
    EXPECT_TRUE(crafting->craft("Planks", *inventory, result));
    
    // All resources consumed
    EXPECT_EQ(0.0f, inventory->getResourceAmount(ResourceType::Wood));
}

TEST_F(CraftingSystemTest, Craft_OneResourceShortBySmallAmount_Fails) {
    // Wooden Pickaxe requires: Plank 3, Wood 2
    inventory->addResource(ResourceType::Plank, 2.99f);
    inventory->addResource(ResourceType::Wood, 2.0f);
    
    CraftedItem result;
    EXPECT_FALSE(crafting->craft("Wooden Pickaxe", *inventory, result));
}

// Search/Filter Tests
TEST_F(CraftingSystemTest, SearchRecipes_EmptyQuery_ReturnsAll) {
    auto results = crafting->searchRecipes("");
    auto allRecipes = crafting->getAllRecipeNames();
    EXPECT_EQ(results.size(), allRecipes.size());
}

TEST_F(CraftingSystemTest, SearchRecipes_ExactName_ReturnsMatch) {
    auto results = crafting->searchRecipes("Wooden Pickaxe");
    EXPECT_EQ(1u, results.size());
    EXPECT_EQ("Wooden Pickaxe", results[0]);
}

TEST_F(CraftingSystemTest, SearchRecipes_SubstringMatch) {
    auto results = crafting->searchRecipes("Sword");
    EXPECT_GE(results.size(), 2u); // Multiple sword variants
    
    bool hasWooden = false, hasStone = false;
    for (const auto& name : results) {
        if (name == "Wooden Sword") hasWooden = true;
        if (name == "Stone Sword") hasStone = true;
    }
    EXPECT_TRUE(hasWooden);
    EXPECT_TRUE(hasStone);
}

TEST_F(CraftingSystemTest, SearchRecipes_CaseInsensitive) {
    auto results = crafting->searchRecipes("wooden");
    EXPECT_GE(results.size(), 2u); // Wooden Pickaxe, Wooden Sword, etc.
    
    auto results2 = crafting->searchRecipes("PICKAXE");
    EXPECT_GE(results2.size(), 1u);
}

TEST_F(CraftingSystemTest, SearchRecipes_NoMatch) {
    auto results = crafting->searchRecipes("NonExistentRecipe");
    EXPECT_TRUE(results.empty());
}

TEST_F(CraftingSystemTest, GetRecipesByType_Tool) {
    auto results = crafting->getRecipesByType(ItemCategory::Tool);
    EXPECT_GE(results.size(), 2u); // Multiple tool recipes
    
    for (const auto& name : results) {
        const CraftingRecipe* recipe = crafting->getRecipe(name);
        ASSERT_NE(nullptr, recipe);
        EXPECT_EQ(ItemCategory::Tool, recipe->result.getType());
    }
}

TEST_F(CraftingSystemTest, GetRecipesByType_Weapon) {
    auto results = crafting->getRecipesByType(ItemCategory::Weapon);
    EXPECT_GE(results.size(), 1u);
    
    for (const auto& name : results) {
        const CraftingRecipe* recipe = crafting->getRecipe(name);
        ASSERT_NE(nullptr, recipe);
        EXPECT_EQ(ItemCategory::Weapon, recipe->result.getType());
    }
}

TEST_F(CraftingSystemTest, GetRecipesByType_NoMatch) {
    auto results = crafting->getRecipesByType(ItemCategory::Decoration);
    EXPECT_TRUE(results.empty());
}

TEST_F(CraftingSystemTest, GetCraftableRecipes_NoResources_ReturnsEmpty) {
    auto results = crafting->getCraftableRecipes(*inventory);
    EXPECT_TRUE(results.empty());
}

TEST_F(CraftingSystemTest, GetCraftableRecipes_SomeResources) {
    // Give enough for Planks and Torch
    inventory->addResource(ResourceType::Wood, 10.0f);
    inventory->addResource(ResourceType::Coal, 5.0f);
    
    auto results = crafting->getCraftableRecipes(*inventory);
    EXPECT_GE(results.size(), 2u);
    
    bool hasPlanks = false, hasTorch = false;
    for (const auto& name : results) {
        if (name == "Planks") hasPlanks = true;
        if (name == "Torch") hasTorch = true;
    }
    EXPECT_TRUE(hasPlanks);
    EXPECT_TRUE(hasTorch);
}

TEST_F(CraftingSystemTest, GetCraftableRecipes_AllResources) {
    // Use a large-capacity inventory to fit all resources needed
    Inventory largeInventory(100000.0f);
    largeInventory.addResource(ResourceType::Wood, 10000.0f);
    largeInventory.addResource(ResourceType::Plank, 10000.0f);
    largeInventory.addResource(ResourceType::Cobblestone, 10000.0f);
    largeInventory.addResource(ResourceType::IronIngot, 10000.0f);
    largeInventory.addResource(ResourceType::Diamond, 10000.0f);
    largeInventory.addResource(ResourceType::Coal, 10000.0f);
    largeInventory.addResource(ResourceType::Wheat, 10000.0f);
    
    auto results = crafting->getCraftableRecipes(largeInventory);
    auto allRecipes = crafting->getAllRecipeNames();
    EXPECT_EQ(results.size(), allRecipes.size());
}
