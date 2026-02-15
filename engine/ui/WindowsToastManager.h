#pragma once

#ifdef _WIN32

#include <string>
#include <functional>
#include <memory>

namespace fresh
{

/**
 * @brief Manages Windows Toast Notifications (Windows 10/11)
 * 
 * Provides native Windows toast notifications that appear in the Action Center.
 * Requires Windows 10 or later.
 */
class WindowsToastManager
{
public:
    /**
     * @brief Toast notification types
     */
    enum class ToastType
    {
        Info,       // Informational notification
        Success,    // Success notification (green)
        Warning,    // Warning notification (yellow)
        Error       // Error notification (red)
    };

    /**
     * @brief Toast button definition
     */
    struct ToastButton
    {
        std::string text;
        std::string arguments;  // Arguments passed when button is clicked
    };

    /**
     * @brief Callback function type for toast interactions
     * @param arguments The arguments from the clicked button or action
     */
    using ToastCallback = std::function<void(const std::string& arguments)>;

    WindowsToastManager();
    ~WindowsToastManager();

    /**
     * @brief Initialize the toast manager
     * @param appUserModelId Application User Model ID (required for toasts)
     * @return true if successful
     */
    bool initialize(const std::string& appUserModelId);

    /**
     * @brief Show a simple toast notification
     * @param title Notification title
     * @param message Notification message
     * @param type Notification type (default: Info)
     * @return true if successful
     */
    bool showToast(const std::string& title,
                   const std::string& message,
                   ToastType type = ToastType::Info);

    /**
     * @brief Show a toast notification with buttons
     * @param title Notification title
     * @param message Notification message
     * @param buttons Vector of buttons to show
     * @param callback Callback function for button clicks
     * @param type Notification type (default: Info)
     * @return true if successful
     */
    bool showToastWithButtons(const std::string& title,
                             const std::string& message,
                             const std::vector<ToastButton>& buttons,
                             ToastCallback callback,
                             ToastType type = ToastType::Info);

    /**
     * @brief Show a toast notification with an image
     * @param title Notification title
     * @param message Notification message
     * @param imagePath Path to image file
     * @param type Notification type (default: Info)
     * @return true if successful
     */
    bool showToastWithImage(const std::string& title,
                           const std::string& message,
                           const std::string& imagePath,
                           ToastType type = ToastType::Info);

    /**
     * @brief Clear all notifications from the Action Center
     * @return true if successful
     */
    bool clearNotifications();

    /**
     * @brief Check if toast notifications are supported on this system
     * @return true if Windows 10 or later
     */
    static bool isSupported();

    /**
     * @brief Check if the app is registered for notifications
     */
    bool isRegistered() const { return m_initialized; }

private:
    bool initializeCOM();
    void cleanupCOM();
    std::string buildToastXml(const std::string& title,
                             const std::string& message,
                             ToastType type,
                             const std::string& imagePath = "",
                             const std::vector<ToastButton>& buttons = {});
    
    std::string m_appUserModelId;
    bool m_initialized;
    bool m_comInitialized;
    void* m_toastNotifier;  // IToastNotifier*
    std::vector<std::pair<void*, ToastCallback>> m_activeToasts;  // IToastNotification*, callback
};

} // namespace fresh

#endif // _WIN32
