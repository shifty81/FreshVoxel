// Prevent Windows.h from defining min/max macros (must be first)
#ifdef _WIN32
    #ifndef NOMINMAX
    #define NOMINMAX
    #endif
#endif

#include "editor/CameraController.h"
#include "gameplay/Camera.h"
#include "gameplay/Player.h"
#include "core/Logger.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <algorithm>
#include <fstream>
#include <sstream>

namespace fresh {

CameraController::CameraController() = default;

void CameraController::initialize(Player* player) {
    if (!player) {
        LOG_ERROR_C("Cannot initialize CameraController with null player", "CameraController");
        return;
    }
    
    m_player = player;
    m_camera = &player->getCamera();
    
    LOG_INFO_C("CameraController initialized", "CameraController");
}

void CameraController::update(float deltaTime) {
    if (m_transitioning) {
        updateTransition(deltaTime);
    }
}

std::string CameraController::getViewModeName(ViewMode mode) {
    switch (mode) {
        case ViewMode::Perspective: return "Perspective";
        case ViewMode::Top:         return "Top";
        case ViewMode::Front:       return "Front";
        case ViewMode::Side:        return "Side (Right)";
        case ViewMode::Bottom:      return "Bottom";
        case ViewMode::Back:        return "Back";
        case ViewMode::Left:        return "Left";
        default:                    return "Unknown";
    }
}

void CameraController::setViewMode(ViewMode mode) {
    if (!m_camera || !m_player) {
        LOG_WARNING_C("Camera or player not initialized", "CameraController");
        return;
    }
    
    m_previousViewMode = m_viewMode;
    m_viewMode = mode;
    
    if (mode == ViewMode::Perspective) {
        // Return to perspective mode
        m_camera->setCameraMode(CameraMode::Perspective3D);
        LOG_INFO_C("Switched to Perspective view", "CameraController");
    } else {
        applyOrthographicView(mode);
    }
}

void CameraController::applyOrthographicView(ViewMode mode) {
    if (!m_camera || !m_player) return;
    
    // Calculate target position (center of view or current focus)
    glm::vec3 currentPos = m_camera->getPosition();
    glm::vec3 currentFront = m_camera->getFront();
    
    // If we were in perspective, calculate a good target point
    if (m_previousViewMode == ViewMode::Perspective) {
        // Use a point ahead of the camera as the target
        m_orthoTarget = currentPos + currentFront * 10.0f;
    }
    
    // Set camera mode to orthographic
    m_camera->setCameraMode(CameraMode::Perspective3D); // We'll handle ortho manually
    
    glm::vec3 newPos;
    float newPitch, newYaw;
    
    switch (mode) {
        case ViewMode::Top:
            // Look straight down (Y-)
            newPos = m_orthoTarget + glm::vec3(0.0f, m_orthoDistance, 0.0f);
            newPitch = -90.0f;
            newYaw = 0.0f;
            break;
            
        case ViewMode::Bottom:
            // Look straight up (Y+)
            newPos = m_orthoTarget + glm::vec3(0.0f, -m_orthoDistance, 0.0f);
            newPitch = 90.0f;
            newYaw = 0.0f;
            break;
            
        case ViewMode::Front:
            // Look along Z- (standard front view)
            newPos = m_orthoTarget + glm::vec3(0.0f, 0.0f, m_orthoDistance);
            newPitch = 0.0f;
            newYaw = -90.0f;
            break;
            
        case ViewMode::Back:
            // Look along Z+
            newPos = m_orthoTarget + glm::vec3(0.0f, 0.0f, -m_orthoDistance);
            newPitch = 0.0f;
            newYaw = 90.0f;
            break;
            
        case ViewMode::Side: // Right side
            // Look along X-
            newPos = m_orthoTarget + glm::vec3(m_orthoDistance, 0.0f, 0.0f);
            newPitch = 0.0f;
            newYaw = 0.0f;
            break;
            
        case ViewMode::Left:
            // Look along X+
            newPos = m_orthoTarget + glm::vec3(-m_orthoDistance, 0.0f, 0.0f);
            newPitch = 0.0f;
            newYaw = 180.0f;
            break;
            
        default:
            return;
    }
    
    if (m_smoothTransitions) {
        // Start smooth transition
        m_transitioning = true;
        m_transitionTime = 0.0f;
        m_transitionStartPos = m_camera->getPosition();
        m_transitionEndPos = newPos;
        m_transitionStartPitch = m_camera->getPitch();
        m_transitionStartYaw = m_camera->getYaw();
        m_transitionEndPitch = newPitch;
        m_transitionEndYaw = newYaw;
    } else {
        // Immediate change
        m_camera->setPosition(newPos);
        m_camera->setRotation(newPitch, newYaw);
        m_player->setPosition(newPos);
    }
    
    LOG_INFO_C("Switched to " + getViewModeName(mode) + " view", "CameraController");
}

void CameraController::focusOn(const glm::vec3& target, float distance) {
    if (!m_camera || !m_player) return;
    
    m_orthoTarget = target;
    
    // If distance not specified, use current distance from target
    if (distance < 0.0f) {
        distance = glm::length(m_camera->getPosition() - target);
        distance = std::max(distance, 5.0f); // Minimum distance
    }
    
    // Calculate new position maintaining current viewing direction
    glm::vec3 direction = glm::normalize(m_camera->getPosition() - target);
    glm::vec3 newPos = target + direction * distance;
    
    if (m_smoothTransitions) {
        m_transitioning = true;
        m_transitionTime = 0.0f;
        m_transitionStartPos = m_camera->getPosition();
        m_transitionEndPos = newPos;
        m_transitionStartPitch = m_camera->getPitch();
        m_transitionStartYaw = m_camera->getYaw();
        m_transitionEndPitch = m_camera->getPitch();
        m_transitionEndYaw = m_camera->getYaw();
    } else {
        m_camera->setPosition(newPos);
        m_player->setPosition(newPos);
    }
    
    LOG_INFO_C("Focused camera on target", "CameraController");
}

void CameraController::frameBox(const glm::vec3& min, const glm::vec3& max) {
    if (!m_camera || !m_player) return;
    
    // Calculate center and size
    glm::vec3 center = (min + max) * 0.5f;
    glm::vec3 size = max - min;
    float maxDim = std::max(std::max(size.x, size.y), size.z);
    
    // Calculate distance needed to fit the box in view
    float fov = m_camera->getFOV();
    float distance = maxDim / (2.0f * tan(glm::radians(fov * 0.5f)));
    distance *= 1.2f; // Add 20% padding
    
    frameSphere(center, distance);
}

void CameraController::frameSphere(const glm::vec3& center, float radius) {
    if (!m_camera || !m_player) return;
    
    m_orthoTarget = center;
    
    // Calculate distance based on FOV to fit sphere in view
    float fov = m_camera->getFOV();
    float distance = radius / tan(glm::radians(fov * 0.5f));
    distance *= 1.2f; // Add 20% padding
    
    focusOn(center, distance);
}

void CameraController::applySpeedToPlayer() {
    if (!m_player) return;
    
    // The actual speed application would depend on input state
    // This sets the base speeds that the player will use
    m_player->setWalkSpeed(m_normalSpeed);
    m_player->setSprintSpeed(m_fastSpeed);
    
    LOG_INFO_C("Applied camera speeds to player", "CameraController");
}

bool CameraController::saveBookmark(const std::string& name) {
    if (!m_camera || name.empty()) {
        LOG_WARNING_C("Cannot save bookmark: invalid name or camera", "CameraController");
        return false;
    }
    
    // Check if bookmark already exists and update it
    for (auto& bookmark : m_bookmarks) {
        if (bookmark.name == name) {
            bookmark.position = m_camera->getPosition();
            bookmark.pitch = m_camera->getPitch();
            bookmark.yaw = m_camera->getYaw();
            bookmark.viewMode = m_viewMode;
            LOG_INFO_C("Updated bookmark: " + name, "CameraController");
            return true;
        }
    }
    
    // Create new bookmark
    m_bookmarks.emplace_back(
        name,
        m_camera->getPosition(),
        m_camera->getPitch(),
        m_camera->getYaw(),
        m_viewMode
    );
    
    LOG_INFO_C("Saved bookmark: " + name, "CameraController");
    return true;
}

bool CameraController::loadBookmark(const std::string& name) {
    if (!m_camera || !m_player) {
        LOG_WARNING_C("Cannot load bookmark: camera or player not initialized", "CameraController");
        return false;
    }
    
    // Find bookmark
    for (const auto& bookmark : m_bookmarks) {
        if (bookmark.name == name) {
            if (m_smoothTransitions) {
                m_transitioning = true;
                m_transitionTime = 0.0f;
                m_transitionStartPos = m_camera->getPosition();
                m_transitionEndPos = bookmark.position;
                m_transitionStartPitch = m_camera->getPitch();
                m_transitionStartYaw = m_camera->getYaw();
                m_transitionEndPitch = bookmark.pitch;
                m_transitionEndYaw = bookmark.yaw;
            } else {
                m_camera->setPosition(bookmark.position);
                m_camera->setRotation(bookmark.pitch, bookmark.yaw);
                m_player->setPosition(bookmark.position);
            }
            
            setViewMode(bookmark.viewMode);
            
            LOG_INFO_C("Loaded bookmark: " + name, "CameraController");
            return true;
        }
    }
    
    LOG_WARNING_C("Bookmark not found: " + name, "CameraController");
    return false;
}

bool CameraController::deleteBookmark(const std::string& name) {
    auto it = std::remove_if(m_bookmarks.begin(), m_bookmarks.end(),
        [&name](const Bookmark& b) { return b.name == name; });
    
    if (it != m_bookmarks.end()) {
        m_bookmarks.erase(it, m_bookmarks.end());
        LOG_INFO_C("Deleted bookmark: " + name, "CameraController");
        return true;
    }
    
    LOG_WARNING_C("Bookmark not found: " + name, "CameraController");
    return false;
}

bool CameraController::saveBookmarksToFile(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        LOG_ERROR_C("Failed to open file for writing: " + filename, "CameraController");
        return false;
    }
    
