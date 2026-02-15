#pragma once

#include <string>
#include <vector>
#include <unordered_map>

namespace fresh
{

/**
 * @brief Layout configuration structure
 */
struct LayoutConfig
{
    std::string name;
    bool showSceneHierarchy;
    bool showInspector;
    bool showContentBrowser;
    bool showConsole;
    bool showToolPalette;
    
    // Default constructor
    LayoutConfig()
        : showSceneHierarchy(true),
          showInspector(true),
          showContentBrowser(true),
          showConsole(true),
          showToolPalette(true)
    {
    }
};

/**
 * @brief Manages editor layout configurations
 * 
 * Handles saving, loading, and applying different workspace layouts.
 * Supports predefined layouts (Default, Minimal, Debugging) and custom user layouts.
 */
class LayoutManager
{
public:
    LayoutManager();
    ~LayoutManager();
    
    /**
     * @brief Initialize the layout manager
     * @param configPath Path to the layout configuration file
     * @return true if successful
     */
    bool initialize(const std::string& configPath = "configs/layouts.ini");
    
    /**
     * @brief Save the current layout configuration
     * @param name Layout name
     * @param config Layout configuration to save
     * @return true if successful
     */
    bool saveLayout(const std::string& name, const LayoutConfig& config);
    
    /**
     * @brief Load a layout configuration
     * @param name Layout name
     * @param config Output parameter for loaded configuration
     * @return true if successful
     */
    bool loadLayout(const std::string& name, LayoutConfig& config);
    
    /**
     * @brief Get a predefined layout configuration
     * @param name Layout name ("Default", "Minimal", "Debugging")
     * @param config Output parameter for layout configuration
     * @return true if the layout exists
     */
    bool getPredefinedLayout(const std::string& name, LayoutConfig& config);
    
    /**
     * @brief Get list of available layout names
     * @return Vector of layout names
     */
    std::vector<std::string> getAvailableLayouts() const;
    
    /**
     * @brief Save current state to configuration file
     * @return true if successful
     */
    bool saveToFile();
    
    /**
     * @brief Load layouts from configuration file
     * @return true if successful
     */
    bool loadFromFile();
    
    /**
     * @brief Get the current layout name
     * @return Current layout name
     */
    const std::string& getCurrentLayoutName() const { return m_currentLayoutName; }
    
    /**
     * @brief Set the current layout name
     * @param name Layout name
     */
    void setCurrentLayoutName(const std::string& name) { m_currentLayoutName = name; }

private:
    void initializePredefinedLayouts();
    
private:
    std::string m_configPath;
    std::string m_currentLayoutName;
    std::unordered_map<std::string, LayoutConfig> m_layouts;
    std::unordered_map<std::string, LayoutConfig> m_predefinedLayouts;
    bool m_initialized;
};

} // namespace fresh
