#pragma once

#include <memory>
#include <string>

#include <glm/glm.hpp>

#include "viewport/InputContext.h"
#include "viewport/ViewportRenderTarget.h"

namespace fresh
{

class Camera;
class VoxelWorld;
class IRenderContext;

/**
 * @brief Viewport type matching ENGINE.md specification
 */
enum class ViewportType {
    TopDown2D,     ///< Orthographic top-down (Zomboid-style)
    Isometric,     ///< Axonometric 2.5D
    Perspective3D  ///< Full 3D perspective (Minecraft-style)
};

/**
 * @brief The atomic unit of the engine's rendering and input system
 *
 * Per ENGINE.md: "The viewport is the atomic unit of the engine.
 * Everything routes through ViewportContext."
 *
 * Data flow:
 *   OS Events → ViewportContext → InputContext → Systems
 *                                  → Camera → Scene/VoxelWorld
 *                                  → ViewportRenderTarget → DX11/DX12/Offscreen
 *
 * Each ViewportContext owns:
 *   - A Camera (viewport-owned, not scene-owned)
 *   - An InputContext (viewport-owned input routing)
 *   - A ViewportRenderTarget (explicit render target binding)
 *   - A reference to the world being viewed
 *
 * Rules enforced:
 *   - Nothing renders unless a ViewportRenderTarget is bound
 *   - Input routes through viewport-owned InputContext (never global)
 *   - GUI panels never render voxel worlds directly
 */
class ViewportContext
{
public:
    /**
     * @brief Construct a viewport context
     * @param name Human-readable name (e.g. "Main Editor Viewport", "PIE Viewport")
     * @param type Viewport type (affects camera projection)
     */
    explicit ViewportContext(const std::string& name = "Viewport",
                            ViewportType type = ViewportType::Perspective3D);
    ~ViewportContext();

    // Non-copyable, movable
    ViewportContext(const ViewportContext&) = delete;
    ViewportContext& operator=(const ViewportContext&) = delete;
    ViewportContext(ViewportContext&&) noexcept;
    ViewportContext& operator=(ViewportContext&&) noexcept;

    /**
     * @brief Initialize the viewport with a render context and optional window handle
     * @param renderContext Render context for GPU access
     * @param windowHandle Native window handle for the viewport panel (nullptr for offscreen)
     * @return true if initialization succeeded
     */
    bool initialize(IRenderContext* renderContext, void* windowHandle = nullptr);

    /**
     * @brief Shutdown and release all resources
     */
    void shutdown();

    /**
     * @brief Begin a frame: bind render target, update input, prepare camera
     * @return true if the viewport is ready to render
     */
    bool beginFrame();

    /**
     * @brief End a frame: present the rendered image
     */
    void endFrame();

    /**
     * @brief Resize the viewport
     * @param width New width
     * @param height New height
     * @return true if resize succeeded
     */
    bool resize(int width, int height);

    // --- Accessors ---

    /**
     * @brief Get the viewport name
     */
    [[nodiscard]] const std::string& getName() const noexcept { return m_name; }

    /**
     * @brief Get the viewport type
     */
    [[nodiscard]] ViewportType getType() const noexcept { return m_type; }

    /**
     * @brief Set the viewport type (may change camera projection)
     */
    void setType(ViewportType type);

    /**
     * @brief Get the owned camera
     *
     * Per ENGINE.md: "Cameras are viewport-owned — Not scene-owned"
     */
    [[nodiscard]] Camera* getCamera() const noexcept { return m_camera.get(); }

    /**
     * @brief Get the owned input context
     *
     * Per ENGINE.md: "Input is never global — Input routes through
     * viewport-owned InputContext"
     */
    [[nodiscard]] InputContext& getInputContext() noexcept { return m_inputContext; }
    [[nodiscard]] const InputContext& getInputContext() const noexcept { return m_inputContext; }

    /**
     * @brief Get the render target
     *
     * Per ENGINE.md: "Nothing renders unless a ViewportRenderTarget is bound"
     */
    [[nodiscard]] ViewportRenderTarget& getRenderTarget() noexcept { return m_renderTarget; }
    [[nodiscard]] const ViewportRenderTarget& getRenderTarget() const noexcept { return m_renderTarget; }

    /**
     * @brief Set the world this viewport is viewing
     *
     * PIE clones the world; the viewport points at the clone.
     */
    void setWorld(VoxelWorld* world) noexcept { m_world = world; }

    /**
     * @brief Get the world this viewport is viewing
     */
    [[nodiscard]] VoxelWorld* getWorld() const noexcept { return m_world; }

    /**
     * @brief Check if the viewport is initialized and ready
     */
    [[nodiscard]] bool isReady() const noexcept { return m_initialized && m_renderTarget.isReady(); }

    /**
     * @brief Get view matrix from the owned camera
     */
    [[nodiscard]] glm::mat4 getViewMatrix() const;

    /**
     * @brief Get projection matrix from the owned camera
     */
    [[nodiscard]] glm::mat4 getProjectionMatrix() const;

    /**
     * @brief Get the viewport width
     */
    [[nodiscard]] int getWidth() const noexcept { return m_renderTarget.getWidth(); }

    /**
     * @brief Get the viewport height
     */
    [[nodiscard]] int getHeight() const noexcept { return m_renderTarget.getHeight(); }

    /**
     * @brief Set whether this viewport has focus
     *
     * Delegates to InputContext.
     */
    void setFocused(bool focused);

    /**
     * @brief Check if this viewport has focus
     */
    [[nodiscard]] bool isFocused() const noexcept { return m_inputContext.isFocused(); }

private:
    void configureCameraForType();

    std::string m_name;
    ViewportType m_type;
    bool m_initialized = false;

    std::unique_ptr<Camera> m_camera;
    InputContext m_inputContext;
    ViewportRenderTarget m_renderTarget;

    VoxelWorld* m_world = nullptr; // Non-owning reference
};

} // namespace fresh
