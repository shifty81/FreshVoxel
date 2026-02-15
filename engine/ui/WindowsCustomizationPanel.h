#pragma once

#ifdef _WIN32

namespace fresh
{

class WindowsThemeManager;
class WindowsDialogManager;
class WindowsTaskbarManager;

/**
 * @brief UI Panel for Windows-native customization
 * 
 * Provides interface for:
 * - Theme selection and customization
 * - Native dialog testing
 * - Taskbar feature controls
 * - Windows integration settings
 */
class WindowsCustomizationPanel
{
public:
    WindowsCustomizationPanel();
    ~WindowsCustomizationPanel();

    /**
     * @brief Initialize the customization panel
     * @param themeManager Theme manager instance
     * @param dialogManager Dialog manager instance
     * @param taskbarManager Taskbar manager instance
     * @return true if successful
     */
    bool initialize(WindowsThemeManager* themeManager,
                   WindowsDialogManager* dialogManager,
                   WindowsTaskbarManager* taskbarManager);

    /**
     * @brief Render the customization panel UI
     */
    void render();

    /**
     * @brief Show/hide the customization panel
     */
    void setVisible(bool visible) { m_visible = visible; }

    /**
     * @brief Check if panel is visible
     */
    bool isVisible() const { return m_visible; }

    /**
     * @brief Toggle panel visibility
     */
    void toggle() { m_visible = !m_visible; }

private:
    void renderThemeSection();
    void renderDialogSection();
    void renderTaskbarSection();
    void renderCustomThemeEditor();

private:
    bool m_visible;
    WindowsThemeManager* m_themeManager;
    WindowsDialogManager* m_dialogManager;
    WindowsTaskbarManager* m_taskbarManager;

    // UI state
    int m_selectedThemeIndex;
    bool m_showCustomThemeEditor;
    
    // Custom theme colors (RGB, 0-255)
    float m_customBackground[3];
    float m_customAccent[3];
    float m_customText[3];
    
    // Taskbar demo
    int m_progressValue;
    int m_progressMax;
};

} // namespace fresh

#endif // _WIN32
