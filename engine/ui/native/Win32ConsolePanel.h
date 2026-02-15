#pragma once

#ifdef _WIN32

// Prevent Windows.h from defining min/max macros
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>
#include <richedit.h>
#include <string>
#include <memory>
#include <vector>
#include <functional>
#include "ui/native/Win32Panel.h"
#include "core/Logger.h" // For ILogListener

namespace fresh
{

/**
 * @brief Native Win32 Console Panel using RichEdit control
 * 
 * Provides a professional native Windows console interface with:
 * - RichEdit control for colored log output
 * - Command input field
 * - Filter buttons (Info, Warning, Error)
 * - Clear and auto-scroll buttons
 * - Unreal Engine-like dark theme styling
 * - Automatic integration with Logger system
 */
class Win32ConsolePanel : public Win32Panel, public ILogListener
{
public:
    enum class MessageType {
        Info,
        Warning,
        Error
    };

    struct ConsoleMessage {
        MessageType type;
        std::string timestamp;
        std::string text;
    };

    using CommandCallback = std::function<void(const std::string&)>;

    Win32ConsolePanel();
    virtual ~Win32ConsolePanel();

    /**
     * @brief Initialize the console panel (simplified interface)
     * @return true if successful
     */
    bool initialize();

    /**
     * @brief Initialize the console panel
     * @param parent Parent window handle
     * @param x X position
     * @param y Y position
     * @param width Panel width
     * @param height Panel height
     * @return true if successful
     */
    bool create(HWND parent, int x, int y, int width, int height);

    /**
     * @brief Add a message to the console
     * @param type Message type (Info, Warning, Error)
     * @param message Message text
     */
    void addMessage(MessageType type, const std::string& message);

    /**
     * @brief Clear all messages
     */
    void clear();

    /**
     * @brief Set auto-scroll behavior
     * @param enabled true to auto-scroll to bottom on new messages
     */
    void setAutoScroll(bool enabled) { m_autoScroll = enabled; }

    /**
     * @brief Set message type filtering
     */
    void setFilterInfo(bool show) { m_filterInfo = show; updateDisplay(); }
    void setFilterWarning(bool show) { m_filterWarning = show; updateDisplay(); }
    void setFilterError(bool show) { m_filterError = show; updateDisplay(); }

    /**
     * @brief Set callback for command execution
     */
    void setCommandCallback(CommandCallback callback) { m_commandCallback = callback; }

    // ILogListener interface
    void onLogMessage(LogLevel level, const std::string& message, const std::string& component) override;

protected:
    void onCreate() override;
    void onResize(int width, int height) override;
    void onDestroy() override;
    bool handleMessage(UINT msg, WPARAM wParam, LPARAM lParam, LRESULT& result) override;

private:
    // Helper functions
    void createControls();
    void updateDisplay();
    void executeCommand();
    void appendToRichEdit(const std::string& text, COLORREF color);
    std::string formatTimestamp() const;
    COLORREF getMessageColor(MessageType type) const;

    // Child controls
    HWND m_richEdit;          // RichEdit control for log display
    HWND m_commandEdit;       // Edit control for command input
    HWND m_clearButton;       // Clear button
    HWND m_autoScrollCheck;   // Auto-scroll checkbox
    HWND m_filterInfoCheck;   // Filter Info checkbox
    HWND m_filterWarningCheck; // Filter Warning checkbox
    HWND m_filterErrorCheck;  // Filter Error checkbox

    // State
    std::vector<ConsoleMessage> m_messages;
    bool m_autoScroll;
    bool m_filterInfo;
    bool m_filterWarning;
    bool m_filterError;
    int m_maxMessages;
    CommandCallback m_commandCallback;

    // Layout constants
    static constexpr int TOOLBAR_HEIGHT = 30;
    static constexpr int COMMAND_HEIGHT = 25;
    static constexpr int BUTTON_WIDTH = 60;
    static constexpr int CHECKBOX_WIDTH = 80;
    static constexpr int MARGIN = 5;
};

} // namespace fresh

#endif // _WIN32
