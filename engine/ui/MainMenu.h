#pragma once

#include <functional>
#include <string>
#include <vector>

namespace fresh
{

/**
 * @brief Menu state enumeration
 */
enum class MenuState { MainMenu, NewWorld, LoadWorld, Settings, InGame, Paused };

/**
 * @brief Main menu system
 *
 * Handles the main menu interface for creating new worlds,
 * loading existing worlds, and configuring settings.
 */
class MainMenu
{
public:
    MainMenu();
    ~MainMenu();

    /**
     * @brief Initialize the main menu
     * @return true if successful
     */
    bool initialize();

    /**
     * @brief Update menu state
     * @param deltaTime Time since last update
     */
    void update(float deltaTime);

    /**
     * @brief Render the main menu
     */
    void render();

    /**
     * @brief Get current menu state
     * @return Current MenuState
     */
    MenuState getState() const
    {
        return m_state;
    }

    /**
     * @brief Set menu state
     * @param state New menu state
     */
    void setState(MenuState state)
    {
        m_state = state;
    }

    /**
     * @brief Check if a new world should be created
     * @return true if user requested new world
     */
    bool shouldCreateNewWorld() const
    {
        return m_createNewWorld;
    }

    /**
     * @brief Check if a world should be loaded
     * @return true if user requested to load world
     */
    bool shouldLoadWorld() const
    {
        return m_loadWorld;
    }

    /**
     * @brief Get the world name for new world
     * @return World name
     */
    const std::string& getNewWorldName() const
    {
        return m_newWorldName;
    }

    /**
     * @brief Get the selected world to load
     * @return World filename
     */
    const std::string& getLoadWorldName() const
    {
        return m_loadWorldName;
    }

    /**
     * @brief Get the seed for new world generation
     * @return World seed
     */
    int getWorldSeed() const
    {
        return m_worldSeed;
    }

    /**
     * @brief Clear the create/load flags
     */
    void clearFlags();

    /**
     * @brief Scan for available world saves
     */
    void scanWorldSaves();

    /**
     * @brief Get list of available world saves
     * @return Vector of world filenames
     */
    const std::vector<std::string>& getWorldSaves() const
    {
        return m_worldSaves;
    }

private:
    void renderMainMenu();
    void renderNewWorldMenu();
    void renderLoadWorldMenu();
    void renderSettingsMenu();

private:
    MenuState m_state;
    bool m_createNewWorld;
    bool m_loadWorld;
    std::string m_newWorldName;
    std::string m_loadWorldName;
    int m_worldSeed;
    int m_selectedWorldIndex;
    std::vector<std::string> m_worldSaves;

    // Menu navigation
    int m_selectedMainMenuItem;
    char m_worldNameBuffer[256];
    char m_seedBuffer[32];
};

} // namespace fresh
