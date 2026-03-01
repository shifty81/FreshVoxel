#pragma once

#include <cstdint>
#include <memory>

#include "renderer/GraphicsAPI.h"

namespace fresh
{

class IRenderContext;

/**
 * @brief Viewport render target state
 *
 * Tracks whether the render target is valid and ready for rendering.
 */
enum class RenderTargetState {
    Uninitialized, ///< Not yet created
    Ready,         ///< Valid and ready to render
    NeedsResize,   ///< Needs swap chain recreation due to resize
    Invalid        ///< In an error state
};

/**
 * @brief Abstraction for a viewport's render target
 *
 * Per ENGINE.md: "Nothing renders unless a ViewportRenderTarget is bound."
 * Each viewport has its own render target and swapchain. The main window
 * backbuffer is for editor composite only.
 *
 * ViewportRenderTarget wraps a render surface (backed by an IRenderContext
 * swap chain) that a ViewportContext binds before issuing draw calls.
 */
class ViewportRenderTarget
{
public:
    ViewportRenderTarget();
    ~ViewportRenderTarget();

    // Non-copyable, movable
    ViewportRenderTarget(const ViewportRenderTarget&) = delete;
    ViewportRenderTarget& operator=(const ViewportRenderTarget&) = delete;
    ViewportRenderTarget(ViewportRenderTarget&&) noexcept;
    ViewportRenderTarget& operator=(ViewportRenderTarget&&) noexcept;

    /**
     * @brief Initialize the render target with a window handle and render context
     * @param windowHandle Native window handle (HWND on Windows)
     * @param renderContext Render context that owns the swap chain
     * @return true if initialization succeeded
     */
    bool initialize(void* windowHandle, IRenderContext* renderContext);

    /**
     * @brief Shutdown and release GPU resources
     */
    void shutdown();

    /**
     * @brief Bind this render target for rendering
     *
     * Must be called before any draw calls targeting this viewport.
     * @return true if the render target is bound and ready
     */
    bool bind();

    /**
     * @brief Unbind this render target after rendering
     */
    void unbind();

    /**
     * @brief Present the rendered frame
     *
     * Swaps buffers / presents the swap chain for this viewport.
     */
    void present();

    /**
     * @brief Resize the render target
     * @param width New width in pixels
     * @param height New height in pixels
     * @return true if resize succeeded
     */
    bool resize(int width, int height);

    /**
     * @brief Get current width
     */
    [[nodiscard]] int getWidth() const noexcept { return m_width; }

    /**
     * @brief Get current height
     */
    [[nodiscard]] int getHeight() const noexcept { return m_height; }

    /**
     * @brief Get aspect ratio (width / height)
     */
    [[nodiscard]] float getAspectRatio() const noexcept;

    /**
     * @brief Get render target state
     */
    [[nodiscard]] RenderTargetState getState() const noexcept { return m_state; }

    /**
     * @brief Check if the render target is ready for rendering
     */
    [[nodiscard]] bool isReady() const noexcept { return m_state == RenderTargetState::Ready; }

    /**
     * @brief Get the underlying render context
     */
    [[nodiscard]] IRenderContext* getRenderContext() const noexcept { return m_renderContext; }

    /**
     * @brief Get the native window handle
     */
    [[nodiscard]] void* getWindowHandle() const noexcept { return m_windowHandle; }

private:
    void* m_windowHandle = nullptr;
    IRenderContext* m_renderContext = nullptr;
    int m_width = 0;
    int m_height = 0;
    RenderTargetState m_state = RenderTargetState::Uninitialized;
    bool m_bound = false;
};

} // namespace fresh
