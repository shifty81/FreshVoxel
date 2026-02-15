/**
 * @file LuaECSBindings.cpp
 * @brief Implementation of Lua bindings for the ECS system
 */

#include "scripting/lua/LuaECSBindings.h"

#ifdef FRESH_LUA_AVAILABLE
#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

#include "ecs/TransformComponent.h"
#include <glm/gtx/euler_angles.hpp>
#include <iostream>

namespace fresh {
namespace scripting {

void LuaECSBindings::registerBindings(LuaScriptingEngine* engine, 
                                     ecs::EntityManager* entityManager) {
    if (!engine || !entityManager || !engine->getLuaState()) {
        std::cerr << "[LuaECSBindings] Error: Invalid engine or entity manager" << std::endl;
        return;
    }

    sol::state* lua = engine->getLuaState();

    // Register Entity type
    lua->new_usertype<ecs::Entity>("Entity",
        sol::constructors<ecs::Entity(), ecs::Entity(uint32_t)>(),
        "id", sol::property(&ecs::Entity::getId),
        "isValid", [entityManager](const ecs::Entity& e) { 
            return entityManager->isEntityValid(e); 
        }
    );

    // Register Vec3Result for returning vectors
    lua->new_usertype<Vec3Result>("Vec3Result",
        "x", &Vec3Result::x,
        "y", &Vec3Result::y,
        "z", &Vec3Result::z
    );

    // Create ECS namespace table
    sol::table ecsTable = lua->create_named_table("ECS");

    // Entity management
    ecsTable["createEntity"] = [entityManager]() {
        return createEntity(entityManager);
    };

    ecsTable["destroyEntity"] = [entityManager](ecs::Entity entity) {
        destroyEntity(entityManager, entity);
    };

    ecsTable["isEntityValid"] = [entityManager](ecs::Entity entity) {
        return isEntityValid(entityManager, entity);
    };

    ecsTable["getAllEntities"] = [entityManager]() {
        return getAllEntities(entityManager);
    };

    // Transform component management
    ecsTable["addTransform"] = [entityManager](ecs::Entity entity, float x, float y, float z) {
        addTransform(entityManager, entity, x, y, z);
    };

    ecsTable["addTransformWithRotation"] = [entityManager](ecs::Entity entity,
                                                          float x, float y, float z,
                                                          float rx, float ry, float rz) {
        addTransformWithRotation(entityManager, entity, x, y, z, rx, ry, rz);
    };

    ecsTable["hasTransform"] = [entityManager](ecs::Entity entity) {
        return hasTransform(entityManager, entity);
    };

    ecsTable["removeTransform"] = [entityManager](ecs::Entity entity) {
        removeTransform(entityManager, entity);
    };

    // Transform property setters
    ecsTable["setPosition"] = [entityManager](ecs::Entity entity, float x, float y, float z) {
        setPosition(entityManager, entity, x, y, z);
    };

    ecsTable["setRotation"] = [entityManager](ecs::Entity entity, float rx, float ry, float rz) {
        setRotation(entityManager, entity, rx, ry, rz);
    };

    ecsTable["setScale"] = [entityManager](ecs::Entity entity, float sx, float sy, float sz) {
        setScale(entityManager, entity, sx, sy, sz);
    };

    // Transform property getters
    ecsTable["getPosition"] = [entityManager](ecs::Entity entity) {
        return getPosition(entityManager, entity);
    };

    ecsTable["getRotation"] = [entityManager](ecs::Entity entity) {
        return getRotation(entityManager, entity);
    };

    ecsTable["getScale"] = [entityManager](ecs::Entity entity) {
        return getScale(entityManager, entity);
    };

    // Entity queries
    ecsTable["getEntitiesWithTransform"] = [entityManager]() {
        return getEntitiesWithTransform(entityManager);
    };

    std::cout << "[LuaECSBindings] ECS bindings registered successfully" << std::endl;
}

// Entity management implementations

ecs::Entity LuaECSBindings::createEntity(ecs::EntityManager* mgr) {
    return mgr->createEntity();
}

void LuaECSBindings::destroyEntity(ecs::EntityManager* mgr, ecs::Entity entity) {
    mgr->destroyEntity(entity);
}

bool LuaECSBindings::isEntityValid(ecs::EntityManager* mgr, ecs::Entity entity) {
    return mgr->isEntityValid(entity);
}

// Transform component implementations

void LuaECSBindings::addTransform(ecs::EntityManager* mgr, ecs::Entity entity,
                                 float x, float y, float z) {
    auto* transform = new ecs::TransformComponent();
    transform->position = glm::vec3(x, y, z);
    mgr->addComponent(entity, transform);
}

void LuaECSBindings::addTransformWithRotation(ecs::EntityManager* mgr, ecs::Entity entity,
                                             float x, float y, float z,
                                             float rx, float ry, float rz) {
    auto* transform = new ecs::TransformComponent();
    transform->position = glm::vec3(x, y, z);
    transform->setEulerAngles(glm::vec3(rx, ry, rz));
    mgr->addComponent(entity, transform);
}

bool LuaECSBindings::hasTransform(ecs::EntityManager* mgr, ecs::Entity entity) {
    return mgr->hasComponent<ecs::TransformComponent>(entity);
}

void LuaECSBindings::removeTransform(ecs::EntityManager* mgr, ecs::Entity entity) {
    mgr->removeComponent<ecs::TransformComponent>(entity);
}

void LuaECSBindings::setPosition(ecs::EntityManager* mgr, ecs::Entity entity,
                                float x, float y, float z) {
    auto* transform = mgr->getComponent<ecs::TransformComponent>(entity);
    if (transform) {
        transform->position = glm::vec3(x, y, z);
    }
}

void LuaECSBindings::setRotation(ecs::EntityManager* mgr, ecs::Entity entity,
                                float rx, float ry, float rz) {
    auto* transform = mgr->getComponent<ecs::TransformComponent>(entity);
    if (transform) {
        transform->setEulerAngles(glm::vec3(rx, ry, rz));
    }
}

void LuaECSBindings::setScale(ecs::EntityManager* mgr, ecs::Entity entity,
                             float sx, float sy, float sz) {
    auto* transform = mgr->getComponent<ecs::TransformComponent>(entity);
    if (transform) {
        transform->scale = glm::vec3(sx, sy, sz);
    }
}

LuaECSBindings::Vec3Result LuaECSBindings::getPosition(ecs::EntityManager* mgr, ecs::Entity entity) {
    auto* transform = mgr->getComponent<ecs::TransformComponent>(entity);
    if (transform) {
        return { transform->position.x, transform->position.y, transform->position.z };
    }
    return { 0.0f, 0.0f, 0.0f };
}

LuaECSBindings::Vec3Result LuaECSBindings::getRotation(ecs::EntityManager* mgr, ecs::Entity entity) {
    auto* transform = mgr->getComponent<ecs::TransformComponent>(entity);
    if (transform) {
        glm::vec3 euler = transform->getEulerAngles();
        return { euler.x, euler.y, euler.z };
    }
    return { 0.0f, 0.0f, 0.0f };
}

LuaECSBindings::Vec3Result LuaECSBindings::getScale(ecs::EntityManager* mgr, ecs::Entity entity) {
    auto* transform = mgr->getComponent<ecs::EntityManager>(entity);
    if (transform) {
        return { transform->scale.x, transform->scale.y, transform->scale.z };
    }
    return { 1.0f, 1.0f, 1.0f };
}

// Entity queries

std::vector<ecs::Entity> LuaECSBindings::getEntitiesWithTransform(ecs::EntityManager* mgr) {
    return mgr->getEntitiesWithComponent<ecs::TransformComponent>();
}

std::vector<ecs::Entity> LuaECSBindings::getAllEntities(ecs::EntityManager* mgr) {
    return mgr->getAllEntities();
}

} // namespace scripting
} // namespace fresh

#else // FRESH_LUA_AVAILABLE not defined

// Stub implementation when Lua is not available
#include "scripting/lua/LuaECSBindings.h"
#include <iostream>

namespace fresh {
namespace scripting {

void LuaECSBindings::registerBindings(LuaScriptingEngine* engine, 
                                     ecs::EntityManager* entityManager) {
    (void)engine;
    (void)entityManager;
    std::cout << "[LuaECSBindings] Lua support not compiled in (stub implementation)" << std::endl;
}

} // namespace scripting
} // namespace fresh

#endif // FRESH_LUA_AVAILABLE
