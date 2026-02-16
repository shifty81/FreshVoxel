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

// SubsystemUpgrade Tests
TEST(SubsystemUpgradeTest, Constructor_Default_CreatesValidUpgrade) {
    SubsystemUpgrade upgrade;
    EXPECT_EQ(SubsystemType::Shield, upgrade.getType());
    EXPECT_EQ(1, upgrade.getLevel());
    EXPECT_EQ("Basic Upgrade", upgrade.getName());
}

TEST(SubsystemUpgradeTest, Constructor_WithParameters_SetsCorrectValues) {
    SubsystemUpgrade upgrade(SubsystemType::Weapon, 5, "Advanced Laser");
    EXPECT_EQ(SubsystemType::Weapon, upgrade.getType());
    EXPECT_EQ(5, upgrade.getLevel());
    EXPECT_EQ("Advanced Laser", upgrade.getName());
}

TEST(SubsystemUpgradeTest, SetStatBonus_AndGet_ReturnsCorrectValue) {
    SubsystemUpgrade upgrade;
    upgrade.setStatBonus("damage", 50.0f);
    upgrade.setStatBonus("fireRate", 2.5f);
    
    EXPECT_EQ(50.0f, upgrade.getStatBonus("damage"));
    EXPECT_EQ(2.5f, upgrade.getStatBonus("fireRate"));
}

TEST(SubsystemUpgradeTest, GetStatBonus_NonExistent_ReturnsZero) {
    SubsystemUpgrade upgrade;
    EXPECT_EQ(0.0f, upgrade.getStatBonus("nonexistent"));
}

TEST(SubsystemUpgradeTest, SetCost_AndGetCost_ReturnsCorrectMap) {
    SubsystemUpgrade upgrade;
    std::map<ResourceType, float> cost;
    cost[ResourceType::Iron] = 100.0f;
    cost[ResourceType::Titanium] = 50.0f;
    
    upgrade.setCost(cost);
    const auto& retrievedCost = upgrade.getCost();
    
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
    const CraftingRecipe* recipe = crafting->getRecipe("Basic Shield");
    ASSERT_NE(nullptr, recipe);
    EXPECT_EQ("Basic Shield", recipe->name);
    EXPECT_EQ(SubsystemType::Shield, recipe->result.getType());
}

TEST_F(CraftingSystemTest, GetRecipe_NonExistent_ReturnsNull) {
    const CraftingRecipe* recipe = crafting->getRecipe("NonExistent Recipe");
    EXPECT_EQ(nullptr, recipe);
}

TEST_F(CraftingSystemTest, GetAllRecipeNames_ReturnsAllRecipes) {
    auto recipes = crafting->getAllRecipeNames();
    EXPECT_FALSE(recipes.empty());
    
    // Check for known default recipes
    bool hasBasicShield = false;
    bool hasAdvancedShield = false;
    bool hasBasicWeapon = false;
    bool hasCargoExpansion = false;
    
    for (const auto& name : recipes) {
        if (name == "Basic Shield") hasBasicShield = true;
        if (name == "Advanced Shield") hasAdvancedShield = true;
        if (name == "Basic Weapon") hasBasicWeapon = true;
        if (name == "Cargo Expansion") hasCargoExpansion = true;
    }
    
    EXPECT_TRUE(hasBasicShield);
    EXPECT_TRUE(hasAdvancedShield);
    EXPECT_TRUE(hasBasicWeapon);
    EXPECT_TRUE(hasCargoExpansion);
}

TEST_F(CraftingSystemTest, AddRecipe_CustomRecipe_CanBeRetrieved) {
    CraftingRecipe customRecipe;
    customRecipe.name = "Custom Upgrade";
    customRecipe.result = SubsystemUpgrade(SubsystemType::Engine, 1, "Basic Engine");
    customRecipe.requirements[ResourceType::Iron] = 50.0f;
    customRecipe.craftingTime = 20.0f;
    
    crafting->addRecipe(customRecipe);
    
    const CraftingRecipe* retrieved = crafting->getRecipe("Custom Upgrade");
    ASSERT_NE(nullptr, retrieved);
    EXPECT_EQ("Custom Upgrade", retrieved->name);
    EXPECT_EQ(SubsystemType::Engine, retrieved->result.getType());
}

// CanCraft Tests
TEST_F(CraftingSystemTest, CanCraft_SufficientResources_ReturnsTrue) {
    // Basic Shield requires: Iron 50, Titanium 20
    inventory->addResource(ResourceType::Iron, 100.0f);
    inventory->addResource(ResourceType::Titanium, 50.0f);
    
    EXPECT_TRUE(crafting->canCraft("Basic Shield", *inventory));
}

