#ifdef _WIN32

#include "ui/native/Win32Toolbar.h"
#include "core/Logger.h"

#pragma comment(lib, "comctl32.lib")

namespace fresh
{

Win32Toolbar::Win32Toolbar()
    : m_hwnd(nullptr)
    , m_parent(nullptr)
    , m_imageList(nullptr)
    , m_nextButtonIndex(0)
{
}

Win32Toolbar::~Win32Toolbar()
{
    if (m_imageList) {
        ImageList_Destroy(m_imageList);
    }
    if (m_hwnd) {
        DestroyWindow(m_hwnd);
    }
}

std::wstring Win32Toolbar::toWideString(const std::string& str)
{
    if (str.empty()) return std::wstring();
    
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), nullptr, 0);
    std::wstring wstr(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), &wstr[0], size_needed);
    return wstr;
}

bool Win32Toolbar::create(HWND parent)
{
    if (m_hwnd) {
        LOG_WARNING_C("Toolbar already created", "Win32Toolbar");
        return false;
    }

    // Initialize common controls
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_BAR_CLASSES;
    InitCommonControlsEx(&icex);

    m_parent = parent;

    // Create toolbar
    m_hwnd = CreateWindowEx(
        0,
        TOOLBARCLASSNAME,
        nullptr,
        WS_CHILD | WS_VISIBLE | TBSTYLE_FLAT | TBSTYLE_TOOLTIPS,
        0, 0, 0, 0,
        parent,
        nullptr,
        GetModuleHandle(nullptr),
        nullptr
    );

    if (!m_hwnd) {
        LOG_ERROR_C("Failed to create toolbar", "Win32Toolbar");
        return false;
    }

    // Set button size
    SendMessage(m_hwnd, TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0);

    // Create image list
    m_imageList = ImageList_Create(16, 16, ILC_COLOR32 | ILC_MASK, 0, 10);
    if (m_imageList) {
        SendMessage(m_hwnd, TB_SETIMAGELIST, 0, (LPARAM)m_imageList);
    }

    return true;
}

bool Win32Toolbar::addButton(int id, const std::string& text, HICON icon, ButtonCallback callback)
{
    if (!m_hwnd) {
        LOG_ERROR_C("Toolbar not created", "Win32Toolbar");
        return false;
    }

    // Add icon to image list if provided
    int imageIndex = -1;
    if (icon && m_imageList) {
        imageIndex = ImageList_AddIcon(m_imageList, icon);
    }

    // Create button
    TBBUTTON button = {};
    button.iBitmap = imageIndex;
    button.idCommand = id;
    button.fsState = TBSTATE_ENABLED;
    button.fsStyle = BTNS_BUTTON;
    button.dwData = 0;
    button.iString = 0; // Will set later with TB_SETBUTTONINFO

    if (!SendMessage(m_hwnd, TB_ADDBUTTONS, 1, (LPARAM)&button)) {
        LOG_ERROR_C("Failed to add button: " + text, "Win32Toolbar");
        return false;
    }

    // Set button text using TB_SETBUTTONINFO to avoid string lifetime issues
    std::wstring wtext = toWideString(text);
    TBBUTTONINFOW btnInfo = {};
    btnInfo.cbSize = sizeof(TBBUTTONINFOW);
    btnInfo.dwMask = TBIF_TEXT;
    btnInfo.pszText = const_cast<LPWSTR>(wtext.c_str());
    SendMessage(m_hwnd, TB_SETBUTTONINFOW, static_cast<WPARAM>(static_cast<unsigned int>(id)), (LPARAM)&btnInfo);

    // Store button info
    ButtonInfo info;
    info.id = id;
    info.text = text;
    info.callback = callback;
    m_buttons[id] = info;

    m_nextButtonIndex++;
    autoSize();

    return true;
}

void Win32Toolbar::addSeparator()
{
    if (!m_hwnd) {
        return;
    }

    TBBUTTON separator = {};
    separator.iBitmap = 0;
    separator.idCommand = 0;
    separator.fsState = 0;
    separator.fsStyle = BTNS_SEP;
    separator.dwData = 0;
    separator.iString = 0;

    SendMessage(m_hwnd, TB_ADDBUTTONS, 1, (LPARAM)&separator);
    autoSize();
}

void Win32Toolbar::setButtonEnabled(int id, bool enabled)
{
    if (m_hwnd) {
        SendMessage(m_hwnd, TB_ENABLEBUTTON, static_cast<WPARAM>(static_cast<unsigned int>(id)), MAKELONG(enabled, 0));
    }
}

void Win32Toolbar::setButtonChecked(int id, bool checked)
{
    if (m_hwnd) {
        SendMessage(m_hwnd, TB_CHECKBUTTON, static_cast<WPARAM>(static_cast<unsigned int>(id)), MAKELONG(checked, 0));
    }
}

bool Win32Toolbar::handleCommand(int commandId)
{
    auto it = m_buttons.find(commandId);
    if (it != m_buttons.end() && it->second.callback) {
        it->second.callback();
        return true;
    }
    return false;
}

void Win32Toolbar::autoSize()
{
    if (m_hwnd) {
        SendMessage(m_hwnd, TB_AUTOSIZE, 0, 0);
    }
}

int Win32Toolbar::getHeight() const
{
    if (!m_hwnd) {
        return 0;
    }
    
    // Get the client rect of the toolbar to determine its usable interior height
    // GetClientRect returns dimensions from (0,0) to (width, height) in client coordinates
    RECT rect;
    GetClientRect(m_hwnd, &rect);
    return rect.bottom - rect.top;
}

} // namespace fresh

#endif // _WIN32
