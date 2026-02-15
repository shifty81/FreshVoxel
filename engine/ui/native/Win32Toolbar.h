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
#include <map>
#include <functional>

namespace fresh
{

/**
 * @brief Win32 native toolbar implementation
 * 
 * Provides a native Windows toolbar with buttons, separators, and dropdown menus.
 */
class Win32Toolbar
{
public:
    using ButtonCallback = std::function<void()>;

    Win32Toolbar();
    ~Win32Toolbar();

    // Prevent copying
    Win32Toolbar(const Win32Toolbar&) = delete;
    Win32Toolbar& operator=(const Win32Toolbar&) = delete;

    /**
     * @brief Create toolbar and attach to window
     * @param parent Parent window handle
     * @return true if successful
     */
    bool create(HWND parent);

    /**
     * @brief Add button to toolbar
     * @param id Button ID
     * @param text Button text/tooltip
     * @param icon Icon handle (optional)
     * @param callback Function to call when clicked
     * @return true if successful
     */
    bool addButton(int id, const std::string& text, HICON icon, ButtonCallback callback);

    /**
     * @brief Add separator to toolbar
     */
    void addSeparator();

    /**
     * @brief Enable or disable button
     * @param id Button ID
     * @param enabled true to enable, false to disable
     */
    void setButtonEnabled(int id, bool enabled);

    /**
     * @brief Set button checked state
     * @param id Button ID
     * @param checked true to check, false to uncheck
     */
    void setButtonChecked(int id, bool checked);

    /**
     * @brief Handle toolbar button command
     * @param commandId Command ID
     * @return true if command was handled
     */
    bool handleCommand(int commandId);

    /**
     * @brief Get toolbar window handle
     */
    HWND getHandle() const { return m_hwnd; }

    /**
     * @brief Auto-size the toolbar
     */
    void autoSize();

    /**
     * @brief Get toolbar height
     * @return Height of toolbar in pixels
     */
    int getHeight() const;

private:
    struct ButtonInfo {
        int id;
        std::string text;
        ButtonCallback callback;
    };

    HWND m_hwnd;
    HWND m_parent;
    HIMAGELIST m_imageList;
    std::map<int, ButtonInfo> m_buttons;
    int m_nextButtonIndex;

    std::wstring toWideString(const std::string& str);
};

} // namespace fresh

#endif // _WIN32
