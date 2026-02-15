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
#include <functional>

namespace fresh
{

/**
 * @brief Win32 ListView wrapper for list/grid data display
 * 
 * Used for content browser, asset lists, etc.
 * Provides native Windows ListView with multiple view modes, icons, and selection.
 */
class Win32ListView
{
public:
    enum class ViewMode {
        Icon,       // Large icons
        SmallIcon,  // Small icons
        List,       // List with small icons
        Details     // Details with columns
    };

    using SelectionCallback = std::function<void(int index, void* userData)>;
    using ActivationCallback = std::function<void(int index, void* userData)>;

    Win32ListView();
    ~Win32ListView();

    // Prevent copying
    Win32ListView(const Win32ListView&) = delete;
    Win32ListView& operator=(const Win32ListView&) = delete;

    /**
     * @brief Create list view control
     * @param parent Parent window handle
     * @param x X position
     * @param y Y position
     * @param width Control width
     * @param height Control height
     * @return true if successful
     */
    bool create(HWND parent, int x, int y, int width, int height);

    /**
     * @brief Add column (for Details view)
     * @param text Column header text
     * @param width Column width
     * @return Column index
     */
    int addColumn(const std::string& text, int width);

    /**
     * @brief Add item
     * @param text Item text
     * @param icon Icon index (optional)
     * @param userData User data pointer
     * @return Item index
     */
    int addItem(const std::string& text, int icon = -1, void* userData = nullptr);

    /**
     * @brief Set item text for column
     * @param itemIndex Item index
     * @param columnIndex Column index
     * @param text Text to set
     */
    void setItemText(int itemIndex, int columnIndex, const std::string& text);

    /**
     * @brief Remove item
     * @param index Item index
     */
    void removeItem(int index);

    /**
     * @brief Clear all items
     */
    void clear();

    /**
     * @brief Get selected item index
     * @return Selected index or -1
     */
    int getSelectedItem() const;

    /**
     * @brief Get all selected items
     * @return Vector of selected indices
     */
    std::vector<int> getSelectedItems() const;

    /**
     * @brief Set selected item
     * @param index Item index
     */
    void setSelectedItem(int index);

    /**
     * @brief Get user data for item
     * @param index Item index
     * @return User data pointer
     */
    void* getItemData(int index) const;

    /**
     * @brief Set user data for item
     * @param index Item index
     * @param userData User data pointer
     */
    void setItemData(int index, void* userData);

    /**
     * @brief Get item text
     * @param index Item index
     * @param column Column index (default 0)
     * @return Item text
     */
    std::string getItemText(int index, int column = 0) const;

    /**
     * @brief Set view mode
     * @param mode View mode to set
     */
    void setViewMode(ViewMode mode);

    /**
     * @brief Get item count
     * @return Number of items
     */
    int getItemCount() const;

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

    /**
     * @brief Set image list for icons
     * @param imageList Image list handle
     * @param large true for large icons, false for small
     */
    void setImageList(HIMAGELIST imageList, bool large);

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
