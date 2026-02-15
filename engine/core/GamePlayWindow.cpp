#ifdef _WIN32

#include "core/GamePlayWindow.h"
#include "core/Logger.h"

#include <windowsx.h>

namespace fresh
{

GamePlayWindow::GamePlayWindow()
    : m_hwnd(nullptr),
      m_hinstance(GetModuleHandle(nullptr)),
      m_width(0),
      m_height(0),
      m_shouldClose(false),
      m_resized(false)
{
}

GamePlayWindow::~GamePlayWindow()
{
    close();
}

bool GamePlayWindow::registerWindowClass()
{
    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = m_hinstance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszClassName = GAME_WINDOW_CLASS_NAME;
    wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wc.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);

    // RegisterClassExW returns 0 on failure, but also if already registered
    if (!RegisterClassExW(&wc)) {
        DWORD err = GetLastError();
        if (err != ERROR_CLASS_ALREADY_EXISTS) {
            LOG_ERROR_C("Failed to register game window class: " + std::to_string(err), "GamePlayWindow");
            return false;
        }
    }

    return true;
}

bool GamePlayWindow::create(uint32_t width, uint32_t height, const std::string& title)
{
    if (m_hwnd) {
        LOG_WARNING_C("Game play window already open", "GamePlayWindow");
        return false;
    }

    if (!registerWindowClass()) {
        return false;
    }

    m_width = width;
    m_height = height;
    m_shouldClose = false;
    m_resized = false;

    // Calculate window size to get desired client area
    RECT rect = {0, 0, static_cast<LONG>(width), static_cast<LONG>(height)};
    DWORD style = WS_OVERLAPPEDWINDOW;
    DWORD exStyle = WS_EX_APPWINDOW;
    AdjustWindowRectEx(&rect, style, FALSE, exStyle);

    // Convert title to wide string
    int titleLen = MultiByteToWideChar(CP_UTF8, 0, title.c_str(), -1, nullptr, 0);
    std::wstring wTitle(titleLen, 0);
    MultiByteToWideChar(CP_UTF8, 0, title.c_str(), -1, &wTitle[0], titleLen);

    m_hwnd = CreateWindowExW(
        exStyle,
        GAME_WINDOW_CLASS_NAME,
        wTitle.c_str(),
        style,
        CW_USEDEFAULT, CW_USEDEFAULT,
        rect.right - rect.left,
        rect.bottom - rect.top,
        nullptr,  // No parent - independent top-level window
        nullptr,
        m_hinstance,
        this
    );

    if (!m_hwnd) {
        LOG_ERROR_C("Failed to create game play window: " + std::to_string(GetLastError()), "GamePlayWindow");
        return false;
    }

    ShowWindow(m_hwnd, SW_SHOW);
    UpdateWindow(m_hwnd);
    SetForegroundWindow(m_hwnd);

    LOG_INFO_C("Game play window created (" + std::to_string(width) + "x" + std::to_string(height) + ")", "GamePlayWindow");
    return true;
}

void GamePlayWindow::close()
{
    if (m_hwnd) {
        DestroyWindow(m_hwnd);
        m_hwnd = nullptr;
        m_shouldClose = true;
        LOG_INFO_C("Game play window closed", "GamePlayWindow");
    }
}

bool GamePlayWindow::pollEvents()
{
    if (!m_hwnd) {
        return false;
    }

    MSG msg;
    while (PeekMessageW(&msg, m_hwnd, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    if (m_shouldClose) {
        close();
        return false;
    }

    return true;
}

LRESULT CALLBACK GamePlayWindow::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    GamePlayWindow* window = nullptr;

    if (uMsg == WM_CREATE) {
        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        window = reinterpret_cast<GamePlayWindow*>(pCreate->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));
    } else {
        window = reinterpret_cast<GamePlayWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }

    if (window) {
        switch (uMsg) {
        case WM_CLOSE:
            window->m_shouldClose = true;
            if (window->m_closeCallback) {
                window->m_closeCallback();
            }
            return 0;

        case WM_SIZE: {
            RECT clientRect;
            if (GetClientRect(hwnd, &clientRect)) {
                UINT w = clientRect.right - clientRect.left;
                UINT h = clientRect.bottom - clientRect.top;
                if (w != window->m_width || h != window->m_height) {
                    window->m_width = w;
                    window->m_height = h;
                    window->m_resized = true;
                }
            }
            return 0;
        }

        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
            if (window->m_keyCallback) {
                window->m_keyCallback(static_cast<int>(wParam), true);
            }
            return 0;

        case WM_KEYUP:
        case WM_SYSKEYUP:
            if (window->m_keyCallback) {
                window->m_keyCallback(static_cast<int>(wParam), false);
            }
            return 0;

        case WM_LBUTTONDOWN:
            if (window->m_mouseButtonCallback) {
                window->m_mouseButtonCallback(VK_LBUTTON, true);
            }
            return 0;

        case WM_LBUTTONUP:
            if (window->m_mouseButtonCallback) {
                window->m_mouseButtonCallback(VK_LBUTTON, false);
            }
            return 0;

        case WM_RBUTTONDOWN:
            if (window->m_mouseButtonCallback) {
                window->m_mouseButtonCallback(VK_RBUTTON, true);
            }
            return 0;

        case WM_RBUTTONUP:
            if (window->m_mouseButtonCallback) {
                window->m_mouseButtonCallback(VK_RBUTTON, false);
            }
            return 0;

        case WM_MBUTTONDOWN:
            if (window->m_mouseButtonCallback) {
                window->m_mouseButtonCallback(VK_MBUTTON, true);
            }
            return 0;

        case WM_MBUTTONUP:
            if (window->m_mouseButtonCallback) {
                window->m_mouseButtonCallback(VK_MBUTTON, false);
            }
            return 0;

        case WM_MOUSEMOVE: {
            int x = GET_X_LPARAM(lParam);
            int y = GET_Y_LPARAM(lParam);
            if (window->m_mouseMoveCallback) {
                window->m_mouseMoveCallback(x, y);
            }
            return 0;
        }

        case WM_PAINT: {
            // Just validate the window region without GDI drawing.
            // DirectX manages its own rendering via the swap chain, so any
            // GDI fill here would overwrite the DirectX output and cause
            // the game play window to appear blank.
            PAINTSTRUCT ps;
            BeginPaint(hwnd, &ps);
            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_ERASEBKGND:
            return 1;

        case WM_DESTROY:
            window->m_shouldClose = true;
            return 0;
        }
    }

    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

} // namespace fresh

#endif // _WIN32
