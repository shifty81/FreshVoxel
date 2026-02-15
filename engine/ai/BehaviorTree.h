#pragma once

#include <vector>

#include <glm/glm.hpp>

namespace fresh
{

// Forward declare from ECS namespace
namespace ecs
{
class Entity;
}
class VoxelWorld;

/**
 * @brief Basic behavior tree node types
 */
enum class NodeStatus { Success, Failure, Running };

/**
 * @brief Base node for behavior tree
 */
class BehaviorNode
{
public:
    virtual ~BehaviorNode() = default;
    virtual NodeStatus execute(ecs::Entity* entity, float deltaTime) = 0;
};

/**
 * @brief Behavior tree for AI decision making
 */
class BehaviorTree
{
public:
    BehaviorTree();
    ~BehaviorTree();

    /**
     * @brief Set the root node of the behavior tree
     */
    void setRoot(BehaviorNode* root);

    /**
     * @brief Execute the behavior tree
     */
    NodeStatus tick(ecs::Entity* entity, float deltaTime);

private:
    BehaviorNode* rootNode;
};

/**
 * @brief Sequence node - executes children in order until one fails
 */
class SequenceNode : public BehaviorNode
{
public:
    void addChild(BehaviorNode* child);
    NodeStatus execute(ecs::Entity* entity, float deltaTime) override;

private:
    std::vector<BehaviorNode*> children;
};

/**
 * @brief Selector node - executes children until one succeeds
 */
class SelectorNode : public BehaviorNode
{
public:
    void addChild(BehaviorNode* child);
    NodeStatus execute(ecs::Entity* entity, float deltaTime) override;

private:
    std::vector<BehaviorNode*> children;
};

/**
 * @brief Example behavior: Wander randomly
 */
class WanderBehavior : public BehaviorNode
{
public:
    WanderBehavior(VoxelWorld* world, float wanderRadius = 10.0f,
                   float wanderInterval = 3.0f);
    NodeStatus execute(ecs::Entity* entity, float deltaTime) override;

private:
    VoxelWorld* world;
    float wanderTimer;
    glm::vec3 targetPosition;
    float wanderRadius;
    float wanderInterval;
};

/**
 * @brief Example behavior: Follow target
 */
class FollowBehavior : public BehaviorNode
{
public:
    FollowBehavior(ecs::Entity* target, float followDistance = 5.0f);
    NodeStatus execute(ecs::Entity* entity, float deltaTime) override;

private:
    ecs::Entity* targetEntity;
    float followDistance;
};

} // namespace fresh
