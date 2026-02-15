#pragma once

#ifdef _WIN32

#include <cstdint>
#include <string>
#include <functional>
#include <Windows.h>

namespace fresh
{

/**
 * @brief Separate window for gameplay testing (Play-in-Editor)
 *
 * When the user presses "Play" in the editor, this window opens to run
 * the game in isolation. Closing the window returns to the editor.
 * This approach cleanly separates editor and game contexts, similar to
 * how professional game engines handle play testing.
 */
class GamePlayWindow
{
public:
    using CloseCallback = std::function<void()>;
    using KeyCallback = std::function<void(int vkCode, bool isDown)>;
    using MouseMoveCallback = std::function<void(int x, int y)>;
    using MouseButtonCallback = std::function<void(int button, bool isDown)>;

    GamePlayWindow();
    ~GamePlayWindow();

    // Prevent copying
    GamePlayWindow(const GamePlayWindow&) = delete;
    GamePlayWindow& operator=(const GamePlayWindow&) = delete;

    /**
     * @brief Create and show the game play window
     * @param width Window width
     * @param height Window height
     * @param title Window title
     * @return true if window was created successfully
     */
    bool create(uint32_t width, uint32_t height, const std::string& title);

    /**
     * @brief Close and destroy the window
     */
    void close();

    /**
     * @brief Poll window messages
     * @return true if window is still open, false if closed
     */
    bool pollEvents();

    /**
     * @brief Check if the window is open
     */
    bool isOpen() const { return m_hwnd != nullptr && !m_shouldClose; }

    /**
     * @brief Get the native window handle
     */
    HWND getHandle() const { return m_hwnd; }

    /**
     * @brief Get window client area width
     */
    uint32_t getWidth() const { return m_width; }

    /**
     * @brief Get window client area height
     */
    uint32_t getHeight() const { return m_height; }

    /**
     * @brief Check if the window was resized since last check
     */
    bool wasResized() const { return m_resized; }

    /**
     * @brief Reset the resize flag
     */
    void resetResizedFlag() { m_resized = false; }

    /**
     * @brief Set callback for when the window is closed
     */
    void setCloseCallback(CloseCallback callback) { m_closeCallback = std::move(callback); }

    /**
     * @brief Set keyboard input callback
     */
    void setKeyCallback(KeyCallback callback) { m_keyCallback = std::move(callback); }

    /**
     * @brief Set mouse move callback
     */
    void setMouseMoveCallback(MouseMoveCallback callback) { m_mouseMoveCallback = std::move(callback); }

    /**
     * @brief Set mouse button callback
     */
    void setMouseButtonCallback(MouseButtonCallback callback) { m_mouseButtonCallback = std::move(callback); }

private:
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    bool registerWindowClass();

    HWND m_hwnd;
    HINSTANCE m_hinstance;
    uint32_t m_width;
    uint32_t m_height;
    bool m_shouldClose;
    bool m_resized;

    CloseCallback m_closeCallback;
    KeyCallback m_keyCallback;
    MouseMoveCallback m_mouseMoveCallback;
    MouseButtonCallback m_mouseButtonCallback;

    static constexpr const wchar_t* GAME_WINDOW_CLASS_NAME = L"FreshGamePlayWindow";
};

} // namespace fresh

#endif // _WIN32
