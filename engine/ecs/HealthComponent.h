#pragma once
#include "IComponent.h"

namespace fresh
{
namespace ecs
{

/**
 * @brief Component for managing entity health
 *
 * This component demonstrates Component-Based Architecture by:
 * - Encapsulating health-related data
 * - Being independent and reusable
 * - Allowing entities to have health without inheritance
 */
class HealthComponent : public IComponent
{
public:
    HealthComponent(float maxHealth = 100.0f)
        : maxHealth(maxHealth), currentHealth(maxHealth), isAlive(true)
    {
    }

    COMPONENT_TYPE(HealthComponent)

    // Health management
    void takeDamage(float amount)
    {
        if (!isAlive)
            return;

        currentHealth -= amount;
        if (currentHealth <= 0.0f) {
            currentHealth = 0.0f;
            isAlive = false;
        }
    }

    void heal(float amount)
    {
        if (!isAlive)
            return;

        currentHealth += amount;
        if (currentHealth > maxHealth) {
            currentHealth = maxHealth;
        }
    }

    void setMaxHealth(float max)
    {
        maxHealth = max;
        if (currentHealth > maxHealth) {
            currentHealth = maxHealth;
        }
    }

    // Getters
    float getCurrentHealth() const
    {
        return currentHealth;
    }
    float getMaxHealth() const
    {
        return maxHealth;
    }
    float getHealthPercentage() const
    {
        return (maxHealth > 0.0f) ? (currentHealth / maxHealth) : 0.0f;
    }
    bool getIsAlive() const
    {
        return isAlive;
    }

    // Restore to full health
    void revive()
    {
        currentHealth = maxHealth;
        isAlive = true;
    }

private:
    float maxHealth;
    float currentHealth;
    bool isAlive;
};

} // namespace ecs
} // namespace fresh
