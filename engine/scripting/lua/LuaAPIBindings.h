#pragma once
#include "scripting/lua/ScriptingEngine.h"
#include "scripting/lua/ScriptingEngineImpl.h"

// Include engine systems to bind
#include "voxel/VoxelTypes.h"
#include "voxel/VoxelWorld.h"
#include "gameplay/Player.h"
#include "gameplay/Camera.h"
#include "input/InputManager.h"

#include <glm/glm.hpp>

namespace fresh
{
namespace scripting
{

/**
 * @brief Register all engine systems with Lua
 * 
 * This function binds core engine classes and functions to Lua,
 * allowing scripts to interact with the game world, player, input, etc.
 */
class LuaAPIBindings
{
public:
    /**
     * @brief Register all engine API bindings
     * @param engine The scripting engine to register with
     */
    static void registerAll(ScriptingEngine& engine);

private:
    // Individual binding functions
    static void registerVoxelTypes(ScriptingEngine& engine);
    static void registerVoxelWorld(ScriptingEngine& engine);
    static void registerPlayer(ScriptingEngine& engine);
    static void registerCamera(ScriptingEngine& engine);
    static void registerInput(ScriptingEngine& engine);
    static void registerMath(ScriptingEngine& engine);
    static void registerUtilities(ScriptingEngine& engine);
};

} // namespace scripting
} // namespace fresh
