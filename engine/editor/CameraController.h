#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <string>
#include <vector>
#include <memory>

namespace fresh {

class Camera;
class Player;

/**
 * @brief Enhanced camera controller for editor
 * 
 * Provides advanced camera features for the editor including:
 * - Orthographic views (Top, Front, Side)
 * - Focus on selection (F key)
 * - Frame selection in view
 * - Camera speed controls
 * - Camera bookmarks/presets
 */
class CameraController {
public:
    /**
     * @brief Camera view modes
     */
    enum class ViewMode {
        Perspective,    ///< Standard 3D perspective view
        Top,           ///< Orthographic top view (looking down)
        Front,         ///< Orthographic front view
        Side,          ///< Orthographic side view (right)
        Bottom,        ///< Orthographic bottom view (looking up)
        Back,          ///< Orthographic back view
        Left           ///< Orthographic left view
    };

    /**
     * @brief Camera bookmark data
     */
    struct Bookmark {
        std::string name;
        glm::vec3 position;
        float pitch;
        float yaw;
        ViewMode viewMode;
        
        Bookmark(const std::string& n, const glm::vec3& pos, float p, float y, ViewMode mode)
            : name(n), position(pos), pitch(p), yaw(y), viewMode(mode) {}
    };

    /**
     * @brief Construct camera controller
     */
    CameraController();

    /**
     * @brief Initialize with player reference
     * @param player Player instance
     */
    void initialize(Player* player);

    /**
     * @brief Update camera controller
     * @param deltaTime Time since last frame
     */
    void update(float deltaTime);

    // View mode controls
    
    /**
     * @brief Set camera view mode
     * @param mode View mode to set
     */
    void setViewMode(ViewMode mode);

    /**
     * @brief Get current view mode
     * @return Current view mode
     */
    ViewMode getViewMode() const { return m_viewMode; }

    /**
     * @brief Get view mode name as string
     * @param mode View mode
     * @return View mode name
     */
    static std::string getViewModeName(ViewMode mode);

    // Selection-based camera operations

    /**
     * @brief Focus camera on a world position
     * @param target Target position
     * @param distance Distance from target (default maintains current)
     */
    void focusOn(const glm::vec3& target, float distance = -1.0f);

    /**
     * @brief Frame a bounding box in view
     * @param min Minimum corner of bounding box
     * @param max Maximum corner of bounding box
     */
    void frameBox(const glm::vec3& min, const glm::vec3& max);

    /**
     * @brief Frame a sphere in view
     * @param center Center of sphere
     * @param radius Radius of sphere
     */
    void frameSphere(const glm::vec3& center, float radius);

    // Camera speed controls

    /**
     * @brief Set normal camera speed
     * @param speed Speed value
     */
    void setNormalSpeed(float speed) { m_normalSpeed = speed; }

    /**
     * @brief Get normal camera speed
     * @return Normal speed
     */
    float getNormalSpeed() const { return m_normalSpeed; }

    /**
     * @brief Set fast camera speed (with Shift)
     * @param speed Speed value
     */
    void setFastSpeed(float speed) { m_fastSpeed = speed; }

    /**
     * @brief Get fast camera speed
     * @return Fast speed
     */
    float getFastSpeed() const { return m_fastSpeed; }

    /**
     * @brief Set slow camera speed (with Ctrl)
     * @param speed Speed value
     */
    void setSlowSpeed(float speed) { m_slowSpeed = speed; }

    /**
     * @brief Get slow camera speed
     * @return Slow speed
     */
    float getSlowSpeed() const { return m_slowSpeed; }

    /**
     * @brief Apply current speed to player
     */
    void applySpeedToPlayer();

    // Camera bookmarks

    /**
     * @brief Save current camera position as bookmark
     * @param name Bookmark name
     * @return true if saved successfully
     */
    bool saveBookmark(const std::string& name);

    /**
     * @brief Load a bookmark
     * @param name Bookmark name
     * @return true if loaded successfully
     */
    bool loadBookmark(const std::string& name);

    /**
     * @brief Delete a bookmark
     * @param name Bookmark name
     * @return true if deleted successfully
     */
    bool deleteBookmark(const std::string& name);

    /**
     * @brief Get list of all bookmarks
     * @return Vector of bookmarks
     */
    const std::vector<Bookmark>& getBookmarks() const { return m_bookmarks; }

    /**
     * @brief Save bookmarks to file
     * @param filename File path
     * @return true if saved successfully
     */
    bool saveBookmarksToFile(const std::string& filename) const;

    /**
     * @brief Load bookmarks from file
     * @param filename File path
     * @return true if loaded successfully
     */
    bool loadBookmarksFromFile(const std::string& filename);

    // Smooth camera transitions

    /**
     * @brief Enable smooth camera transitions
     * @param enabled true to enable
     */
    void setSmoothTransitions(bool enabled) { m_smoothTransitions = enabled; }

    /**
     * @brief Check if smooth transitions are enabled
     * @return true if enabled
     */
    bool isSmoothTransitionsEnabled() const { return m_smoothTransitions; }

    /**
     * @brief Set transition duration
     * @param duration Duration in seconds
     */
    void setTransitionDuration(float duration) { m_transitionDuration = duration; }

    /**
     * @brief Get transition duration
     * @return Duration in seconds
     */
    float getTransitionDuration() const { return m_transitionDuration; }

private:
    /**
     * @brief Apply orthographic view for given mode
     * @param mode View mode
     */
    void applyOrthographicView(ViewMode mode);

    /**
     * @brief Update smooth transition
     * @param deltaTime Time since last frame
     */
    void updateTransition(float deltaTime);

    /**
     * @brief Interpolate between two positions
     * @param from Start position
     * @param to End position
     * @param t Interpolation factor (0-1)
     * @return Interpolated position
     */
    glm::vec3 interpolatePosition(const glm::vec3& from, const glm::vec3& to, float t) const;

    /**
     * @brief Interpolate between two rotations
     * @param fromPitch Start pitch
     * @param fromYaw Start yaw
     * @param toPitch End pitch
     * @param toYaw End yaw
     * @param t Interpolation factor (0-1)
     * @param outPitch Output pitch
     * @param outYaw Output yaw
     */
    void interpolateRotation(float fromPitch, float fromYaw, float toPitch, float toYaw, 
                            float t, float& outPitch, float& outYaw) const;

private:
    Player* m_player = nullptr;
    Camera* m_camera = nullptr;
    
    ViewMode m_viewMode = ViewMode::Perspective;
    ViewMode m_previousViewMode = ViewMode::Perspective;
    
    // Camera speeds
    float m_normalSpeed = 10.0f;
    float m_fastSpeed = 30.0f;
    float m_slowSpeed = 3.0f;
    
    // Bookmarks
    std::vector<Bookmark> m_bookmarks;
    
    // Smooth transitions
    bool m_smoothTransitions = true;
    float m_transitionDuration = 0.3f;
    bool m_transitioning = false;
    float m_transitionTime = 0.0f;
    
    // Transition state
    glm::vec3 m_transitionStartPos;
    glm::vec3 m_transitionEndPos;
    float m_transitionStartPitch;
    float m_transitionStartYaw;
    float m_transitionEndPitch;
    float m_transitionEndYaw;
    
    // Orthographic view parameters
    float m_orthoDistance = 50.0f; // Distance from target in ortho views
    glm::vec3 m_orthoTarget{0.0f, 0.0f, 0.0f}; // Target point for ortho views
};

} // namespace fresh
