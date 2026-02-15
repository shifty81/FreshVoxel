#include "editor/TransformGizmo.h"
#include "gameplay/Camera.h"
#include "core/Logger.h"
#include "devtools/DebugRenderer.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>
#include <cmath>

namespace fresh {

namespace {
    // Gizmo visual constants
    constexpr float ARROW_LENGTH = 1.0f;
    constexpr float ARROW_HEAD_LENGTH = 0.2f;
    constexpr float ARROW_HEAD_RADIUS = 0.05f;
    constexpr float ARROW_SHAFT_RADIUS = 0.02f;
    
    constexpr float CIRCLE_RADIUS = 0.8f;
    constexpr int CIRCLE_SEGMENTS = 32;
    
    constexpr float SCALE_BOX_SIZE = 0.1f;
    constexpr float SCALE_LINE_LENGTH = 0.8f;
    
    constexpr float SELECTION_THRESHOLD = 0.1f; // Screen space threshold for selection
    
    // Gizmo colors
    const glm::vec3 COLOR_X_AXIS(1.0f, 0.0f, 0.0f);  // Red
    const glm::vec3 COLOR_Y_AXIS(0.0f, 1.0f, 0.0f);  // Green
    const glm::vec3 COLOR_Z_AXIS(0.0f, 0.0f, 1.0f);  // Blue
    const glm::vec3 COLOR_HIGHLIGHT(1.0f, 1.0f, 0.0f);  // Yellow
}

TransformGizmo::TransformGizmo() = default;

void TransformGizmo::setTransform(const glm::mat4& transform) {
    transform_ = transform;
    decomposeTransform();
}

void TransformGizmo::decomposeTransform() {
    glm::vec3 skew;
    glm::vec4 perspective;
    glm::decompose(transform_, scale_, rotation_, position_, skew, perspective);
}

void TransformGizmo::composeTransform() {
    transform_ = glm::translate(glm::mat4(1.0f), position_);
    transform_ *= glm::mat4_cast(rotation_);
    transform_ *= glm::scale(glm::mat4(1.0f), scale_);
}

float TransformGizmo::applySnap(float value) const {
    if (!snapEnabled_ || snapValue_ <= 0.0f) {
        return value;
    }
    return std::round(value / snapValue_) * snapValue_;
}

glm::vec3 TransformGizmo::getAxisDirection(Component axis) const {
    glm::vec3 direction(0.0f);
    
    switch (axis) {
        case Component::XAxis:
            direction = glm::vec3(1.0f, 0.0f, 0.0f);
            break;
        case Component::YAxis:
            direction = glm::vec3(0.0f, 1.0f, 0.0f);
            break;
        case Component::ZAxis:
            direction = glm::vec3(0.0f, 0.0f, 1.0f);
            break;
        default:
            break;
    }
    
    // Transform direction by rotation if in local space
    if (space_ == Space::Local) {
        direction = glm::rotate(rotation_, direction);
    }
    
    return direction;
}

TransformGizmo::Component TransformGizmo::hitTest(const glm::vec2& mousePos, const Camera& camera) {
    if (mode_ == Mode::None) {
        return Component::None;
    }
    
    // Get ray from mouse position
    glm::vec3 rayOrigin, rayDirection;
    unprojectRay(mousePos, camera, rayOrigin, rayDirection);
    
    float closestDistance = std::numeric_limits<float>::max();
    Component closestComponent = Component::None;
    
    // Test each axis based on current mode
    if (mode_ == Mode::Translate || mode_ == Mode::Universal) {
        // Test X axis arrow
        float t;
        if (rayIntersectArrow(rayOrigin, rayDirection, position_, 
                             getAxisDirection(Component::XAxis), t)) {
            if (t < closestDistance) {
                closestDistance = t;
                closestComponent = Component::XAxis;
            }
        }
        
        // Test Y axis arrow
        if (rayIntersectArrow(rayOrigin, rayDirection, position_, 
                             getAxisDirection(Component::YAxis), t)) {
            if (t < closestDistance) {
                closestDistance = t;
                closestComponent = Component::YAxis;
            }
        }
        
        // Test Z axis arrow
        if (rayIntersectArrow(rayOrigin, rayDirection, position_, 
                             getAxisDirection(Component::ZAxis), t)) {
            if (t < closestDistance) {
                closestDistance = t;
                closestComponent = Component::ZAxis;
            }
        }
    }
    
    if (mode_ == Mode::Rotate || mode_ == Mode::Universal) {
        // Test rotation circles
        float t;
        if (rayIntersectCircle(rayOrigin, rayDirection, position_, 
                              getAxisDirection(Component::XAxis), CIRCLE_RADIUS * size_, t)) {
            if (t < closestDistance) {
                closestDistance = t;
                closestComponent = Component::XAxis;
            }
        }
        
        if (rayIntersectCircle(rayOrigin, rayDirection, position_, 
                              getAxisDirection(Component::YAxis), CIRCLE_RADIUS * size_, t)) {
            if (t < closestDistance) {
                closestDistance = t;
                closestComponent = Component::YAxis;
            }
        }
        
        if (rayIntersectCircle(rayOrigin, rayDirection, position_, 
                              getAxisDirection(Component::ZAxis), CIRCLE_RADIUS * size_, t)) {
            if (t < closestDistance) {
                closestDistance = t;
                closestComponent = Component::ZAxis;
            }
        }
    }
    
    hoveredComponent_ = closestComponent;
    return closestComponent;
}

bool TransformGizmo::beginManipulation(const glm::vec2& mousePos, const Camera& camera) {
    (void)camera; // Parameter reserved for future use
    if (mode_ == Mode::None || hoveredComponent_ == Component::None) {
        return false;
    }
    
    isManipulating_ = true;
    selectedComponent_ = hoveredComponent_;
    initialMousePos_ = mousePos;
    initialPosition_ = position_;
    initialRotation_ = rotation_;
    initialScale_ = scale_;
    
    deltaTranslation_ = glm::vec3(0.0f);
    deltaRotation_ = glm::vec3(0.0f);
    deltaScale_ = glm::vec3(0.0f);
    
    Logger::getInstance().info("TransformGizmo: Begin manipulation", "TransformGizmo");
    return true;
}

bool TransformGizmo::updateManipulation(const glm::vec2& mousePos, const Camera& camera) {
    if (!isManipulating_) {
        return false;
    }
    
    bool changed = false;
    
    switch (mode_) {
        case Mode::Translate:
        case Mode::Universal:
            deltaTranslation_ = computeTranslation(mousePos, camera);
            position_ = initialPosition_ + deltaTranslation_;
            changed = true;
            break;
            
        case Mode::Rotate:
            deltaRotation_ = computeRotation(mousePos, camera);
            // Apply rotation
            rotation_ = initialRotation_;
            rotation_ = glm::rotate(rotation_, glm::radians(deltaRotation_.x), glm::vec3(1, 0, 0));
            rotation_ = glm::rotate(rotation_, glm::radians(deltaRotation_.y), glm::vec3(0, 1, 0));
            rotation_ = glm::rotate(rotation_, glm::radians(deltaRotation_.z), glm::vec3(0, 0, 1));
            changed = true;
            break;
            
        case Mode::Scale:
            deltaScale_ = computeScale(mousePos, camera);
            scale_ = initialScale_ + deltaScale_;
            // Prevent negative or zero scale
            scale_ = glm::max(scale_, glm::vec3(0.01f));
            changed = true;
            break;
            
        default:
            break;
    }
    
    if (changed) {
        composeTransform();
    }
    
    return changed;
}

void TransformGizmo::endManipulation() {
    if (isManipulating_) {
        Logger::getInstance().info("TransformGizmo: End manipulation", "TransformGizmo");
        isManipulating_ = false;
        selectedComponent_ = Component::None;
    }
}

void TransformGizmo::render(const Camera& camera) {
    if (mode_ == Mode::None) {
        return;
    }
    
    switch (mode_) {
        case Mode::Translate:
            renderTranslateGizmo(camera);
            break;
        case Mode::Rotate:
            renderRotateGizmo(camera);
            break;
        case Mode::Scale:
            renderScaleGizmo(camera);
            break;
        case Mode::Universal:
            // Render all gizmos overlapping
            renderTranslateGizmo(camera);
            renderRotateGizmo(camera);
            renderScaleGizmo(camera);
            break;
        default:
            break;
    }
}

void TransformGizmo::renderTranslateGizmo(const Camera& camera) {
    (void)camera; // Parameter reserved for future use
    // Draw X axis arrow (red)
    bool highlightX = (hoveredComponent_ == Component::XAxis || selectedComponent_ == Component::XAxis);
    drawArrow(position_, getAxisDirection(Component::XAxis), 
              highlightX ? COLOR_HIGHLIGHT : COLOR_X_AXIS, highlightX);
    
    // Draw Y axis arrow (green)
    bool highlightY = (hoveredComponent_ == Component::YAxis || selectedComponent_ == Component::YAxis);
    drawArrow(position_, getAxisDirection(Component::YAxis), 
              highlightY ? COLOR_HIGHLIGHT : COLOR_Y_AXIS, highlightY);
    
    // Draw Z axis arrow (blue)
    bool highlightZ = (hoveredComponent_ == Component::ZAxis || selectedComponent_ == Component::ZAxis);
    drawArrow(position_, getAxisDirection(Component::ZAxis), 
              highlightZ ? COLOR_HIGHLIGHT : COLOR_Z_AXIS, highlightZ);
}

void TransformGizmo::renderRotateGizmo(const Camera& camera) {
    (void)camera; // Parameter reserved for future use
    // Draw X rotation circle (red)
    bool highlightX = (hoveredComponent_ == Component::XAxis || selectedComponent_ == Component::XAxis);
    drawCircle(position_, getAxisDirection(Component::XAxis), CIRCLE_RADIUS * size_,
               highlightX ? COLOR_HIGHLIGHT : COLOR_X_AXIS, highlightX);
    
    // Draw Y rotation circle (green)
    bool highlightY = (hoveredComponent_ == Component::YAxis || selectedComponent_ == Component::YAxis);
    drawCircle(position_, getAxisDirection(Component::YAxis), CIRCLE_RADIUS * size_,
               highlightY ? COLOR_HIGHLIGHT : COLOR_Y_AXIS, highlightY);
    
    // Draw Z rotation circle (blue)
    bool highlightZ = (hoveredComponent_ == Component::ZAxis || selectedComponent_ == Component::ZAxis);
    drawCircle(position_, getAxisDirection(Component::ZAxis), CIRCLE_RADIUS * size_,
               highlightZ ? COLOR_HIGHLIGHT : COLOR_Z_AXIS, highlightZ);
}

void TransformGizmo::renderScaleGizmo(const Camera& camera) {
    (void)camera; // Parameter reserved for future use
    // Draw X scale handle (red)
    bool highlightX = (hoveredComponent_ == Component::XAxis || selectedComponent_ == Component::XAxis);
    glm::vec3 xEnd = position_ + getAxisDirection(Component::XAxis) * SCALE_LINE_LENGTH * size_;
    drawBox(xEnd, glm::vec3(SCALE_BOX_SIZE * size_),
            highlightX ? COLOR_HIGHLIGHT : COLOR_X_AXIS, highlightX);
    
    // Draw Y scale handle (green)
    bool highlightY = (hoveredComponent_ == Component::YAxis || selectedComponent_ == Component::YAxis);
    glm::vec3 yEnd = position_ + getAxisDirection(Component::YAxis) * SCALE_LINE_LENGTH * size_;
    drawBox(yEnd, glm::vec3(SCALE_BOX_SIZE * size_),
            highlightY ? COLOR_HIGHLIGHT : COLOR_Y_AXIS, highlightY);
    
    // Draw Z scale handle (blue)
    bool highlightZ = (hoveredComponent_ == Component::ZAxis || selectedComponent_ == Component::ZAxis);
    glm::vec3 zEnd = position_ + getAxisDirection(Component::ZAxis) * SCALE_LINE_LENGTH * size_;
    drawBox(zEnd, glm::vec3(SCALE_BOX_SIZE * size_),
            highlightZ ? COLOR_HIGHLIGHT : COLOR_Z_AXIS, highlightZ);
}

// Rendering functions using DebugRenderer
void TransformGizmo::drawArrow(const glm::vec3& position, const glm::vec3& direction, 
                               const glm::vec3& color, bool highlighted) {
    if (!debugRenderer_) {
        return; // No renderer available
    }
    
    // Scale arrow based on gizmo size
    glm::vec3 end = position + direction * ARROW_LENGTH * size_;
    
    // Convert color to vec4
    glm::vec4 color4 = glm::vec4(color, 1.0f);
    if (highlighted) {
        color4 = glm::vec4(COLOR_HIGHLIGHT, 1.0f);
    }
    
    // Draw the arrow using DebugRenderer
    debugRenderer_->drawArrow(position, end, color4, 0.0f);
}

void TransformGizmo::drawCircle(const glm::vec3& position, const glm::vec3& normal, 
                                float radius, const glm::vec3& color, bool highlighted) {
    if (!debugRenderer_) {
        return; // No renderer available
    }
    
    // Convert color to vec4
    glm::vec4 color4 = glm::vec4(color, 1.0f);
    if (highlighted) {
        color4 = glm::vec4(COLOR_HIGHLIGHT, 1.0f);
    }
    
    // Create perpendicular vectors to the normal
    glm::vec3 up = glm::abs(normal.y) < 0.999f ? glm::vec3(0, 1, 0) : glm::vec3(1, 0, 0);
    glm::vec3 tangent = glm::normalize(glm::cross(normal, up));
    glm::vec3 bitangent = glm::cross(normal, tangent);
    
    // Draw circle as line segments
    glm::vec3 prevPoint = position + tangent * radius;
    for (int i = 1; i <= CIRCLE_SEGMENTS; ++i) {
        float angle = (static_cast<float>(i) / CIRCLE_SEGMENTS) * 2.0f * glm::pi<float>();
        float c = std::cos(angle);
        float s = std::sin(angle);
        glm::vec3 point = position + (tangent * c + bitangent * s) * radius;
        
        debugRenderer_->drawLine(prevPoint, point, color4, 0.0f);
        prevPoint = point;
    }
}

void TransformGizmo::drawBox(const glm::vec3& position, const glm::vec3& size, 
                             const glm::vec3& color, bool highlighted) {
    if (!debugRenderer_) {
        return; // No renderer available
    }
    
    // Convert color to vec4
    glm::vec4 color4 = glm::vec4(color, 1.0f);
    if (highlighted) {
        color4 = glm::vec4(COLOR_HIGHLIGHT, 1.0f);
    }
    
    // Draw box using DebugRenderer
    debugRenderer_->drawBox(position, size, color4, 0.0f);
}

glm::vec2 TransformGizmo::projectToScreen(const glm::vec3& worldPos, const Camera& camera) const {
    // Use a standard aspect ratio for projection calculations
    // In a real implementation, this should be passed from the viewport
    constexpr float aspectRatio = 16.0f / 9.0f;
    glm::mat4 viewProj = camera.getProjectionMatrix(aspectRatio) * camera.getViewMatrix();
    glm::vec4 clipSpace = viewProj * glm::vec4(worldPos, 1.0f);
    
    if (clipSpace.w == 0.0f) {
        return glm::vec2(0.0f);
    }
    
    glm::vec3 ndc = glm::vec3(clipSpace) / clipSpace.w;
    return glm::vec2((ndc.x + 1.0f) * 0.5f, (1.0f - ndc.y) * 0.5f);
}

void TransformGizmo::unprojectRay(const glm::vec2& screenPos, const Camera& camera,
                                  glm::vec3& rayOrigin, glm::vec3& rayDirection) const {
    // Convert screen position to NDC
    glm::vec2 ndc = screenPos * 2.0f - 1.0f;
    ndc.y = -ndc.y; // Flip Y
    
    // Use a standard aspect ratio for projection calculations
    // In a real implementation, this should be passed from the viewport
    constexpr float aspectRatio = 16.0f / 9.0f;
    glm::mat4 viewProj = camera.getProjectionMatrix(aspectRatio) * camera.getViewMatrix();
    glm::mat4 invViewProj = glm::inverse(viewProj);
    
    // Unproject near and far points
    glm::vec4 nearPoint = invViewProj * glm::vec4(ndc, -1.0f, 1.0f);
    glm::vec4 farPoint = invViewProj * glm::vec4(ndc, 1.0f, 1.0f);
    
    nearPoint /= nearPoint.w;
    farPoint /= farPoint.w;
    
    rayOrigin = glm::vec3(nearPoint);
    rayDirection = glm::normalize(glm::vec3(farPoint) - glm::vec3(nearPoint));
}

bool TransformGizmo::rayIntersectArrow(const glm::vec3& rayOrigin, const glm::vec3& rayDirection,
                                       const glm::vec3& arrowStart, const glm::vec3& arrowDir,
                                       float& t) const {
    // Simplified ray-cylinder intersection test
    // This should be a proper ray-arrow intersection with shaft and head
    
    glm::vec3 oc = rayOrigin - arrowStart;
    float arrowLength = ARROW_LENGTH * size_;
    
    // Project ray onto arrow direction
    float a = glm::dot(rayDirection, arrowDir);
    float b = glm::dot(oc, arrowDir);
    
    // Find closest point on arrow line to ray
    float t0 = -b / (1.0f - a * a);
    
    if (t0 < 0.0f || t0 > arrowLength) {
        return false;
    }
    
    glm::vec3 closestPoint = arrowStart + arrowDir * t0;
    float distance = glm::length(closestPoint - (rayOrigin + rayDirection * glm::dot(closestPoint - rayOrigin, rayDirection)));
    
    if (distance < ARROW_SHAFT_RADIUS * size_ * 5.0f) { // Larger hit area for usability
        t = glm::length(closestPoint - rayOrigin);
        return true;
    }
    
    return false;
}

bool TransformGizmo::rayIntersectCircle(const glm::vec3& rayOrigin, const glm::vec3& rayDirection,
                                        const glm::vec3& circleCenter, const glm::vec3& circleNormal,
                                        float radius, float& t) const {
    // Ray-plane intersection
    float denom = glm::dot(circleNormal, rayDirection);
    
    if (std::abs(denom) < 1e-6f) {
        return false; // Ray parallel to plane
    }
    
    t = glm::dot(circleCenter - rayOrigin, circleNormal) / denom;
    
    if (t < 0.0f) {
        return false; // Intersection behind ray origin
    }
    
    glm::vec3 hitPoint = rayOrigin + rayDirection * t;
    float distanceFromCenter = glm::length(hitPoint - circleCenter);
    
    // Check if hit point is near the circle (within a tolerance)
    float tolerance = 0.1f * size_;
    return std::abs(distanceFromCenter - radius) < tolerance;
}

glm::vec3 TransformGizmo::computeTranslation(const glm::vec2& currentMousePos, const Camera& camera) {
    // Calculate mouse delta in screen space
    glm::vec2 mouseDelta = currentMousePos - initialMousePos_;
    
    // Get axis direction
    glm::vec3 axis = getAxisDirection(selectedComponent_);
    
    // Project axis to screen space to determine movement scale
    glm::vec2 axisScreenStart = projectToScreen(position_, camera);
    glm::vec2 axisScreenEnd = projectToScreen(position_ + axis, camera);
    glm::vec2 axisScreenDir = glm::normalize(axisScreenEnd - axisScreenStart);
    
    // Project mouse delta onto axis direction in screen space
    float screenDistance = glm::dot(mouseDelta, axisScreenDir);
    
    // Scale by camera distance and apply to world space axis
    float cameraDistance = glm::length(camera.getPosition() - position_);
    float worldMovement = screenDistance * cameraDistance * 2.0f; // Scale factor
    
    // Apply snap
    worldMovement = applySnap(worldMovement);
    
    return axis * worldMovement;
}

glm::vec3 TransformGizmo::computeRotation(const glm::vec2& currentMousePos, const Camera& camera) {
    (void)camera; // Parameter reserved for future use
    // Calculate mouse delta in screen space
    glm::vec2 mouseDelta = currentMousePos - initialMousePos_;
    
    // Convert mouse movement to rotation angle
    float rotationAngle = glm::length(mouseDelta) * 180.0f; // Scale to degrees
    
    // Apply snap
    rotationAngle = applySnap(rotationAngle);
    
    // Return rotation based on selected axis
    glm::vec3 rotation(0.0f);
    switch (selectedComponent_) {
        case Component::XAxis:
            rotation.x = rotationAngle;
            break;
        case Component::YAxis:
            rotation.y = rotationAngle;
            break;
        case Component::ZAxis:
            rotation.z = rotationAngle;
            break;
        default:
            break;
    }
    
    return rotation;
}

glm::vec3 TransformGizmo::computeScale(const glm::vec2& currentMousePos, const Camera& camera) {
    (void)camera; // Parameter reserved for future use
    // Calculate mouse delta in screen space
    glm::vec2 mouseDelta = currentMousePos - initialMousePos_;
    
    // Convert to scale factor
    float scaleFactor = glm::length(mouseDelta) * 2.0f;
    if (mouseDelta.x < 0.0f || mouseDelta.y < 0.0f) {
        scaleFactor = -scaleFactor;
    }
    
    // Apply snap
    scaleFactor = applySnap(scaleFactor);
    
    // Return scale based on selected axis
    glm::vec3 scale(0.0f);
    
    if (selectedComponent_ == Component::AllAxes) {
        // Uniform scale
        scale = glm::vec3(scaleFactor);
    } else {
        // Axis-specific scale
        switch (selectedComponent_) {
            case Component::XAxis:
                scale.x = scaleFactor;
                break;
            case Component::YAxis:
                scale.y = scaleFactor;
                break;
            case Component::ZAxis:
                scale.z = scaleFactor;
                break;
            default:
                break;
        }
    }
    
    return scale;
}

} // namespace fresh
