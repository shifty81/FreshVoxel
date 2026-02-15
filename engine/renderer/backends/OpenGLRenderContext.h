#pragma once

#include "renderer/RenderContext.h"

#if defined(FRESH_OPENGL_SUPPORT) && defined(FRESH_GLEW_AVAILABLE)

    #include <memory>
    #include <vector>

    #include <GL/glew.h>

namespace fresh
{

class Window;

/**
 * @brief OpenGL implementation of render context
 *
 * Provides cross-platform OpenGL 4.5+ rendering support for
 * Linux, macOS, and Windows.
 */
class OpenGLRenderContext : public IRenderContext
{
public:
    OpenGLRenderContext();
    ~OpenGLRenderContext() override;

    bool initialize(void* window) override;
    void shutdown() override;
    bool beginFrame() override;
    void endFrame() override;
    void waitIdle() override;

    void setViewport(int x, int y, int width, int height) override;
    void setScissor(int x, int y, int width, int height) override;
    void clearColor(float r, float g, float b, float a) override;
    void clearDepth(float depth) override;

    GraphicsAPI getAPI() const override
    {
        return GraphicsAPI::OpenGL;
    }
    void* getNativeDevice() override
    {
        return nullptr;
    } // OpenGL doesn't have a device object

    int getSwapchainWidth() const override
    {
        return width;
    }
    int getSwapchainHeight() const override
    {
        return height;
    }

    /**
     * @brief Set the viewport child window for rendering (stub for OpenGL)
     * @param viewportHwnd Handle to viewport window (not used in OpenGL)
     * @return true (no-op on OpenGL)
     */
    bool setViewportWindow(void* viewportHwnd) override
    {
        (void)viewportHwnd;
        return true; // OpenGL doesn't need this - it renders to the context's window
    }

    /**
     * @brief Recreate the swap chain (stub for OpenGL)
     * @param newWidth New width (OpenGL resizes automatically)
     * @param newHeight New height (OpenGL resizes automatically)
     * @return true if dimensions are valid
     */
    bool recreateSwapChain(int newWidth, int newHeight) override
    {
        if (newWidth <= 0 || newHeight <= 0) {
            return false; // Invalid dimensions
        }
        width = newWidth;
        height = newHeight;
        return true; // OpenGL doesn't have a swap chain to recreate
    }

    std::shared_ptr<RenderBuffer> createVertexBuffer(const void* data, size_t size) override;
    std::shared_ptr<RenderBuffer> createIndexBuffer(const void* data, size_t size) override;
    std::shared_ptr<RenderBuffer> createUniformBuffer(size_t size) override;
    std::shared_ptr<RenderTexture> createTexture(int width, int height, const void* data) override;
    std::shared_ptr<RenderShader> createShader(const std::string& vertexCode,
                                               const std::string& fragmentCode) override;

private:
    bool initializeGLEW();
    void checkGLErrors(const char* context);

    void* window = nullptr;  // Points to Window or Win32Window
    int width = 0;
    int height = 0;

    // OpenGL state
    GLuint defaultVAO = 0; // Default Vertex Array Object
    glm::vec4 clearColorValue = glm::vec4(0.1f, 0.1f, 0.1f, 1.0f);
    float clearDepthValue = 1.0f;
};

} // namespace fresh

#endif // FRESH_OPENGL_SUPPORT && FRESH_GLEW_AVAILABLE
