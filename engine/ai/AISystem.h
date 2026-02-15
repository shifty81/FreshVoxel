#pragma once

#include <memory>

namespace fresh
{

// Forward declare from ECS namespace
namespace ecs
{
class Entity;
}

class BehaviorTree;

// Forward declare implementation
class AISystemImpl;

/**
 * @brief AI system for NPC behavior management
 */
class AISystem
{
public:
    AISystem();
    ~AISystem();

    /**
     * @brief Initialize AI system
     * @return true if successful
     */
    bool initialize();

    /**
     * @brief Update AI (tick all behavior trees)
     * @param deltaTime Time since last update
     */
    void update(float deltaTime);

    /**
     * @brief Add an NPC with behavior tree
     * @param entity Entity to control
     * @param behavior Behavior tree for decision making
     */
    void addNPC(std::unique_ptr<ecs::Entity> entity, std::unique_ptr<BehaviorTree> behavior);

    /**
     * @brief Remove an NPC from the system
     * @param entity Entity to remove
     */
    void removeNPC(ecs::Entity* entity);

    /**
     * @brief Get number of active NPCs
     */
    size_t getNPCCount() const;

private:
    std::unique_ptr<AISystemImpl> pImpl;
};

} // namespace fresh
