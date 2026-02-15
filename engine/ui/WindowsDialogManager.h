#pragma once

#ifdef _WIN32

#include <string>
#include <vector>
#include <functional>

namespace fresh
{

/**
 * @brief File filter for file dialogs
 */
struct FileFilter
{
    std::string description;
    std::string pattern; // e.g., "*.txt" or "*.png;*.jpg"
};

/**
 * @brief Message box button types
 */
enum class MessageBoxButtons
{
    OK,
    OKCancel,
    YesNo,
    YesNoCancel,
    RetryCancel,
    AbortRetryIgnore
};

/**
 * @brief Message box icon types
 */
enum class MessageBoxIcon
{
    None,
    Information,
    Warning,
    Error,
    Question
};

/**
 * @brief Message box result
 */
enum class MessageBoxResult
{
    OK,
    Cancel,
    Yes,
    No,
    Retry,
    Abort,
    Ignore
};

/**
 * @brief Manages native Windows dialogs
 * 
 * Provides Windows-native file dialogs, message boxes, and other
 * common dialogs that integrate seamlessly with the Windows UI.
 */
class WindowsDialogManager
{
public:
    WindowsDialogManager();
    ~WindowsDialogManager();

    /**
     * @brief Initialize the dialog manager
     * @param windowHandle Native window handle (HWND)
     * @return true if initialization was successful
     */
    bool initialize(void* windowHandle);

    /**
     * @brief Shutdown and cleanup
     */
    void shutdown();

    /**
     * @brief Show native Open File dialog
     * @param title Dialog title
     * @param filters File type filters
     * @param multiselect Allow multiple file selection
     * @return Selected file path(s), empty if cancelled
     */
    std::vector<std::string> showOpenFileDialog(
        const std::string& title,
        const std::vector<FileFilter>& filters = {},
        bool multiselect = false);

    /**
     * @brief Show native Save File dialog
     * @param title Dialog title
     * @param defaultFilename Default filename
     * @param filters File type filters
     * @return Selected file path, empty if cancelled
     */
    std::string showSaveFileDialog(
        const std::string& title,
        const std::string& defaultFilename = "",
        const std::vector<FileFilter>& filters = {});

    /**
     * @brief Show native folder browser dialog
     * @param title Dialog title
     * @return Selected folder path, empty if cancelled
     */
    std::string showFolderBrowserDialog(const std::string& title);

    /**
     * @brief Show native message box
     * @param title Dialog title
     * @param message Message text
     * @param buttons Button configuration
     * @param icon Icon type
     * @return User's button selection
     */
    MessageBoxResult showMessageBox(
        const std::string& title,
        const std::string& message,
        MessageBoxButtons buttons = MessageBoxButtons::OK,
        MessageBoxIcon icon = MessageBoxIcon::Information);

    /**
     * @brief Check if dialog manager is initialized
     * @return true if initialized
     */
    bool isInitialized() const { return m_initialized; }

    /**
     * @brief Set default directory for file dialogs
     * @param path Default directory path
     */
    void setDefaultDirectory(const std::string& path) { m_defaultDirectory = path; }

    /**
     * @brief Get default directory for file dialogs
     * @return Default directory path
     */
    const std::string& getDefaultDirectory() const { return m_defaultDirectory; }

private:
    std::wstring toWideString(const std::string& str);
    std::string toNarrowString(const std::wstring& wstr);
    std::wstring buildFilterString(const std::vector<FileFilter>& filters);

private:
    bool m_initialized;
    void* m_windowHandle; // HWND
    std::string m_defaultDirectory;
};

} // namespace fresh

#endif // _WIN32
