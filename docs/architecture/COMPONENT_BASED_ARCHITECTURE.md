# Component-Based Architecture (CBA) Guide

## Overview

Fresh Voxel Engine uses a **Component-Based Architecture** (also known as Entity-Component-System or ECS) that treats game objects as collections of independent components rather than monolithic classes. This architectural pattern is fundamental to modern game engines and enables:

- **Modularity**: Each component manages specific, independent functionality
- **Flexibility**: Easy to add, remove, or modify behaviors without affecting other systems
- **Reusability**: Components can be shared across different entity types
- **Testability**: Components can be unit-tested in isolation

## Key Concepts

### 1. Entity

An **Entity** is a unique identifier (ID) representing a game object. It's lightweight and contains no data or behavior itself.

```cpp
#include "ecs/Entity.h"

// Create an entity
Entity player = entityManager->createEntity();
Entity enemy = entityManager->createEntity();
```

### 2. Component

A **Component** is a pure data container attached to entities. Components should not contain logic - that belongs in Systems.

```cpp
#include "ecs/HealthComponent.h"
#include "ecs/MovementComponent.h"

// Add components to entity
entityManager->addComponent(player, new HealthComponent(100.0f));
entityManager->addComponent(player, new MovementComponent(5.0f, 10.0f));
```

### 3. System

A **System** contains the logic that processes entities with specific components. Each system focuses on a single responsibility.

```cpp
#include "ecs/CombatSystem.h"

// Create and use a system
CombatSystem combatSystem(entityManager);
combatSystem.initialize();

// Apply damage to entity
combatSystem.applyDamage(enemy, 25.0f);

// Update system each frame
combatSystem.update(deltaTime);
```

## Built-in Components

### HealthComponent

Manages entity health, damage, healing, and death state.

```cpp
HealthComponent health(100.0f);  // Max health 100

health.takeDamage(30.0f);
float current = health.getCurrentHealth();  // 70.0
float percentage = health.getHealthPercentage();  // 0.7

health.heal(20.0f);
bool alive = health.getIsAlive();

if (!alive) {
    health.revive();  // Restore to full health
}
```

**Benefits:**
- Any entity can have health without inheritance
- Easy to test damage/healing logic in isolation
- Can be added/removed dynamically

### MovementComponent

Manages entity movement, velocity, and acceleration.

```cpp
MovementComponent movement(5.0f, 10.0f);  // maxSpeed, acceleration

movement.setDesiredDirection(glm::vec3(1.0f, 0.0f, 0.0f));
movement.update(deltaTime);

glm::vec3 velocity = movement.getVelocity();
float currentSpeed = movement.getCurrentSpeed();

movement.setCanMove(false);  // Disable movement
movement.stop();  // Immediately stop
```

**Benefits:**
- Separates movement from other entity behaviors
- Different entities can have different movement characteristics
- Easy to implement movement buffs/debuffs

### Transform2DComponent

Position, rotation, and scale for 2D entities.

```cpp
Transform2DComponent transform;
transform.setPosition(glm::vec2(100.0f, 200.0f));
transform.setRotation(45.0f);
transform.setScale(glm::vec2(2.0f, 2.0f));
```

### RigidBody2DComponent

Physics properties for 2D entities.

```cpp
RigidBody2DComponent rigidBody;
rigidBody.setMass(1.0f);
rigidBody.setVelocity(glm::vec2(5.0f, 0.0f));
rigidBody.applyForce(glm::vec2(0.0f, -9.8f));  // Gravity
```

## Built-in Systems

### CombatSystem

Processes combat interactions and emits events.

```cpp
CombatSystem combatSystem(entityManager);

// Apply damage (emits "entity_damaged" event)
combatSystem.applyDamage(target, 30.0f, attacker);

// Apply healing (emits "entity_healed" event)
combatSystem.applyHealing(player, 25.0f);

// Update detects deaths and emits "entity_died" events
combatSystem.update(deltaTime);
```

### Physics2DSystem

Updates physics for 2D entities with RigidBody2DComponent.

### Collision2DSystem

Detects and resolves collisions between 2D entities.

### Camera2DSystem

Manages camera following and bounds for 2D scenes.

## Creating Custom Components

### Step 1: Define Component Class

```cpp
// engine/ecs/StaminaComponent.h
#pragma once
#include "ecs/IComponent.h"

namespace fresh::ecs
{

class StaminaComponent : public IComponent
{
public:
    StaminaComponent(float maxStamina = 100.0f)
        : maxStamina(maxStamina), currentStamina(maxStamina)
    {
    }

    COMPONENT_TYPE(StaminaComponent)  // Required macro

    void consume(float amount)
    {
        currentStamina = std::max(0.0f, currentStamina - amount);
    }

    void regenerate(float amount)
    {
        currentStamina = std::min(maxStamina, currentStamina + amount);
    }

    float getCurrentStamina() const { return currentStamina; }
    float getMaxStamina() const { return maxStamina; }
    float getStaminaPercentage() const { return currentStamina / maxStamina; }

private:
    float maxStamina;
    float currentStamina;
};

} // namespace fresh::ecs
```

### Step 2: Use in EntityManager

```cpp
Entity player = entityManager->createEntity();
entityManager->addComponent(player, new StaminaComponent(100.0f));

auto* stamina = entityManager->getComponent<StaminaComponent>(player);
stamina->consume(25.0f);  // Use stamina for sprint
```

### Step 3: Create System (Optional)

