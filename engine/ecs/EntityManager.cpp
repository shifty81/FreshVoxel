#include "ecs/EntityManager.h"

namespace fresh
{
namespace ecs
{

EntityManager::EntityManager() : nextEntityId(1) {}

EntityManager::~EntityManager()
{
    clear();
}

Entity EntityManager::createEntity()
{
    Entity::ID id = generateEntityId();
    entities[id] = true;
    return Entity(id);
}

void EntityManager::destroyEntity(Entity entity)
{
    if (!isEntityValid(entity)) {
        return;
    }

    removeAllComponentsForEntity(entity.getId());
    entities.erase(entity.getId());
}

bool EntityManager::isEntityValid(Entity entity) const
{
    auto it = entities.find(entity.getId());
    return it != entities.end() && it->second;
}

std::vector<Entity> EntityManager::getAllEntities() const
{
    std::vector<Entity> result;
    result.reserve(entities.size());

    for (const auto& pair : entities) {
        if (pair.second) {
            result.push_back(Entity(pair.first));
        }
    }

    return result;
}

void EntityManager::clear()
{
    components.clear();
    entities.clear();
    nextEntityId = 1;
}

Entity::ID EntityManager::generateEntityId()
{
    return nextEntityId++;
}

void EntityManager::removeAllComponentsForEntity(Entity::ID entityId)
{
    for (auto& compMapPair : components) {
        compMapPair.second.erase(entityId);
    }
}

} // namespace ecs
} // namespace fresh
