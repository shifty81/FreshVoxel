#include "viewport/ViewportContext.h"

#include "core/Logger.h"
#include "gameplay/Camera.h"
#include "renderer/RenderContext.h"

namespace fresh
{

ViewportContext::ViewportContext(const std::string& name, ViewportType type)
    : m_name(name)
    , m_type(type)
    , m_camera(std::make_unique<Camera>())
{
    configureCameraForType();
}

ViewportContext::~ViewportContext()
{
    shutdown();
}

ViewportContext::ViewportContext(ViewportContext&& other) noexcept
    : m_name(std::move(other.m_name))
    , m_type(other.m_type)
    , m_initialized(other.m_initialized)
    , m_camera(std::move(other.m_camera))
    , m_inputContext(std::move(other.m_inputContext))
    , m_renderTarget(std::move(other.m_renderTarget))
    , m_world(other.m_world)
{
    other.m_initialized = false;
    other.m_world = nullptr;
}

ViewportContext& ViewportContext::operator=(ViewportContext&& other) noexcept
{
    if (this != &other) {
        shutdown();
        m_name = std::move(other.m_name);
        m_type = other.m_type;
        m_initialized = other.m_initialized;
        m_camera = std::move(other.m_camera);
        m_inputContext = std::move(other.m_inputContext);
        m_renderTarget = std::move(other.m_renderTarget);
        m_world = other.m_world;

        other.m_initialized = false;
        other.m_world = nullptr;
    }
    return *this;
}

bool ViewportContext::initialize(IRenderContext* renderContext, void* windowHandle)
{
    if (!renderContext) {
        LOG_ERROR_C("ViewportContext::initialize '" + m_name + "' - null render context", "ViewportContext");
        return false;
    }

    // Initialize input context
    m_inputContext.initialize();

    // Initialize render target
    if (!m_renderTarget.initialize(windowHandle, renderContext)) {
        LOG_ERROR_C("ViewportContext::initialize '" + m_name + "' - render target init failed", "ViewportContext");
        return false;
    }

    // Update camera aspect ratio from render target
    if (m_camera) {
        m_camera->setAspectRatio(m_renderTarget.getAspectRatio());
    }

    m_initialized = true;
    LOG_INFO_C("ViewportContext '" + m_name + "' initialized (" +
               std::to_string(m_renderTarget.getWidth()) + "x" +
               std::to_string(m_renderTarget.getHeight()) + ", type=" +
               std::to_string(static_cast<int>(m_type)) + ")", "ViewportContext");
    return true;
}

void ViewportContext::shutdown()
{
    if (!m_initialized) {
        return;
    }

    m_renderTarget.shutdown();
    m_world = nullptr;
    m_initialized = false;

    Logger::getInstance().info("ViewportContext '" + m_name + "' shut down", "ViewportContext");
}

bool ViewportContext::beginFrame()
{
    if (!m_initialized) {
        return false;
    }

    // Update input context (clear per-frame state)
    m_inputContext.update();

    // Bind the render target
    if (!m_renderTarget.bind()) {
        return false;
    }

    return true;
}

void ViewportContext::endFrame()
{
    if (!m_initialized) {
        return;
    }

    m_renderTarget.present();
}

bool ViewportContext::resize(int width, int height)
{
    if (!m_initialized) {
        return false;
    }

    if (!m_renderTarget.resize(width, height)) {
        return false;
    }

    // Update camera aspect ratio
    if (m_camera) {
        m_camera->setAspectRatio(m_renderTarget.getAspectRatio());
    }

    Logger::getInstance().info("ViewportContext '" + m_name + "' resized to " + std::to_string(width) + "x" + std::to_string(height), "ViewportContext");
    return true;
}

void ViewportContext::setType(ViewportType type)
{
    m_type = type;
    configureCameraForType();
}

glm::mat4 ViewportContext::getViewMatrix() const
{
    if (!m_camera) {
        return glm::mat4(1.0f);
    }
    return m_camera->getViewMatrix();
}

glm::mat4 ViewportContext::getProjectionMatrix() const
{
    if (!m_camera) {
        return glm::mat4(1.0f);
    }
    return m_camera->getProjectionMatrix(m_renderTarget.getAspectRatio());
}

void ViewportContext::setFocused(bool focused)
{
    m_inputContext.setFocused(focused);
}

void ViewportContext::configureCameraForType()
{
    if (!m_camera) {
        return;
    }

    switch (m_type) {
    case ViewportType::TopDown2D:
        m_camera->setCameraMode(CameraMode::OrthographicTopDown);
        break;
    case ViewportType::Isometric:
        // Isometric uses orthographic projection with a fixed rotation
        m_camera->setCameraMode(CameraMode::OrthographicTopDown);
        m_camera->setPitch(-45.0f);
        m_camera->setYaw(-45.0f);
        break;
    case ViewportType::Perspective3D:
        m_camera->setCameraMode(CameraMode::Perspective3D);
        break;
    }
}

} // namespace fresh
