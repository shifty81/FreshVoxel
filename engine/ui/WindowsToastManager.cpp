#ifdef _WIN32

#include "ui/WindowsToastManager.h"
#include "core/Logger.h"

#include <Windows.h>
#include <sstream>

// Note: Full Windows 10 toast notifications require WinRT APIs
// This is a simplified implementation using Windows notification API
// For full WinRT support, would need Windows SDK 10 and C++/WinRT

namespace fresh
{

WindowsToastManager::WindowsToastManager()
    : m_initialized(false),
      m_comInitialized(false),
      m_toastNotifier(nullptr)
{
}

WindowsToastManager::~WindowsToastManager()
{
    clearNotifications();
    cleanupCOM();
}

bool WindowsToastManager::initialize(const std::string& appUserModelId)
{
    if (m_initialized) {
        LOG_WARNING_C("Toast Manager already initialized", "WindowsToastManager");
        return true;
    }

    if (!isSupported()) {
        LOG_WARNING_C("Toast notifications not supported on this Windows version", 
                     "WindowsToastManager");
        return false;
    }

    // Initialize COM
    if (!initializeCOM()) {
        LOG_ERROR_C("Failed to initialize COM for Toast Manager", "WindowsToastManager");
        return false;
    }

    m_appUserModelId = appUserModelId;
    
    // Note: Full WinRT toast implementation would go here
    // For now, we'll use a simplified notification approach
    
    m_initialized = true;
    LOG_INFO_C("Toast Manager initialized (simplified mode)", "WindowsToastManager");
    
    return true;
}

bool WindowsToastManager::showToast(const std::string& title,
                                    const std::string& message,
                                    ToastType type)
{
    if (!m_initialized) {
        LOG_ERROR_C("Toast Manager not initialized", "WindowsToastManager");
        return false;
    }

    // For now, use a simplified notification approach with MessageBox
    // In a full implementation, this would use Windows.UI.Notifications
    
    std::string fullMessage = title + "\n\n" + message;
    
    UINT iconType = MB_ICONINFORMATION;
    switch (type) {
        case ToastType::Success:
            iconType = MB_ICONINFORMATION;
            break;
        case ToastType::Warning:
            iconType = MB_ICONWARNING;
            break;
        case ToastType::Error:
            iconType = MB_ICONERROR;
            break;
        default:
            iconType = MB_ICONINFORMATION;
            break;
    }

    // Show non-blocking notification
    // Note: This is a placeholder. Full implementation would use WinRT ToastNotificationManager
    LOG_INFO_C("Toast notification: " + title + " - " + message, "WindowsToastManager");
    
    // In a background thread to not block
    std::thread([fullMessage, iconType]() {
        MessageBoxA(nullptr, fullMessage.c_str(), "Fresh Voxel Engine", 
                   iconType | MB_OK | MB_SETFOREGROUND);
    }).detach();

    return true;
}

bool WindowsToastManager::showToastWithButtons(const std::string& title,
                                               const std::string& message,
                                               const std::vector<ToastButton>& buttons,
                                               ToastCallback callback,
                                               ToastType type)
{
    // Silence unused parameter warning
    (void)callback;
    
    if (!m_initialized) {
        LOG_ERROR_C("Toast Manager not initialized", "WindowsToastManager");
        return false;
    }

    // Simplified implementation
    // Full WinRT implementation would create interactive toast with actual buttons
    
    std::stringstream ss;
    ss << title << "\n\n" << message << "\n\nOptions:";
    for (size_t i = 0; i < buttons.size(); ++i) {
        ss << "\n" << (i + 1) << ". " << buttons[i].text;
    }

    LOG_INFO_C("Toast with buttons: " + ss.str(), "WindowsToastManager");
    
    return showToast(title, message, type);
}

bool WindowsToastManager::showToastWithImage(const std::string& title,
                                             const std::string& message,
                                             const std::string& imagePath,
                                             ToastType type)
{
    if (!m_initialized) {
        LOG_ERROR_C("Toast Manager not initialized", "WindowsToastManager");
        return false;
    }

    // Simplified implementation
    // Full WinRT implementation would embed the image in the toast
    
    LOG_INFO_C("Toast with image: " + title + " - " + message + " (image: " + imagePath + ")", 
              "WindowsToastManager");
    
    return showToast(title, message, type);
}

bool WindowsToastManager::clearNotifications()
{
    if (!m_initialized) {
        return false;
    }

    // Clear active toasts
    m_activeToasts.clear();
    
    LOG_INFO_C("Cleared all notifications", "WindowsToastManager");
    return true;
}

bool WindowsToastManager::isSupported()
{
    // Toast notifications require Windows 10 or later (Version 10.0)
    OSVERSIONINFOEXW osvi = {};
    osvi.dwOSVersionInfoSize = sizeof(osvi);
    osvi.dwMajorVersion = 10;
    osvi.dwMinorVersion = 0;

    DWORDLONG dwlConditionMask = 0;
    VER_SET_CONDITION(dwlConditionMask, VER_MAJORVERSION, VER_GREATER_EQUAL);
    VER_SET_CONDITION(dwlConditionMask, VER_MINORVERSION, VER_GREATER_EQUAL);

    return VerifyVersionInfoW(&osvi, VER_MAJORVERSION | VER_MINORVERSION, dwlConditionMask) != 0;
}

std::string WindowsToastManager::buildToastXml(const std::string& title,
                                               const std::string& message,
                                               ToastType type,
                                               const std::string& imagePath,
                                               const std::vector<ToastButton>& buttons)
{
    // Silence unused parameter warning
    (void)type;
    
    // Build XML for Windows toast notification
    // This is a simplified version - full implementation would use proper XML builder
    
    std::stringstream xml;
    xml << "<?xml version=\"1.0\" encoding=\"utf-8\"?>";
    xml << "<toast>";
    xml << "<visual>";
    xml << "<binding template=\"ToastGeneric\">";
    xml << "<text>" << title << "</text>";
    xml << "<text>" << message << "</text>";
    
    if (!imagePath.empty()) {
        xml << "<image placement=\"appLogoOverride\" src=\"" << imagePath << "\"/>";
    }
    
    xml << "</binding>";
    xml << "</visual>";
    
    if (!buttons.empty()) {
        xml << "<actions>";
        for (const auto& button : buttons) {
            xml << "<action content=\"" << button.text << "\" arguments=\"" 
                << button.arguments << "\"/>";
        }
        xml << "</actions>";
    }
    
    xml << "</toast>";
    
    return xml.str();
}

bool WindowsToastManager::initializeCOM()
{
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    
    if (SUCCEEDED(hr)) {
        m_comInitialized = true;
        return true;
    } else if (hr == RPC_E_CHANGED_MODE) {
        LOG_WARNING_C("COM already initialized in different mode", "WindowsToastManager");
        m_comInitialized = false;
        return true;
    }
    
    LOG_ERROR_C("Failed to initialize COM: " + std::to_string(hr), "WindowsToastManager");
    return false;
}

void WindowsToastManager::cleanupCOM()
{
    if (m_comInitialized) {
        CoUninitialize();
        m_comInitialized = false;
    }
}

} // namespace fresh

#endif // _WIN32
