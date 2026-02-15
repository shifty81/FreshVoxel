#pragma once

#ifdef _WIN32

// Prevent Windows.h from defining min/max macros
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>
#include <commctrl.h>
#include <string>
#include <map>
#include <functional>

namespace fresh
{

/**
 * @brief Win32 TreeView wrapper for hierarchical data display
 * 
 * Used for scene hierarchy, file trees, etc.
 * Provides native Windows TreeView with expand/collapse, selection, and drag-drop.
 */
class Win32TreeView
{
public:
    using SelectionCallback = std::function<void(void* userData)>;
    using ActivationCallback = std::function<void(void* userData)>;

    Win32TreeView();
    ~Win32TreeView();

    // Prevent copying
    Win32TreeView(const Win32TreeView&) = delete;
    Win32TreeView& operator=(const Win32TreeView&) = delete;

    /**
     * @brief Create tree view control
     * @param parent Parent window handle
     * @param x X position
     * @param y Y position
     * @param width Control width
     * @param height Control height
     * @return true if successful
     */
    bool create(HWND parent, int x, int y, int width, int height);

    /**
     * @brief Add root item
     * @param text Item text
     * @param userData User data pointer (e.g., entity pointer)
     * @return Item handle
     */
    HTREEITEM addRootItem(const std::string& text, void* userData = nullptr);

    /**
     * @brief Add child item
     * @param parent Parent item handle
     * @param text Item text
     * @param userData User data pointer
     * @return Item handle
     */
    HTREEITEM addChildItem(HTREEITEM parent, const std::string& text, void* userData = nullptr);

    /**
     * @brief Remove item
     * @param item Item handle
     */
    void removeItem(HTREEITEM item);

    /**
     * @brief Clear all items
     */
    void clear();

    /**
     * @brief Get selected item
     * @return Selected item handle or nullptr
     */
    HTREEITEM getSelectedItem() const;

    /**
     * @brief Set selected item
     * @param item Item handle
     */
    void setSelectedItem(HTREEITEM item);

    /**
     * @brief Get user data for item
     * @param item Item handle
     * @return User data pointer
     */
    void* getItemData(HTREEITEM item) const;

    /**
     * @brief Set user data for item
     * @param item Item handle
     * @param userData User data pointer
     */
    void setItemData(HTREEITEM item, void* userData);

    /**
     * @brief Get item text
     * @param item Item handle
     * @return Item text
     */
    std::string getItemText(HTREEITEM item) const;

    /**
     * @brief Set item text
     * @param item Item handle
     * @param text New text
     */
    void setItemText(HTREEITEM item, const std::string& text);

    /**
     * @brief Expand item
     * @param item Item handle
     */
    void expandItem(HTREEITEM item);

    /**
     * @brief Collapse item
     * @param item Item handle
     */
    void collapseItem(HTREEITEM item);

    /**
     * @brief Set selection callback
     * @param callback Function to call when selection changes
     */
    void setSelectionCallback(SelectionCallback callback) {
        m_selectionCallback = callback;
    }

    /**
     * @brief Set activation callback (double-click or Enter)
     * @param callback Function to call when item is activated
     */
    void setActivationCallback(ActivationCallback callback) {
        m_activationCallback = callback;
    }

    /**
     * @brief Handle notification messages
     * @param nmhdr Notification header
     * @return true if handled
     */
    bool handleNotify(NMHDR* nmhdr);

    /**
     * @brief Get control window handle
     */
    HWND getHandle() const { return m_hwnd; }

    /**
     * @brief Set position
     */
    void setPosition(int x, int y);

    /**
     * @brief Set size
     */
    void setSize(int width, int height);

private:
    HWND m_hwnd;
    HWND m_parent;
    SelectionCallback m_selectionCallback;
    ActivationCallback m_activationCallback;

    std::wstring toWideString(const std::string& str);
    std::string toUtf8String(const std::wstring& wstr) const;
};

} // namespace fresh

#endif // _WIN32
