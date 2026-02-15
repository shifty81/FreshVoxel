#pragma once
#include "Entity.h"
#include "EntityManager.h"

namespace fresh
{
namespace ecs
{

/**
 * @brief Base class for game systems
 *
 * Systems contain the logic that processes entities with specific components.
 * Each system should focus on a single responsibility.
 */
class SystemBase
{
public:
    explicit SystemBase(EntityManager* manager) : entityManager(manager), enabled(true) {}
    virtual ~SystemBase() = default;

    /**
     * @brief Update the system
     * @param deltaTime Time since last update in seconds
     */
    virtual void update(float deltaTime) = 0;

    /**
     * @brief Initialize the system
     */
    virtual void initialize() {}

    /**
     * @brief Shutdown the system
     */
    virtual void shutdown() {}

    // Enable/disable system
    void setEnabled(bool value)
    {
        enabled = value;
    }
    bool isEnabled() const
    {
        return enabled;
    }

protected:
    EntityManager* entityManager;
    bool enabled;
};

} // namespace ecs
} // namespace fresh
