#ifdef _WIN32

#include "ui/WindowsTaskbarManager.h"
#include "core/Logger.h"

#include <windows.h>
#include <shobjidl.h>
#include <comdef.h>

namespace fresh
{

WindowsTaskbarManager::WindowsTaskbarManager()
    : m_initialized(false),
      m_windowHandle(nullptr),
      m_taskbarList(nullptr)
{
}

WindowsTaskbarManager::~WindowsTaskbarManager()
{
    shutdown();
}

bool WindowsTaskbarManager::initialize(void* windowHandle)
{
    if (m_initialized) {
        LOG_WARNING_C("WindowsTaskbarManager already initialized", "WindowsTaskbarManager");
        return true;
    }

    if (!windowHandle) {
        LOG_ERROR_C("Invalid window handle", "WindowsTaskbarManager");
        return false;
    }

    m_windowHandle = windowHandle;

    // Initialize COM
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(hr) && hr != RPC_E_CHANGED_MODE) {
        LOG_ERROR_C("Failed to initialize COM", "WindowsTaskbarManager");
        return false;
    }

    // Create ITaskbarList3 instance
    ITaskbarList3* taskbarList = nullptr;
    hr = CoCreateInstance(CLSID_TaskbarList, nullptr, CLSCTX_INPROC_SERVER,
                          IID_PPV_ARGS(&taskbarList));
    
    if (FAILED(hr)) {
        LOG_ERROR_C("Failed to create taskbar list instance", "WindowsTaskbarManager");
        return false;
    }

    hr = taskbarList->HrInit();
    if (FAILED(hr)) {
        taskbarList->Release();
        LOG_ERROR_C("Failed to initialize taskbar list", "WindowsTaskbarManager");
        return false;
    }

    m_taskbarList = taskbarList;
    m_initialized = true;

    LOG_INFO_C("Windows Taskbar Manager initialized", "WindowsTaskbarManager");
    return true;
}

void WindowsTaskbarManager::shutdown()
{
    if (!m_initialized) {
        return;
    }

    if (m_taskbarList) {
        ITaskbarList3* taskbarList = static_cast<ITaskbarList3*>(m_taskbarList);
        taskbarList->Release();
        m_taskbarList = nullptr;
    }

    m_initialized = false;
    m_windowHandle = nullptr;

    LOG_INFO_C("Windows Taskbar Manager shutdown", "WindowsTaskbarManager");
}

void WindowsTaskbarManager::setProgressState(TaskbarProgressState state)
{
    if (!m_initialized || !m_taskbarList) {
        return;
    }

    ITaskbarList3* taskbarList = static_cast<ITaskbarList3*>(m_taskbarList);
    HWND hwnd = static_cast<HWND>(m_windowHandle);

    TBPFLAG tbpFlag;
    switch (state) {
    case TaskbarProgressState::NoProgress:
        tbpFlag = TBPF_NOPROGRESS;
        break;
    case TaskbarProgressState::Indeterminate:
        tbpFlag = TBPF_INDETERMINATE;
        break;
    case TaskbarProgressState::Normal:
        tbpFlag = TBPF_NORMAL;
        break;
    case TaskbarProgressState::Error:
        tbpFlag = TBPF_ERROR;
        break;
    case TaskbarProgressState::Paused:
        tbpFlag = TBPF_PAUSED;
        break;
    default:
        tbpFlag = TBPF_NOPROGRESS;
        break;
    }

    taskbarList->SetProgressState(hwnd, tbpFlag);
}

void WindowsTaskbarManager::setProgressValue(uint64_t completed, uint64_t total)
{
    if (!m_initialized || !m_taskbarList) {
        return;
    }

    ITaskbarList3* taskbarList = static_cast<ITaskbarList3*>(m_taskbarList);
    HWND hwnd = static_cast<HWND>(m_windowHandle);

    taskbarList->SetProgressValue(hwnd, completed, total);
}

void WindowsTaskbarManager::flashTaskbar(bool flashUntilForeground)
{
    if (!m_initialized) {
        return;
    }

    HWND hwnd = static_cast<HWND>(m_windowHandle);

    FLASHWINFO fwi = {};
    fwi.cbSize = sizeof(FLASHWINFO);
    fwi.hwnd = hwnd;
    fwi.dwFlags = flashUntilForeground ? FLASHW_ALL | FLASHW_TIMERNOFG : FLASHW_ALL | FLASHW_TRAY;
    fwi.uCount = 3;  // Flash 3 times
    fwi.dwTimeout = 0; // Use default cursor blink rate

    FlashWindowEx(&fwi);
    LOG_INFO_C("Taskbar flashed", "WindowsTaskbarManager");
}

void WindowsTaskbarManager::setOverlayIcon(const std::string& iconPath, const std::string& description)
{
    if (!m_initialized || !m_taskbarList) {
        return;
    }

    // Load icon from file
    HICON hIcon = (HICON)LoadImageA(nullptr, iconPath.c_str(), IMAGE_ICON, 0, 0,
                                     LR_LOADFROMFILE | LR_DEFAULTSIZE);
    
    if (!hIcon) {
        LOG_WARNING_C("Failed to load overlay icon: " + iconPath, "WindowsTaskbarManager");
        return;
    }

    ITaskbarList3* taskbarList = static_cast<ITaskbarList3*>(m_taskbarList);
    HWND hwnd = static_cast<HWND>(m_windowHandle);

    // Convert description to wide string
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, description.c_str(), -1, nullptr, 0);
    std::wstring wDescription(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, description.c_str(), -1, &wDescription[0], size_needed);

    HRESULT hr = taskbarList->SetOverlayIcon(hwnd, hIcon, wDescription.c_str());
    
    DestroyIcon(hIcon);

    if (SUCCEEDED(hr)) {
        LOG_INFO_C("Overlay icon set: " + description, "WindowsTaskbarManager");
    } else {
        LOG_WARNING_C("Failed to set overlay icon", "WindowsTaskbarManager");
    }
}

void WindowsTaskbarManager::clearOverlayIcon()
{
    if (!m_initialized || !m_taskbarList) {
        return;
    }

    ITaskbarList3* taskbarList = static_cast<ITaskbarList3*>(m_taskbarList);
    HWND hwnd = static_cast<HWND>(m_windowHandle);

    taskbarList->SetOverlayIcon(hwnd, nullptr, L"");
    LOG_INFO_C("Overlay icon cleared", "WindowsTaskbarManager");
}

} // namespace fresh

#endif // _WIN32
