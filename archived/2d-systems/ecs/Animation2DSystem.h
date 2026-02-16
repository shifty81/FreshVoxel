#pragma once
#include "ecs/SystemBase.h"

namespace fresh
{
namespace ecs
{

/**
 * @brief System for 2D sprite animation playback
 *
 * Updates Animation2D components, advancing frames based on time
 * and updating Sprite2D components with current frame.
 */
class Animation2DSystem : public SystemBase
{
public:
    explicit Animation2DSystem(EntityManager* manager);
    virtual ~Animation2DSystem() = default;

    /**
     * @brief Initialize animation system
     */
    void initialize() override;

    /**
     * @brief Update all animations
     */
    void update(float deltaTime) override;

private:
    /**
     * @brief Update a single animation component
     */
    void updateAnimation(class Animation2DComponent* animation, 
                        class Sprite2DComponent* sprite, 
                        float deltaTime);
};

} // namespace ecs
} // namespace fresh
