#pragma once

#ifdef _WIN32

// Prevent Windows.h from defining min/max macros
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>
#include <string>
#include <vector>
#include <map>
#include <functional>

namespace fresh
{

/**
 * @brief Win32 native menu bar implementation
 * 
 * Provides a native Windows menu bar with menus and menu items.
 * Supports nested submenus and command callbacks.
 */
class Win32MenuBar
{
public:
    using MenuCallback = std::function<void()>;

    Win32MenuBar();
    ~Win32MenuBar();

    // Prevent copying
    Win32MenuBar(const Win32MenuBar&) = delete;
    Win32MenuBar& operator=(const Win32MenuBar&) = delete;

    /**
     * @brief Create menu bar and attach to window
     * @param hwnd Window handle
     * @return true if successful
     */
    bool create(HWND hwnd);

    /**
     * @brief Initialize menu bar (simplified interface)
     * @param world VoxelWorld pointer (for future use)
     * @param worldEditor WorldEditor pointer (for future use)
     * @return true if successful
     */
    bool initialize(void* world, void* worldEditor);

    /**
     * @brief Add a top-level menu
     * @param name Menu name (e.g., "File", "Edit")
     * @return Menu ID
     */
    int addMenu(const std::string& name);

    /**
     * @brief Add menu item to a menu
     * @param menuId Parent menu ID
     * @param name Item name
     * @param callback Function to call when clicked
     * @return Item ID
     */
    int addMenuItem(int menuId, const std::string& name, MenuCallback callback);

    /**
     * @brief Add menu separator
     * @param menuId Parent menu ID
     */
    void addSeparator(int menuId);

    /**
     * @brief Add submenu
     * @param menuId Parent menu ID
     * @param name Submenu name
     * @return Submenu ID
     */
    int addSubmenu(int menuId, const std::string& name);

    /**
     * @brief Enable or disable menu item
     * @param itemId Item ID
     * @param enabled true to enable, false to disable
     */
    void setItemEnabled(int itemId, bool enabled);

    /**
     * @brief Set menu item checked state
     * @param itemId Item ID
     * @param checked true to check, false to uncheck
     */
    void setItemChecked(int itemId, bool checked);

    /**
     * @brief Handle menu command
     * @param commandId Command ID from WM_COMMAND
     * @return true if command was handled
     */
    bool handleCommand(int commandId);

    /**
     * @brief Get menu bar handle
     */
    HMENU getHandle() const { return m_menuBar; }

    /**
     * @brief Set panel visibility flags (for integration with EditorManager)
     */
    void setSceneHierarchyVisible(bool* visible) { m_sceneHierarchyVisible = visible; }
    void setInspectorVisible(bool* visible) { m_inspectorVisible = visible; }
    void setContentBrowserVisible(bool* visible) { m_contentBrowserVisible = visible; }
    void setConsoleVisible(bool* visible) { m_consoleVisible = visible; }
    void setToolPaletteVisible(bool* visible) { m_toolPaletteVisible = visible; }

    /**
     * @brief Set callbacks for various menu actions
     */
    void setSettingsCallback(MenuCallback callback) { m_settingsCallback = callback; }
    void setImportAssetsCallback(MenuCallback callback) { m_importAssetsCallback = callback; }
    void setSelectAllCallback(MenuCallback callback) { m_selectAllCallback = callback; }
    void setDeselectAllCallback(MenuCallback callback) { m_deselectAllCallback = callback; }
    void setUndoCallback(MenuCallback callback) { m_undoCallback = callback; }
    void setRedoCallback(MenuCallback callback) { m_redoCallback = callback; }

private:
    struct MenuItem {
        int id;
        std::string name;
        MenuCallback callback;
        HMENU submenu;
    };

    HWND m_hwnd;
    HMENU m_menuBar;
    std::map<int, HMENU> m_menus;
    std::map<int, MenuItem> m_items;
    int m_nextId;

    // Panel visibility flags
    bool* m_sceneHierarchyVisible = nullptr;
    bool* m_inspectorVisible = nullptr;
    bool* m_contentBrowserVisible = nullptr;
    bool* m_consoleVisible = nullptr;
    bool* m_toolPaletteVisible = nullptr;

    // Menu action callbacks
    MenuCallback m_settingsCallback = nullptr;
    MenuCallback m_importAssetsCallback = nullptr;
    MenuCallback m_selectAllCallback = nullptr;
    MenuCallback m_deselectAllCallback = nullptr;
    MenuCallback m_undoCallback = nullptr;
    MenuCallback m_redoCallback = nullptr;

    HMENU getMenu(int menuId);
    std::wstring toWideString(const std::string& str);
};

} // namespace fresh

#endif // _WIN32
