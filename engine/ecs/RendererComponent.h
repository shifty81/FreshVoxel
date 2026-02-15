#pragma once
#include "ecs/IComponent.h"

namespace fresh
{
namespace ecs
{

/**
 * @brief Component for rendering properties
 *
 * Controls how an entity is rendered, including shadow casting/receiving
 * and render layer settings.
 */
class RendererComponent : public IComponent
{
public:
    RendererComponent();
    virtual ~RendererComponent() = default;

    COMPONENT_TYPE(RendererComponent)

    // Rendering properties
    bool castShadows;
    bool receiveShadows;
    int renderLayer; // 0-31, used for selective rendering
    bool visible;    // Whether this object should be rendered
};

} // namespace ecs
} // namespace fresh
