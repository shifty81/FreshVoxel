#include "ui/CraftingPanel.h"
#include "core/Logger.h"
#include <algorithm>

namespace fresh
{

CraftingPanel::CraftingPanel()
    : m_craftingSystem(nullptr)
    , m_inventoryManager(nullptr)
    , m_visible(false)
    , m_selectedRecipe(-1)
    , m_crafting(false)
    , m_craftingProgress(0.0f)
    , m_craftingTimeTotal(0.0f)
    , m_craftingTimeElapsed(0.0f)
    , m_recipesDirty(true)
{
}

bool CraftingPanel::initialize(InventoryManager* inventoryManager)
{
    if (!inventoryManager) {
        LOG_ERROR_C("InventoryManager is null", "CraftingPanel");
        return false;
    }

    m_inventoryManager = inventoryManager;
    LOG_INFO_C("Crafting Panel initialized", "CraftingPanel");
    return true;
}

void CraftingPanel::update(float deltaTime)
{
    if (!m_visible) {
        return;
    }

    // Update crafting progress
    if (m_crafting) {
        updateCrafting(deltaTime);
    }

    // Refresh recipe list if needed
    if (m_recipesDirty && m_craftingSystem) {
        m_recipeNames = m_craftingSystem->getAllRecipeNames();
        m_recipesDirty = false;
    }
}

void CraftingPanel::render()
{
    if (!m_visible) {
        return;
    }

    // Note: Actual rendering would use Win32 native controls
    // This is a placeholder for the rendering structure
    // In a real implementation, this would:
    // 1. Create/update a Win32 ListView for recipe list
    // 2. Display recipe details in a panel
    // 3. Show requirements with progress bars
    // 4. Display crafting progress bar if active
    // 5. Enable/disable craft button based on requirements
}

void CraftingPanel::setCraftingSystem(rpg::CraftingSystem* craftingSystem)
{
    m_craftingSystem = craftingSystem;
    m_recipesDirty = true;
}

void CraftingPanel::setVisible(bool visible)
{
    m_visible = visible;
    if (visible && m_recipesDirty) {
        // Refresh recipes when showing panel
        if (m_craftingSystem) {
            m_recipeNames = m_craftingSystem->getAllRecipeNames();
            m_recipesDirty = false;
        }
    }
}

void CraftingPanel::setStationFilter(const std::string& stationType)
{
    m_stationFilter = stationType;
    m_recipesDirty = true;
}

std::vector<CraftingPanel::RecipeDisplay> CraftingPanel::getAvailableRecipes() const
{
    std::vector<RecipeDisplay> displays;

    if (!m_craftingSystem || !m_inventoryManager) {
        return displays;
    }

    for (const auto& recipeName : m_recipeNames) {
        if (matchesFilters(recipeName)) {
            displays.push_back(buildRecipeDisplay(recipeName));
        }
    }

    return displays;
}

void CraftingPanel::setSelectedRecipe(int recipeIndex)
{
    if (recipeIndex >= 0 && recipeIndex < static_cast<int>(m_recipeNames.size())) {
        m_selectedRecipe = recipeIndex;
    } else {
        m_selectedRecipe = -1;
    }
}

bool CraftingPanel::craftSelectedRecipe()
{
    if (!m_craftingSystem || !m_inventoryManager || m_crafting) {
        return false;
    }

    if (m_selectedRecipe < 0 || m_selectedRecipe >= static_cast<int>(m_recipeNames.size())) {
        LOG_WARNING_C("No recipe selected", "CraftingPanel");
        return false;
    }

    const std::string& recipeName = m_recipeNames[m_selectedRecipe];
    const rpg::CraftingRecipe* recipe = m_craftingSystem->getRecipe(recipeName);

    if (!recipe) {
        LOG_ERROR_C("Recipe not found: " + recipeName, "CraftingPanel");
        return false;
    }

    // Check if we can craft
    if (!m_craftingSystem->canCraft(recipeName, m_inventoryManager->getInventory())) {
        LOG_WARNING_C("Insufficient resources for: " + recipeName, "CraftingPanel");
        if (m_onCraftingFailed) {
            m_onCraftingFailed("Insufficient resources");
        }
        return false;
    }

    // Start crafting
    m_crafting = true;
    m_currentRecipe = recipeName;
    m_craftingTimeTotal = recipe->craftingTime;
    m_craftingTimeElapsed = 0.0f;
    m_craftingProgress = 0.0f;
    m_craftingResult = recipe->result;

    // Consume resources immediately
    for (const auto& [type, amount] : recipe->requirements) {
        m_inventoryManager->removeItem(type, amount);
    }

    LOG_INFO_C("Started crafting: " + recipeName, "CraftingPanel");

    if (m_onCraftingStarted) {
        m_onCraftingStarted(recipeName);
    }

    return true;
}

float CraftingPanel::getCraftingProgress() const
{
    return m_craftingProgress;
}

void CraftingPanel::cancelCrafting()
{
    if (!m_crafting) {
        return;
    }

    // Return resources (partial refund based on progress)
    if (m_craftingSystem && m_inventoryManager) {
        const rpg::CraftingRecipe* recipe = m_craftingSystem->getRecipe(m_currentRecipe);
        if (recipe) {
            float refundRate = 1.0f - m_craftingProgress; // Return unused portion
            for (const auto& [type, amount] : recipe->requirements) {
                float refundAmount = amount * refundRate;
                m_inventoryManager->addItem(type, refundAmount);
            }
        }
    }

    m_crafting = false;
    m_craftingProgress = 0.0f;
    LOG_INFO_C("Cancelled crafting: " + m_currentRecipe, "CraftingPanel");
}

void CraftingPanel::setSearchFilter(const std::string& searchText)
{
    m_searchFilter = searchText;
}

void CraftingPanel::sortByName()
{
    if (m_recipeNames.empty()) {
        return;
    }

    std::sort(m_recipeNames.begin(), m_recipeNames.end());
}

void CraftingPanel::sortByCraftable()
{
    if (!m_craftingSystem || !m_inventoryManager || m_recipeNames.empty()) {
        return;
    }

    std::sort(m_recipeNames.begin(), m_recipeNames.end(),
        [this](const std::string& a, const std::string& b) {
            bool canCraftA = m_craftingSystem->canCraft(a, m_inventoryManager->getInventory());
            bool canCraftB = m_craftingSystem->canCraft(b, m_inventoryManager->getInventory());
            
            if (canCraftA != canCraftB) {
                return canCraftA; // Craftable first
            }
            return a < b; // Then alphabetically
        });
}

CraftingPanel::RecipeDisplay CraftingPanel::buildRecipeDisplay(const std::string& recipeName) const
{
    RecipeDisplay display;
    display.name = recipeName;
    display.canCraft = false;

    if (!m_craftingSystem || !m_inventoryManager) {
        return display;
    }

    const rpg::CraftingRecipe* recipe = m_craftingSystem->getRecipe(recipeName);
    if (!recipe) {
        return display;
    }

    // Check if player can craft
    display.canCraft = m_craftingSystem->canCraft(recipeName, m_inventoryManager->getInventory());
    display.craftingTime = recipe->craftingTime;

    // Build requirements list
    for (const auto& [type, amount] : recipe->requirements) {
        float hasAmount = m_inventoryManager->getItemAmount(type);
        std::string req = rpg::getResourceName(type) + ": " +
                         std::to_string(static_cast<int>(hasAmount)) + "/" +
                         std::to_string(static_cast<int>(amount));
        display.requirements.push_back(req);
    }

    // Build result description
    display.result = recipe->result.getName() + " (Level " + 
                    std::to_string(recipe->result.getLevel()) + ")";

    return display;
}

bool CraftingPanel::matchesFilters(const std::string& recipeName) const
{
    // Apply search filter
    if (!m_searchFilter.empty()) {
        // Case-insensitive search
        std::string lowerRecipe = recipeName;
        std::string lowerSearch = m_searchFilter;
        std::transform(lowerRecipe.begin(), lowerRecipe.end(), lowerRecipe.begin(), ::tolower);
        std::transform(lowerSearch.begin(), lowerSearch.end(), lowerSearch.begin(), ::tolower);
        
        if (lowerRecipe.find(lowerSearch) == std::string::npos) {
            return false;
        }
    }

    // Apply station filter (future enhancement)
    // For now, station filter is not implemented in recipe data

    return true;
}

void CraftingPanel::updateCrafting(float deltaTime)
{
    if (!m_crafting) {
        return;
    }

    m_craftingTimeElapsed += deltaTime;
    m_craftingProgress = std::min(1.0f, m_craftingTimeElapsed / m_craftingTimeTotal);

    // Check if crafting is complete
    if (m_craftingTimeElapsed >= m_craftingTimeTotal) {
        // Crafting complete!
        m_crafting = false;
        m_craftingProgress = 1.0f;

        LOG_INFO_C("Completed crafting: " + m_currentRecipe, "CraftingPanel");

        if (m_onCraftingComplete) {
            m_onCraftingComplete(m_currentRecipe);
        }

        // Note: In a full implementation, the crafted upgrade would be
        // added to player's equipment/upgrade inventory
        // For now, we just log completion
    }
}

} // namespace fresh
