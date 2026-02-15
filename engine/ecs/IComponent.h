#pragma once
#include <cstddef>
#include <typeindex>

namespace fresh
{
namespace ecs
{

/**
 * @brief Interface for all components in the ECS
 *
 * Components are pure data containers that can be attached to entities.
 * They should not contain logic - that belongs in Systems.
 */
class IComponent
{
public:
    virtual ~IComponent() = default;

    /**
     * @brief Get the type index of this component
     * Used for type identification in the ECS
     */
    virtual std::type_index getTypeIndex() const = 0;

    /**
     * @brief Clone this component
     * Used when copying entities
     */
    virtual IComponent* clone() const = 0;
};

/**
 * @brief Helper macro to implement component type identification
 */
#define COMPONENT_TYPE(ComponentClass)                                                             \
    std::type_index getTypeIndex() const override                                                  \
    {                                                                                              \
        return std::type_index(typeid(ComponentClass));                                            \
    }                                                                                              \
    IComponent* clone() const override                                                             \
    {                                                                                              \
        return new ComponentClass(*this);                                                          \
    }

} // namespace ecs
} // namespace fresh