TEST_F(CraftingSystemTest, CanCraft_InsufficientResources_ReturnsFalse) {
    // Basic Shield requires: Iron 50, Titanium 20
    inventory->addResource(ResourceType::Iron, 30.0f);
    inventory->addResource(ResourceType::Titanium, 10.0f);
    
    EXPECT_FALSE(crafting->canCraft("Basic Shield", *inventory));
}

TEST_F(CraftingSystemTest, CanCraft_MissingOneResource_ReturnsFalse) {
    // Basic Shield requires: Iron 50, Titanium 20
    inventory->addResource(ResourceType::Iron, 100.0f);
    // Missing Titanium
    
    EXPECT_FALSE(crafting->canCraft("Basic Shield", *inventory));
}

TEST_F(CraftingSystemTest, CanCraft_ExactResources_ReturnsTrue) {
    // Basic Shield requires: Iron 50, Titanium 20
    inventory->addResource(ResourceType::Iron, 50.0f);
    inventory->addResource(ResourceType::Titanium, 20.0f);
    
    EXPECT_TRUE(crafting->canCraft("Basic Shield", *inventory));
}

TEST_F(CraftingSystemTest, CanCraft_NonExistentRecipe_ReturnsFalse) {
    inventory->addResource(ResourceType::Iron, 1000.0f);
    EXPECT_FALSE(crafting->canCraft("NonExistent", *inventory));
}

// Craft Tests
TEST_F(CraftingSystemTest, Craft_ValidRecipeAndResources_Succeeds) {
    // Basic Shield requires: Iron 50, Titanium 20
    inventory->addResource(ResourceType::Iron, 100.0f);
    inventory->addResource(ResourceType::Titanium, 50.0f);
    
    SubsystemUpgrade result;
    EXPECT_TRUE(crafting->craft("Basic Shield", *inventory, result));
    
    // Check resources were consumed
    EXPECT_EQ(50.0f, inventory->getResourceAmount(ResourceType::Iron));
    EXPECT_EQ(30.0f, inventory->getResourceAmount(ResourceType::Titanium));
    
    // Check result
    EXPECT_EQ(SubsystemType::Shield, result.getType());
    EXPECT_EQ("Basic Shield", result.getName());
    EXPECT_EQ(100.0f, result.getStatBonus("shieldCapacity"));
    EXPECT_EQ(10.0f, result.getStatBonus("shieldRecharge"));
}

TEST_F(CraftingSystemTest, Craft_InsufficientResources_Fails) {
    // Basic Shield requires: Iron 50, Titanium 20
    inventory->addResource(ResourceType::Iron, 30.0f);
    inventory->addResource(ResourceType::Titanium, 10.0f);
    
    SubsystemUpgrade result;
    EXPECT_FALSE(crafting->craft("Basic Shield", *inventory, result));
    
    // Resources should not be consumed
    EXPECT_EQ(30.0f, inventory->getResourceAmount(ResourceType::Iron));
    EXPECT_EQ(10.0f, inventory->getResourceAmount(ResourceType::Titanium));
}

TEST_F(CraftingSystemTest, Craft_NonExistentRecipe_Fails) {
    inventory->addResource(ResourceType::Iron, 1000.0f);
    
    SubsystemUpgrade result;
    EXPECT_FALSE(crafting->craft("NonExistent", *inventory, result));
}

TEST_F(CraftingSystemTest, Craft_MultipleTimes_ConsumesCorrectly) {
    // Basic Weapon requires: Iron 30, Titanium 30
    inventory->addResource(ResourceType::Iron, 100.0f);
    inventory->addResource(ResourceType::Titanium, 100.0f);
    
    SubsystemUpgrade result1, result2;
    
    EXPECT_TRUE(crafting->craft("Basic Weapon", *inventory, result1));
    EXPECT_EQ(70.0f, inventory->getResourceAmount(ResourceType::Iron));
    EXPECT_EQ(70.0f, inventory->getResourceAmount(ResourceType::Titanium));
    
    EXPECT_TRUE(crafting->craft("Basic Weapon", *inventory, result2));
    EXPECT_EQ(40.0f, inventory->getResourceAmount(ResourceType::Iron));
    EXPECT_EQ(40.0f, inventory->getResourceAmount(ResourceType::Titanium));
}

// Default Recipe Tests
TEST_F(CraftingSystemTest, DefaultRecipe_BasicShield_HasCorrectRequirements) {
    const CraftingRecipe* recipe = crafting->getRecipe("Basic Shield");
    ASSERT_NE(nullptr, recipe);
    
    EXPECT_EQ(2u, recipe->requirements.size());
    EXPECT_EQ(50.0f, recipe->requirements.at(ResourceType::Iron));
    EXPECT_EQ(20.0f, recipe->requirements.at(ResourceType::Titanium));
    EXPECT_EQ(30.0f, recipe->craftingTime);
}

