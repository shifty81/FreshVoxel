/**
 * @file CombatSystemTests.cpp
 * @brief Integration tests for CombatSystem demonstrating Event-Driven Architecture
 */

#include <gtest/gtest.h>
#include "ecs/CombatSystem.h"
#include "ecs/HealthComponent.h"
#include "ecs/EntityManager.h"
#include "scripting/EventSystem.h"

using namespace fresh;
using namespace fresh::ecs;

/**
 * Test fixture for CombatSystem
 */
class CombatSystemTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventSystem::getInstance().clear();
        entityManager = std::make_unique<EntityManager>();
        combatSystem = std::make_unique<CombatSystem>(entityManager.get());
        combatSystem->initialize();
    }

    void TearDown() override
    {
        combatSystem->shutdown();
        combatSystem.reset();
        entityManager.reset();
        EventSystem::getInstance().clear();
    }

    std::unique_ptr<EntityManager> entityManager;
    std::unique_ptr<CombatSystem> combatSystem;
};

/**
 * Test basic damage application
 */
TEST_F(CombatSystemTest, ApplyDamage_ReducesHealth)
{
    // Arrange
    Entity entity = entityManager->createEntity();
    entityManager->addComponent(entity, new HealthComponent(100.0f));

    // Act
    combatSystem->applyDamage(entity, 30.0f);

    // Assert
    auto* health = entityManager->getComponent<HealthComponent>(entity);
    EXPECT_FLOAT_EQ(health->getCurrentHealth(), 70.0f);
}

/**
 * Test damage event is emitted (Event-Driven Architecture)
 */
TEST_F(CombatSystemTest, ApplyDamage_EmitsDamageEvent)
{
    // Arrange
    Entity entity = entityManager->createEntity();
    entityManager->addComponent(entity, new HealthComponent(100.0f));

    bool eventReceived = false;
    float receivedDamage = 0.0f;

    EventSystem::getInstance().subscribe("entity_damaged", [&](const EventData& data) {
        eventReceived = true;
        receivedDamage = data.get<float>("damage");
    });

    // Act
    combatSystem->applyDamage(entity, 25.0f);

    // Assert
    EXPECT_TRUE(eventReceived);
    EXPECT_FLOAT_EQ(receivedDamage, 25.0f);
}

/**
 * Test healing functionality
 */
TEST_F(CombatSystemTest, ApplyHealing_IncreasesHealth)
{
    // Arrange
    Entity entity = entityManager->createEntity();
    auto* health = entityManager->addComponent(entity, new HealthComponent(100.0f));
    health->takeDamage(50.0f);

    // Act
    combatSystem->applyHealing(entity, 20.0f);

    // Assert
    EXPECT_FLOAT_EQ(health->getCurrentHealth(), 70.0f);
}

/**
 * Test healing event is emitted
 */
TEST_F(CombatSystemTest, ApplyHealing_EmitsHealEvent)
{
    // Arrange
    Entity entity = entityManager->createEntity();
    auto* health = entityManager->addComponent(entity, new HealthComponent(100.0f));
    health->takeDamage(50.0f);

    bool eventReceived = false;
    float receivedHealing = 0.0f;

    EventSystem::getInstance().subscribe("entity_healed", [&](const EventData& data) {
        eventReceived = true;
        receivedHealing = data.get<float>("healing");
    });

    // Act
    combatSystem->applyHealing(entity, 25.0f);

    // Assert
    EXPECT_TRUE(eventReceived);
    EXPECT_FLOAT_EQ(receivedHealing, 25.0f);
}

/**
 * Test death event detection (demonstrates system update loop)
 */
TEST_F(CombatSystemTest, Update_DetectsEntityDeath)
{
    // Arrange
    Entity entity = entityManager->createEntity();
    entityManager->addComponent(entity, new HealthComponent(100.0f));

    bool deathEventReceived = false;
    EventSystem::getInstance().subscribe("entity_died", [&](const EventData& /*data*/) {
        deathEventReceived = true;
    });

    // Act
    combatSystem->applyDamage(entity, 100.0f);
    combatSystem->update(0.016f); // Simulate one frame

    // Assert
    EXPECT_TRUE(deathEventReceived);
}

