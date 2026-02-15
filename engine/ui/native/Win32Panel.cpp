#ifdef _WIN32

#include "ui/native/Win32Panel.h"
#include "ui/native/UnrealStyleTheme.h"
#include "core/Logger.h"

namespace fresh
{

const wchar_t* Win32Panel::WINDOW_CLASS_NAME = L"FreshWin32Panel";
bool Win32Panel::s_classRegistered = false;
HBRUSH Win32Panel::s_backgroundBrush = nullptr;

Win32Panel::Win32Panel()
    : m_hwnd(nullptr)
    , m_parent(nullptr)
    , m_width(0)
    , m_height(0)
    , m_showTitleBar(false)
    , m_title(L"Panel")
{
}

Win32Panel::~Win32Panel()
{
    destroy();
}

bool Win32Panel::registerWindowClass()
{
    if (s_classRegistered) {
        return true;
    }

    // Create the dark background brush once
    if (!s_backgroundBrush) {
        s_backgroundBrush = CreateSolidBrush(UnrealStyleTheme::PanelBackground);
    }

    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.hIcon = nullptr;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = s_backgroundBrush;  // Use dark theme background
    wc.lpszMenuName = nullptr;
    wc.lpszClassName = WINDOW_CLASS_NAME;
    wc.hIconSm = nullptr;

    if (!RegisterClassExW(&wc)) {
        LOG_ERROR_C("Failed to register Win32Panel window class", "Win32Panel");
        return false;
    }

    s_classRegistered = true;
    return true;
}

bool Win32Panel::create(HWND parent, int x, int y, int width, int height, const wchar_t* title)
{
    if (m_hwnd) {
        LOG_WARNING_C("Panel already created", "Win32Panel");
        return false;
    }

    if (!registerWindowClass()) {
        return false;
    }

    m_parent = parent;
    m_width = width;
    m_height = height;
    m_title = title ? title : L"Panel";

    m_hwnd = CreateWindowExW(
        0,  // No extended styles - prevents gaps caused by 3D sunken border effect of WS_EX_CLIENTEDGE
        WINDOW_CLASS_NAME,
        title,
        WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_BORDER,
        x, y, width, height,
        parent,
        nullptr,
        GetModuleHandle(nullptr),
        this  // Pass 'this' pointer for retrieval in WM_CREATE
    );

    if (!m_hwnd) {
        LOG_ERROR_C("Failed to create Win32Panel window", "Win32Panel");
        return false;
    }

    return true;
}

void Win32Panel::destroy()
{
    if (m_hwnd) {
        DestroyWindow(m_hwnd);
        m_hwnd = nullptr;
    }
}

void Win32Panel::setVisible(bool show)
{
    if (m_hwnd) {
        ShowWindow(m_hwnd, show ? SW_SHOW : SW_HIDE);
    }
}

bool Win32Panel::isVisible() const
{
    return m_hwnd && IsWindowVisible(m_hwnd);
}

void Win32Panel::setPosition(int x, int y)
{
    if (m_hwnd) {
        SetWindowPos(m_hwnd, nullptr, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
    }
}

void Win32Panel::setSize(int width, int height)
{
    if (m_hwnd) {
        m_width = width;
        m_height = height;
        SetWindowPos(m_hwnd, nullptr, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER);
    }
}

void Win32Panel::setTitle(const wchar_t* title)
{
    m_title = title ? title : L"Panel";
    if (m_hwnd && m_showTitleBar) {
        InvalidateRect(m_hwnd, nullptr, TRUE);
    }
}

int Win32Panel::getContentHeight() const
{
    if (m_showTitleBar) {
        return m_height - UnrealStyleTheme::TitleBarHeight;
    }
    return m_height;
}

int Win32Panel::getContentYOffset() const
{
    if (m_showTitleBar) {
        return UnrealStyleTheme::TitleBarHeight;
    }
    return 0;
}

void Win32Panel::paintBackground(HDC hdc)
{
    RECT clientRect;
    GetClientRect(m_hwnd, &clientRect);
    
    // Use cached brush from theme for better performance
    FillRect(hdc, &clientRect, UnrealStyleTheme::GetPanelBackgroundBrush());
    
    // Draw simple single-pixel border using theme's cached pen
    // This provides visual separation without the gaps caused by WS_EX_CLIENTEDGE
    HPEN oldPen = (HPEN)SelectObject(hdc, UnrealStyleTheme::GetBorderDarkPen());
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
    Rectangle(hdc, clientRect.left, clientRect.top, clientRect.right, clientRect.bottom);
    SelectObject(hdc, oldPen);
    SelectObject(hdc, oldBrush);
}

void Win32Panel::paintTitleBar(HDC hdc)
{
    if (!m_showTitleBar) {
        return;
    }
    
    RECT titleRect;
    titleRect.left = 0;
    titleRect.top = 0;
    titleRect.right = m_width;
    titleRect.bottom = UnrealStyleTheme::TitleBarHeight;
    
    // Draw title bar using theme helper
    UnrealStyleTheme::DrawPanelTitleBar(hdc, titleRect, m_title.c_str(), true);
    
    // Draw separator line below title bar
    UnrealStyleTheme::DrawSeparator(hdc, 0, UnrealStyleTheme::TitleBarHeight - 1, 
                                     m_width, UnrealStyleTheme::TitleBarHeight - 1);
}

bool Win32Panel::handleMessage(UINT msg, WPARAM wParam, LPARAM lParam, LRESULT& result)
{
    (void)lParam; // Suppress unreferenced parameter warning
    switch (msg) {
        case WM_ERASEBKGND:
            // We handle background painting ourselves for flicker-free rendering
            result = 1;
            return true;
            
        case WM_CTLCOLORSTATIC:
        case WM_CTLCOLOREDIT:
        case WM_CTLCOLORBTN: {
            // Apply dark theme colors to child controls
            HDC hdcControl = (HDC)wParam;
            SetTextColor(hdcControl, UnrealStyleTheme::TextPrimary);
            SetBkColor(hdcControl, UnrealStyleTheme::InputBackground);
            result = (LRESULT)UnrealStyleTheme::GetInputBackgroundBrush();
            return true;
        }
        
        case WM_CTLCOLORLISTBOX: {
            // Apply dark theme to list boxes
            HDC hdcControl = (HDC)wParam;
            SetTextColor(hdcControl, UnrealStyleTheme::TextPrimary);
            SetBkColor(hdcControl, UnrealStyleTheme::DarkBackground);
            result = (LRESULT)UnrealStyleTheme::GetDarkBackgroundBrush();
            return true;
        }
    }
    
    return false;
}

LRESULT CALLBACK Win32Panel::WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    Win32Panel* panel = nullptr;

    if (msg == WM_CREATE) {
        // Retrieve 'this' pointer from CreateWindowEx
        CREATESTRUCT* cs = reinterpret_cast<CREATESTRUCT*>(lParam);
        panel = reinterpret_cast<Win32Panel*>(cs->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(panel));
        
        if (panel) {
            panel->onCreate();
        }
        return 0;
    }

    // Retrieve stored pointer
    panel = reinterpret_cast<Win32Panel*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

    if (panel) {
        LRESULT result = 0;
        if (panel->handleMessage(msg, wParam, lParam, result)) {
            return result;
        }

        switch (msg) {
            case WM_PAINT: {
                PAINTSTRUCT ps;
                HDC hdc = BeginPaint(hwnd, &ps);
                
                // Paint background and title bar first
                panel->paintBackground(hdc);
                panel->paintTitleBar(hdc);
                
                // Then let derived class paint its content
                panel->onPaint(hdc);
                
                EndPaint(hwnd, &ps);
                return 0;
            }

            case WM_SIZE: {
                int width = LOWORD(lParam);
                int height = HIWORD(lParam);
                panel->m_width = width;
                panel->m_height = height;
                panel->onResize(width, height);
                return 0;
            }

            case WM_DESTROY:
                panel->onDestroy();
                return 0;
        }
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

} // namespace fresh

#endif // _WIN32
