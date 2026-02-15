#ifdef _WIN32

#include "ui/WindowsThemeManager.h"
#include "core/Logger.h"

#include <windows.h>
#include <dwmapi.h>

// Link with DWM API for theme detection
#pragma comment(lib, "dwmapi.lib")

namespace fresh
{

// Registry path for Windows theme settings
constexpr const wchar_t* THEME_REGISTRY_PATH = L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize";
constexpr const wchar_t* ACCENT_COLOR_REGISTRY_PATH = L"Software\\Microsoft\\Windows\\DWM";

WindowsThemeManager::WindowsThemeManager()
    : m_initialized(false),
      m_currentTheme(WindowsTheme::Auto),
      m_windowsDarkModeEnabled(false),
      m_windowsAccentColor(0xFF0078D4) // Default Windows blue
{
    // Initialize default dark theme colors
    m_themeColors = {
        0xFF1E1E1E, // background
        0xFF252526, // foreground
        0xFF0078D4, // accent (Windows blue)
        0xFF3F3F46, // border
        0xFFE0E0E0, // text
        0xFF808080, // textDisabled
        0xFF264F78, // highlight
        0xFF000000  // shadow
    };

    // Initialize custom theme with same defaults
    m_customThemeColors = m_themeColors;
}

WindowsThemeManager::~WindowsThemeManager()
{
    shutdown();
}

bool WindowsThemeManager::initialize()
{
    if (m_initialized) {
        LOG_WARNING_C("WindowsThemeManager already initialized", "WindowsThemeManager");
        return true;
    }

    LOG_INFO_C("Initializing Windows Theme Manager", "WindowsThemeManager");

    // Read Windows theme settings
    m_windowsDarkModeEnabled = readWindowsThemeSetting();
    m_windowsAccentColor = readWindowsAccentColor();

    // Apply auto theme based on Windows settings
    if (m_currentTheme == WindowsTheme::Auto) {
        setTheme(WindowsTheme::Auto);
    }

    m_initialized = true;
    LOG_INFO_C("Windows Theme Manager initialized successfully", "WindowsThemeManager");
    return true;
}

void WindowsThemeManager::shutdown()
{
    if (!m_initialized) {
        return;
    }

    m_initialized = false;
    LOG_INFO_C("Windows Theme Manager shutdown", "WindowsThemeManager");
}

void WindowsThemeManager::setTheme(WindowsTheme theme)
{
    m_currentTheme = theme;

    // Load Windows settings if needed
    if (theme == WindowsTheme::Auto) {
        loadThemeFromWindows();
    }

    // Apply the theme
    switch (m_currentTheme) {
    case WindowsTheme::Auto:
        // Already applied via loadThemeFromWindows
        break;
    case WindowsTheme::Light:
        applyLightTheme();
        break;
    case WindowsTheme::Dark:
        applyDarkTheme();
        break;
    case WindowsTheme::HighContrast:
        applyHighContrastTheme();
        break;
    case WindowsTheme::Classic:
        applyClassicTheme();
        break;
    case WindowsTheme::Custom:
        applyCustomTheme();
        break;
    }

    // Note: ImGui has been removed - themes now apply only to native Win32 UI

    LOG_INFO_C("Theme changed to: " + std::to_string(static_cast<int>(theme)), "WindowsThemeManager");
}

bool WindowsThemeManager::isWindowsDarkMode() const
{
    return m_windowsDarkModeEnabled;
}

uint32_t WindowsThemeManager::getWindowsAccentColor() const
{
    return m_windowsAccentColor;
}

void WindowsThemeManager::setCustomTheme(const ThemeColors& colors)
{
    m_customThemeColors = colors;
    if (m_currentTheme == WindowsTheme::Custom) {
        m_themeColors = colors;
        // ImGui removed - no need to apply
    }
}

std::vector<std::string> WindowsThemeManager::getAvailableThemes() const
{
    return {
        "Auto (Windows Setting)",
        "Light",
        "Dark",
        "High Contrast",
        "Classic",
        "Custom"
    };
}

void WindowsThemeManager::loadThemeFromWindows()
{
    // Refresh Windows settings
    m_windowsDarkModeEnabled = readWindowsThemeSetting();
    m_windowsAccentColor = readWindowsAccentColor();

    // Apply theme based on Windows setting
    if (m_windowsDarkModeEnabled) {
        applyDarkTheme();
    } else {
        applyLightTheme();
    }

    // Update accent color
    m_themeColors.accent = m_windowsAccentColor;
}

void WindowsThemeManager::applyLightTheme()
{
    m_themeColors = {
        0xFFF0F0F0, // background - Light gray
        0xFFFFFFFF, // foreground - White
        m_windowsAccentColor, // accent - Windows accent
        0xFFD0D0D0, // border - Medium gray
        0xFF000000, // text - Black
        0xFF808080, // textDisabled - Gray
        0xFFE5F3FF, // highlight - Light blue
        0xFF404040  // shadow - Dark gray
    };
}

void WindowsThemeManager::applyDarkTheme()
{
    m_themeColors = {
        0xFF1E1E1E, // background - Very dark gray
        0xFF252526, // foreground - Dark gray
        m_windowsAccentColor, // accent - Windows accent
        0xFF3F3F46, // border - Medium dark gray
        0xFFE0E0E0, // text - Light gray
        0xFF808080, // textDisabled - Gray
        0xFF264F78, // highlight - Dark blue
        0xFF000000  // shadow - Black
    };
}

void WindowsThemeManager::applyHighContrastTheme()
{
    m_themeColors = {
        0xFF000000, // background - Black
        0xFF000000, // foreground - Black
        0xFF00FFFF, // accent - Cyan
        0xFFFFFFFF, // border - White
        0xFFFFFFFF, // text - White
        0xFF808080, // textDisabled - Gray
        0xFF0000FF, // highlight - Blue
        0xFFFFFFFF  // shadow - White
    };
}

void WindowsThemeManager::applyClassicTheme()
{
    m_themeColors = {
        0xFFC0C0C0, // background - Classic gray
        0xFFD4D0C8, // foreground - Classic window color
        0xFF000080, // accent - Classic blue
        0xFF808080, // border - Dark gray
        0xFF000000, // text - Black
        0xFF808080, // textDisabled - Gray
        0xFF000080, // highlight - Blue
        0xFF000000  // shadow - Black
    };
}

void WindowsThemeManager::applyCustomTheme()
{
    m_themeColors = m_customThemeColors;
}

bool WindowsThemeManager::readWindowsThemeSetting()
{
    HKEY hKey;
    DWORD value = 0;
    DWORD dataSize = sizeof(DWORD);

    // Try to read "AppsUseLightTheme" from registry
    LONG result = RegOpenKeyExW(HKEY_CURRENT_USER, THEME_REGISTRY_PATH, 0, KEY_READ, &hKey);
    if (result == ERROR_SUCCESS) {
        result = RegQueryValueExW(hKey, L"AppsUseLightTheme", nullptr, nullptr, 
                                  reinterpret_cast<LPBYTE>(&value), &dataSize);
        RegCloseKey(hKey);
        
        if (result == ERROR_SUCCESS) {
            // value == 0 means dark mode, value == 1 means light mode
            return (value == 0);
        }
    }

    // Fallback: Try DWM API
    BOOL darkMode = FALSE;
    HRESULT hr = DwmGetWindowAttribute(GetDesktopWindow(), DWMWA_USE_IMMERSIVE_DARK_MODE,
                                       &darkMode, sizeof(darkMode));
    if (SUCCEEDED(hr)) {
        return darkMode != FALSE;
    }

    // Default to light mode if we can't detect
    LOG_WARNING_C("Could not detect Windows theme, defaulting to light mode", "WindowsThemeManager");
    return false;
}

uint32_t WindowsThemeManager::readWindowsAccentColor()
{
    HKEY hKey;
    DWORD color = 0xFF0078D4; // Default Windows blue
    DWORD dataSize = sizeof(DWORD);

    // Try to read accent color from registry
    LONG result = RegOpenKeyExW(HKEY_CURRENT_USER, ACCENT_COLOR_REGISTRY_PATH, 0, KEY_READ, &hKey);
    if (result == ERROR_SUCCESS) {
        result = RegQueryValueExW(hKey, L"AccentColor", nullptr, nullptr,
                                  reinterpret_cast<LPBYTE>(&color), &dataSize);
        RegCloseKey(hKey);
        
        if (result == ERROR_SUCCESS) {
            // Windows stores color as ABGR, convert to RGBA
            uint32_t a = (color >> 24) & 0xFF;
            uint32_t b = (color >> 16) & 0xFF;
            uint32_t g = (color >> 8) & 0xFF;
            uint32_t r = color & 0xFF;
            return (r << 24) | (g << 16) | (b << 8) | a;
        }
    }

    LOG_INFO_C("Using default Windows accent color", "WindowsThemeManager");
    return color;
}

} // namespace fresh

#endif // _WIN32
