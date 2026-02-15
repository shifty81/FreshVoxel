#include "editor/EditorSettingsDialog.h"
#include "core/Logger.h"
#include <fstream>
#include <sstream>
#include <filesystem>

namespace fresh
{

EditorSettingsDialog::EditorSettingsDialog()
    : m_visible(false),
      m_initialized(false),
      m_settingsModified(false)
{
}

EditorSettingsDialog::~EditorSettingsDialog()
{
}

bool EditorSettingsDialog::initialize(const std::string& configPath)
{
    m_configPath = configPath;
    
    // Create configs directory if it doesn't exist
    std::filesystem::path path(configPath);
    std::filesystem::path dir = path.parent_path();
    if (!dir.empty() && !std::filesystem::exists(dir)) {
        try {
            std::filesystem::create_directories(dir);
            LOG_INFO_C("Created configs directory: " + dir.string(), "EditorSettingsDialog");
        } catch (const std::exception& e) {
            LOG_ERROR_C("Failed to create configs directory: " + std::string(e.what()), "EditorSettingsDialog");
            return false;
        }
    }
    
    // Load settings from file or use defaults
    if (!loadSettings()) {
        LOG_WARNING_C("Could not load editor settings, using defaults", "EditorSettingsDialog");
        m_settings = EditorSettings(); // Use default settings
    }
    
    m_tempSettings = m_settings;
    m_initialized = true;
    
    LOG_INFO_C("Editor settings dialog initialized", "EditorSettingsDialog");
    return true;
}

void EditorSettingsDialog::render()
{
    if (!m_visible || !m_initialized) {
        return;
    }
    
    // TODO: Implement with Windows native UI (ImGui has been removed from this project)
    LOG_WARNING_C("EditorSettingsDialog::render() - ImGui UI not available, use Win32SettingsDialog instead", "EditorSettingsDialog");
}

void EditorSettingsDialog::renderAutoSaveSettings()
{
    // Stub - ImGui UI not available
}

void EditorSettingsDialog::renderGridSettings()
{
    // Stub - ImGui UI not available
}

void EditorSettingsDialog::renderSnapSettings()
{
    // Stub - ImGui UI not available
}

void EditorSettingsDialog::renderUISettings()
{
    // Stub - ImGui UI not available
}

void EditorSettingsDialog::renderToolSettings()
{
    // Stub - ImGui UI not available
}

void EditorSettingsDialog::renderCameraSettings()
{
    // Stub - ImGui UI not available
}

void EditorSettingsDialog::renderPerformanceSettings()
{
    // Stub - ImGui UI not available
}

void EditorSettingsDialog::setSettings(const EditorSettings& settings)
{
    m_settings = settings;
    m_tempSettings = settings;
    notifySettingsChanged();
}

void EditorSettingsDialog::applySettings()
{
    m_settings = m_tempSettings;
    saveSettings();
    notifySettingsChanged();
    LOG_INFO_C("Editor settings applied", "EditorSettingsDialog");
}

void EditorSettingsDialog::notifySettingsChanged()
{
    if (m_settingsChangedCallback) {
        m_settingsChangedCallback(m_settings);
    }
}

bool EditorSettingsDialog::saveSettings()
{
    std::ofstream file(m_configPath);
    if (!file.is_open()) {
        LOG_ERROR_C("Failed to open settings file for writing: " + m_configPath, "EditorSettingsDialog");
        return false;
    }
    
    file << "[AutoSave]" << std::endl;
    file << "enabled=" << (m_settings.autoSaveEnabled ? "1" : "0") << std::endl;
    file << "intervalMinutes=" << m_settings.autoSaveIntervalMinutes << std::endl;
    file << std::endl;
    
    file << "[Grid]" << std::endl;
    file << "show=" << (m_settings.showGrid ? "1" : "0") << std::endl;
    file << "size=" << m_settings.gridSize << std::endl;
    file << "subdivisions=" << m_settings.gridSubdivisions << std::endl;
    file << "opacity=" << m_settings.gridOpacity << std::endl;
    file << std::endl;
    
    file << "[Snap]" << std::endl;
    file << "snapToGrid=" << (m_settings.snapToGrid ? "1" : "0") << std::endl;
    file << "snapValue=" << m_settings.snapValue << std::endl;
    file << "snapRotation=" << (m_settings.snapRotation ? "1" : "0") << std::endl;
    file << "snapRotationDegrees=" << m_settings.snapRotationDegrees << std::endl;
    file << std::endl;
    
    file << "[UI]" << std::endl;
    file << "scale=" << m_settings.uiScale << std::endl;
    file << "fontSize=" << m_settings.fontSize << std::endl;
    file << "showTooltips=" << (m_settings.showTooltips ? "1" : "0") << std::endl;
    file << std::endl;
    
    file << "[Tools]" << std::endl;
    file << "defaultBrushSize=" << m_settings.defaultBrushSize << std::endl;
    file << "defaultBrushShape=" << m_settings.defaultBrushShape << std::endl;
    file << std::endl;
    
    file << "[Camera]" << std::endl;
    file << "defaultSpeed=" << m_settings.defaultCameraSpeed << std::endl;
    file << "defaultFastSpeed=" << m_tempSettings.defaultCameraFastSpeed << std::endl;
    file << std::endl;
    
    file << "[Performance]" << std::endl;
    file << "vsync=" << (m_settings.vsync ? "1" : "0") << std::endl;
    file << "targetFPS=" << m_settings.targetFPS << std::endl;
    
    file.close();
    LOG_INFO_C("Editor settings saved to: " + m_configPath, "EditorSettingsDialog");
    return true;
}

bool EditorSettingsDialog::loadSettings()
{
    std::ifstream file(m_configPath);
    if (!file.is_open()) {
        LOG_WARNING_C("Settings file not found: " + m_configPath, "EditorSettingsDialog");
        return false;
    }
    
    std::string line;
    std::string currentSection;
    
    while (std::getline(file, line)) {
        // Remove whitespace
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);
        
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#' || line[0] == ';') {
            continue;
        }
        
