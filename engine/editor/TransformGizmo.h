#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

namespace fresh {

class Camera;

namespace devtools {
    class DebugRenderer;
}

/**
 * @brief 3D Transform Gizmo for visual object manipulation
 * 
 * Provides Unreal Engine-like visual gizmos for transforming objects in 3D space.
 * Supports move, rotate, and scale operations with snap-to-grid functionality.
 */
class TransformGizmo {
public:
    /**
     * @brief Gizmo operation modes
     */
    enum class Mode {
        None,       ///< No gizmo active
        Translate,  ///< Move tool (3 arrows)
        Rotate,     ///< Rotate tool (3 circles)
        Scale,      ///< Scale tool (3 boxes)
        Universal   ///< All-in-one tool
    };

    /**
     * @brief Coordinate space for transformations
     */
    enum class Space {
        Local,  ///< Relative to object's rotation
        World   ///< Relative to world axes
    };

    /**
     * @brief Gizmo component that can be selected
     */
    enum class Component {
        None,
        XAxis,
        YAxis,
        ZAxis,
        XYPlane,
        YZPlane,
        XZPlane,
        AllAxes  // For uniform scale
    };

    /**
     * @brief Construct a transform gizmo
     */
    TransformGizmo();

    /**
     * @brief Set current mode
     */
    void setMode(Mode mode) { mode_ = mode; }
    Mode getMode() const { return mode_; }

    /**
     * @brief Set coordinate space
     */
    void setSpace(Space space) { space_ = space; }
    Space getSpace() const { return space_; }

    /**
     * @brief Enable/disable grid snapping
     */
    void setSnapEnabled(bool enabled) { snapEnabled_ = enabled; }
    bool isSnapEnabled() const { return snapEnabled_; }

    /**
     * @brief Set snap value (distance for translate, degrees for rotate, factor for scale)
     */
    void setSnapValue(float value) { snapValue_ = value; }
    float getSnapValue() const { return snapValue_; }

    /**
     * @brief Set gizmo size (visual scale)
     */
    void setSize(float size) { size_ = size; }

    /**
     * @brief Set target transform matrix
     */
    void setTransform(const glm::mat4& transform);
    const glm::mat4& getTransform() const { return transform_; }

    /**
     * @brief Check if mouse is over gizmo and which component
     * @param mousePos Mouse position in screen space (0-1 normalized)
     * @param camera Current camera
     * @return Component under mouse
     */
    Component hitTest(const glm::vec2& mousePos, const Camera& camera);

    /**
     * @brief Begin manipulation
     * @param mousePos Initial mouse position
     * @param camera Current camera
     * @return true if manipulation started
     */
    bool beginManipulation(const glm::vec2& mousePos, const Camera& camera);

    /**
     * @brief Update manipulation
     * @param mousePos Current mouse position
     * @param camera Current camera
     * @return true if transform changed
     */
    bool updateManipulation(const glm::vec2& mousePos, const Camera& camera);

    /**
     * @brief End manipulation
     */
    void endManipulation();

    /**
     * @brief Check if currently manipulating
     */
    bool isManipulating() const { return isManipulating_; }

    /**
     * @brief Get delta transformation since manipulation began
     */
    glm::vec3 getDeltaTranslation() const { return deltaTranslation_; }
    glm::vec3 getDeltaRotation() const { return deltaRotation_; }
    glm::vec3 getDeltaScale() const { return deltaScale_; }

    /**
     * @brief Set the debug renderer for visualization
     * @param debugRenderer Pointer to debug renderer
     */
    void setDebugRenderer(devtools::DebugRenderer* debugRenderer) { debugRenderer_ = debugRenderer; }

    /**
     * @brief Render gizmo
     * @param camera Current camera for view/projection
     * @note This should be called from the rendering system
     */
    void render(const Camera& camera);

private:
    Mode mode_ = Mode::Translate;
    Space space_ = Space::World;
    bool snapEnabled_ = false;
    float snapValue_ = 1.0f;
    float size_ = 1.0f;
    
    devtools::DebugRenderer* debugRenderer_ = nullptr;

    glm::mat4 transform_ = glm::mat4(1.0f);
    glm::vec3 position_ = glm::vec3(0.0f);
    glm::quat rotation_ = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    glm::vec3 scale_ = glm::vec3(1.0f);

    Component hoveredComponent_ = Component::None;
    Component selectedComponent_ = Component::None;
    bool isManipulating_ = false;

    glm::vec2 initialMousePos_ = glm::vec2(0.0f);
    glm::vec3 initialPosition_ = glm::vec3(0.0f);
    glm::quat initialRotation_ = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    glm::vec3 initialScale_ = glm::vec3(1.0f);

    glm::vec3 deltaTranslation_ = glm::vec3(0.0f);
    glm::vec3 deltaRotation_ = glm::vec3(0.0f);
    glm::vec3 deltaScale_ = glm::vec3(0.0f);

    /**
     * @brief Extract position, rotation, scale from matrix
     */
    void decomposeTransform();

    /**
     * @brief Compose matrix from position, rotation, scale
     */
    void composeTransform();

    /**
     * @brief Apply snap to value
     */
    float applySnap(float value) const;

    /**
     * @brief Get axis direction in world space
     */
    glm::vec3 getAxisDirection(Component axis) const;

    /**
     * @brief Render translate gizmo
     */
    void renderTranslateGizmo(const Camera& camera);

    /**
     * @brief Render rotate gizmo
     */
    void renderRotateGizmo(const Camera& camera);

    /**
     * @brief Render scale gizmo
     */
    void renderScaleGizmo(const Camera& camera);

    /**
     * @brief Draw arrow for an axis
     */
    void drawArrow(const glm::vec3& position, const glm::vec3& direction, 
                   const glm::vec3& color, bool highlighted);

    /**
     * @brief Draw circle for rotation axis
     */
    void drawCircle(const glm::vec3& position, const glm::vec3& normal, 
                    float radius, const glm::vec3& color, bool highlighted);

    /**
     * @brief Draw box for scale handle
     */
    void drawBox(const glm::vec3& position, const glm::vec3& size, 
                 const glm::vec3& color, bool highlighted);

    /**
     * @brief Project 3D point to screen space
     */
    glm::vec2 projectToScreen(const glm::vec3& worldPos, const Camera& camera) const;

    /**
     * @brief Unproject screen point to 3D ray
     */
    void unprojectRay(const glm::vec2& screenPos, const Camera& camera,
                     glm::vec3& rayOrigin, glm::vec3& rayDirection) const;

    /**
     * @brief Test ray intersection with axis arrow
     */
    bool rayIntersectArrow(const glm::vec3& rayOrigin, const glm::vec3& rayDirection,
                          const glm::vec3& arrowStart, const glm::vec3& arrowDir,
                          float& t) const;

    /**
     * @brief Test ray intersection with circle
     */
    bool rayIntersectCircle(const glm::vec3& rayOrigin, const glm::vec3& rayDirection,
                           const glm::vec3& circleCenter, const glm::vec3& circleNormal,
                           float radius, float& t) const;

    /**
     * @brief Compute translation based on mouse movement
     */
    glm::vec3 computeTranslation(const glm::vec2& currentMousePos, const Camera& camera);

    /**
     * @brief Compute rotation based on mouse movement
     */
    glm::vec3 computeRotation(const glm::vec2& currentMousePos, const Camera& camera);

    /**
     * @brief Compute scale based on mouse movement
     */
    glm::vec3 computeScale(const glm::vec2& currentMousePos, const Camera& camera);
};

} // namespace fresh
