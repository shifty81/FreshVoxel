/**
 * @file LuaAPIBindings.cpp
 * @brief Implementation of Lua API bindings for engine systems
 */

#include "scripting/lua/LuaAPIBindings.h"
#include "core/Logger.h"

#ifdef FRESH_LUA_AVAILABLE
#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

namespace fresh
{
namespace scripting
{

void LuaAPIBindings::registerAll(ScriptingEngine& engine)
{
    LOG_INFO_C("Registering Lua API bindings...", "LuaAPI");

    registerMath(engine);
    registerVoxelTypes(engine);
    registerVoxelWorld(engine);
    registerCamera(engine);
    registerPlayer(engine);
    registerInput(engine);
    registerUtilities(engine);

    LOG_INFO_C("Lua API bindings registered successfully", "LuaAPI");
}

void LuaAPIBindings::registerMath(ScriptingEngine& engine)
{
    sol::state* lua = engine.getState();
    if (!lua) return;

    // Register glm::vec3
    lua->new_usertype<glm::vec3>("Vec3",
        sol::constructors<glm::vec3(), glm::vec3(float, float, float)>(),
        "x", &glm::vec3::x,
        "y", &glm::vec3::y,
        "z", &glm::vec3::z,
        // Vector operations
        sol::meta_function::addition, [](const glm::vec3& a, const glm::vec3& b) { return a + b; },
        sol::meta_function::subtraction, [](const glm::vec3& a, const glm::vec3& b) { return a - b; },
        sol::meta_function::multiplication, sol::overload(
            [](const glm::vec3& v, float s) { return v * s; },
            [](float s, const glm::vec3& v) { return s * v; }
        ),
        // Utility functions
        "length", [](const glm::vec3& v) { return glm::length(v); },
        "normalize", [](const glm::vec3& v) { return glm::normalize(v); },
        "dot", [](const glm::vec3& a, const glm::vec3& b) { return glm::dot(a, b); },
        "cross", [](const glm::vec3& a, const glm::vec3& b) { return glm::cross(a, b); },
        "distance", [](const glm::vec3& a, const glm::vec3& b) { return glm::distance(a, b); }
    );

    // Register glm::ivec3 (integer vector for voxel coordinates)
    lua->new_usertype<glm::ivec3>("IVec3",
        sol::constructors<glm::ivec3(), glm::ivec3(int, int, int)>(),
        "x", &glm::ivec3::x,
        "y", &glm::ivec3::y,
        "z", &glm::ivec3::z
    );

    LOG_INFO_C("Registered math types (Vec3, IVec3)", "LuaAPI");
}

void LuaAPIBindings::registerVoxelTypes(ScriptingEngine& engine)
{
    sol::state* lua = engine.getState();
    if (!lua) return;

    // Register VoxelType enum
    lua->new_enum<VoxelType>("VoxelType",
        {
            {"Air", VoxelType::Air},
            {"Stone", VoxelType::Stone},
            {"Dirt", VoxelType::Dirt},
            {"Grass", VoxelType::Grass},
            {"Sand", VoxelType::Sand},
            {"Water", VoxelType::Water},
            {"Wood", VoxelType::Wood},
            {"Leaves", VoxelType::Leaves},
            {"Coal", VoxelType::Coal},
            {"Iron", VoxelType::Iron},
            {"Gold", VoxelType::Gold},
            {"Diamond", VoxelType::Diamond},
            {"Bedrock", VoxelType::Bedrock}
        }
    );

    LOG_INFO_C("Registered VoxelType enum", "LuaAPI");
}

void LuaAPIBindings::registerVoxelWorld(ScriptingEngine& engine)
{
    sol::state* lua = engine.getState();
    if (!lua) return;

    // Register VoxelWorld class
    lua->new_usertype<VoxelWorld>("VoxelWorld",
        sol::no_constructor,  // World is created by engine, not Lua
        
        // Voxel access
        "get_voxel", &VoxelWorld::getVoxel,
        "set_voxel", &VoxelWorld::setVoxel,
        
        // World queries
        "is_solid", &VoxelWorld::isSolid,
        "is_transparent", &VoxelWorld::isTransparent,
        
        // Utility
        "world_to_chunk", [](VoxelWorld& world, int x, int y, int z) {
            return world.worldToChunk(x, y, z);
        }
    );

    LOG_INFO_C("Registered VoxelWorld class", "LuaAPI");
}

void LuaAPIBindings::registerCamera(ScriptingEngine& engine)
{
    sol::state* lua = engine.getState();
    if (!lua) return;

    // Register Camera class
    lua->new_usertype<Camera>("Camera",
        sol::no_constructor,
        
        // Position and orientation
        "get_position", &Camera::getPosition,
        "get_front", &Camera::getFront,
        "get_up", &Camera::getUp,
        "get_right", &Camera::getRight,
        
        // Camera parameters
        "get_fov", &Camera::getFOV,
        "set_fov", &Camera::setFOV,
        "get_near_plane", &Camera::getNearPlane,
        "get_far_plane", &Camera::getFarPlane
    );

    LOG_INFO_C("Registered Camera class", "LuaAPI");
}

void LuaAPIBindings::registerPlayer(ScriptingEngine& engine)
{
    sol::state* lua = engine.getState();
    if (!lua) return;

    // Register Player class
    lua->new_usertype<Player>("Player",
        sol::no_constructor,
        
        // Position and movement
        "get_position", &Player::getPosition,
        "set_position", &Player::setPosition,
        "get_velocity", &Player::getVelocity,
        
        // Camera access
        "get_camera", [](Player& player) -> Camera& {
            return player.getCamera();
        },
        
        // State queries
        "is_grounded", &Player::isGrounded,
        "is_flying", &Player::isFlying,
        "set_flying", &Player::setFlying
    );

    LOG_INFO_C("Registered Player class", "LuaAPI");
}

void LuaAPIBindings::registerInput(ScriptingEngine& engine)
{
    sol::state* lua = engine.getState();
    if (!lua) return;

    // Register InputManager class
    lua->new_usertype<InputManager>("InputManager",
        sol::no_constructor,
        
        // Key state
        "is_key_pressed", &InputManager::isKeyPressed,
        "is_key_down", &InputManager::isKeyDown,
        "is_key_up", &InputManager::isKeyUp,
        
        // Mouse state
        "is_mouse_button_pressed", &InputManager::isMouseButtonPressed,
        "get_mouse_position", &InputManager::getMousePosition,
        "get_mouse_delta", &InputManager::getMouseDelta
    );

    LOG_INFO_C("Registered InputManager class", "LuaAPI");
}

void LuaAPIBindings::registerUtilities(ScriptingEngine& engine)
{
    sol::state* lua = engine.getState();
    if (!lua) return;

    // Register utility functions
    (*lua)["log"] = [](const std::string& message) {
        LOG_INFO_C(message, "Lua");
    };

    (*lua)["log_warning"] = [](const std::string& message) {
        LOG_WARNING_C(message, "Lua");
    };

    (*lua)["log_error"] = [](const std::string& message) {
        LOG_ERROR_C(message, "Lua");
    };

    // Time utilities
    (*lua)["get_time"] = []() {
        return static_cast<float>(glfwGetTime());
    };

    LOG_INFO_C("Registered utility functions", "LuaAPI");
}

} // namespace scripting
} // namespace fresh

#else // FRESH_LUA_AVAILABLE not defined

// Stub implementation when Lua is not available
#include "scripting/lua/LuaAPIBindings.h"
#include "core/Logger.h"

namespace fresh
{
namespace scripting
{

void LuaAPIBindings::registerAll(ScriptingEngine& engine)
{
    (void)engine;
    LOG_INFO_C("Lua API bindings not available (Lua support not compiled in)", "LuaAPI");
}

void LuaAPIBindings::registerMath(ScriptingEngine& engine) { (void)engine; }
void LuaAPIBindings::registerVoxelTypes(ScriptingEngine& engine) { (void)engine; }
void LuaAPIBindings::registerVoxelWorld(ScriptingEngine& engine) { (void)engine; }
void LuaAPIBindings::registerCamera(ScriptingEngine& engine) { (void)engine; }
void LuaAPIBindings::registerPlayer(ScriptingEngine& engine) { (void)engine; }
void LuaAPIBindings::registerInput(ScriptingEngine& engine) { (void)engine; }
void LuaAPIBindings::registerUtilities(ScriptingEngine& engine) { (void)engine; }

} // namespace scripting
} // namespace fresh

#endif // FRESH_LUA_AVAILABLE
