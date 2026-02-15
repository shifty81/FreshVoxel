#pragma once
#include <algorithm>
#include <memory>
#include <typeindex>
#include <unordered_map>
#include <vector>

#include "Entity.h"
#include "IComponent.h"

namespace fresh
{
namespace ecs
{

/**
 * @brief Manages entities and their components
 *
 * The EntityManager is the central hub of the ECS architecture.
 * It handles entity creation/destruction and component storage/retrieval.
 */
class EntityManager
{
public:
    EntityManager();
    ~EntityManager();

    // Entity management
    Entity createEntity();
    void destroyEntity(Entity entity);
    bool isEntityValid(Entity entity) const;
    std::vector<Entity> getAllEntities() const;

    // Component management
    template <typename T>
    T* addComponent(Entity entity, T* component);

    template <typename T>
    T* getComponent(Entity entity);

    template <typename T>
    const T* getComponent(Entity entity) const;

    template <typename T>
    bool hasComponent(Entity entity) const;

    template <typename T>
    void removeComponent(Entity entity);

    // Query entities by component type
    template <typename T>
    std::vector<Entity> getEntitiesWithComponent() const;

    // Query entities with multiple components
    template <typename... Components>
    std::vector<Entity> getEntitiesWithComponents() const;

    // Clear all entities and components
    void clear();

    // Get statistics
    size_t getEntityCount() const
    {
        return entities.size();
    }

private:
    Entity::ID nextEntityId;
    std::unordered_map<Entity::ID, bool> entities; // Entity ID -> exists

    // Component storage: ComponentType -> (EntityID -> Component)
    std::unordered_map<std::type_index, std::unordered_map<Entity::ID, std::unique_ptr<IComponent>>>
        components;

    Entity::ID generateEntityId();
    void removeAllComponentsForEntity(Entity::ID entityId);
};

// Template implementations

template <typename T>
T* EntityManager::addComponent(Entity entity, T* component)
{
    static_assert(std::is_base_of<IComponent, T>::value, "T must inherit from IComponent");

    if (!isEntityValid(entity)) {
        delete component;
        return nullptr;
    }

    auto typeIndex = std::type_index(typeid(T));
    components[typeIndex][entity.getId()].reset(component);
    return component;
}

template <typename T>
T* EntityManager::getComponent(Entity entity)
{
    static_assert(std::is_base_of<IComponent, T>::value, "T must inherit from IComponent");

    auto typeIndex = std::type_index(typeid(T));
    auto compMapIt = components.find(typeIndex);
    if (compMapIt == components.end()) {
        return nullptr;
    }

    auto compIt = compMapIt->second.find(entity.getId());
    if (compIt == compMapIt->second.end()) {
        return nullptr;
    }

    return static_cast<T*>(compIt->second.get());
}

template <typename T>
const T* EntityManager::getComponent(Entity entity) const
{
    static_assert(std::is_base_of<IComponent, T>::value, "T must inherit from IComponent");

    auto typeIndex = std::type_index(typeid(T));
    auto compMapIt = components.find(typeIndex);
    if (compMapIt == components.end()) {
        return nullptr;
    }

    auto compIt = compMapIt->second.find(entity.getId());
    if (compIt == compMapIt->second.end()) {
        return nullptr;
    }

    return static_cast<const T*>(compIt->second.get());
}

template <typename T>
bool EntityManager::hasComponent(Entity entity) const
{
    return getComponent<T>(entity) != nullptr;
}

template <typename T>
void EntityManager::removeComponent(Entity entity)
{
    auto typeIndex = std::type_index(typeid(T));
    auto compMapIt = components.find(typeIndex);
    if (compMapIt != components.end()) {
        compMapIt->second.erase(entity.getId());
    }
}

template <typename T>
std::vector<Entity> EntityManager::getEntitiesWithComponent() const
{
    std::vector<Entity> result;
    auto typeIndex = std::type_index(typeid(T));
    auto compMapIt = components.find(typeIndex);

    if (compMapIt != components.end()) {
        for (const auto& pair : compMapIt->second) {
            result.push_back(Entity(pair.first));
        }
    }

    return result;
}

template <typename... Components>
std::vector<Entity> EntityManager::getEntitiesWithComponents() const
{
    std::vector<Entity> result;

    for (const auto& entityPair : entities) {
        Entity entity(entityPair.first);
        bool hasAll = (hasComponent<Components>(entity) && ...);
        if (hasAll) {
            result.push_back(entity);
        }
    }

    return result;
}

} // namespace ecs
} // namespace fresh
