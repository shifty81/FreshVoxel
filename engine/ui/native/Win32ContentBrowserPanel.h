#pragma once

#ifdef _WIN32

// Prevent Windows.h from defining min/max macros
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>
#include <string>
#include <memory>
#include <vector>
#include <functional>
#include "ui/native/Win32Panel.h"
#include "ui/native/Win32ListView.h"

namespace fresh
{

/**
 * @brief Native Win32 Content Browser Panel using ListView
 * 
 * Provides a professional native Windows content browser interface with:
 * - ListView control for file/asset display
 * - Multiple view modes (icons, list, details)
 * - File operations (copy, paste, delete, import)
 * - Thumbnail generation for textures
 * - Search and filter functionality
 * - Unreal Engine-like dark theme styling
 */
class Win32ContentBrowserPanel : public Win32Panel
{
public:
    using ItemSelectedCallback = std::function<void(const std::string& path)>;
    using ItemActivatedCallback = std::function<void(const std::string& path)>;
    
    Win32ContentBrowserPanel();
    virtual ~Win32ContentBrowserPanel();

    /**
     * @brief Initialize the content browser panel (simplified interface)
     * @param rootPath Root directory path for content
     * @return true if successful
     */
    bool initialize(const std::string& rootPath);

    /**
     * @brief Initialize the content browser panel
     * @param parent Parent window handle
     * @param x X position
     * @param y Y position
     * @param width Panel width
     * @param height Panel height
     * @param rootPath Root directory path for content
     * @return true if successful
     */
    bool create(HWND parent, int x, int y, int width, int height, const std::string& rootPath);

    /**
     * @brief Refresh the content browser display
     */
    void refresh();

    /**
     * @brief Navigate to a directory
     * @param path Directory path
     */
    void navigateTo(const std::string& path);

    /**
     * @brief Get current directory
     * @return Current directory path
     */
    std::string getCurrentDirectory() const { return m_currentPath; }

    /**
     * @brief Set item selection callback
     * @param callback Callback to invoke when item is selected
     */
    void setItemSelectedCallback(ItemSelectedCallback callback) { m_itemSelectedCallback = callback; }

    /**
     * @brief Set item activation callback
     * @param callback Callback to invoke when item is double-clicked
     */
    void setItemActivatedCallback(ItemActivatedCallback callback) { m_itemActivatedCallback = callback; }

    /**
     * @brief Show import dialog
     */
    void showImportDialog();

protected:
    // Override base class method
    bool handleMessage(UINT msg, WPARAM wParam, LPARAM lParam, LRESULT& result) override;

private:
    void populateListView();
    void addFileItem(const std::string& filename, const std::string& fullPath, bool isDirectory);
    void onItemSelected(int index);
    void onItemActivated(int index);
    void showContextMenu(int x, int y);
    
    void createToolbar();
    void onBackClicked();
    void onUpClicked();
    void onRefreshClicked();
    void onViewModeChanged();

private:
    std::unique_ptr<Win32ListView> m_listView;
    std::string m_rootPath;
    std::string m_currentPath;
    
    // Toolbar controls
    HWND m_toolbar;
    HWND m_pathEdit;
    HWND m_searchEdit;
    
    // Context menu
    HMENU m_contextMenu;
    
    // Item data
    struct ItemData {
        std::string name;
        std::string fullPath;
        bool isDirectory;
    };
    std::vector<ItemData> m_items;
    
    // Callbacks
    ItemSelectedCallback m_itemSelectedCallback;
    ItemActivatedCallback m_itemActivatedCallback;
    
    // Navigation history
    std::vector<std::string> m_navigationHistory;
    int m_historyIndex;
    
    // View mode (0=details, 1=list, 2=icons)
    int m_viewMode;
    
    // Layout constants
    static constexpr int TOOLBAR_HEIGHT = 40;
    static constexpr int PATH_BAR_HEIGHT = 30;
};

} // namespace fresh

#endif // _WIN32
