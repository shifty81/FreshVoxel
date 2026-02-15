#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include <glm/glm.hpp>

#include "GraphicsAPI.h"

namespace fresh
{

class RenderBuffer;
class RenderTexture;
class RenderShader;
class RenderPipeline;

/**
 * @brief Abstract rendering context interface
 *
 * This is the main interface for all graphics API backends.
 * Each API (Vulkan, OpenGL, DX11, DX12) implements this interface.
 */
class IRenderContext
{
public:
    virtual ~IRenderContext() = default;

    /**
     * @brief Initialize rendering context
     * @param window Window to render to (void* to support both Window and Win32Window)
     * @return true if successful
     */
    virtual bool initialize(void* window) = 0;

    /**
     * @brief Shutdown and cleanup
     */
    virtual void shutdown() = 0;

    /**
     * @brief Begin rendering a frame
     * @return true if ready to render
     */
    virtual bool beginFrame() = 0;

    /**
     * @brief End frame and present
     */
    virtual void endFrame() = 0;

    /**
     * @brief Wait for GPU to finish all work
     */
    virtual void waitIdle() = 0;

    /**
     * @brief Set viewport
     */
    virtual void setViewport(int x, int y, int width, int height) = 0;

    /**
     * @brief Set scissor rectangle
     */
    virtual void setScissor(int x, int y, int width, int height) = 0;

    /**
     * @brief Clear color buffer
     */
    virtual void clearColor(float r, float g, float b, float a) = 0;

    /**
     * @brief Clear depth buffer
     */
    virtual void clearDepth(float depth) = 0;

    /**
     * @brief Get current graphics API
     */
    virtual GraphicsAPI getAPI() const = 0;

    /**
     * @brief Get API-specific device handle (for advanced usage)
     */
    virtual void* getNativeDevice() = 0;

    /**
     * @brief Get swapchain width
     */
    virtual int getSwapchainWidth() const = 0;

    /**
     * @brief Get swapchain height
     */
    virtual int getSwapchainHeight() const = 0;

    /**
     * @brief Set the viewport child window for rendering
     * @param viewportHwnd Handle to the viewport child window (cast from HWND on Windows)
     * @return true if successful
     * 
     * This allows rendering to a child window instead of the main window,
     * enabling proper editor layout where the 3D view is contained within a viewport panel.
     * The swap chain will be recreated to target the viewport window.
     */
    virtual bool setViewportWindow(void* viewportHwnd) = 0;

    /**
     * @brief Recreate the swap chain with new dimensions
     * @param width New width in pixels
     * @param height New height in pixels
     * @return true if successful
     * 
     * Call this when the viewport panel is resized to update the swap chain.
     */
    virtual bool recreateSwapChain(int width, int height) = 0;

    // Resource creation
    virtual std::shared_ptr<RenderBuffer> createVertexBuffer(const void* data, size_t size) = 0;
    virtual std::shared_ptr<RenderBuffer> createIndexBuffer(const void* data, size_t size) = 0;
    virtual std::shared_ptr<RenderBuffer> createUniformBuffer(size_t size) = 0;
    virtual std::shared_ptr<RenderTexture> createTexture(int width, int height,
                                                         const void* data) = 0;
    virtual std::shared_ptr<RenderShader> createShader(const std::string& vertexCode,
                                                       const std::string& fragmentCode) = 0;
};

/**
 * @brief Factory for creating render contexts
 */
class RenderContextFactory
{
public:
    /**
     * @brief Create render context for specified API
     * @param api Graphics API to use
     * @return Unique pointer to render context
     */
    static std::unique_ptr<IRenderContext> create(GraphicsAPI api);

    /**
     * @brief Create render context with auto-detection
     */
    static std::unique_ptr<IRenderContext> createBest();
};

/**
 * @brief Abstract buffer interface (vertex, index, uniform)
 */
class RenderBuffer
{
public:
    virtual ~RenderBuffer() = default;

    virtual void bind() = 0;
    virtual void unbind() = 0;
    virtual void updateData(const void* data, size_t size, size_t offset = 0) = 0;
    virtual size_t getSize() const = 0;
    virtual void* getNativeHandle() = 0;
};

/**
 * @brief Abstract texture interface
 */
class RenderTexture
{
public:
    virtual ~RenderTexture() = default;

    virtual void bind(int unit) = 0;
    virtual void unbind() = 0;
    virtual int getWidth() const = 0;
    virtual int getHeight() const = 0;
    virtual void* getNativeHandle() = 0;
};

/**
 * @brief Abstract shader interface
 */
class RenderShader
{
public:
    virtual ~RenderShader() = default;

    virtual void bind() = 0;
    virtual void unbind() = 0;
    virtual void setUniformInt(const std::string& name, int value) = 0;
    virtual void setUniformFloat(const std::string& name, float value) = 0;
    virtual void setUniformVec2(const std::string& name, const glm::vec2& value) = 0;
    virtual void setUniformVec3(const std::string& name, const glm::vec3& value) = 0;
    virtual void setUniformVec4(const std::string& name, const glm::vec4& value) = 0;
    virtual void setUniformMat4(const std::string& name, const glm::mat4& value) = 0;
    virtual void* getNativeHandle() = 0;
};

} // namespace fresh