TEST_F(CraftingSystemTest, DefaultRecipe_AdvancedShield_HasCorrectRequirements) {
    const CraftingRecipe* recipe = crafting->getRecipe("Advanced Shield");
    ASSERT_NE(nullptr, recipe);
    
    EXPECT_EQ(2u, recipe->requirements.size());
    EXPECT_EQ(100.0f, recipe->requirements.at(ResourceType::Titanium));
    EXPECT_EQ(50.0f, recipe->requirements.at(ResourceType::Naonite));
    EXPECT_EQ(60.0f, recipe->craftingTime);
    EXPECT_EQ(250.0f, recipe->result.getStatBonus("shieldCapacity"));
}

TEST_F(CraftingSystemTest, DefaultRecipe_BasicWeapon_HasCorrectRequirements) {
    const CraftingRecipe* recipe = crafting->getRecipe("Basic Weapon");
    ASSERT_NE(nullptr, recipe);
    
    EXPECT_EQ(2u, recipe->requirements.size());
    EXPECT_EQ(30.0f, recipe->requirements.at(ResourceType::Iron));
    EXPECT_EQ(30.0f, recipe->requirements.at(ResourceType::Titanium));
    EXPECT_EQ(45.0f, recipe->craftingTime);
    EXPECT_EQ(SubsystemType::Weapon, recipe->result.getType());
}

TEST_F(CraftingSystemTest, DefaultRecipe_CargoExpansion_HasCorrectRequirements) {
    const CraftingRecipe* recipe = crafting->getRecipe("Cargo Expansion");
    ASSERT_NE(nullptr, recipe);
    
    EXPECT_EQ(1u, recipe->requirements.size());
    EXPECT_EQ(100.0f, recipe->requirements.at(ResourceType::Iron));
    EXPECT_EQ(40.0f, recipe->craftingTime);
    EXPECT_EQ(500.0f, recipe->result.getStatBonus("cargoCapacity"));
}

// Integration Tests
TEST_F(CraftingSystemTest, Integration_FullCraftingWorkflow_Succeeds) {
    // Start with empty inventory
    EXPECT_EQ(0.0f, inventory->getTotalUsed());
    
    // Gather resources
    inventory->addResource(ResourceType::Iron, 200.0f);
    inventory->addResource(ResourceType::Titanium, 100.0f);
    
    // Check what we can craft
    EXPECT_TRUE(crafting->canCraft("Basic Shield", *inventory));
    EXPECT_TRUE(crafting->canCraft("Basic Weapon", *inventory));
    EXPECT_TRUE(crafting->canCraft("Cargo Expansion", *inventory));
    EXPECT_FALSE(crafting->canCraft("Advanced Shield", *inventory)); // Missing Naonite
    
    // Craft Basic Shield
    SubsystemUpgrade shield;
    EXPECT_TRUE(crafting->craft("Basic Shield", *inventory, shield));
    EXPECT_EQ(SubsystemType::Shield, shield.getType());
    
    // Verify remaining resources
    EXPECT_EQ(150.0f, inventory->getResourceAmount(ResourceType::Iron));
    EXPECT_EQ(80.0f, inventory->getResourceAmount(ResourceType::Titanium));
    
    // Craft Basic Weapon
    SubsystemUpgrade weapon;
    EXPECT_TRUE(crafting->craft("Basic Weapon", *inventory, weapon));
    EXPECT_EQ(SubsystemType::Weapon, weapon.getType());
    
    // Verify final resources
    EXPECT_EQ(120.0f, inventory->getResourceAmount(ResourceType::Iron));
    EXPECT_EQ(50.0f, inventory->getResourceAmount(ResourceType::Titanium));
}

// Edge Cases
TEST_F(CraftingSystemTest, Craft_ExactResourceAmount_Succeeds) {
    // Exactly enough resources
    inventory->addResource(ResourceType::Iron, 50.0f);
    inventory->addResource(ResourceType::Titanium, 20.0f);
    
    SubsystemUpgrade result;
    EXPECT_TRUE(crafting->craft("Basic Shield", *inventory, result));
    
    // All resources consumed
    EXPECT_EQ(0.0f, inventory->getResourceAmount(ResourceType::Iron));
    EXPECT_EQ(0.0f, inventory->getResourceAmount(ResourceType::Titanium));
}

