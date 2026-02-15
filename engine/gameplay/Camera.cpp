#include "gameplay/Camera.h"

#include <algorithm>

#include <glm/gtc/matrix_transform.hpp>

namespace fresh
{

Camera::Camera(float fov, float nearPlane, float farPlane)
    : fov(fov), nearPlane(nearPlane), farPlane(farPlane)
{
    updateVectors();
}

void Camera::updateVectors()
{
    // For orthographic top-down camera, lock the view direction
    if (m_cameraMode == CameraMode::OrthographicTopDown) {
        // Looking straight down
        front = glm::vec3(0.0f, -1.0f, 0.0f);
        right = glm::vec3(1.0f, 0.0f, 0.0f);
        up = glm::vec3(0.0f, 0.0f, -1.0f); // Forward in world space
        return;
    }
    
    // For 2D side-scrolling, lock camera to side view
    if (m_cameraMode == CameraMode::Orthographic2D) {
        // Looking from the side (along Z axis)
        front = glm::vec3(0.0f, 0.0f, -1.0f);
        right = glm::vec3(1.0f, 0.0f, 0.0f);
        up = glm::vec3(0.0f, 1.0f, 0.0f);
        return;
    }
    
    // Standard 3D camera - calculate front vector from yaw and pitch
    glm::vec3 newFront;
    newFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    newFront.y = sin(glm::radians(pitch));
    newFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    front = glm::normalize(newFront);

    // Recalculate right and up vectors
    right = glm::normalize(glm::cross(front, worldUp));
    up = glm::normalize(glm::cross(right, front));
}

void Camera::processMouseMovement(float xOffset, float yOffset, float sensitivity)
{
    // For 2D cameras, mouse movement pans the camera instead of rotating
    if (m_cameraMode == CameraMode::Orthographic2D) {
        // Pan in X and Y for side-scrolling
        position.x -= xOffset * sensitivity * 50.0f / m_orthoZoom;
        position.y += yOffset * sensitivity * 50.0f / m_orthoZoom;
        return;
    }
    
    if (m_cameraMode == CameraMode::OrthographicTopDown) {
        // Pan in X and Z for top-down
        position.x -= xOffset * sensitivity * 50.0f / m_orthoZoom;
        position.z += yOffset * sensitivity * 50.0f / m_orthoZoom;
        return;
    }
    
    // Standard 3D camera rotation
    xOffset *= sensitivity;
    yOffset *= sensitivity;

    yaw += xOffset;
    pitch += yOffset;

    // Constrain pitch to avoid gimbal lock
    pitch = std::clamp(pitch, -89.0f, 89.0f);

    updateVectors();
}

void Camera::setPosition(const glm::vec3& pos)
{
    position = pos;
}

void Camera::setRotation(float newPitch, float newYaw)
{
    pitch = std::clamp(newPitch, -89.0f, 89.0f);
    yaw = newYaw;
    updateVectors();
}

void Camera::setCameraMode(CameraMode mode)
{
    m_cameraMode = mode;
    
    // Reset camera orientation based on mode
    if (mode == CameraMode::OrthographicTopDown) {
        // For top-down, position camera above the world
        pitch = -90.0f; // Look down
        yaw = 0.0f;
    } else if (mode == CameraMode::Orthographic2D) {
        // For side-scrolling, position camera to the side
        pitch = 0.0f;
        yaw = -90.0f;
    }
    
    updateVectors();
}

glm::mat4 Camera::getViewMatrix() const
{
    if (m_cameraMode == CameraMode::OrthographicTopDown) {
        // Look straight down from above
        return glm::lookAt(position, position + glm::vec3(0.0f, -1.0f, 0.0f), 
                          glm::vec3(0.0f, 0.0f, -1.0f));
    } else if (m_cameraMode == CameraMode::Orthographic2D) {
        // Look from the side
        return glm::lookAt(position, position + glm::vec3(0.0f, 0.0f, -1.0f),
                          glm::vec3(0.0f, 1.0f, 0.0f));
    }
    
    // Standard 3D view
    return glm::lookAt(position, position + front, up);
}

glm::mat4 Camera::getProjectionMatrix(float aspectRatio) const
{
    if (m_cameraMode == CameraMode::Orthographic2D || 
        m_cameraMode == CameraMode::OrthographicTopDown) {
        // Orthographic projection for 2D games
        float orthoHeight = 30.0f / m_orthoZoom; // Adjustable view height
        float orthoWidth = orthoHeight * aspectRatio;
        
        return glm::ortho(-orthoWidth, orthoWidth,   // left, right
                         -orthoHeight, orthoHeight,  // bottom, top
                         nearPlane, farPlane);       // near, far
    }
    
    // Perspective projection for 3D
    return glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
}

} // namespace fresh
