#include "editor/LayoutManager.h"
#include "core/Logger.h"

#include <fstream>
#include <sstream>
#include <filesystem>

namespace fresh
{

LayoutManager::LayoutManager()
    : m_currentLayoutName("Default"),
      m_initialized(false)
{
}

LayoutManager::~LayoutManager()
{
    if (m_initialized) {
        saveToFile();
    }
}

bool LayoutManager::initialize(const std::string& configPath)
{
    m_configPath = configPath;
    
    // Initialize predefined layouts
    initializePredefinedLayouts();
    
    // Try to load from file
    loadFromFile();
    
    m_initialized = true;
    LOG_INFO_C("Layout Manager initialized", "LayoutManager");
    return true;
}

void LayoutManager::initializePredefinedLayouts()
{
    // Default layout - all panels visible
    LayoutConfig defaultLayout;
    defaultLayout.name = "Default";
    defaultLayout.showSceneHierarchy = true;
    defaultLayout.showInspector = true;
    defaultLayout.showContentBrowser = true;
    defaultLayout.showConsole = true;
    defaultLayout.showToolPalette = true;
    m_predefinedLayouts["Default"] = defaultLayout;
    
    // Minimal layout - only essential panels
    LayoutConfig minimalLayout;
    minimalLayout.name = "Minimal";
    minimalLayout.showSceneHierarchy = false;
    minimalLayout.showInspector = true;
    minimalLayout.showContentBrowser = false;
    minimalLayout.showConsole = false;
    minimalLayout.showToolPalette = true;
    m_predefinedLayouts["Minimal"] = minimalLayout;
    
    // Debugging layout - console and inspector prominent
    LayoutConfig debuggingLayout;
    debuggingLayout.name = "Debugging";
    debuggingLayout.showSceneHierarchy = true;
    debuggingLayout.showInspector = true;
    debuggingLayout.showContentBrowser = false;
    debuggingLayout.showConsole = true;
    debuggingLayout.showToolPalette = false;
    m_predefinedLayouts["Debugging"] = debuggingLayout;
    
    LOG_INFO_C("Predefined layouts initialized (Default, Minimal, Debugging)", "LayoutManager");
}

bool LayoutManager::saveLayout(const std::string& name, const LayoutConfig& config)
{
    LayoutConfig savedConfig = config;
    savedConfig.name = name;
    m_layouts[name] = savedConfig;
    
    LOG_INFO_C("Layout saved: " + name, "LayoutManager");
    return saveToFile();
}

bool LayoutManager::loadLayout(const std::string& name, LayoutConfig& config)
{
    // First check user-saved layouts
    auto it = m_layouts.find(name);
    if (it != m_layouts.end()) {
        config = it->second;
        m_currentLayoutName = name;
        LOG_INFO_C("Loaded user layout: " + name, "LayoutManager");
        return true;
    }
    
    // Then check predefined layouts
    auto predefinedIt = m_predefinedLayouts.find(name);
    if (predefinedIt != m_predefinedLayouts.end()) {
        config = predefinedIt->second;
        m_currentLayoutName = name;
        LOG_INFO_C("Loaded predefined layout: " + name, "LayoutManager");
        return true;
    }
    
    LOG_WARNING_C("Layout not found: " + name, "LayoutManager");
    return false;
}

bool LayoutManager::getPredefinedLayout(const std::string& name, LayoutConfig& config)
{
    auto it = m_predefinedLayouts.find(name);
    if (it != m_predefinedLayouts.end()) {
        config = it->second;
        return true;
    }
    return false;
}

std::vector<std::string> LayoutManager::getAvailableLayouts() const
{
    std::vector<std::string> layouts;
    
    // Add predefined layouts first
    for (const auto& pair : m_predefinedLayouts) {
        layouts.push_back(pair.first);
    }
    
    // Add user layouts
    for (const auto& pair : m_layouts) {
        layouts.push_back(pair.first);
    }
    
    return layouts;
}

bool LayoutManager::saveToFile()
{
    try {
        // Create directory if it doesn't exist
        std::filesystem::path configPath(m_configPath);
        std::filesystem::path configDir = configPath.parent_path();
        
        if (!configDir.empty() && !std::filesystem::exists(configDir)) {
            std::filesystem::create_directories(configDir);
        }
        
        // Open file for writing
        std::ofstream file(m_configPath);
        if (!file.is_open()) {
            LOG_ERROR_C("Failed to open layout config file for writing: " + m_configPath, "LayoutManager");
            return false;
        }
        
        // Write current layout name
        file << "[Current]\n";
        file << "layout=" << m_currentLayoutName << "\n\n";
        
        // Write user layouts
        for (const auto& pair : m_layouts) {
            const LayoutConfig& config = pair.second;
            file << "[" << pair.first << "]\n";
            file << "showSceneHierarchy=" << (config.showSceneHierarchy ? "1" : "0") << "\n";
            file << "showInspector=" << (config.showInspector ? "1" : "0") << "\n";
            file << "showContentBrowser=" << (config.showContentBrowser ? "1" : "0") << "\n";
            file << "showConsole=" << (config.showConsole ? "1" : "0") << "\n";
            file << "showToolPalette=" << (config.showToolPalette ? "1" : "0") << "\n";
            file << "\n";
        }
        
        file.close();
        LOG_INFO_C("Layout configuration saved to: " + m_configPath, "LayoutManager");
        return true;
        
    } catch (const std::exception& e) {
        LOG_ERROR_C("Exception saving layout config: " + std::string(e.what()), "LayoutManager");
        return false;
    }
}

bool LayoutManager::loadFromFile()
{
    try {
        // Check if file exists
        if (!std::filesystem::exists(m_configPath)) {
            LOG_INFO_C("Layout config file not found, using defaults: " + m_configPath, "LayoutManager");
            return true; // Not an error, just use defaults
        }
        
        std::ifstream file(m_configPath);
        if (!file.is_open()) {
            LOG_WARNING_C("Failed to open layout config file: " + m_configPath, "LayoutManager");
            return false;
        }
        
        std::string line;
        std::string currentSection;
        LayoutConfig currentConfig;
        
        while (std::getline(file, line)) {
            // Trim whitespace
            line.erase(0, line.find_first_not_of(" \t\r\n"));
            line.erase(line.find_last_not_of(" \t\r\n") + 1);
            
            // Skip empty lines and comments
            if (line.empty() || line[0] == '#' || line[0] == ';') {
                continue;
            }
            
            // Check for section header
            if (line[0] == '[' && line[line.length() - 1] == ']') {
                // Save previous section if not "Current"
                if (!currentSection.empty() && currentSection != "Current") {
                    currentConfig.name = currentSection;
                    m_layouts[currentSection] = currentConfig;
                    currentConfig = LayoutConfig(); // Reset
                }
                
                currentSection = line.substr(1, line.length() - 2);
                continue;
            }
            
            // Parse key=value
            size_t pos = line.find('=');
            if (pos != std::string::npos) {
                std::string key = line.substr(0, pos);
                std::string value = line.substr(pos + 1);
                
                // Trim key and value
                key.erase(key.find_last_not_of(" \t") + 1);
                value.erase(0, value.find_first_not_of(" \t"));
                
                if (currentSection == "Current") {
                    if (key == "layout") {
                        m_currentLayoutName = value;
                    }
                } else {
                    // Parse layout properties
                    bool boolValue = (value == "1" || value == "true");
                    
                    if (key == "showSceneHierarchy") {
                        currentConfig.showSceneHierarchy = boolValue;
                    } else if (key == "showInspector") {
                        currentConfig.showInspector = boolValue;
                    } else if (key == "showContentBrowser") {
                        currentConfig.showContentBrowser = boolValue;
                    } else if (key == "showConsole") {
                        currentConfig.showConsole = boolValue;
                    } else if (key == "showToolPalette") {
                        currentConfig.showToolPalette = boolValue;
                    }
                }
            }
        }
        
        // Save last section
        if (!currentSection.empty() && currentSection != "Current") {
            currentConfig.name = currentSection;
            m_layouts[currentSection] = currentConfig;
        }
        
        file.close();
        LOG_INFO_C("Layout configuration loaded from: " + m_configPath, "LayoutManager");
        return true;
        
    } catch (const std::exception& e) {
        LOG_ERROR_C("Exception loading layout config: " + std::string(e.what()), "LayoutManager");
        return false;
    }
}

} // namespace fresh
