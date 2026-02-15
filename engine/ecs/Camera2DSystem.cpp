#include "ecs/Camera2DSystem.h"
#include "ecs/Camera2DComponent.h"
#include "ecs/Transform2DComponent.h"
#include "ecs/EntityManager.h"
#include <algorithm>

namespace fresh
{
namespace ecs
{

Camera2DSystem::Camera2DSystem(EntityManager* manager)
    : SystemBase(manager)
{
}

void Camera2DSystem::initialize()
{
    // Find primary camera
    primaryCamera_ = Entity();
    
    for (const auto& entity : entityManager->getAllEntities())
    {
        auto* camera = entityManager->getComponent<Camera2DComponent>(entity);
        if (camera && camera->isPrimary)
        {
            primaryCamera_ = entity;
            break;
        }
    }
}

void Camera2DSystem::update(float deltaTime)
{
    if (!enabled)
        return;

    // Update all cameras
    for (const auto& entity : entityManager->getAllEntities())
    {
        auto* camera = entityManager->getComponent<Camera2DComponent>(entity);
        auto* transform = entityManager->getComponent<Transform2DComponent>(entity);
        
        if (!camera || !transform)
            continue;
        
        // Clamp zoom
        camera->zoom = std::clamp(camera->zoom, camera->minZoom, camera->maxZoom);
        
        // Update follow behavior
        if (camera->followTarget.isValid())
        {
            updateFollow(camera, transform, deltaTime);
        }
        
        // Apply bounds
        if (camera->useBounds)
        {
            applyBounds(camera, transform);
        }
    }
}

Entity Camera2DSystem::getPrimaryCamera() const
{
    return primaryCamera_;
}

void Camera2DSystem::setScreenAspectRatio(float aspectRatio)
{
    for (const auto& entity : entityManager->getAllEntities())
    {
        auto* camera = entityManager->getComponent<Camera2DComponent>(entity);
        if (camera)
        {
            camera->aspectRatio = aspectRatio;
        }
    }
}

void Camera2DSystem::updateFollow(Camera2DComponent* camera,
                                  Transform2DComponent* cameraTransform,
                                  float deltaTime)
{
    auto* targetTransform = entityManager->getComponent<Transform2DComponent>(camera->followTarget);
    if (!targetTransform)
    {
        camera->followTarget = Entity(); // Invalid target or target has no transform
        return;
    }
    
    // Calculate target position
    glm::vec2 targetPos = targetTransform->position + camera->followOffset;
    
    // Apply smoothing
    if (camera->followSmoothing > 0.0f)
    {
        float smoothFactor = 1.0f - std::pow(camera->followSmoothing, deltaTime * 60.0f);
        cameraTransform->position = glm::mix(cameraTransform->position, targetPos, smoothFactor);
    }
    else
    {
        // Instant follow
        cameraTransform->position = targetPos;
    }
}

void Camera2DSystem::applyBounds(Camera2DComponent* camera,
                                Transform2DComponent* cameraTransform)
{
    // Calculate camera extents
    float effectiveSize = camera->orthographicSize / camera->zoom;
    float halfWidth = effectiveSize * camera->aspectRatio;
    float halfHeight = effectiveSize;
    
    // Clamp position to bounds
    glm::vec2 minPos = camera->boundsMin + glm::vec2(halfWidth, halfHeight);
    glm::vec2 maxPos = camera->boundsMax - glm::vec2(halfWidth, halfHeight);
    
    cameraTransform->position.x = std::clamp(cameraTransform->position.x, minPos.x, maxPos.x);
    cameraTransform->position.y = std::clamp(cameraTransform->position.y, minPos.y, maxPos.y);
}

} // namespace ecs
} // namespace fresh
