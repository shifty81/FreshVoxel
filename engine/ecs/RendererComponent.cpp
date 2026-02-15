#include "ecs/RendererComponent.h"

namespace fresh
{
namespace ecs
{

RendererComponent::RendererComponent()
    : castShadows(true), receiveShadows(true), renderLayer(0), visible(true)
{
}

} // namespace ecs
} // namespace fresh