        // Parse section headers
        if (line[0] == '[' && line[line.length() - 1] == ']') {
            currentSection = line.substr(1, line.length() - 2);
            continue;
        }
        
        // Parse key=value pairs
        size_t equalPos = line.find('=');
        if (equalPos != std::string::npos) {
            std::string key = line.substr(0, equalPos);
            std::string value = line.substr(equalPos + 1);
            
            // Trim whitespace from key and value
            key.erase(0, key.find_first_not_of(" \t"));
            key.erase(key.find_last_not_of(" \t") + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t") + 1);
            
            // Apply settings based on section
            if (currentSection == "AutoSave") {
                if (key == "enabled") m_settings.autoSaveEnabled = (value == "1");
                else if (key == "intervalMinutes") m_settings.autoSaveIntervalMinutes = std::stoi(value);
            }
            else if (currentSection == "Grid") {
                if (key == "show") m_settings.showGrid = (value == "1");
                else if (key == "size") m_settings.gridSize = std::stof(value);
                else if (key == "subdivisions") m_settings.gridSubdivisions = std::stoi(value);
                else if (key == "opacity") m_settings.gridOpacity = std::stof(value);
            }
            else if (currentSection == "Snap") {
                if (key == "snapToGrid") m_settings.snapToGrid = (value == "1");
                else if (key == "snapValue") m_settings.snapValue = std::stof(value);
                else if (key == "snapRotation") m_settings.snapRotation = (value == "1");
                else if (key == "snapRotationDegrees") m_settings.snapRotationDegrees = std::stof(value);
            }
            else if (currentSection == "UI") {
                if (key == "scale") m_settings.uiScale = std::stof(value);
                else if (key == "fontSize") m_settings.fontSize = std::stoi(value);
                else if (key == "showTooltips") m_settings.showTooltips = (value == "1");
            }
            else if (currentSection == "Tools") {
                if (key == "defaultBrushSize") m_settings.defaultBrushSize = std::stoi(value);
                else if (key == "defaultBrushShape") m_settings.defaultBrushShape = std::stoi(value);
            }
            else if (currentSection == "Camera") {
                if (key == "defaultSpeed") m_settings.defaultCameraSpeed = std::stof(value);
                else if (key == "defaultFastSpeed") m_settings.defaultCameraFastSpeed = std::stof(value);
            }
            else if (currentSection == "Performance") {
                if (key == "vsync") m_settings.vsync = (value == "1");
                else if (key == "targetFPS") m_settings.targetFPS = std::stoi(value);
            }
        }
    }
    
    file.close();
    LOG_INFO_C("Editor settings loaded from: " + m_configPath, "EditorSettingsDialog");
    return true;
}

void EditorSettingsDialog::resetToDefaults()
{
    m_tempSettings = EditorSettings(); // Reset to default constructor values
    LOG_INFO_C("Editor settings reset to defaults", "EditorSettingsDialog");
}

} // namespace fresh
