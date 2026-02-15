#pragma once

#include "scripting/lua/LuaScriptingEngine.h"
#include "ecs/EntityManager.h"
#include "ecs/Entity.h"

namespace fresh {
namespace scripting {

/**
 * @brief Lua bindings for the Entity-Component-System (ECS)
 * 
 * This class provides Lua bindings for the ECS, allowing scripts to:
 * - Create and destroy entities
 * - Add, remove, and query components
 * - Iterate over entities with specific components
 * 
 * Usage from Lua:
 * ```lua
 * -- Create entity
 * local entity = ECS.createEntity()
 * 
 * -- Add transform component
 * ECS.addTransform(entity, 10.0, 65.0, 10.0)
 * 
 * -- Get transform
 * local pos = ECS.getPosition(entity)
 * 
 * -- Query entities
 * local entities = ECS.getEntitiesWithTransform()
 * ```
 */
class LuaECSBindings {
public:
    /**
     * @brief Register ECS bindings with a Lua scripting engine
     * @param engine The scripting engine to register bindings with
     * @param entityManager The entity manager to bind to
     */
    static void registerBindings(LuaScriptingEngine* engine, 
                                ecs::EntityManager* entityManager);

private:
    // Entity management functions (static for Lua binding)
    static ecs::Entity createEntity(ecs::EntityManager* mgr);
    static void destroyEntity(ecs::EntityManager* mgr, ecs::Entity entity);
    static bool isEntityValid(ecs::EntityManager* mgr, ecs::Entity entity);
    
    // Transform component functions
    static void addTransform(ecs::EntityManager* mgr, ecs::Entity entity, 
                           float x, float y, float z);
    static void addTransformWithRotation(ecs::EntityManager* mgr, ecs::Entity entity,
                                        float x, float y, float z,
                                        float rx, float ry, float rz);
    static bool hasTransform(ecs::EntityManager* mgr, ecs::Entity entity);
    static void removeTransform(ecs::EntityManager* mgr, ecs::Entity entity);
    static void setPosition(ecs::EntityManager* mgr, ecs::Entity entity,
                          float x, float y, float z);
    static void setRotation(ecs::EntityManager* mgr, ecs::Entity entity,
                          float rx, float ry, float rz);
    static void setScale(ecs::EntityManager* mgr, ecs::Entity entity,
                        float sx, float sy, float sz);
    
    // Transform queries (returning tables)
    struct Vec3Result {
        float x, y, z;
    };
    
    static Vec3Result getPosition(ecs::EntityManager* mgr, ecs::Entity entity);
    static Vec3Result getRotation(ecs::EntityManager* mgr, ecs::Entity entity);
    static Vec3Result getScale(ecs::EntityManager* mgr, ecs::Entity entity);
    
    // Entity queries
    static std::vector<ecs::Entity> getEntitiesWithTransform(ecs::EntityManager* mgr);
    static std::vector<ecs::Entity> getAllEntities(ecs::EntityManager* mgr);
};

} // namespace scripting
} // namespace fresh
