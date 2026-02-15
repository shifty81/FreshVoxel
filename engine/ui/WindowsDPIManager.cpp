#ifdef _WIN32

#include "ui/WindowsDPIManager.h"
#include "core/Logger.h"

#include <Windows.h>
#include <shellscalingapi.h>

#pragma comment(lib, "Shcore.lib")

namespace fresh
{

WindowsDPIManager::WindowsDPIManager()
    : m_initialized(false),
      m_currentMode(DPIAwarenessMode::Unaware),
      m_systemDPIScale(1.0f)
{
}

WindowsDPIManager::~WindowsDPIManager()
{
}

bool WindowsDPIManager::initialize(DPIAwarenessMode mode)
{
    if (m_initialized) {
        LOG_WARNING_C("DPI Manager already initialized", "WindowsDPIManager");
        return true;
    }

    if (!isHighDPISupported()) {
        LOG_WARNING_C("High DPI not supported on this Windows version", "WindowsDPIManager");
        m_initialized = true;
        m_currentMode = DPIAwarenessMode::Unaware;
        return true;
    }

    // Try to set DPI awareness context (Windows 10 1607+)
    if (!setDPIAwarenessContext(mode)) {
        // Fall back to older API (Windows 8.1+)
        if (!setDPIAwareness(mode)) {
            LOG_ERROR_C("Failed to set DPI awareness", "WindowsDPIManager");
            return false;
        }
    }

    m_currentMode = mode;
    m_systemDPIScale = static_cast<float>(getSystemDPI()) / 96.0f;
    m_initialized = true;

    LOG_INFO_C("DPI Manager initialized with mode: " + std::to_string(static_cast<int>(mode)) +
              ", system scale: " + std::to_string(m_systemDPIScale), 
              "WindowsDPIManager");

    return true;
}

float WindowsDPIManager::getDPIScaleForWindow(void* windowHandle)
{
    if (!windowHandle) {
        return m_systemDPIScale;
    }

    HWND hwnd = static_cast<HWND>(windowHandle);

    // Try GetDpiForWindow (Windows 10 1607+)
    typedef UINT(WINAPI* GetDpiForWindowFunc)(HWND);
    HMODULE user32 = GetModuleHandleW(L"user32.dll");
    if (user32) {
        auto getDpiForWindow = reinterpret_cast<GetDpiForWindowFunc>(
            GetProcAddress(user32, "GetDpiForWindow")
        );
        if (getDpiForWindow) {
            UINT dpi = getDpiForWindow(hwnd);
            return static_cast<float>(dpi) / 96.0f;
        }
    }

    // Fall back to GetDpiForMonitor (Windows 8.1+)
    HMONITOR monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
    if (monitor) {
        UINT dpiX = 96, dpiY = 96;
        HRESULT hr = GetDpiForMonitor(monitor, MDT_EFFECTIVE_DPI, &dpiX, &dpiY);
        if (SUCCEEDED(hr)) {
            return static_cast<float>(dpiX) / 96.0f;
        }
    }

    // Fall back to system DPI
    return m_systemDPIScale;
}

unsigned int WindowsDPIManager::getSystemDPI()
{
    HDC screen = GetDC(nullptr);
    int dpi = GetDeviceCaps(screen, LOGPIXELSX);
    ReleaseDC(nullptr, screen);
    return static_cast<unsigned int>(dpi);
}

int WindowsDPIManager::logicalToPhysical(int logicalPixels, float scale)
{
    return static_cast<int>(logicalPixels * scale + 0.5f);
}

int WindowsDPIManager::physicalToLogical(int physicalPixels, float scale)
{
    if (scale > 0.0f) {
        return static_cast<int>(physicalPixels / scale + 0.5f);
    }
    return physicalPixels;
}

bool WindowsDPIManager::isHighDPISupported()
{
    // High DPI awareness requires Windows 8.1 or later (Version 6.3)
    OSVERSIONINFOEXW osvi = {};
    osvi.dwOSVersionInfoSize = sizeof(osvi);
    osvi.dwMajorVersion = 6;
    osvi.dwMinorVersion = 3;

    DWORDLONG dwlConditionMask = 0;
    VER_SET_CONDITION(dwlConditionMask, VER_MAJORVERSION, VER_GREATER_EQUAL);
    VER_SET_CONDITION(dwlConditionMask, VER_MINORVERSION, VER_GREATER_EQUAL);

    return VerifyVersionInfoW(&osvi, VER_MAJORVERSION | VER_MINORVERSION, dwlConditionMask) != 0;
}

bool WindowsDPIManager::setDPIAwarenessContext(DPIAwarenessMode mode)
{
    // SetProcessDpiAwarenessContext is available on Windows 10 1607+
    typedef BOOL(WINAPI* SetProcessDpiAwarenessContextFunc)(DPI_AWARENESS_CONTEXT);
    
    HMODULE user32 = GetModuleHandleW(L"user32.dll");
    if (!user32) {
        return false;
    }

    auto setProcessDpiAwarenessContext = reinterpret_cast<SetProcessDpiAwarenessContextFunc>(
        GetProcAddress(user32, "SetProcessDpiAwarenessContext")
    );

    if (!setProcessDpiAwarenessContext) {
        return false;
    }

    DPI_AWARENESS_CONTEXT context;
    switch (mode) {
        case DPIAwarenessMode::Unaware:
            context = DPI_AWARENESS_CONTEXT_UNAWARE;
            break;
        case DPIAwarenessMode::SystemAware:
            context = DPI_AWARENESS_CONTEXT_SYSTEM_AWARE;
            break;
        case DPIAwarenessMode::PerMonitorAware:
            context = DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE;
            break;
        case DPIAwarenessMode::PerMonitorAwareV2:
            context = DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2;
            break;
        default:
            context = DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2;
            break;
    }

    BOOL result = setProcessDpiAwarenessContext(context);
    if (result) {
        LOG_INFO_C("DPI awareness context set successfully", "WindowsDPIManager");
        return true;
    }

    DWORD error = GetLastError();
    LOG_WARNING_C("Failed to set DPI awareness context: " + std::to_string(error), 
                 "WindowsDPIManager");
    return false;
}

bool WindowsDPIManager::setDPIAwareness(DPIAwarenessMode mode)
{
    // SetProcessDpiAwareness is available on Windows 8.1+
    PROCESS_DPI_AWARENESS awareness;
    switch (mode) {
        case DPIAwarenessMode::Unaware:
            awareness = PROCESS_DPI_UNAWARE;
            break;
        case DPIAwarenessMode::SystemAware:
            awareness = PROCESS_SYSTEM_DPI_AWARE;
            break;
        case DPIAwarenessMode::PerMonitorAware:
        case DPIAwarenessMode::PerMonitorAwareV2:
            awareness = PROCESS_PER_MONITOR_DPI_AWARE;
            break;
        default:
            awareness = PROCESS_PER_MONITOR_DPI_AWARE;
            break;
    }

    HRESULT hr = SetProcessDpiAwareness(awareness);
    if (SUCCEEDED(hr)) {
        LOG_INFO_C("DPI awareness set successfully (legacy API)", "WindowsDPIManager");
        return true;
    }

    if (hr == E_ACCESSDENIED) {
        LOG_WARNING_C("DPI awareness already set (possibly by manifest)", "WindowsDPIManager");
        return true; // Not an error if already set
    }

    LOG_ERROR_C("Failed to set DPI awareness: " + std::to_string(hr), "WindowsDPIManager");
    return false;
}

} // namespace fresh

#endif // _WIN32
