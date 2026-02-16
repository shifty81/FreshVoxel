#include "ecs/Animation2DSystem.h"
#include "ecs/Animation2DComponent.h"
#include "ecs/Sprite2DComponent.h"
#include "ecs/EntityManager.h"

namespace fresh
{
namespace ecs
{

Animation2DSystem::Animation2DSystem(EntityManager* manager)
    : SystemBase(manager)
{
}

void Animation2DSystem::initialize()
{
    // Animation system initialization
}

void Animation2DSystem::update(float deltaTime)
{
    if (!enabled)
        return;

    // Update all entities with animation and sprite components
    for (const auto& entity : entityManager->getAllEntities())
    {
        auto* animation = entityManager->getComponent<Animation2DComponent>(entity);
        auto* sprite = entityManager->getComponent<Sprite2DComponent>(entity);
        
        if (animation && sprite)
        {
            updateAnimation(animation, sprite, deltaTime);
        }
    }
}

void Animation2DSystem::updateAnimation(Animation2DComponent* animation, 
                                       Sprite2DComponent* sprite, 
                                       float deltaTime)
{
    // Skip if not playing or paused
    if (!animation->playing || animation->paused)
        return;
    
    // Get current clip
    auto it = animation->clips.find(animation->currentClip);
    if (it == animation->clips.end())
        return;
    
    const auto& clip = it->second;
    
    // Check if clip has frames
    if (clip.frames.empty())
        return;
    
    // Update frame timer
    animation->frameTimer += deltaTime * animation->playbackSpeed;
    
    // Calculate frame duration
    float frameDuration = 1.0f / clip.frameRate;
    
    // Advance frames
    while (animation->frameTimer >= frameDuration)
    {
        animation->frameTimer -= frameDuration;
        animation->currentFrame++;
        
        // Handle loop/end
        if (animation->currentFrame >= static_cast<int>(clip.frames.size()))
        {
            if (clip.loop)
            {
                animation->currentFrame = 0;
            }
            else
            {
                // Animation finished
                animation->currentFrame = static_cast<int>(clip.frames.size()) - 1;
                animation->playing = false;
                break;
            }
        }
    }
    
    // Update sprite with current frame
    glm::ivec2 frameCoords = animation->getCurrentFrameCoords();
    sprite->setSprite(frameCoords.x, frameCoords.y);
}

} // namespace ecs
} // namespace fresh
