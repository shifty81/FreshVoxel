#include "core/Window.h"

#include <iostream>

#include <GLFW/glfw3.h>

// Platform-specific includes for native window handle
#ifdef _WIN32
    #define GLFW_EXPOSE_NATIVE_WIN32
    #include <GLFW/glfw3native.h>
#endif

namespace fresh
{

Window::Window(uint32_t width, uint32_t height, const std::string& title)
    : m_window(nullptr),
      m_width(width),
      m_height(height),
      m_title(title),
      m_framebufferResized(false)
{
}

Window::~Window()
{
    if (m_window) {
        glfwDestroyWindow(m_window);
    }
    glfwTerminate();
}

bool Window::initialize(bool useOpenGL)
{
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }

    if (useOpenGL) {
        // Create OpenGL context with compatibility profile for legacy rendering
        // Using 3.3 compatibility provides both modern and legacy OpenGL features
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
        std::cout << "Creating window with OpenGL 3.3 context (compatibility)" << std::endl;
    } else {
        // Don't create OpenGL context (for Vulkan/DirectX)
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        std::cout << "Creating window without API context" << std::endl;
    }

    // Create window
    m_window = glfwCreateWindow(m_width, m_height, m_title.c_str(), nullptr, nullptr);
    if (!m_window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }

    if (useOpenGL) {
        // Make the OpenGL context current
        glfwMakeContextCurrent(m_window);
        std::cout << "OpenGL context made current" << std::endl;
    }

    glfwSetWindowUserPointer(m_window, this);
    glfwSetFramebufferSizeCallback(m_window, framebufferResizeCallback);

    return true;
}

void Window::pollEvents()
{
    glfwPollEvents();
}

bool Window::shouldClose() const
{
    return glfwWindowShouldClose(m_window);
}

void Window::setSize(uint32_t width, uint32_t height)
{
    m_width = width;
    m_height = height;
    if (m_window) {
        glfwSetWindowSize(m_window, width, height);
    }
}

void Window::framebufferResizeCallback(GLFWwindow* window, int width, int height)
{
    auto app = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
    app->m_framebufferResized = true;
    app->m_width = static_cast<uint32_t>(width);
    app->m_height = static_cast<uint32_t>(height);
}

void Window::swapBuffers()
{
    if (m_window) {
        glfwSwapBuffers(m_window);
    }
}

void* Window::getNativeWindowHandle() const
{
#ifdef _WIN32
    if (m_window) {
        return (void*)glfwGetWin32Window(m_window);
    }
#endif
    return nullptr;
}

} // namespace fresh