TEST_F(CraftingSystemTest, Craft_OneResourceShortBySmallAmount_Fails) {
    inventory->addResource(ResourceType::Iron, 49.99f);
    inventory->addResource(ResourceType::Titanium, 20.0f);
    
    SubsystemUpgrade result;
    EXPECT_FALSE(crafting->craft("Basic Shield", *inventory, result));
}

// Search/Filter Tests
TEST_F(CraftingSystemTest, SearchRecipes_EmptyQuery_ReturnsAll) {
    auto results = crafting->searchRecipes("");
    auto allRecipes = crafting->getAllRecipeNames();
    EXPECT_EQ(results.size(), allRecipes.size());
}

TEST_F(CraftingSystemTest, SearchRecipes_ExactName_ReturnsMatch) {
    auto results = crafting->searchRecipes("Basic Shield");
    EXPECT_EQ(1u, results.size());
    EXPECT_EQ("Basic Shield", results[0]);
}

TEST_F(CraftingSystemTest, SearchRecipes_SubstringMatch) {
    auto results = crafting->searchRecipes("Shield");
    EXPECT_GE(results.size(), 2u); // Basic Shield and Advanced Shield
    
    bool hasBasic = false, hasAdvanced = false;
    for (const auto& name : results) {
        if (name == "Basic Shield") hasBasic = true;
        if (name == "Advanced Shield") hasAdvanced = true;
    }
    EXPECT_TRUE(hasBasic);
    EXPECT_TRUE(hasAdvanced);
}

TEST_F(CraftingSystemTest, SearchRecipes_CaseInsensitive) {
    auto results = crafting->searchRecipes("basic");
    EXPECT_GE(results.size(), 2u); // Basic Shield and Basic Weapon
    
    auto results2 = crafting->searchRecipes("SHIELD");
    EXPECT_GE(results2.size(), 2u);
}

TEST_F(CraftingSystemTest, SearchRecipes_NoMatch) {
    auto results = crafting->searchRecipes("NonExistentRecipe");
    EXPECT_TRUE(results.empty());
}

TEST_F(CraftingSystemTest, GetRecipesByType_Shield) {
    auto results = crafting->getRecipesByType(SubsystemType::Shield);
    EXPECT_GE(results.size(), 2u); // Basic and Advanced Shield
    
    for (const auto& name : results) {
        const CraftingRecipe* recipe = crafting->getRecipe(name);
        ASSERT_NE(nullptr, recipe);
        EXPECT_EQ(SubsystemType::Shield, recipe->result.getType());
    }
}

TEST_F(CraftingSystemTest, GetRecipesByType_Weapon) {
    auto results = crafting->getRecipesByType(SubsystemType::Weapon);
    EXPECT_GE(results.size(), 1u); // Basic Weapon
    
    for (const auto& name : results) {
        const CraftingRecipe* recipe = crafting->getRecipe(name);
        ASSERT_NE(nullptr, recipe);
        EXPECT_EQ(SubsystemType::Weapon, recipe->result.getType());
    }
}

TEST_F(CraftingSystemTest, GetRecipesByType_NoMatch) {
    auto results = crafting->getRecipesByType(SubsystemType::Computer);
    EXPECT_TRUE(results.empty());
}

TEST_F(CraftingSystemTest, GetCraftableRecipes_NoResources_ReturnsEmpty) {
    auto results = crafting->getCraftableRecipes(*inventory);
    EXPECT_TRUE(results.empty());
}

TEST_F(CraftingSystemTest, GetCraftableRecipes_SomeResources) {
    // Give enough for Basic Shield and Cargo Expansion only
    inventory->addResource(ResourceType::Iron, 100.0f);
    inventory->addResource(ResourceType::Titanium, 20.0f);
    
    auto results = crafting->getCraftableRecipes(*inventory);
    EXPECT_GE(results.size(), 2u);
    
    bool hasBasicShield = false, hasCargo = false;
    for (const auto& name : results) {
        if (name == "Basic Shield") hasBasicShield = true;
        if (name == "Cargo Expansion") hasCargo = true;
    }
    EXPECT_TRUE(hasBasicShield);
    EXPECT_TRUE(hasCargo);
}

TEST_F(CraftingSystemTest, GetCraftableRecipes_AllResources) {
    // Use a large-capacity inventory to fit all resources needed
    Inventory largeInventory(100000.0f);
    largeInventory.addResource(ResourceType::Iron, 10000.0f);
    largeInventory.addResource(ResourceType::Titanium, 10000.0f);
    largeInventory.addResource(ResourceType::Naonite, 10000.0f);
    
    auto results = crafting->getCraftableRecipes(largeInventory);
    auto allRecipes = crafting->getAllRecipeNames();
    EXPECT_EQ(results.size(), allRecipes.size());
}
