#include "ecs/MaterialComponent.h"

namespace fresh
{
namespace ecs
{

MaterialComponent::MaterialComponent()
    : shaderType(ShaderType::Standard),
      color(1.0f, 1.0f, 1.0f, 1.0f),
      texturePath(""),
      metallic(0.0f),
      smoothness(0.5f),
      emission(0.0f)
{
}

} // namespace ecs
} // namespace fresh