    file << "# Fresh Voxel Engine - Camera Bookmarks\n";
    file << "# Format: name,posX,posY,posZ,pitch,yaw,viewMode\n";
    
    for (const auto& bookmark : m_bookmarks) {
        file << bookmark.name << ","
             << bookmark.position.x << "," << bookmark.position.y << "," << bookmark.position.z << ","
             << bookmark.pitch << "," << bookmark.yaw << ","
             << static_cast<int>(bookmark.viewMode) << "\n";
    }
    
    file.close();
    LOG_INFO_C("Saved bookmarks to file: " + filename, "CameraController");
    return true;
}

bool CameraController::loadBookmarksFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        LOG_WARNING_C("Could not open bookmarks file: " + filename, "CameraController");
        return false;
    }
    
    m_bookmarks.clear();
    std::string line;
    
    while (std::getline(file, line)) {
        // Skip comments and empty lines
        if (line.empty() || line[0] == '#') continue;
        
        std::istringstream iss(line);
        std::string name;
        glm::vec3 position;
        float pitch, yaw;
        int viewModeInt;
        
        // Parse CSV format
        if (std::getline(iss, name, ',')) {
            char comma;
            if (iss >> position.x >> comma >> position.y >> comma >> position.z >> comma
                    >> pitch >> comma >> yaw >> comma >> viewModeInt) {
                m_bookmarks.emplace_back(
                    name,
                    position,
                    pitch,
                    yaw,
                    static_cast<ViewMode>(viewModeInt)
                );
            }
        }
    }
    
    file.close();
    LOG_INFO_C("Loaded " + std::to_string(m_bookmarks.size()) + " bookmarks from file: " + filename, "CameraController");
    return true;
}

