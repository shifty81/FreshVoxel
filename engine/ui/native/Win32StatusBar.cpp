#ifdef _WIN32

#include "ui/native/Win32StatusBar.h"
#include "ui/native/UnrealStyleTheme.h"
#include "core/Logger.h"
#include <sstream>
#include <iomanip>

namespace fresh
{

Win32StatusBar::Win32StatusBar()
    : m_hwnd(nullptr)
    , m_parent(nullptr)
    , m_numPanes(3)
{
}

Win32StatusBar::~Win32StatusBar()
{
    destroy();
}

bool Win32StatusBar::create(HWND parent, int numPanes, const PaneConfig* paneConfigs)
{
    if (m_hwnd) {
        LOG_WARNING_C("Status bar already created", "Win32StatusBar");
        return false;
    }

    // Ensure common controls are initialized
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_BAR_CLASSES;
    InitCommonControlsEx(&icex);

    m_parent = parent;
    m_numPanes = numPanes;

    // Create the status bar window
    m_hwnd = CreateWindowExW(
        0,
        STATUSCLASSNAMEW,
        nullptr,
        WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
        0, 0, 0, 0,  // Position and size are auto-calculated
        parent,
        nullptr,
        GetModuleHandle(nullptr),
        nullptr
    );

    if (!m_hwnd) {
        LOG_ERROR_C("Failed to create status bar window", "Win32StatusBar");
        return false;
    }

    // Initialize pane data
    m_paneWidths.resize(numPanes);
    m_paneTexts.resize(numPanes);

    // Apply pane configurations if provided
    if (paneConfigs) {
        for (int i = 0; i < numPanes; ++i) {
            m_paneWidths[i] = paneConfigs[i].width;
            m_paneTexts[i] = paneConfigs[i].text;
        }
    } else {
        // Default configuration: Ready | Position | Selection
        m_paneWidths[0] = -1;  // Stretch to fill
        m_paneWidths[1] = 150;
        m_paneWidths[2] = 120;
        
        m_paneTexts[0] = L"Ready";
        m_paneTexts[1] = L"";
        m_paneTexts[2] = L"";
    }

    // Calculate and set pane widths
    calculatePaneWidths();

    // Apply dark theme
    applyDarkTheme();
    
    // Set font for better readability - use the same font as other UI elements
    SendMessageW(m_hwnd, WM_SETFONT, (WPARAM)UnrealStyleTheme::GetFont(), TRUE);

    // Set initial text for all panes
    for (int i = 0; i < numPanes; ++i) {
        if (!m_paneTexts[i].empty()) {
            setPaneText(i, m_paneTexts[i]);
        }
    }

    LOG_INFO_C("Win32StatusBar created successfully", "Win32StatusBar");
    return true;
}

void Win32StatusBar::destroy()
{
    if (m_hwnd) {
        DestroyWindow(m_hwnd);
        m_hwnd = nullptr;
    }
}

void Win32StatusBar::setPaneText(int paneIndex, const std::wstring& text)
{
    if (!m_hwnd || paneIndex < 0 || paneIndex >= m_numPanes) {
        return;
    }

    m_paneTexts[paneIndex] = text;
    SendMessageW(m_hwnd, SB_SETTEXTW, paneIndex, reinterpret_cast<LPARAM>(text.c_str()));
}

void Win32StatusBar::setPaneText(int paneIndex, const std::string& text)
{
    // Convert narrow string to wide string
    // First call gets required buffer length including null terminator
    int wideLength = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, nullptr, 0);
    if (wideLength <= 0) {
        return;
    }
    
    // Allocate buffer for the full length including null terminator
    std::wstring wideText(static_cast<size_t>(wideLength), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, wideText.data(), wideLength);
    
    // Remove the null terminator from the string length (it's still there in the buffer)
    wideText.resize(static_cast<size_t>(wideLength - 1));
    
    setPaneText(paneIndex, wideText);
}

void Win32StatusBar::updateLayout()
{
    if (!m_hwnd || !m_parent) {
        return;
    }

    // Notify the status bar that the parent has resized
    SendMessage(m_hwnd, WM_SIZE, 0, 0);

    // Recalculate pane widths based on new parent size
    calculatePaneWidths();
}

