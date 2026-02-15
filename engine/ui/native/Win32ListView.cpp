#ifdef _WIN32

#include "ui/native/Win32ListView.h"
#include "core/Logger.h"

#pragma comment(lib, "comctl32.lib")

namespace fresh
{

Win32ListView::Win32ListView()
    : m_hwnd(nullptr)
    , m_parent(nullptr)
{
}

Win32ListView::~Win32ListView()
{
    if (m_hwnd) {
        DestroyWindow(m_hwnd);
    }
}

std::wstring Win32ListView::toWideString(const std::string& str)
{
    if (str.empty()) return std::wstring();
    
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), nullptr, 0);
    std::wstring wstr(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), &wstr[0], size_needed);
    return wstr;
}

std::string Win32ListView::toUtf8String(const std::wstring& wstr) const
{
    if (wstr.empty()) return std::string();
    
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), nullptr, 0, nullptr, nullptr);
    std::string str(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), &str[0], size_needed, nullptr, nullptr);
    return str;
}

bool Win32ListView::create(HWND parent, int x, int y, int width, int height)
{
    if (m_hwnd) {
        LOG_WARNING_C("ListView already created", "Win32ListView");
        return false;
    }

    // Initialize common controls
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_LISTVIEW_CLASSES;
    InitCommonControlsEx(&icex);

    m_parent = parent;

    // Create list view control
    m_hwnd = CreateWindowExW(
        WS_EX_CLIENTEDGE,
        WC_LISTVIEWW,
        L"",
        WS_CHILD | WS_VISIBLE | WS_BORDER | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SINGLESEL,
        x, y, width, height,
        parent,
        nullptr,
        GetModuleHandle(nullptr),
        nullptr
    );

    if (!m_hwnd) {
        LOG_ERROR_C("Failed to create ListView control", "Win32ListView");
        return false;
    }

    // Enable full row select and grid lines
    ListView_SetExtendedListViewStyle(m_hwnd, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

    return true;
}

int Win32ListView::addColumn(const std::string& text, int width)
{
    if (!m_hwnd) {
        return -1;
    }

    int columnCount = Header_GetItemCount(ListView_GetHeader(m_hwnd));

    std::wstring wideText = toWideString(text);
    LVCOLUMNW column = {};
    column.mask = LVCF_TEXT | LVCF_WIDTH;
    column.cx = width;
    column.pszText = const_cast<LPWSTR>(wideText.c_str());

    return ListView_InsertColumn(m_hwnd, columnCount, &column);
}

int Win32ListView::addItem(const std::string& text, int icon, void* userData)
{
    if (!m_hwnd) {
        return -1;
    }

    std::wstring wideText = toWideString(text);
    LVITEMW item = {};
    item.mask = LVIF_TEXT | LVIF_PARAM;
    item.iItem = ListView_GetItemCount(m_hwnd);
    item.iSubItem = 0;
    item.pszText = const_cast<LPWSTR>(wideText.c_str());
    item.lParam = (LPARAM)userData;

    if (icon >= 0) {
        item.mask |= LVIF_IMAGE;
        item.iImage = icon;
    }

    return ListView_InsertItem(m_hwnd, &item);
}

void Win32ListView::setItemText(int itemIndex, int columnIndex, const std::string& text)
{
    if (!m_hwnd) {
        return;
    }

    std::wstring wideText = toWideString(text);
    LVITEMW item = {};
    item.iSubItem = columnIndex;
    item.pszText = const_cast<LPWSTR>(wideText.c_str());
    SendMessageW(m_hwnd, LVM_SETITEMTEXTW, static_cast<WPARAM>(static_cast<unsigned int>(itemIndex)), (LPARAM)&item);
}

void Win32ListView::removeItem(int index)
{
    if (m_hwnd) {
        ListView_DeleteItem(m_hwnd, index);
    }
}

void Win32ListView::clear()
{
    if (m_hwnd) {
        ListView_DeleteAllItems(m_hwnd);
    }
}

int Win32ListView::getSelectedItem() const
{
    if (m_hwnd) {
        return ListView_GetNextItem(m_hwnd, -1, LVNI_SELECTED);
    }
    return -1;
}

std::vector<int> Win32ListView::getSelectedItems() const
{
    std::vector<int> selected;
    if (!m_hwnd) {
        return selected;
    }

    int index = -1;
    while ((index = ListView_GetNextItem(m_hwnd, index, LVNI_SELECTED)) != -1) {
        selected.push_back(index);
    }

    return selected;
}

void Win32ListView::setSelectedItem(int index)
{
    if (m_hwnd) {
        ListView_SetItemState(m_hwnd, index, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
        ListView_EnsureVisible(m_hwnd, index, FALSE);
    }
}

void* Win32ListView::getItemData(int index) const
{
    if (!m_hwnd) {
        return nullptr;
    }

    LVITEMW item = {};
    item.mask = LVIF_PARAM;
    item.iItem = index;

    if (ListView_GetItem(m_hwnd, &item)) {
        return (void*)item.lParam;
    }

    return nullptr;
}

void Win32ListView::setItemData(int index, void* userData)
{
    if (!m_hwnd) {
        return;
    }

    LVITEMW item = {};
    item.mask = LVIF_PARAM;
    item.iItem = index;
    item.lParam = (LPARAM)userData;

    ListView_SetItem(m_hwnd, &item);
}

std::string Win32ListView::getItemText(int index, int column) const
{
    if (!m_hwnd) {
        return "";
    }

    wchar_t buffer[256];
    LVITEMW item = {};
    item.iSubItem = column;
    item.pszText = buffer;
    item.cchTextMax = 256;
    SendMessageW(m_hwnd, LVM_GETITEMTEXTW, static_cast<WPARAM>(static_cast<unsigned int>(index)), (LPARAM)&item);
    return toUtf8String(buffer);
}

void Win32ListView::setViewMode(ViewMode mode)
{
    if (!m_hwnd) {
        return;
    }

    DWORD style = GetWindowLong(m_hwnd, GWL_STYLE);
    style &= ~LVS_TYPEMASK;

    switch (mode) {
        case ViewMode::Icon:
            style |= LVS_ICON;
            break;
        case ViewMode::SmallIcon:
            style |= LVS_SMALLICON;
            break;
        case ViewMode::List:
            style |= LVS_LIST;
            break;
        case ViewMode::Details:
            style |= LVS_REPORT;
            break;
    }

    SetWindowLong(m_hwnd, GWL_STYLE, style);
}

int Win32ListView::getItemCount() const
{
    if (m_hwnd) {
        return ListView_GetItemCount(m_hwnd);
    }
    return 0;
}

bool Win32ListView::handleNotify(NMHDR* nmhdr)
{
    if (!nmhdr || nmhdr->hwndFrom != m_hwnd) {
        return false;
    }

    switch (nmhdr->code) {
        case LVN_ITEMCHANGED: {
            if (m_selectionCallback) {
                NMLISTVIEW* nmlv = (NMLISTVIEW*)nmhdr;
                if (nmlv->uNewState & LVIS_SELECTED) {
                    void* userData = getItemData(nmlv->iItem);
                    m_selectionCallback(nmlv->iItem, userData);
                }
            }
            return true;
        }

        case NM_DBLCLK: {
            if (m_activationCallback) {
                int selected = getSelectedItem();
                if (selected >= 0) {
                    void* userData = getItemData(selected);
                    m_activationCallback(selected, userData);
                }
            }
            return true;
        }
    }

    return false;
}

void Win32ListView::setPosition(int x, int y)
{
    if (m_hwnd) {
        SetWindowPos(m_hwnd, nullptr, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
    }
}

void Win32ListView::setSize(int width, int height)
{
    if (m_hwnd) {
        SetWindowPos(m_hwnd, nullptr, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER);
    }
}

void Win32ListView::setImageList(HIMAGELIST imageList, bool large)
{
    if (m_hwnd) {
        ListView_SetImageList(m_hwnd, imageList, large ? LVSIL_NORMAL : LVSIL_SMALL);
    }
}

} // namespace fresh

#endif // _WIN32
