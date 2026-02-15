#pragma once

#ifdef _WIN32

// Prevent Windows.h from defining min/max macros
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>
#include <commctrl.h>
#include <string>
#include <vector>

namespace fresh
{

/**
 * @brief Native Win32 Status Bar component with Unreal-style dark theme
 * 
 * Provides a professional status bar at the bottom of the editor window with:
 * - Multiple panes for different status information
 * - Dark theme styling matching the rest of the editor
 * - Automatic resizing when window is resized
 * - Commonly used for: cursor position, selection count, memory usage, FPS, etc.
 */
class Win32StatusBar
{
public:
    /**
     * @brief Status pane configuration
     */
    struct PaneConfig {
        int width;           ///< Width in pixels (-1 for auto/stretch)
        std::wstring text;   ///< Initial text for the pane
        int style;           ///< Style flags (SBT_NOBORDERS, SBT_OWNERDRAW, etc.)
    };

    Win32StatusBar();
    ~Win32StatusBar();

    // Prevent copying
    Win32StatusBar(const Win32StatusBar&) = delete;
    Win32StatusBar& operator=(const Win32StatusBar&) = delete;

    /**
     * @brief Create the status bar
     * @param parent Parent window handle
     * @param numPanes Number of status panes
     * @param paneConfigs Array of pane configurations (optional)
     * @return true if successful
     */
    bool create(HWND parent, int numPanes = 3, const PaneConfig* paneConfigs = nullptr);

    /**
     * @brief Destroy the status bar
     */
    void destroy();

    /**
     * @brief Set text for a specific pane
     * @param paneIndex Index of the pane (0-based)
     * @param text Text to display
     */
    void setPaneText(int paneIndex, const std::wstring& text);

    /**
     * @brief Set text for a specific pane (narrow string version)
     * @param paneIndex Index of the pane (0-based)
     * @param text Text to display
     */
    void setPaneText(int paneIndex, const std::string& text);

    /**
     * @brief Update status bar layout when parent window is resized
     * Call this in response to WM_SIZE of the parent window
     */
    void updateLayout();

    /**
     * @brief Get the window handle
     */
    HWND getHandle() const { return m_hwnd; }

    /**
     * @brief Get the height of the status bar
     */
    int getHeight() const;

    /**
     * @brief Show or hide the status bar
     * @param show true to show, false to hide
     */
    void setVisible(bool show);

    /**
     * @brief Check if status bar is visible
     */
    bool isVisible() const;

    /**
     * @brief Set the ready/idle message text (first pane)
     * @param text Status text to display
     */
    void setStatusText(const std::wstring& text);

    /**
     * @brief Set cursor/position information (typically second pane)
     * @param x X coordinate
     * @param y Y coordinate
     * @param z Z coordinate (for 3D editors)
     */
    void setCursorPosition(float x, float y, float z = 0.0f);

    /**
     * @brief Set selection information (typically third pane)
     * @param count Number of selected items
     * @param type Type of selected items (e.g., "entities", "voxels")
     */
    void setSelectionInfo(int count, const std::wstring& type = L"items");

    /**
     * @brief Set FPS display (typically last pane)
     * @param fps Current frames per second
     */
    void setFPS(float fps);

    /**
     * @brief Set memory usage display
     * @param usedMB Memory used in megabytes
     * @param totalMB Total available memory in megabytes
     */
    void setMemoryUsage(float usedMB, float totalMB);

private:
    void applyDarkTheme();
    void calculatePaneWidths();

    HWND m_hwnd;
    HWND m_parent;
    int m_numPanes;
    std::vector<int> m_paneWidths;
    std::vector<std::wstring> m_paneTexts;
};

} // namespace fresh

#endif // _WIN32
