#pragma once

#ifdef _WIN32

#include <cstdint>
#include <string>
#include <vector>

namespace fresh
{

/**
 * @brief Theme preset for Windows styling
 */
enum class WindowsTheme
{
    Auto,          // Automatically detect Windows theme
    Light,         // Windows Light theme
    Dark,          // Windows Dark theme
    HighContrast,  // Windows High Contrast theme
    Classic,       // Classic Windows theme
    Custom         // User-defined custom theme
};

/**
 * @brief Color scheme for UI theming
 */
struct ThemeColors
{
    uint32_t background;
    uint32_t foreground;
    uint32_t accent;
    uint32_t border;
    uint32_t text;
    uint32_t textDisabled;
    uint32_t highlight;
    uint32_t shadow;
};

/**
 * @brief Manages Windows-native theming and appearance
 * 
 * Integrates with Windows 10/11 theme system to provide:
 * - Dark mode detection and support
 * - Windows accent color integration
 * - Custom theme presets
 * - Native Win32 UI styling
 */
class WindowsThemeManager
{
public:
    WindowsThemeManager();
    ~WindowsThemeManager();

    /**
     * @brief Initialize the theme manager
     * @return true if initialization was successful
     */
    bool initialize();

    /**
     * @brief Shutdown and cleanup
     */
    void shutdown();

    /**
     * @brief Set the active theme
     * @param theme Theme preset to apply
     */
    void setTheme(WindowsTheme theme);

    /**
     * @brief Get the current active theme
     * @return Current theme
     */
    WindowsTheme getCurrentTheme() const { return m_currentTheme; }

    /**
     * @brief Check if Windows is using dark mode
     * @return true if dark mode is enabled
     */
    bool isWindowsDarkMode() const;

    /**
     * @brief Get the Windows accent color
     * @return Accent color as RGBA
     */
    uint32_t getWindowsAccentColor() const;

    /**
     * @brief Get theme colors for current theme
     * @return Theme color scheme
     */
    const ThemeColors& getThemeColors() const { return m_themeColors; }

    /**
     * @brief Set custom theme colors
     * @param colors Custom color scheme
     */
    void setCustomTheme(const ThemeColors& colors);

    /**
     * @brief Get list of available theme presets
     * @return Vector of theme names
     */
    std::vector<std::string> getAvailableThemes() const;

    /**
     * @brief Check if theme manager is initialized
     * @return true if initialized
     */
    bool isInitialized() const { return m_initialized; }

private:
    void loadThemeFromWindows();
    void applyLightTheme();
    void applyDarkTheme();
    void applyHighContrastTheme();
    void applyClassicTheme();
    void applyCustomTheme();

    bool readWindowsThemeSetting();
    uint32_t readWindowsAccentColor();

private:
    bool m_initialized;
    WindowsTheme m_currentTheme;
    ThemeColors m_themeColors;
    ThemeColors m_customThemeColors;
    bool m_windowsDarkModeEnabled;
    uint32_t m_windowsAccentColor;
};

} // namespace fresh

#endif // _WIN32