int Win32StatusBar::getHeight() const
{
    if (!m_hwnd) {
        return 0;
    }

    RECT rect;
    GetWindowRect(m_hwnd, &rect);
    return rect.bottom - rect.top;
}

void Win32StatusBar::setVisible(bool show)
{
    if (m_hwnd) {
        ShowWindow(m_hwnd, show ? SW_SHOW : SW_HIDE);
    }
}

bool Win32StatusBar::isVisible() const
{
    return m_hwnd && IsWindowVisible(m_hwnd);
}

void Win32StatusBar::setStatusText(const std::wstring& text)
{
    setPaneText(0, text);
}

void Win32StatusBar::setCursorPosition(float x, float y, float z)
{
    std::wstringstream ss;
    ss << std::fixed << std::setprecision(1);
    ss << L"X: " << x << L"  Y: " << y;
    if (z != 0.0f) {
        ss << L"  Z: " << z;
    }
    
    // Typically position is in the second pane
    if (m_numPanes >= 2) {
        setPaneText(1, ss.str());
    }
}

void Win32StatusBar::setSelectionInfo(int count, const std::wstring& type)
{
    std::wstringstream ss;
    if (count == 0) {
        ss << L"No selection";
    } else if (count == 1) {
        ss << L"1 " << type;
    } else {
        ss << count << L" " << type;
    }
    
    // Typically selection info is in the third pane
    if (m_numPanes >= 3) {
        setPaneText(2, ss.str());
    }
}

void Win32StatusBar::setFPS(float fps)
{
    std::wstringstream ss;
    ss << std::fixed << std::setprecision(1);
    ss << L"FPS: " << fps;
    
    // FPS is typically in the last pane
    if (m_numPanes >= 1) {
        setPaneText(m_numPanes - 1, ss.str());
    }
}

void Win32StatusBar::setMemoryUsage(float usedMB, float totalMB)
{
    std::wstringstream ss;
    ss << std::fixed << std::setprecision(0);
    ss << L"Memory: " << usedMB << L" / " << totalMB << L" MB";
    
    // Memory is typically in a middle or last pane
    if (m_numPanes >= 2) {
        setPaneText(m_numPanes - 1, ss.str());
    }
}

void Win32StatusBar::applyDarkTheme()
{
    if (!m_hwnd) {
        return;
    }

    // Set background color
    // Note: Status bar has limited theming support in Win32
    // For full dark theme, we'd need owner-draw, but basic colors can be set
    SendMessage(m_hwnd, SB_SETBKCOLOR, 0, UnrealStyleTheme::ToolbarBackground);
}

void Win32StatusBar::calculatePaneWidths()
{
    if (!m_hwnd || !m_parent || m_numPanes <= 0) {
        return;
    }

    // Get parent client rect to calculate widths
    RECT parentRect;
    GetClientRect(m_parent, &parentRect);
    int totalWidth = parentRect.right - parentRect.left;

    // Calculate actual pane positions (cumulative widths from left)
    // SB_SETPARTS expects the right edge of each pane
    std::vector<int> panePositions(m_numPanes);
    
    // First, sum up fixed-width panes and find stretch pane
    int fixedWidth = 0;
    int stretchPaneIndex = -1;
    
    for (int i = 0; i < m_numPanes; ++i) {
        if (m_paneWidths[i] == -1) {
            stretchPaneIndex = i;
        } else {
            fixedWidth += m_paneWidths[i];
        }
    }

    // Calculate stretch pane width (remaining space after fixed panes)
    int stretchWidth = totalWidth - fixedWidth;
    if (stretchWidth < 50) stretchWidth = 50;  // Minimum width for stretch pane

    // Calculate positions from left to right
    int currentPos = 0;
    
    for (int i = 0; i < m_numPanes; ++i) {
        if (m_paneWidths[i] == -1) {
            // Stretch pane gets remaining space
            currentPos += stretchWidth;
        } else {
            currentPos += m_paneWidths[i];
        }
        panePositions[i] = currentPos;
    }
    
    // Last pane extends to the right edge
    panePositions[m_numPanes - 1] = -1;

    // Set the pane positions
    SendMessage(m_hwnd, SB_SETPARTS, m_numPanes, reinterpret_cast<LPARAM>(panePositions.data()));
}

} // namespace fresh

#endif // _WIN32