void CameraController::updateTransition(float deltaTime) {
    if (!m_transitioning || !m_camera || !m_player) return;
    
    m_transitionTime += deltaTime;
    float t = std::min(m_transitionTime / m_transitionDuration, 1.0f);
    
    // Smooth interpolation (ease in-out)
    t = t * t * (3.0f - 2.0f * t);
    
    // Interpolate position
    glm::vec3 currentPos = interpolatePosition(m_transitionStartPos, m_transitionEndPos, t);
    m_camera->setPosition(currentPos);
    m_player->setPosition(currentPos);
    
    // Interpolate rotation
    float currentPitch, currentYaw;
    interpolateRotation(m_transitionStartPitch, m_transitionStartYaw,
                       m_transitionEndPitch, m_transitionEndYaw,
                       t, currentPitch, currentYaw);
    m_camera->setRotation(currentPitch, currentYaw);
    
    // Check if transition is complete
    if (t >= 1.0f) {
        m_transitioning = false;
    }
}

glm::vec3 CameraController::interpolatePosition(const glm::vec3& from, const glm::vec3& to, float t) const {
    return glm::mix(from, to, t);
}

void CameraController::interpolateRotation(float fromPitch, float fromYaw, float toPitch, float toYaw,
                                          float t, float& outPitch, float& outYaw) const {
    // Handle yaw wrapping (shortest path)
    float yawDiff = toYaw - fromYaw;
    
    // Normalize to [-180, 180]
    while (yawDiff > 180.0f) yawDiff -= 360.0f;
    while (yawDiff < -180.0f) yawDiff += 360.0f;
    
    outYaw = fromYaw + yawDiff * t;
    outPitch = fromPitch + (toPitch - fromPitch) * t;
}

} // namespace fresh