/**
 * Test revive event detection
 */
TEST_F(CombatSystemTest, Update_DetectsEntityRevive)
{
    // Arrange
    Entity entity = entityManager->createEntity();
    auto* health = entityManager->addComponent(entity, new HealthComponent(100.0f));
    health->takeDamage(100.0f);
    combatSystem->update(0.016f); // Process death

    bool reviveEventReceived = false;
    EventSystem::getInstance().subscribe("entity_revived", [&](const EventData& /*data*/) {
        reviveEventReceived = true;
    });

    // Act
    health->revive();
    combatSystem->update(0.016f); // Process revive

    // Assert
    EXPECT_TRUE(reviveEventReceived);
}

/**
 * Test damage with source entity
 */
TEST_F(CombatSystemTest, ApplyDamage_WithSource_IncludesSourceInEvent)
{
    // Arrange
    Entity target = entityManager->createEntity();
    Entity source = entityManager->createEntity();
    entityManager->addComponent(target, new HealthComponent(100.0f));

    Entity::ID receivedSource = 0;
    EventSystem::getInstance().subscribe("entity_damaged", [&](const EventData& data) {
        if (data.has("source")) {
            receivedSource = data.get<Entity::ID>("source");
        }
    });

    // Act
    combatSystem->applyDamage(target, 30.0f, source);

    // Assert
    EXPECT_EQ(receivedSource, source.getId());
}

/**
 * Test multiple entities interaction (demonstrates decoupling)
 */
TEST_F(CombatSystemTest, MultipleEntities_EachProcessedIndependently)
{
    // Arrange
    Entity entity1 = entityManager->createEntity();
    Entity entity2 = entityManager->createEntity();
    entityManager->addComponent(entity1, new HealthComponent(100.0f));
    entityManager->addComponent(entity2, new HealthComponent(50.0f));

    int damageEventCount = 0;
    EventSystem::getInstance().subscribe("entity_damaged", [&](const EventData& /*data*/) {
        damageEventCount++;
    });

    // Act
    combatSystem->applyDamage(entity1, 30.0f);
    combatSystem->applyDamage(entity2, 20.0f);

    // Assert
    EXPECT_EQ(damageEventCount, 2);
    EXPECT_FLOAT_EQ(entityManager->getComponent<HealthComponent>(entity1)->getCurrentHealth(),
                    70.0f);
    EXPECT_FLOAT_EQ(entityManager->getComponent<HealthComponent>(entity2)->getCurrentHealth(),
                    30.0f);
}

/**
 * Test that dead entities cannot be damaged
 */
TEST_F(CombatSystemTest, ApplyDamage_ToDeadEntity_HasNoEffect)
{
    // Arrange
    Entity entity = entityManager->createEntity();
    auto* health = entityManager->addComponent(entity, new HealthComponent(100.0f));
    health->takeDamage(100.0f);

    int eventCount = 0;
    EventSystem::getInstance().subscribe("entity_damaged", [&](const EventData& /*data*/) {
        eventCount++;
    });

    // Act
    combatSystem->applyDamage(entity, 50.0f);

    // Assert
    EXPECT_EQ(eventCount, 0); // No event should be emitted
    EXPECT_FLOAT_EQ(health->getCurrentHealth(), 0.0f);
}

/**
 * Test system can be disabled
 */
TEST_F(CombatSystemTest, Update_WhenDisabled_DoesNotProcess)
{
    // Arrange
    Entity entity = entityManager->createEntity();
    auto* health = entityManager->addComponent(entity, new HealthComponent(100.0f));
    health->takeDamage(100.0f);

    bool eventReceived = false;
    EventSystem::getInstance().subscribe("entity_died", [&](const EventData& /*data*/) {
        eventReceived = true;
    });

    // Act
    combatSystem->setEnabled(false);
    combatSystem->update(0.016f);

    // Assert
    EXPECT_FALSE(eventReceived);
}
