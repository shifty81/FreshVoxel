#pragma once
#include "SystemBase.h"
#include "HealthComponent.h"
#include "scripting/EventSystem.h"

namespace fresh
{
namespace ecs
{

/**
 * @brief System for managing combat interactions
 *
 * This system demonstrates:
 * - Event-Driven Architecture: Uses events for combat notifications
 * - Component-Based Architecture: Works with HealthComponent
 * - Decoupling: Other systems can react to combat events without direct coupling
 */
class CombatSystem : public SystemBase
{
public:
    explicit CombatSystem(EntityManager* manager) : SystemBase(manager) {}

    void initialize() override
    {
        // System is ready
    }

    void update(float deltaTime) override
    {
        if (!enabled)
            return;

        // Process all entities with health
        auto entities = entityManager->getEntitiesWithComponent<HealthComponent>();

        for (const auto& entity : entities) {
            auto* health = entityManager->getComponent<HealthComponent>(entity);
            if (!health)
                continue;

            // Check for death events
            if (!health->getIsAlive() && !processedDeaths.count(entity.getId())) {
                onEntityDeath(entity);
                processedDeaths.insert(entity.getId());
            }

            // Check for revive events
            if (health->getIsAlive() && processedDeaths.count(entity.getId())) {
                onEntityRevive(entity);
                processedDeaths.erase(entity.getId());
            }
        }
    }

    void shutdown() override
    {
        processedDeaths.clear();
    }

    /**
     * @brief Apply damage to an entity
     * Emits damage event for other systems to react
     */
    void applyDamage(Entity entity, float amount, Entity source = Entity())
    {
        auto* health = entityManager->getComponent<HealthComponent>(entity);
        if (!health || !health->getIsAlive())
            return;

        health->takeDamage(amount);

        // Emit damage event for other systems (AI, audio, particles, etc.)
        EventData eventData;
        eventData.set("entity", entity.getId());
        eventData.set("damage", amount);
        eventData.set("health", health->getCurrentHealth());
        eventData.set("maxHealth", health->getMaxHealth());
        if (source.isValid()) {
            eventData.set("source", source.getId());
        }

        EventSystem::getInstance().emit("entity_damaged", eventData);
    }

    /**
     * @brief Heal an entity
     * Emits heal event for feedback systems
     */
    void applyHealing(Entity entity, float amount)
    {
        auto* health = entityManager->getComponent<HealthComponent>(entity);
        if (!health || !health->getIsAlive())
            return;

        float oldHealth = health->getCurrentHealth();
        health->heal(amount);
        float newHealth = health->getCurrentHealth();
        float actualHealing = newHealth - oldHealth;

        if (actualHealing > 0.0f) {
            // Emit heal event
            EventData eventData;
            eventData.set("entity", entity.getId());
            eventData.set("healing", actualHealing);
            eventData.set("health", newHealth);
            eventData.set("maxHealth", health->getMaxHealth());

            EventSystem::getInstance().emit("entity_healed", eventData);
        }
    }

private:
    std::set<Entity::ID> processedDeaths;

    void onEntityDeath(Entity entity)
    {
        // Emit death event for AI, animation, audio, etc.
        EventData eventData;
        eventData.set("entity", entity.getId());

        EventSystem::getInstance().emit("entity_died", eventData);
    }

    void onEntityRevive(Entity entity)
    {
        // Emit revive event
        EventData eventData;
        eventData.set("entity", entity.getId());

        EventSystem::getInstance().emit("entity_revived", eventData);
    }
};

} // namespace ecs
} // namespace fresh
