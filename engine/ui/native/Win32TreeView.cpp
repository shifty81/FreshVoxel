#ifdef _WIN32

#include "ui/native/Win32TreeView.h"
#include "core/Logger.h"

#pragma comment(lib, "comctl32.lib")

namespace fresh
{

Win32TreeView::Win32TreeView()
    : m_hwnd(nullptr)
    , m_parent(nullptr)
{
}

Win32TreeView::~Win32TreeView()
{
    if (m_hwnd) {
        DestroyWindow(m_hwnd);
    }
}

std::wstring Win32TreeView::toWideString(const std::string& str)
{
    if (str.empty()) return std::wstring();
    
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), nullptr, 0);
    std::wstring wstr(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), &wstr[0], size_needed);
    return wstr;
}

std::string Win32TreeView::toUtf8String(const std::wstring& wstr) const
{
    if (wstr.empty()) return std::string();
    
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), nullptr, 0, nullptr, nullptr);
    std::string str(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), &str[0], size_needed, nullptr, nullptr);
    return str;
}

bool Win32TreeView::create(HWND parent, int x, int y, int width, int height)
{
    if (m_hwnd) {
        LOG_WARNING_C("TreeView already created", "Win32TreeView");
        return false;
    }

    // Initialize common controls
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_TREEVIEW_CLASSES;
    InitCommonControlsEx(&icex);

    m_parent = parent;

    // Create tree view control
    m_hwnd = CreateWindowExW(
        WS_EX_CLIENTEDGE,
        WC_TREEVIEWW,
        L"",
        WS_CHILD | WS_VISIBLE | WS_BORDER | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS | TVS_SHOWSELALWAYS,
        x, y, width, height,
        parent,
        nullptr,
        GetModuleHandle(nullptr),
        nullptr
    );

    if (!m_hwnd) {
        LOG_ERROR_C("Failed to create TreeView control", "Win32TreeView");
        return false;
    }

    return true;
}

HTREEITEM Win32TreeView::addRootItem(const std::string& text, void* userData)
{
    if (!m_hwnd) {
        return nullptr;
    }

    std::wstring wideText = toWideString(text);
    TVINSERTSTRUCTW tvins = {};
    tvins.hParent = TVI_ROOT;
    tvins.hInsertAfter = TVI_LAST;
    tvins.item.mask = TVIF_TEXT | TVIF_PARAM;
    tvins.item.pszText = const_cast<LPWSTR>(wideText.c_str());
    tvins.item.lParam = (LPARAM)userData;

    return TreeView_InsertItem(m_hwnd, &tvins);
}

HTREEITEM Win32TreeView::addChildItem(HTREEITEM parent, const std::string& text, void* userData)
{
    if (!m_hwnd || !parent) {
        return nullptr;
    }

    std::wstring wideText = toWideString(text);
    TVINSERTSTRUCTW tvins = {};
    tvins.hParent = parent;
    tvins.hInsertAfter = TVI_LAST;
    tvins.item.mask = TVIF_TEXT | TVIF_PARAM;
    tvins.item.pszText = const_cast<LPWSTR>(wideText.c_str());
    tvins.item.lParam = (LPARAM)userData;

    return TreeView_InsertItem(m_hwnd, &tvins);
}

void Win32TreeView::removeItem(HTREEITEM item)
{
    if (m_hwnd && item) {
        TreeView_DeleteItem(m_hwnd, item);
    }
}

void Win32TreeView::clear()
{
    if (m_hwnd) {
        TreeView_DeleteAllItems(m_hwnd);
    }
}

HTREEITEM Win32TreeView::getSelectedItem() const
{
    if (m_hwnd) {
        return TreeView_GetSelection(m_hwnd);
    }
    return nullptr;
}

void Win32TreeView::setSelectedItem(HTREEITEM item)
{
    if (m_hwnd && item) {
        TreeView_SelectItem(m_hwnd, item);
    }
}

void* Win32TreeView::getItemData(HTREEITEM item) const
{
    if (!m_hwnd || !item) {
        return nullptr;
    }

    TVITEMW tvitem = {};
    tvitem.hItem = item;
    tvitem.mask = TVIF_PARAM;

    if (TreeView_GetItem(m_hwnd, &tvitem)) {
        return (void*)tvitem.lParam;
    }

    return nullptr;
}

void Win32TreeView::setItemData(HTREEITEM item, void* userData)
{
    if (!m_hwnd || !item) {
        return;
    }

    TVITEMW tvitem = {};
    tvitem.hItem = item;
    tvitem.mask = TVIF_PARAM;
    tvitem.lParam = (LPARAM)userData;

    TreeView_SetItem(m_hwnd, &tvitem);
}

std::string Win32TreeView::getItemText(HTREEITEM item) const
{
    if (!m_hwnd || !item) {
        return "";
    }

    wchar_t buffer[256];
    TVITEMW tvitem = {};
    tvitem.hItem = item;
    tvitem.mask = TVIF_TEXT;
    tvitem.pszText = buffer;
    tvitem.cchTextMax = 256;

    if (TreeView_GetItem(m_hwnd, &tvitem)) {
        return toUtf8String(buffer);
    }

    return "";
}

void Win32TreeView::setItemText(HTREEITEM item, const std::string& text)
{
    if (!m_hwnd || !item) {
        return;
    }

    std::wstring wideText = toWideString(text);
    TVITEMW tvitem = {};
    tvitem.hItem = item;
    tvitem.mask = TVIF_TEXT;
    tvitem.pszText = const_cast<LPWSTR>(wideText.c_str());

    TreeView_SetItem(m_hwnd, &tvitem);
}

void Win32TreeView::expandItem(HTREEITEM item)
{
    if (m_hwnd && item) {
        TreeView_Expand(m_hwnd, item, TVE_EXPAND);
    }
}

void Win32TreeView::collapseItem(HTREEITEM item)
{
    if (m_hwnd && item) {
        TreeView_Expand(m_hwnd, item, TVE_COLLAPSE);
    }
}

bool Win32TreeView::handleNotify(NMHDR* nmhdr)
{
    if (!nmhdr || nmhdr->hwndFrom != m_hwnd) {
        return false;
    }

    switch (nmhdr->code) {
        case TVN_SELCHANGED: {
            if (m_selectionCallback) {
                NMTREEVIEW* nmtv = (NMTREEVIEW*)nmhdr;
                void* userData = (void*)nmtv->itemNew.lParam;
                m_selectionCallback(userData);
            }
            return true;
        }

        case NM_DBLCLK: {
            if (m_activationCallback) {
                HTREEITEM selected = getSelectedItem();
                if (selected) {
                    void* userData = getItemData(selected);
                    m_activationCallback(userData);
                }
            }
            return true;
        }
    }

    return false;
}

void Win32TreeView::setPosition(int x, int y)
{
    if (m_hwnd) {
        SetWindowPos(m_hwnd, nullptr, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
    }
}

void Win32TreeView::setSize(int width, int height)
{
    if (m_hwnd) {
        SetWindowPos(m_hwnd, nullptr, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER);
    }
}

} // namespace fresh

#endif // _WIN32
