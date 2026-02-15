#ifdef _WIN32

#include "ui/native/Win32MenuBar.h"
#include "core/Logger.h"
#include <codecvt>
#include <locale>

namespace fresh
{

Win32MenuBar::Win32MenuBar()
    : m_hwnd(nullptr)
    , m_menuBar(nullptr)
    , m_nextId(1000)
{
}

Win32MenuBar::~Win32MenuBar()
{
    if (m_menuBar) {
        DestroyMenu(m_menuBar);
    }
}

std::wstring Win32MenuBar::toWideString(const std::string& str)
{
    if (str.empty()) return std::wstring();
    
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), nullptr, 0);
    std::wstring wstr(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), &wstr[0], size_needed);
    return wstr;
}

bool Win32MenuBar::create(HWND hwnd)
{
    if (m_menuBar) {
        LOG_WARNING_C("Menu bar already created", "Win32MenuBar");
        return false;
    }

    m_hwnd = hwnd;
    m_menuBar = CreateMenu();

    if (!m_menuBar) {
        LOG_ERROR_C("Failed to create menu bar", "Win32MenuBar");
        return false;
    }

    SetMenu(hwnd, m_menuBar);
    return true;
}

bool Win32MenuBar::initialize(void* world, void* worldEditor)
{
    // For standalone initialization without window handle
    // This is a stub that returns true to indicate the menu bar is ready to be created later
    // The world and worldEditor pointers are currently unused
    (void)world;      // Suppress unused parameter warning
    (void)worldEditor; // Suppress unused parameter warning
    LOG_INFO_C("Menu bar initialized (stub)", "Win32MenuBar");
    return true;
}

int Win32MenuBar::addMenu(const std::string& name)
{
    if (!m_menuBar) {
        LOG_ERROR_C("Menu bar not created", "Win32MenuBar");
        return -1;
    }

    HMENU menu = CreatePopupMenu();
    if (!menu) {
        LOG_ERROR_C("Failed to create menu: " + name, "Win32MenuBar");
        return -1;
    }

    int menuId = m_nextId++;
    m_menus[menuId] = menu;

    std::wstring wname = toWideString(name);
    AppendMenuW(m_menuBar, MF_POPUP, (UINT_PTR)menu, wname.c_str());
    DrawMenuBar(m_hwnd);

    return menuId;
}

int Win32MenuBar::addMenuItem(int menuId, const std::string& name, MenuCallback callback)
{
    HMENU menu = getMenu(menuId);
    if (!menu) {
        LOG_ERROR_C("Invalid menu ID", "Win32MenuBar");
        return -1;
    }

    int itemId = m_nextId++;
    
    MenuItem item;
    item.id = itemId;
    item.name = name;
    item.callback = callback;
    item.submenu = nullptr;
    m_items[itemId] = item;

    std::wstring wname = toWideString(name);
    AppendMenuW(menu, MF_STRING, itemId, wname.c_str());
    DrawMenuBar(m_hwnd);

    return itemId;
}

void Win32MenuBar::addSeparator(int menuId)
{
    HMENU menu = getMenu(menuId);
    if (!menu) {
        LOG_ERROR_C("Invalid menu ID", "Win32MenuBar");
        return;
    }

    AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    DrawMenuBar(m_hwnd);
}

int Win32MenuBar::addSubmenu(int menuId, const std::string& name)
{
    HMENU parentMenu = getMenu(menuId);
    if (!parentMenu) {
        LOG_ERROR_C("Invalid menu ID", "Win32MenuBar");
        return -1;
    }

    HMENU submenu = CreatePopupMenu();
    if (!submenu) {
        LOG_ERROR_C("Failed to create submenu: " + name, "Win32MenuBar");
        return -1;
    }

    int submenuId = m_nextId++;
    m_menus[submenuId] = submenu;

    std::wstring wname = toWideString(name);
    AppendMenuW(parentMenu, MF_POPUP, (UINT_PTR)submenu, wname.c_str());
    DrawMenuBar(m_hwnd);

    return submenuId;
}

void Win32MenuBar::setItemEnabled(int itemId, bool enabled)
{
    if (m_items.find(itemId) == m_items.end()) {
        return;
    }

    // Find which menu contains this item
    for (auto& menuPair : m_menus) {
        MENUITEMINFO mii = {};
        mii.cbSize = sizeof(MENUITEMINFO);
        mii.fMask = MIIM_STATE;

        if (GetMenuItemInfo(menuPair.second, itemId, FALSE, &mii)) {
            EnableMenuItem(menuPair.second, itemId, MF_BYCOMMAND | (enabled ? MF_ENABLED : MF_GRAYED));
            DrawMenuBar(m_hwnd);
            break;
        }
    }
}

void Win32MenuBar::setItemChecked(int itemId, bool checked)
{
    if (m_items.find(itemId) == m_items.end()) {
        return;
    }

    // Find which menu contains this item
    for (auto& menuPair : m_menus) {
        MENUITEMINFO mii = {};
        mii.cbSize = sizeof(MENUITEMINFO);
        mii.fMask = MIIM_STATE;

        if (GetMenuItemInfo(menuPair.second, itemId, FALSE, &mii)) {
            CheckMenuItem(menuPair.second, itemId, MF_BYCOMMAND | (checked ? MF_CHECKED : MF_UNCHECKED));
            DrawMenuBar(m_hwnd);
            break;
        }
    }
}

bool Win32MenuBar::handleCommand(int commandId)
{
    auto it = m_items.find(commandId);
    if (it != m_items.end() && it->second.callback) {
        it->second.callback();
        return true;
    }
    return false;
}

HMENU Win32MenuBar::getMenu(int menuId)
{
    auto it = m_menus.find(menuId);
    return (it != m_menus.end()) ? it->second : nullptr;
}

} // namespace fresh

#endif // _WIN32
