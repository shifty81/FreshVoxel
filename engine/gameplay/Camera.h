#pragma once
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace fresh
{

/**
 * @brief Camera modes for different game types
 */
enum class CameraMode {
    Perspective3D,      ///< Standard 3D first-person camera
    Orthographic2D,     ///< 2D side-scrolling camera (Terraria-style)
    OrthographicTopDown ///< 2D top-down camera (Zelda-style)
};

/**
 * @brief First-person camera system for the player
 *
 * Handles view and projection matrices, mouse look controls,
 * and camera positioning. Works in conjunction with Player class.
 * Supports multiple camera modes for 3D and 2D game types.
 */
class Camera
{
public:
    /**
     * @brief Construct a new Camera
     * @param fov Field of view in degrees
     * @param nearPlane Near clipping plane distance
     * @param farPlane Far clipping plane distance
     */
    Camera(float fov = 75.0f, float nearPlane = 0.1f, float farPlane = 1000.0f);

    /**
     * @brief Update camera direction vectors based on pitch and yaw
     */
    void updateVectors();

    /**
     * @brief Process mouse movement input for look controls
     * @param xOffset Mouse X offset
     * @param yOffset Mouse Y offset
     * @param sensitivity Mouse sensitivity multiplier
     */
    void processMouseMovement(float xOffset, float yOffset, float sensitivity = 0.002f);

    /**
     * @brief Set camera position
     * @param pos New position
     */
    void setPosition(const glm::vec3& pos);

    /**
     * @brief Set camera rotation
     * @param pitch Pitch angle in degrees (-89 to 89)
     * @param yaw Yaw angle in degrees
     */
    void setRotation(float pitch, float yaw);

    /**
     * @brief Set camera mode
     * @param mode Camera mode to use
     */
    void setCameraMode(CameraMode mode);

    /**
     * @brief Get current camera mode
     * @return Current camera mode
     */
    [[nodiscard]] CameraMode getCameraMode() const noexcept
    {
        return m_cameraMode;
    }

    /**
     * @brief Get view matrix for rendering
     * @return View matrix
     */
    [[nodiscard]] glm::mat4 getViewMatrix() const;

    /**
     * @brief Get projection matrix for rendering
     * @param aspectRatio Width/Height ratio of viewport
     * @return Projection matrix
     */
    [[nodiscard]] glm::mat4 getProjectionMatrix(float aspectRatio) const;

    // Getters
    [[nodiscard]] glm::vec3 getPosition() const noexcept
    {
        return position;
    }
    [[nodiscard]] glm::vec3 getFront() const noexcept
    {
        return front;
    }
    [[nodiscard]] glm::vec3 getRight() const noexcept
    {
        return right;
    }
    [[nodiscard]] glm::vec3 getUp() const noexcept
    {
        return up;
    }
    [[nodiscard]] float getPitch() const noexcept
    {
        return pitch;
    }
    [[nodiscard]] float getYaw() const noexcept
    {
        return yaw;
    }
    [[nodiscard]] float getFOV() const noexcept
    {
        return fov;
    }

    // Setters
    void setPitch(float newPitch)
    {
        pitch = std::clamp(newPitch, -89.0f, 89.0f);
        updateVectors();
    }

    void setYaw(float newYaw)
    {
        yaw = newYaw;
        updateVectors();
    }

    void setFOV(float newFov)
    {
        fov = newFov;
    }

    /**
     * @brief Set the aspect ratio for the camera
     * @param aspectRatio Width/Height ratio of viewport
     */
    void setAspectRatio(float aspectRatio)
    {
        m_aspectRatio = aspectRatio;
    }

    /**
     * @brief Get the stored aspect ratio
     * @return Stored aspect ratio (or 16:9 default if not set)
     */
    [[nodiscard]] float getAspectRatio() const noexcept
    {
        return m_aspectRatio;
    }

    /**
     * @brief Set orthographic zoom for 2D cameras
     * @param zoom Zoom level (higher = zoomed in)
     */
    void setOrthographicZoom(float zoom)
    {
        m_orthoZoom = zoom;
    }

    [[nodiscard]] float getOrthographicZoom() const noexcept
    {
        return m_orthoZoom;
    }

private:
    glm::vec3 position{0.0f, 80.0f, 0.0f};
    glm::vec3 front{0.0f, 0.0f, -1.0f};
    glm::vec3 up{0.0f, 1.0f, 0.0f};
    glm::vec3 right{1.0f, 0.0f, 0.0f};
    glm::vec3 worldUp{0.0f, 1.0f, 0.0f};

    float pitch = 0.0f; // Rotation around right axis (look up/down)
    float yaw = -90.0f; // Rotation around up axis (look left/right)
    float fov;          // Field of view in degrees
    float nearPlane;    // Near clipping plane
    float farPlane;     // Far clipping plane
    
    CameraMode m_cameraMode = CameraMode::Perspective3D;
    float m_orthoZoom = 1.0f; // Zoom level for orthographic cameras
    float m_aspectRatio = 16.0f / 9.0f; // Stored aspect ratio (default 16:9)
};

} // namespace fresh
