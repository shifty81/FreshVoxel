#pragma once
#include "ecs/IComponent.h"
#include "Inventory.h"

namespace fresh
{
namespace rpg
{

/**
 * @brief Component for entity inventory
 */
class InventoryComponent : public ecs::IComponent
{
public:
    explicit InventoryComponent(float capacity = 1000.0f);
    virtual ~InventoryComponent() = default;

    COMPONENT_TYPE(InventoryComponent)

    Inventory& getInventory()
    {
        return inventory;
    }
    const Inventory& getInventory() const
    {
        return inventory;
    }

private:
    Inventory inventory;
};

} // namespace rpg
} // namespace fresh
