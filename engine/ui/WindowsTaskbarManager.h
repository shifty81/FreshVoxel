#pragma once

#ifdef _WIN32

#include <string>

namespace fresh
{

/**
 * @brief Taskbar progress state
 */
enum class TaskbarProgressState
{
    NoProgress,    // No progress indicator
    Indeterminate, // Indeterminate progress (pulsing)
    Normal,        // Normal progress (green)
    Error,         // Error state (red)
    Paused         // Paused state (yellow)
};

/**
 * @brief Manages Windows taskbar integration
 * 
 * Provides Windows taskbar features:
 * - Progress indicator
 * - Thumbnail toolbar buttons
 * - Overlay icons
 * - Flash/attention
 */
class WindowsTaskbarManager
{
public:
    WindowsTaskbarManager();
    ~WindowsTaskbarManager();

    /**
     * @brief Initialize the taskbar manager
     * @param windowHandle Native window handle (HWND)
     * @return true if initialization was successful
     */
    bool initialize(void* windowHandle);

    /**
     * @brief Shutdown and cleanup
     */
    void shutdown();

    /**
     * @brief Set taskbar progress state
     * @param state Progress state
     */
    void setProgressState(TaskbarProgressState state);

    /**
     * @brief Set taskbar progress value
     * @param completed Completed units
     * @param total Total units
     */
    void setProgressValue(uint64_t completed, uint64_t total);

    /**
     * @brief Flash taskbar button to get user attention
     * @param flashUntilForeground If true, flashes until window gets focus
     */
    void flashTaskbar(bool flashUntilForeground = false);

    /**
     * @brief Set overlay icon on taskbar button
     * @param iconPath Path to icon file
     * @param description Icon description (tooltip)
     */
    void setOverlayIcon(const std::string& iconPath, const std::string& description);

    /**
     * @brief Clear overlay icon
     */
    void clearOverlayIcon();

    /**
     * @brief Check if taskbar manager is initialized
     * @return true if initialized
     */
    bool isInitialized() const { return m_initialized; }

private:
    bool m_initialized;
    void* m_windowHandle; // HWND
    void* m_taskbarList;  // ITaskbarList3*
};

} // namespace fresh

#endif // _WIN32
