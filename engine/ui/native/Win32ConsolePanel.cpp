#ifdef _WIN32

#include "ui/native/Win32ConsolePanel.h"
#include "ui/native/UnrealStyleTheme.h"
#include "core/Logger.h"
#include <commctrl.h>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace fresh
{

Win32ConsolePanel::Win32ConsolePanel()
    : Win32Panel(),
      m_richEdit(nullptr),
      m_commandEdit(nullptr),
      m_clearButton(nullptr),
      m_autoScrollCheck(nullptr),
      m_filterInfoCheck(nullptr),
      m_filterWarningCheck(nullptr),
      m_filterErrorCheck(nullptr),
      m_autoScroll(true),
      m_filterInfo(true),
      m_filterWarning(true),
      m_filterError(true),
      m_maxMessages(1000)
{
}

Win32ConsolePanel::~Win32ConsolePanel()
{
    // Unregister from Logger
    Logger::getInstance().removeListener(this);
}

bool Win32ConsolePanel::create(HWND parent, int x, int y, int width, int height)
{
    // Load RichEdit library
    static bool richEditLoaded = false;
    if (!richEditLoaded) {
        LoadLibrary(TEXT("Riched20.dll"));
        richEditLoaded = true;
    }

    if (!Win32Panel::create(parent, x, y, width, height)) {
        return false;
    }

    createControls();
    
    // Register with Logger to receive log messages
    Logger::getInstance().addListener(this);
    
    return true;
}

bool Win32ConsolePanel::initialize()
{
    // For standalone initialization without parent window
    // This is a stub that returns true to indicate the panel is ready to be created later
    LOG_INFO_C("Console panel initialized (stub)", "Win32ConsolePanel");
    return true;
}

void Win32ConsolePanel::onCreate()
{
    createControls();
}

void Win32ConsolePanel::createControls()
{
    if (!m_hwnd) return;

    int y = MARGIN;
    int x = MARGIN;
    int controlWidth = m_width - 2 * MARGIN;

    // Create toolbar with buttons and checkboxes
    m_clearButton = CreateWindowExW(
        0, L"BUTTON", L"Clear",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        x, y, BUTTON_WIDTH, TOOLBAR_HEIGHT - 2 * MARGIN,
        m_hwnd, (HMENU)1001, nullptr, nullptr
    );
    SendMessageW(m_clearButton, WM_SETFONT, (WPARAM)UnrealStyleTheme::GetFont(), TRUE);
    x += BUTTON_WIDTH + MARGIN;

    m_autoScrollCheck = CreateWindowExW(
        0, L"BUTTON", L"Auto-scroll",
        WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
        x, y, CHECKBOX_WIDTH, TOOLBAR_HEIGHT - 2 * MARGIN,
        m_hwnd, (HMENU)1002, nullptr, nullptr
    );
    SendMessage(m_autoScrollCheck, BM_SETCHECK, BST_CHECKED, 0);
    SendMessageW(m_autoScrollCheck, WM_SETFONT, (WPARAM)UnrealStyleTheme::GetFont(), TRUE);
    x += CHECKBOX_WIDTH + MARGIN;

    // Filter checkboxes
    m_filterInfoCheck = CreateWindowExW(
        0, L"BUTTON", L"Info",
        WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
        x, y, CHECKBOX_WIDTH, TOOLBAR_HEIGHT - 2 * MARGIN,
        m_hwnd, (HMENU)1003, nullptr, nullptr
    );
    SendMessage(m_filterInfoCheck, BM_SETCHECK, BST_CHECKED, 0);
    SendMessageW(m_filterInfoCheck, WM_SETFONT, (WPARAM)UnrealStyleTheme::GetFont(), TRUE);
    x += CHECKBOX_WIDTH + MARGIN;

    m_filterWarningCheck = CreateWindowExW(
        0, L"BUTTON", L"Warning",
        WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
        x, y, CHECKBOX_WIDTH, TOOLBAR_HEIGHT - 2 * MARGIN,
        m_hwnd, (HMENU)1004, nullptr, nullptr
    );
    SendMessage(m_filterWarningCheck, BM_SETCHECK, BST_CHECKED, 0);
    SendMessageW(m_filterWarningCheck, WM_SETFONT, (WPARAM)UnrealStyleTheme::GetFont(), TRUE);
    x += CHECKBOX_WIDTH + MARGIN;

    m_filterErrorCheck = CreateWindowExW(
        0, L"BUTTON", L"Error",
        WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
        x, y, CHECKBOX_WIDTH, TOOLBAR_HEIGHT - 2 * MARGIN,
        m_hwnd, (HMENU)1005, nullptr, nullptr
    );
    SendMessage(m_filterErrorCheck, BM_SETCHECK, BST_CHECKED, 0);
    SendMessageW(m_filterErrorCheck, WM_SETFONT, (WPARAM)UnrealStyleTheme::GetFont(), TRUE);

    y += TOOLBAR_HEIGHT;

    // Create RichEdit control for log display
    m_richEdit = CreateWindowExW(
        WS_EX_CLIENTEDGE, RICHEDIT_CLASSW, L"",
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL,
        MARGIN, y, controlWidth, m_height - y - COMMAND_HEIGHT - 2 * MARGIN,
        m_hwnd, (HMENU)2001, nullptr, nullptr
    );

    if (m_richEdit) {
        // Set dark background and light text for Unreal theme
        SendMessage(m_richEdit, EM_SETBKGNDCOLOR, 0, UnrealStyleTheme::DARK_BACKGROUND);
        
        // Set font
        CHARFORMAT2W cf = {0};
        cf.cbSize = sizeof(CHARFORMAT2W);
        cf.dwMask = CFM_FACE | CFM_SIZE | CFM_COLOR;
        cf.yHeight = 180; // 9pt
        wcscpy_s(cf.szFaceName, LF_FACESIZE, L"Consolas");
        cf.crTextColor = UnrealStyleTheme::PRIMARY_TEXT;
        SendMessage(m_richEdit, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf);
    }

    // Create command input field
    y = m_height - COMMAND_HEIGHT - MARGIN;
    m_commandEdit = CreateWindowExW(
        WS_EX_CLIENTEDGE, L"EDIT", L"",
        WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
        MARGIN, y, controlWidth, COMMAND_HEIGHT,
        m_hwnd, (HMENU)3001, nullptr, nullptr
    );

    if (m_commandEdit) {
        // Set placeholder text (requires Windows Vista+)
        SendMessage(m_commandEdit, EM_SETCUEBANNER, TRUE, (LPARAM)L"Enter command...");
        // Set font for better readability
        SendMessageW(m_commandEdit, WM_SETFONT, (WPARAM)UnrealStyleTheme::GetFont(), TRUE);
    }
}

void Win32ConsolePanel::onResize(int width, int height)
{
    int controlWidth = width - 2 * MARGIN;
    int y = TOOLBAR_HEIGHT + MARGIN;

    if (m_richEdit) {
        SetWindowPos(m_richEdit, nullptr,
                     MARGIN, y,
                     controlWidth, height - y - COMMAND_HEIGHT - 2 * MARGIN,
                     SWP_NOZORDER);
    }

    if (m_commandEdit) {
        y = height - COMMAND_HEIGHT - MARGIN;
        SetWindowPos(m_commandEdit, nullptr,
                     MARGIN, y,
                     controlWidth, COMMAND_HEIGHT,
                     SWP_NOZORDER);
    }
}

void Win32ConsolePanel::onDestroy()
{
    // Controls are automatically destroyed with parent window
}

bool Win32ConsolePanel::handleMessage(UINT msg, WPARAM wParam, LPARAM lParam, LRESULT& result)
{
    if (msg == WM_COMMAND) {
        int wmId = LOWORD(wParam);
        int wmEvent = HIWORD(wParam);

        switch (wmId) {
            case 1001: // Clear button
                if (wmEvent == BN_CLICKED) {
                    clear();
                    result = 0;
                    return true;
                }
                break;

            case 1002: // Auto-scroll checkbox
                if (wmEvent == BN_CLICKED) {
                    m_autoScroll = (SendMessage(m_autoScrollCheck, BM_GETCHECK, 0, 0) == BST_CHECKED);
                    result = 0;
                    return true;
                }
                break;

            case 1003: // Info filter
                if (wmEvent == BN_CLICKED) {
                    m_filterInfo = (SendMessage(m_filterInfoCheck, BM_GETCHECK, 0, 0) == BST_CHECKED);
                    updateDisplay();
                    result = 0;
                    return true;
                }
                break;

            case 1004: // Warning filter
                if (wmEvent == BN_CLICKED) {
                    m_filterWarning = (SendMessage(m_filterWarningCheck, BM_GETCHECK, 0, 0) == BST_CHECKED);
                    updateDisplay();
                    result = 0;
                    return true;
                }
                break;

            case 1005: // Error filter
                if (wmEvent == BN_CLICKED) {
                    m_filterError = (SendMessage(m_filterErrorCheck, BM_GETCHECK, 0, 0) == BST_CHECKED);
                    updateDisplay();
                    result = 0;
                    return true;
                }
                break;

            case 3001: // Command edit
                if (wmEvent == EN_CHANGE) {
                    // Handle Enter key in command edit
                    if (GetKeyState(VK_RETURN) & 0x8000) {
                        executeCommand();
                        result = 0;
                        return true;
                    }
                }
                break;
        }
    }
    else if (msg == WM_KEYDOWN && wParam == VK_RETURN) {
        // Check if command edit has focus
        if (GetFocus() == m_commandEdit) {
            executeCommand();
            result = 0;
            return true;
        }
    }

    return Win32Panel::handleMessage(msg, wParam, lParam, result);
}

void Win32ConsolePanel::addMessage(MessageType type, const std::string& message)
{
    // Add to message history
    ConsoleMessage msg;
    msg.type = type;
    msg.timestamp = formatTimestamp();
    msg.text = message;
    m_messages.push_back(msg);

    // Limit message history
    if (m_messages.size() > static_cast<size_t>(m_maxMessages)) {
        m_messages.erase(m_messages.begin());
    }

    // Check if message should be displayed based on filters
    bool shouldDisplay = false;
    switch (type) {
        case MessageType::Info:
            shouldDisplay = m_filterInfo;
            break;
        case MessageType::Warning:
            shouldDisplay = m_filterWarning;
            break;
        case MessageType::Error:
            shouldDisplay = m_filterError;
            break;
    }

    if (shouldDisplay && m_richEdit) {
        // Format message
        std::string formattedMsg = "[" + msg.timestamp + "] " + message + "\r\n";
        
        // Append to RichEdit with appropriate color
        appendToRichEdit(formattedMsg, getMessageColor(type));

        // Auto-scroll if enabled
        if (m_autoScroll) {
            // Scroll to end
            SendMessage(m_richEdit, EM_SETSEL, (WPARAM)-1, (LPARAM)-1);
            SendMessage(m_richEdit, EM_SCROLLCARET, 0, 0);
        }
    }
}

void Win32ConsolePanel::clear()
{
    m_messages.clear();
    if (m_richEdit) {
        SetWindowTextW(m_richEdit, L"");
    }
}

void Win32ConsolePanel::updateDisplay()
{
    if (!m_richEdit) return;

    // Clear display
    SetWindowTextW(m_richEdit, L"");

    // Re-add filtered messages
    for (const auto& msg : m_messages) {
        bool shouldDisplay = false;
        switch (msg.type) {
            case MessageType::Info:
                shouldDisplay = m_filterInfo;
                break;
            case MessageType::Warning:
                shouldDisplay = m_filterWarning;
                break;
            case MessageType::Error:
                shouldDisplay = m_filterError;
                break;
        }

        if (shouldDisplay) {
            std::string formattedMsg = "[" + msg.timestamp + "] " + msg.text + "\r\n";
            appendToRichEdit(formattedMsg, getMessageColor(msg.type));
        }
    }

    // Scroll to end if auto-scroll is enabled
    if (m_autoScroll) {
        SendMessage(m_richEdit, EM_SETSEL, (WPARAM)-1, (LPARAM)-1);
        SendMessage(m_richEdit, EM_SCROLLCARET, 0, 0);
    }
}

void Win32ConsolePanel::executeCommand()
{
    if (!m_commandEdit) return;

    // Get command text
    int length = GetWindowTextLengthW(m_commandEdit);
    if (length == 0) return;

    std::wstring wideText(length + 1, L'\0');
    GetWindowTextW(m_commandEdit, &wideText[0], length + 1);

    // Convert to narrow string
    int narrowLength = WideCharToMultiByte(CP_UTF8, 0, wideText.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string command(narrowLength, '\0');
    WideCharToMultiByte(CP_UTF8, 0, wideText.c_str(), -1, &command[0], narrowLength, nullptr, nullptr);
    command.pop_back(); // Remove null terminator

    // Clear command edit
    SetWindowTextW(m_commandEdit, L"");

    // Echo command to console
    addMessage(MessageType::Info, "> " + command);

    // Execute command callback
    if (m_commandCallback) {
        m_commandCallback(command);
    }
}

void Win32ConsolePanel::appendToRichEdit(const std::string& text, COLORREF color)
{
    if (!m_richEdit) return;

    // Convert to wide string
    int wideLength = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, nullptr, 0);
    std::wstring wideText(wideLength, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, &wideText[0], wideLength);

    // Move to end
    CHARRANGE cr;
    cr.cpMin = -1;
    cr.cpMax = -1;
    SendMessage(m_richEdit, EM_EXSETSEL, 0, (LPARAM)&cr);

    // Set color format
    CHARFORMAT2 cf = {0};
    cf.cbSize = sizeof(CHARFORMAT2);
    cf.dwMask = CFM_COLOR;
    cf.crTextColor = color;
    SendMessage(m_richEdit, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);

    // Insert text
    SendMessage(m_richEdit, EM_REPLACESEL, FALSE, (LPARAM)wideText.c_str());
}

std::string Win32ConsolePanel::formatTimestamp() const
{
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;

    std::tm tm;
#ifdef _WIN32
    localtime_s(&tm, &time);
#else
    localtime_r(&time, &tm);
#endif

    std::ostringstream oss;
    oss << std::put_time(&tm, "%H:%M:%S");
    oss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return oss.str();
}

COLORREF Win32ConsolePanel::getMessageColor(MessageType type) const
{
    switch (type) {
        case MessageType::Info:
            return UnrealStyleTheme::INFO_COLOR;
        case MessageType::Warning:
            return UnrealStyleTheme::WARNING_COLOR;
        case MessageType::Error:
            return UnrealStyleTheme::ERROR_COLOR;
        default:
            return UnrealStyleTheme::PRIMARY_TEXT;
    }
}

void Win32ConsolePanel::onLogMessage(LogLevel level, const std::string& message, const std::string& component)
{
    // Convert LogLevel to MessageType
    MessageType type;
    switch (level) {
        case LogLevel::WARNING:
            type = MessageType::Warning;
            break;
        case LogLevel::ERR:
        case LogLevel::FATAL:
            type = MessageType::Error;
            break;
        case LogLevel::INFO:
        default:
            type = MessageType::Info;
            break;
    }

    // Format the message with component if present
    std::string fullMessage = message;
    if (!component.empty()) {
        fullMessage = "[" + component + "] " + message;
    }

    // Add the message to the console
    addMessage(type, fullMessage);
}

} // namespace fresh

#endif // _WIN32
