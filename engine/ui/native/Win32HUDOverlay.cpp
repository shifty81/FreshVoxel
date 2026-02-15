#ifdef _WIN32

#include "ui/native/Win32HUDOverlay.h"
#include "ui/native/Win32HUD.h"
#include "core/Logger.h"

namespace fresh
{

Win32HUDOverlay::Win32HUDOverlay()
    : m_hwnd(nullptr)
    , m_parentHwnd(nullptr)
    , m_hud(nullptr)
    , m_initialized(false)
    , m_clearBrush(nullptr)
{
}

Win32HUDOverlay::~Win32HUDOverlay()
{
    shutdown();
}

bool Win32HUDOverlay::initialize(HWND parentHwnd, Win32HUD* hud)
{
    if (m_initialized) {
        LOG_WARNING_C("Win32HUDOverlay already initialized", "Win32HUDOverlay");
        return true;
    }

    if (!parentHwnd || !hud) {
        LOG_ERROR_C("Invalid parameters for Win32HUDOverlay initialization", "Win32HUDOverlay");
        return false;
    }

    m_parentHwnd = parentHwnd;
    m_hud = hud;

    // Register the window class
    if (!registerWindowClass()) {
        LOG_ERROR_C("Failed to register window class for HUD overlay", "Win32HUDOverlay");
        return false;
    }

    // Create the overlay window
    if (!createOverlayWindow()) {
        LOG_ERROR_C("Failed to create HUD overlay window", "Win32HUDOverlay");
        return false;
    }

    // Create cached brush for clearing background
    m_clearBrush = CreateSolidBrush(RGB(0, 0, 0));
    if (!m_clearBrush) {
        LOG_ERROR_C("Failed to create clear brush for HUD overlay", "Win32HUDOverlay");
        DestroyWindow(m_hwnd);
        m_hwnd = nullptr;
        return false;
    }

    m_initialized = true;
    LOG_INFO_C("Win32HUDOverlay initialized successfully", "Win32HUDOverlay");
    return true;
}

void Win32HUDOverlay::shutdown()
{
    if (m_hwnd) {
        DestroyWindow(m_hwnd);
        m_hwnd = nullptr;
    }
    if (m_clearBrush) {
        DeleteObject(m_clearBrush);
        m_clearBrush = nullptr;
    }
    m_parentHwnd = nullptr;
    m_hud = nullptr;
    m_initialized = false;
}

void Win32HUDOverlay::updatePosition()
{
    if (!m_initialized || !m_hwnd || !m_parentHwnd) {
        return;
    }

    // Get parent window client area
    RECT parentRect;
    GetClientRect(m_parentHwnd, &parentRect);

    // Convert to screen coordinates
    POINT topLeft = {parentRect.left, parentRect.top};
    POINT bottomRight = {parentRect.right, parentRect.bottom};
    ClientToScreen(m_parentHwnd, &topLeft);
    ClientToScreen(m_parentHwnd, &bottomRight);

    // Calculate dimensions
    int width = bottomRight.x - topLeft.x;
    int height = bottomRight.y - topLeft.y;

    // Position the overlay to match parent client area
    // Use SWP_NOZORDER instead of HWND_TOP to avoid changing Z-order unnecessarily
    // Don't use SWP_SHOWWINDOW as visibility is controlled by setVisible()
    SetWindowPos(m_hwnd, nullptr, topLeft.x, topLeft.y, width, height,
                 SWP_NOACTIVATE | SWP_NOZORDER);
}

void Win32HUDOverlay::setVisible(bool visible)
{
    if (m_hwnd) {
        ShowWindow(m_hwnd, visible ? SW_SHOW : SW_HIDE);
    }
}

void Win32HUDOverlay::invalidate()
{
    if (m_hwnd) {
        InvalidateRect(m_hwnd, nullptr, FALSE);
    }
}

bool Win32HUDOverlay::registerWindowClass()
{
    HINSTANCE hInstance = GetModuleHandle(nullptr);

    // Check if class is already registered
    WNDCLASSEXW existingClass = {};
    if (GetClassInfoExW(hInstance, WINDOW_CLASS_NAME, &existingClass)) {
        return true; // Already registered
    }

    // Register new window class
    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = nullptr;  // We handle background painting manually
    wc.lpszClassName = WINDOW_CLASS_NAME;

    if (!RegisterClassExW(&wc)) {
        DWORD error = GetLastError();
        LOG_ERROR_C("Failed to register overlay window class. Error code: " + std::to_string(error), 
                    "Win32HUDOverlay");
        return false;
    }

    return true;
}

bool Win32HUDOverlay::createOverlayWindow()
{
    HINSTANCE hInstance = GetModuleHandle(nullptr);

    // Get parent window dimensions
    RECT parentRect;
    GetClientRect(m_parentHwnd, &parentRect);

    // Convert to screen coordinates
    POINT topLeft = {parentRect.left, parentRect.top};
    POINT bottomRight = {parentRect.right, parentRect.bottom};
    ClientToScreen(m_parentHwnd, &topLeft);
    ClientToScreen(m_parentHwnd, &bottomRight);

    int width = bottomRight.x - topLeft.x;
    int height = bottomRight.y - topLeft.y;

    // Create the overlay window as a popup window
    // WS_EX_LAYERED: Allows transparency and alpha blending
    // WS_EX_TRANSPARENT: Click-through for input (mouse events pass through)
    // WS_POPUP: Frameless top-level window
    // Initially hidden - will be shown when HUD is visible
    m_hwnd = CreateWindowExW(
        WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST,
        WINDOW_CLASS_NAME,
        L"HUD Overlay",
        WS_POPUP,  // Hidden initially (no WS_VISIBLE)
        topLeft.x, topLeft.y, width, height,
        nullptr,  // No parent for popup window
        nullptr,
        hInstance,
        nullptr
    );

    if (!m_hwnd) {
        DWORD error = GetLastError();
        LOG_ERROR_C("Failed to create overlay window. Error code: " + std::to_string(error), 
                    "Win32HUDOverlay");
        return false;
    }

    // Make the window fully opaque initially
    // We use RGB(0,0,0) as transparent color key for the background
    SetLayeredWindowAttributes(m_hwnd, RGB(0, 0, 0), 0, LWA_COLORKEY);

    // Store 'this' pointer in window user data for access in WindowProc
    SetWindowLongPtrW(m_hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

    return true;
}

void Win32HUDOverlay::onPaint()
{
    if (!m_hud) {
        return;
    }

    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(m_hwnd, &ps);

    if (hdc) {
        // Get client area
        RECT clientRect;
        GetClientRect(m_hwnd, &clientRect);

        // Clear background to transparent using cached brush
        if (m_clearBrush) {
            FillRect(hdc, &clientRect, m_clearBrush);
        }

        // Render the HUD (wrapped in try-catch for safety)
        try {
            m_hud->render(hdc, clientRect);
        } catch (...) {
            // If HUD rendering fails, log error but continue
            // The overlay will just show the black transparent background
            LOG_ERROR_C("Exception occurred while rendering HUD", "Win32HUDOverlay");
        }
    }

    EndPaint(m_hwnd, &ps);
}

LRESULT CALLBACK Win32HUDOverlay::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    // Retrieve the Win32HUDOverlay instance
    Win32HUDOverlay* overlay = reinterpret_cast<Win32HUDOverlay*>(
        GetWindowLongPtrW(hwnd, GWLP_USERDATA)
    );

    switch (uMsg) {
        case WM_PAINT:
            if (overlay) {
                overlay->onPaint();
                return 0;
            }
            break;

        case WM_ERASEBKGND:
            // Prevent default erase to avoid flicker
            return 1;

        case WM_DESTROY:
            if (overlay) {
                overlay->m_hwnd = nullptr;
            }
            return 0;

        default:
            break;
    }

    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

} // namespace fresh

#endif // _WIN32
