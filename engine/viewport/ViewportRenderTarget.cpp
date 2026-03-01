#include "viewport/ViewportRenderTarget.h"

#include "core/Logger.h"
#include "renderer/RenderContext.h"

namespace fresh
{

ViewportRenderTarget::ViewportRenderTarget() = default;

ViewportRenderTarget::~ViewportRenderTarget()
{
    shutdown();
}

ViewportRenderTarget::ViewportRenderTarget(ViewportRenderTarget&& other) noexcept
    : m_windowHandle(other.m_windowHandle)
    , m_renderContext(other.m_renderContext)
    , m_width(other.m_width)
    , m_height(other.m_height)
    , m_state(other.m_state)
    , m_bound(other.m_bound)
{
    other.m_windowHandle = nullptr;
    other.m_renderContext = nullptr;
    other.m_state = RenderTargetState::Uninitialized;
    other.m_bound = false;
}

ViewportRenderTarget& ViewportRenderTarget::operator=(ViewportRenderTarget&& other) noexcept
{
    if (this != &other) {
        shutdown();
        m_windowHandle = other.m_windowHandle;
        m_renderContext = other.m_renderContext;
        m_width = other.m_width;
        m_height = other.m_height;
        m_state = other.m_state;
        m_bound = other.m_bound;

        other.m_windowHandle = nullptr;
        other.m_renderContext = nullptr;
        other.m_state = RenderTargetState::Uninitialized;
        other.m_bound = false;
    }
    return *this;
}

bool ViewportRenderTarget::initialize(void* windowHandle, IRenderContext* renderContext)
{
    if (!renderContext) {
        Logger::error("ViewportRenderTarget::initialize - null render context");
        m_state = RenderTargetState::Invalid;
        return false;
    }

    m_windowHandle = windowHandle;
    m_renderContext = renderContext;

    // If a window handle is provided, set it on the render context
    // so the swap chain targets this specific window
    if (m_windowHandle) {
        if (!m_renderContext->setViewportWindow(m_windowHandle)) {
            Logger::warn("ViewportRenderTarget::initialize - setViewportWindow failed, "
                         "rendering to default target");
        }
    }

    m_width = m_renderContext->getSwapchainWidth();
    m_height = m_renderContext->getSwapchainHeight();
    m_state = RenderTargetState::Ready;

    Logger::info("ViewportRenderTarget initialized ({}x{})", m_width, m_height);
    return true;
}

void ViewportRenderTarget::shutdown()
{
    if (m_state == RenderTargetState::Uninitialized) {
        return;
    }

    if (m_bound) {
        unbind();
    }

    m_windowHandle = nullptr;
    m_renderContext = nullptr;
    m_width = 0;
    m_height = 0;
    m_state = RenderTargetState::Uninitialized;
}

bool ViewportRenderTarget::bind()
{
    if (m_state != RenderTargetState::Ready) {
        return false;
    }

    if (!m_renderContext) {
        return false;
    }

    if (!m_renderContext->beginFrame()) {
        return false;
    }

    m_renderContext->setViewport(0, 0, m_width, m_height);
    m_bound = true;
    return true;
}

void ViewportRenderTarget::unbind()
{
    m_bound = false;
}

void ViewportRenderTarget::present()
{
    if (!m_renderContext) {
        return;
    }

    if (m_bound) {
        unbind();
    }

    m_renderContext->endFrame();
}

bool ViewportRenderTarget::resize(int width, int height)
{
    if (width <= 0 || height <= 0) {
        return false;
    }

    if (!m_renderContext) {
        m_state = RenderTargetState::Invalid;
        return false;
    }

    if (m_renderContext->recreateSwapChain(width, height)) {
        m_width = width;
        m_height = height;
        m_state = RenderTargetState::Ready;
        return true;
    }

    m_state = RenderTargetState::NeedsResize;
    return false;
}

float ViewportRenderTarget::getAspectRatio() const noexcept
{
    if (m_height <= 0) {
        return 16.0f / 9.0f; // Default aspect ratio
    }
    return static_cast<float>(m_width) / static_cast<float>(m_height);
}

} // namespace fresh
