#pragma once

#include <functional>
#include <string>

namespace fresh
{

class EditorManager;

/**
 * @brief Editor settings structure
 */
struct EditorSettings
{
    // Auto-save settings
    bool autoSaveEnabled;
    int autoSaveIntervalMinutes;
    
    // Grid settings
    bool showGrid;
    float gridSize;
    int gridSubdivisions;
    float gridOpacity;
    
    // Snap settings
    bool snapToGrid;
    float snapValue;
    bool snapRotation;
    float snapRotationDegrees;
    
    // UI Theme settings
    float uiScale;
    int fontSize;
    bool showTooltips;
    
    // Default tool settings
    int defaultBrushSize;
    int defaultBrushShape; // 0=Sphere, 1=Cube, 2=Cylinder
    
    // Camera settings
    float defaultCameraSpeed;
    float defaultCameraFastSpeed;
    
    // Performance settings
    bool vsync;
    int targetFPS;
    
    // Default constructor with sensible defaults
    EditorSettings()
        : autoSaveEnabled(true),
          autoSaveIntervalMinutes(5),
          showGrid(true),
          gridSize(1.0f),
          gridSubdivisions(10),
          gridOpacity(0.3f),
          snapToGrid(false),
          snapValue(1.0f),
          snapRotation(false),
          snapRotationDegrees(15.0f),
          uiScale(1.0f),
          fontSize(16),
          showTooltips(true),
          defaultBrushSize(3),
          defaultBrushShape(0),
          defaultCameraSpeed(10.0f),
          defaultCameraFastSpeed(30.0f),
          vsync(true),
          targetFPS(60)
    {
    }
};

/**
 * @brief Editor Settings Dialog
 * 
 * Provides UI for configuring editor preferences including:
 * - Auto-save settings
 * - Grid and snap settings
 * - UI theme and font settings
 * - Default tool settings
 * - Performance settings
 */
class EditorSettingsDialog
{
public:
    using SettingsChangedCallback = std::function<void(const EditorSettings&)>;
    
    EditorSettingsDialog();
    ~EditorSettingsDialog();
    
    /**
     * @brief Initialize the settings dialog
     * @param configPath Path to the settings configuration file
     * @return true if successful
     */
    bool initialize(const std::string& configPath = "configs/editor_settings.ini");
    
    /**
     * @brief Render the settings dialog
     */
    void render();
    
    /**
     * @brief Show/hide the settings dialog
     * @param show true to show, false to hide
     */
    void setVisible(bool show) { m_visible = show; }
    
    /**
     * @brief Check if the settings dialog is visible
     * @return true if visible
     */
    bool isVisible() const { return m_visible; }
    
    /**
     * @brief Toggle settings dialog visibility
     */
    void toggle() { m_visible = !m_visible; }
    
    /**
     * @brief Get current editor settings
     * @return Current settings
     */
    const EditorSettings& getSettings() const { return m_settings; }
    
    /**
     * @brief Set editor settings
     * @param settings New settings to apply
     */
    void setSettings(const EditorSettings& settings);
    
    /**
     * @brief Save settings to configuration file
     * @return true if successful
     */
    bool saveSettings();
    
    /**
     * @brief Load settings from configuration file
     * @return true if successful
     */
    bool loadSettings();
    
    /**
     * @brief Reset settings to default values
     */
    void resetToDefaults();
    
    /**
     * @brief Set callback for when settings change
     * @param callback Function to call when settings are modified
     */
    void setSettingsChangedCallback(SettingsChangedCallback callback)
    {
        m_settingsChangedCallback = callback;
    }

private:
    void renderAutoSaveSettings();
    void renderGridSettings();
    void renderSnapSettings();
    void renderUISettings();
    void renderToolSettings();
    void renderCameraSettings();
    void renderPerformanceSettings();
    
    void applySettings();
    void notifySettingsChanged();

private:
    bool m_visible;
    bool m_initialized;
    std::string m_configPath;
    
    EditorSettings m_settings;
    EditorSettings m_tempSettings; // Temporary settings while dialog is open
    
    bool m_settingsModified;
    
    SettingsChangedCallback m_settingsChangedCallback;
};

} // namespace fresh
