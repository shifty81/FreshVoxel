#pragma once

#include <cstdint>
#include <string>

struct GLFWwindow;

namespace fresh
{

/**
 * @brief Window management class using GLFW
 *
 * Handles window creation, event processing, and input management.
 */
class Window
{
public:
    Window(uint32_t width, uint32_t height, const std::string& title);
    ~Window();

    // Prevent copying
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    /**
     * @brief Initialize the window
     * @param useOpenGL If true, creates an OpenGL context. If false, uses no API (for
     * Vulkan/DirectX)
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
    bool shouldClose() const;

    /**
     * @brief Get the GLFW window handle
     * @return pointer to GLFWwindow
     */
    GLFWwindow* getHandle() const
    {
        return m_window;
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
     * @brief Swap front and back buffers (for OpenGL)
     */
    void swapBuffers();

    /**
     * @brief Get native window handle (HWND on Windows, etc.)
     * @return Pointer to the native window handle, or nullptr if not available
     */
    void* getNativeWindowHandle() const;

private:
    static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

private:
    GLFWwindow* m_window;
    uint32_t m_width;
    uint32_t m_height;
    std::string m_title;
    bool m_framebufferResized;
};

} // namespace fresh