```cpp
// engine/ecs/StaminaSystem.h
#pragma once
#include "ecs/SystemBase.h"
#include "ecs/StaminaComponent.h"

namespace fresh::ecs
{

class StaminaSystem : public SystemBase
{
public:
    explicit StaminaSystem(EntityManager* manager) : SystemBase(manager) {}

    void update(float deltaTime) override
    {
        if (!enabled) return;

        // Regenerate stamina for all entities
        auto entities = entityManager->getEntitiesWithComponent<StaminaComponent>();
        
        for (const auto& entity : entities) {
            auto* stamina = entityManager->getComponent<StaminaComponent>(entity);
            stamina->regenerate(10.0f * deltaTime);  // 10 stamina/sec
        }
    }
};

} // namespace fresh::ecs
```

## Querying Entities

### Get entities with single component

```cpp
auto entities = entityManager->getEntitiesWithComponent<HealthComponent>();

for (const auto& entity : entities) {
    auto* health = entityManager->getComponent<HealthComponent>(entity);
    // Process health
}
```

### Get entities with multiple components

```cpp
// Entities that have BOTH HealthComponent AND MovementComponent
auto entities = entityManager->getEntitiesWithComponents<HealthComponent, MovementComponent>();

for (const auto& entity : entities) {
    auto* health = entityManager->getComponent<HealthComponent>(entity);
    auto* movement = entityManager->getComponent<MovementComponent>(entity);
    // Process entities with both health and movement
}
```

## Benefits for Development & Testing

### 1. Modularity

Different aspects of an entity are separated into components:

```cpp
// Player entity composition
Entity player = entityManager->createEntity();
entityManager->addComponent(player, new HealthComponent(100.0f));
entityManager->addComponent(player, new MovementComponent(5.0f, 10.0f));
entityManager->addComponent(player, new InventoryComponent());
entityManager->addComponent(player, new TransformComponent());
```

Each component can be modified independently without affecting others.

### 2. Easy Testing

Components can be unit-tested in isolation:

```cpp
TEST(HealthComponentTest, TakeDamage_ReducesHealth)
{
    HealthComponent health(100.0f);
    health.takeDamage(25.0f);
    EXPECT_FLOAT_EQ(health.getCurrentHealth(), 75.0f);
}
```

No need to set up a complete game state - just test the component.

### 3. Rapid Iteration

Want to try a new behavior? Just add a component:

```cpp
// Make player immune to damage temporarily
entityManager->addComponent(player, new ImmunityComponent(5.0f));  // 5 seconds

// Experiment complete? Remove it
entityManager->removeComponent<ImmunityComponent>(player);
```

### 4. Flexible Entity Types

The same components can create vastly different entities:

```cpp
// Zombie: slow, low health, melee attack
Entity zombie = createEnemy(40.0f, 2.0f, "melee");

// Skeleton: fast, medium health, ranged attack
Entity skeleton = createEnemy(60.0f, 4.5f, "ranged");

Entity createEnemy(float health, float speed, const char* attackType)
{
    Entity enemy = entityManager->createEntity();
    entityManager->addComponent(enemy, new HealthComponent(health));
    entityManager->addComponent(enemy, new MovementComponent(speed, 5.0f));
    entityManager->addComponent(enemy, new AttackComponent(attackType));
    return enemy;
}
```

### 5. Emergent Gameplay

Combine components in unexpected ways:

```cpp
// Explosive barrel (has health but no movement)
Entity barrel = entityManager->createEntity();
entityManager->addComponent(barrel, new HealthComponent(10.0f));
entityManager->addComponent(barrel, new ExplosiveComponent(5.0f));  // Explosion radius

// When barrel takes damage and dies, ExplosiveSystem triggers explosion
```

## Integration with Event System

Components work seamlessly with the event-driven architecture:

```cpp
// System emits events when component state changes
combatSystem.applyDamage(enemy, 50.0f);
// -> Emits "entity_damaged" event

// Other systems can listen and react
EventSystem::getInstance().subscribe("entity_damaged", [](const EventData& data) {
    // Play damage sound
    // Show damage number
    // Trigger AI reaction
});
```

See [EVENT_DRIVEN_ARCHITECTURE.md](EVENT_DRIVEN_ARCHITECTURE.md) for more details.

## Best Practices

### ✅ DO

- **Keep components simple**: Pure data, minimal logic
- **Single responsibility**: One component, one purpose
- **Use systems for logic**: Put behavior in systems, not components
- **Test in isolation**: Unit test components independently
- **Document component purpose**: Clear, concise description

### ❌ DON'T

- **Don't add logic to components**: Keep them as data containers
- **Don't directly reference other components**: Use systems to coordinate
- **Don't make components depend on each other**: Keep them independent
- **Don't store entity references in components**: Store entity IDs if needed

## Performance Considerations

The ECS architecture is designed for performance:

- **Cache-friendly**: Components of the same type stored together
- **Parallel processing**: Systems can process entities independently
- **Easy optimization**: Query and process only entities with needed components

```cpp
// Only process entities that have both health and position
// (ignore entities with just health or just position)
auto entities = entityManager->getEntitiesWithComponents<HealthComponent, TransformComponent>();
```

## Examples

See the following test files for practical examples:

- `tests/ecs/HealthComponentTests.cpp` - Component testing examples
- `tests/ecs/MovementComponentTests.cpp` - Movement logic testing
- `tests/ecs/CombatSystemTests.cpp` - System integration examples

## Further Reading

- [EVENT_DRIVEN_ARCHITECTURE.md](EVENT_DRIVEN_ARCHITECTURE.md) - How events integrate with ECS
- [DATA_DRIVEN_DESIGN.md](DATA_DRIVEN_DESIGN.md) - Loading component data from JSON
- [TESTING_GUIDE.md](../testing/TESTING.md) - Testing strategies for components and systems
