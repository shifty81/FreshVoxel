#pragma once

#ifdef _WIN32

#include <cstdint>
#include <string>
#include <functional>
#include <memory>
#include <Windows.h>

namespace fresh
{

class Win32MenuBar;
class Win32Toolbar;

/**
 * @brief Native Win32 window management class
 *
 * Handles window creation, event processing, and input management using
 * native Win32 APIs for a fully Windows-native GUI experience.
 */
class Win32Window
{
public:
    // Input callback types
    using KeyCallback = std::function<void(int vkCode, bool isDown)>;
    using MouseMoveCallback = std::function<void(int x, int y)>;
    using MouseButtonCallback = std::function<void(int button, bool isDown)>;
    
    Win32Window(uint32_t width, uint32_t height, const std::string& title);
    ~Win32Window();

    // Prevent copying
    Win32Window(const Win32Window&) = delete;
    Win32Window& operator=(const Win32Window&) = delete;

    /**
     * @brief Initialize the window
     * @param useOpenGL If true, creates an OpenGL context. If false, uses no API (for DirectX)
     * @return true if initialization was successful, false otherwise
     */
    bool initialize(bool useOpenGL = false);

    /**
     * @brief Poll window events
     */
    void pollEvents();

    /**
     * @brief Check if window should close
     * @return true if window should close, false otherwise
     */
    bool shouldClose() const
    {
        return m_shouldClose;
    }

    /**
     * @brief Get the native window handle (HWND)
     * @return HWND window handle
     */
    HWND getHandle() const
    {
        return m_hwnd;
    }

    /**
     * @brief Get window width
     * @return window width in pixels
     */
    uint32_t getWidth() const
    {
        return m_width;
    }

    /**
     * @brief Get window height
     * @return window height in pixels
     */
    uint32_t getHeight() const
    {
        return m_height;
    }

    /**
     * @brief Get window title
     * @return window title string
     */
    const std::string& getTitle() const
    {
        return m_title;
    }

    /**
     * @brief Set window size
     * @param width New width in pixels
     * @param height New height in pixels
     */
    void setSize(uint32_t width, uint32_t height);

    /**
     * @brief Swap front and back buffers (for OpenGL if applicable)
     */
    void swapBuffers();

    /**
     * @brief Get native window handle (same as getHandle for Win32)
     * @return Pointer to the native window handle
     */
    void* getNativeWindowHandle() const
    {
        return m_hwnd;
    }

    /**
     * @brief Get HINSTANCE module handle
     * @return HINSTANCE handle
     */
    HINSTANCE getInstance() const
    {
        return m_hinstance;
    }

    /**
     * @brief Check if framebuffer was resized
     * @return true if resized
     */
    bool wasFramebufferResized() const
    {
        return m_framebufferResized;
    }

    /**
     * @brief Reset framebuffer resized flag
     */
    void resetFramebufferResizedFlag()
    {
        m_framebufferResized = false;
    }

    /**
     * @brief Set keyboard input callback
     */
    void setKeyCallback(KeyCallback callback)
    {
        m_keyCallback = callback;
    }

    /**
     * @brief Set mouse move callback
     */
    void setMouseMoveCallback(MouseMoveCallback callback)
    {
        m_mouseMoveCallback = callback;
    }

    /**
     * @brief Set mouse button callback
     */
    void setMouseButtonCallback(MouseButtonCallback callback)
    {
        m_mouseButtonCallback = callback;
    }

    /**
     * @brief Get or create the native menu bar
     * @return Pointer to Win32MenuBar instance
     */
    Win32MenuBar* getMenuBar();

    /**
     * @brief Get or create the native toolbar
     * @return Pointer to Win32Toolbar instance
     */
    Win32Toolbar* getToolbar();

    /**
     * @brief Toggle between windowed and fullscreen mode
     */
    void toggleFullscreen();

    /**
     * @brief Check if window is in fullscreen mode
     * @return true if fullscreen
     */
    bool isFullscreen() const { return m_fullscreen; }

    /**
     * @brief Hide the window
     * 
     * Used when entering play mode to completely detach the game preview
     * from the editor window, preventing the game from rendering behind
     * the editor GUI and being visible through panel border gaps.
     */
    void hide();

    /**
     * @brief Show (restore) the window
     * 
     * Used when exiting play mode to restore the editor window to its
     * previous state after the game preview window is closed.
     */
    void show();

private:
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    bool registerWindowClass();
    bool createWindow();
    bool initializeOpenGL();

private:
    HWND m_hwnd;
    HINSTANCE m_hinstance;
    HDC m_hdc;
    HGLRC m_hglrc; // OpenGL rendering context (if used)
    
    uint32_t m_width;
    uint32_t m_height;
    std::string m_title;
    
    bool m_shouldClose;
    bool m_framebufferResized;
    bool m_useOpenGL;
    bool m_fullscreen;
    RECT m_windowedRect;       // Saved window rect for restoring from fullscreen
    DWORD m_windowedStyle;     // Saved window style for restoring from fullscreen
    
    // Input callbacks
    KeyCallback m_keyCallback;
    MouseMoveCallback m_mouseMoveCallback;
    MouseButtonCallback m_mouseButtonCallback;
    
    // Native UI components
    std::unique_ptr<Win32MenuBar> m_menuBar;
    std::unique_ptr<Win32Toolbar> m_toolbar;
    
    static constexpr const wchar_t* WINDOW_CLASS_NAME = L"FreshVoxelEngineWindow";
};

} // namespace fresh

#endif // _WIN32
