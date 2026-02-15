#pragma once
#include "ecs/SystemBase.h"
#include <glm/glm.hpp>

namespace fresh
{
namespace ecs
{

/**
 * @brief System for 2D camera management
 *
 * Updates camera positions, handles following targets, and manages
 * camera bounds and zoom.
 */
class Camera2DSystem : public SystemBase
{
public:
    explicit Camera2DSystem(EntityManager* manager);
    virtual ~Camera2DSystem() = default;

    /**
     * @brief Initialize camera system
     */
    void initialize() override;

    /**
     * @brief Update cameras (follow targets, apply bounds)
     */
    void update(float deltaTime) override;

    /**
     * @brief Get the primary camera entity
     */
    Entity getPrimaryCamera() const;

    /**
     * @brief Set screen aspect ratio for all cameras
     */
    void setScreenAspectRatio(float aspectRatio);

private:
    Entity primaryCamera_;
    
    /**
     * @brief Update camera follow behavior
     */
    void updateFollow(class Camera2DComponent* camera, 
                     class Transform2DComponent* cameraTransform,
                     float deltaTime);

    /**
     * @brief Apply camera bounds
     */
    void applyBounds(class Camera2DComponent* camera,
                    class Transform2DComponent* cameraTransform);
};

} // namespace ecs
} // namespace fresh
