#pragma once

#include "rpg/CraftingSystem.h"
#include "gameplay/InventoryManager.h"
#include <vector>
#include <string>
#include <functional>

namespace fresh
{

/**
 * @brief UI Panel for the crafting system
 * 
 * Provides an interface for viewing available recipes,
 * checking requirements, and crafting items. Integrates
 * with the InventoryManager to access player resources.
 */
class CraftingPanel
{
public:
    /**
     * @brief Recipe display information
     */
    struct RecipeDisplay {
        std::string name;
        std::string description;
        bool canCraft;                          // Whether player has requirements
        std::vector<std::string> requirements;  // "Iron: 10/5" format
        std::string result;                     // Result item description
        float craftingTime;                     // Time in seconds
    };

    CraftingPanel();
    ~CraftingPanel() = default;

    /**
     * @brief Initialize the crafting panel
     * @param inventoryManager Reference to inventory manager
     * @return true if successful
     */
    bool initialize(InventoryManager* inventoryManager);

    /**
     * @brief Update crafting panel
     * @param deltaTime Time since last update
     */
    void update(float deltaTime);

    /**
     * @brief Render the crafting panel
     */
    void render();

    /**
     * @brief Set the crafting system
     * @param craftingSystem Crafting system to use
     */
    void setCraftingSystem(rpg::CraftingSystem* craftingSystem);

    /**
     * @brief Get crafting system
     * @return Pointer to crafting system
     */
    rpg::CraftingSystem* getCraftingSystem() const { return m_craftingSystem; }

    /**
     * @brief Show/hide the crafting panel
     * @param visible True to show
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
     * @brief Set the required crafting station type
     * @param stationType Station type filter
     */
    void setStationFilter(const std::string& stationType);

    /**
     * @brief Get current station filter
     * @return Station type or empty for all
     */
    const std::string& getStationFilter() const { return m_stationFilter; }

    /**
     * @brief Get all available recipes (filtered by station)
     * @return Vector of recipe display info
     */
    std::vector<RecipeDisplay> getAvailableRecipes() const;

    /**
     * @brief Get selected recipe index
     * @return Selected recipe index or -1
     */
    int getSelectedRecipe() const { return m_selectedRecipe; }

    /**
     * @brief Set selected recipe
     * @param recipeIndex Index of recipe to select
     */
    void setSelectedRecipe(int recipeIndex);

    /**
     * @brief Attempt to craft the selected recipe
     * @return true if crafting succeeded
     */
    bool craftSelectedRecipe();

    /**
     * @brief Check if currently crafting
     * @return True if crafting in progress
     */
    bool isCrafting() const { return m_crafting; }

    /**
     * @brief Get crafting progress (0.0 - 1.0)
     * @return Crafting progress
     */
    float getCraftingProgress() const;

    /**
     * @brief Cancel current crafting operation
     */
    void cancelCrafting();

    /**
     * @brief Set callback for when crafting completes
     * @param callback Function to call with crafted item info
     */
    void setOnCraftingComplete(std::function<void(const std::string&)> callback) {
        m_onCraftingComplete = callback;
    }

    /**
     * @brief Set callback for when crafting starts
     * @param callback Function to call when crafting begins
     */
    void setOnCraftingStarted(std::function<void(const std::string&)> callback) {
        m_onCraftingStarted = callback;
    }

    /**
     * @brief Set callback for when crafting fails
     * @param callback Function to call with error message
     */
    void setOnCraftingFailed(std::function<void(const std::string&)> callback) {
        m_onCraftingFailed = callback;
    }

    /**
     * @brief Set search filter for recipes
     * @param searchText Text to filter recipe names
     */
    void setSearchFilter(const std::string& searchText);

    /**
     * @brief Get current search filter
     * @return Search text
     */
    const std::string& getSearchFilter() const { return m_searchFilter; }

    /**
     * @brief Sort recipes by name
     */
    void sortByName();

    /**
     * @brief Sort recipes by craftability (craftable first)
     */
    void sortByCraftable();

private:
    /**
     * @brief Build recipe display from recipe data
     * @param recipeName Recipe name
     * @return Recipe display info
     */
    RecipeDisplay buildRecipeDisplay(const std::string& recipeName) const;

    /**
     * @brief Check if recipe matches filters
     * @param recipeName Recipe name
     * @return True if matches
     */
    bool matchesFilters(const std::string& recipeName) const;

    /**
     * @brief Update crafting progress
     * @param deltaTime Time delta
     */
    void updateCrafting(float deltaTime);

private:
    rpg::CraftingSystem* m_craftingSystem;
    InventoryManager* m_inventoryManager;

    bool m_visible;
    int m_selectedRecipe;
    std::string m_stationFilter;        // Filter by crafting station
    std::string m_searchFilter;         // Search text filter

    // Crafting state
    bool m_crafting;
    std::string m_currentRecipe;
    float m_craftingProgress;           // 0.0 to 1.0
    float m_craftingTimeTotal;
    float m_craftingTimeElapsed;
    rpg::SubsystemUpgrade m_craftingResult;

    // Cached recipe list
    std::vector<std::string> m_recipeNames;
    bool m_recipesDirty;

    // Callbacks
    std::function<void(const std::string&)> m_onCraftingComplete;
    std::function<void(const std::string&)> m_onCraftingStarted;
    std::function<void(const std::string&)> m_onCraftingFailed;
};

} // namespace fresh
