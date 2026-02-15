/**
 * @file ScriptingEngine.cpp
 * @brief Production-ready Lua scripting engine using Sol2
 *
 * Architecture:
 * - C++ Core: High-performance voxel operations, mesh generation, physics, rendering
 * - Lua Scripting: Game logic, AI, world generation rules, modding
 * - Sol2 Integration: Modern C++ to Lua binding with type safety
 * - Safe API: Controlled exposure of engine functionality
 * - Modding Support: Extensible API for community content
 */

#ifdef FRESH_LUA_AVAILABLE

    #define SOL_ALL_SAFETIES_ON 1
    #include <fstream>
    #include <iostream>
    #include <sstream>

    #include <sol/sol.hpp>

    #include "core/Logger.h"
    #include "input/InputManager.h"
    #include "scripting/lua/ScriptingEngine.h"

namespace fresh
{
namespace scripting
{

using fresh::Logger;

ScriptingEngine::ScriptingEngine() : luaState(nullptr) {}

ScriptingEngine::~ScriptingEngine()
{
    shutdown();
}

bool ScriptingEngine::initialize()
{
    try {
        // Create new Lua state using Sol2
        auto* state = new sol::state();
        luaState = static_cast<void*>(state);

        // Open standard Lua libraries
        state->open_libraries(sol::lib::base, sol::lib::package, sol::lib::coroutine,
                              sol::lib::string, sol::lib::math, sol::lib::table, sol::lib::io);

        // Register engine API
        registerEngineAPI();

        Logger::getInstance().info("Lua scripting engine initialized successfully");
        return true;

    } catch (const std::exception& e) {
        reportError(std::string("Failed to initialize Lua: ") + e.what());
        return false;
    }
}

void ScriptingEngine::shutdown()
{
    if (luaState) {
        auto* state = static_cast<sol::state*>(luaState);
        delete state;
        luaState = nullptr;
        Logger::getInstance().info("Lua scripting engine shut down");
    }
}

bool ScriptingEngine::loadScript(const std::string& filepath)
{
    if (!luaState) {
        reportError("Lua state not initialized");
        return false;
    }

    try {
        auto* state = static_cast<sol::state*>(luaState);
        auto result = state->script_file(filepath);

        if (!result.valid()) {
            sol::error err = result;
            reportError(std::string("Error loading script: ") + err.what());
            return false;
        }

        Logger::getInstance().info("Loaded Lua script: " + filepath);
        return true;

    } catch (const std::exception& e) {
        reportError(std::string("Exception loading script: ") + e.what());
        return false;
    }
}

bool ScriptingEngine::executeScript(const std::string& code)
{
    if (!luaState) {
        reportError("Lua state not initialized");
        return false;
    }

    try {
        auto* state = static_cast<sol::state*>(luaState);
        auto result = state->script(code);

        if (!result.valid()) {
            sol::error err = result;
            reportError(std::string("Error executing script: ") + err.what());
            return false;
        }

        return true;

    } catch (const std::exception& e) {
        reportError(std::string("Exception executing script: ") + e.what());
        return false;
    }
}

bool ScriptingEngine::callFunction(const std::string& functionName)
{
    if (!luaState) {
        reportError("Lua state not initialized");
        return false;
    }

    try {
        auto* state = static_cast<sol::state*>(luaState);
        sol::protected_function func = (*state)[functionName];

        if (!func.valid()) {
            reportError("Function not found: " + functionName);
            return false;
        }

        auto result = func();
        if (!result.valid()) {
            sol::error err = result;
            reportError(std::string("Error calling function: ") + err.what());
            return false;
        }

        return true;

    } catch (const std::exception& e) {
        reportError(std::string("Exception calling function: ") + e.what());
        return false;
    }
}

void ScriptingEngine::registerFunction(const std::string& name, std::function<void()> func)
{
    if (!luaState) {
        reportError("Lua state not initialized");
        return;
    }

    try {
        auto* state = static_cast<sol::state*>(luaState);
        (*state)[name] = func;
    } catch (const std::exception& e) {
        reportError(std::string("Failed to register function: ") + e.what());
    }
}

void ScriptingEngine::setGlobal(const std::string& name, int value)
{
    if (!luaState)
        return;
    auto* state = static_cast<sol::state*>(luaState);
    (*state)[name] = value;
}

void ScriptingEngine::setGlobal(const std::string& name, float value)
{
    if (!luaState)
        return;
    auto* state = static_cast<sol::state*>(luaState);
    (*state)[name] = value;
}

void ScriptingEngine::setGlobal(const std::string& name, const std::string& value)
{
    if (!luaState)
        return;
    auto* state = static_cast<sol::state*>(luaState);
    (*state)[name] = value;
}

int ScriptingEngine::getGlobalInt(const std::string& name)
{
    if (!luaState)
        return 0;
    auto* state = static_cast<sol::state*>(luaState);
    return (*state)[name].get_or(0);
}

float ScriptingEngine::getGlobalFloat(const std::string& name)
{
    if (!luaState)
        return 0.0f;
    auto* state = static_cast<sol::state*>(luaState);
    return (*state)[name].get_or(0.0f);
}

std::string ScriptingEngine::getGlobalString(const std::string& name)
{
    if (!luaState)
        return "";
    auto* state = static_cast<sol::state*>(luaState);
    return (*state)[name].get_or(std::string(""));
}

bool ScriptingEngine::loadMod(const std::string& modDirectory)
{
    if (!luaState) {
        reportError("Lua state not initialized");
        return false;
    }

    try {
        // Look for init.lua in mod directory
        std::string initScript = modDirectory + "/init.lua";

        if (!loadScript(initScript)) {
            reportError("Failed to load mod from: " + modDirectory);
            return false;
        }

        // Call mod initialization if it exists
        callFunction("onModLoad");

        loadedMods.push_back(modDirectory);
        Logger::getInstance().info("Loaded mod: " + modDirectory);
        return true;

    } catch (const std::exception& e) {
        reportError(std::string("Exception loading mod: ") + e.what());
        return false;
    }
}

std::vector<std::string> ScriptingEngine::getLoadedMods() const
{
    return loadedMods;
}

void ScriptingEngine::reportError(const std::string& error)
{
    lastError = error;
    Logger::getInstance().error("[ScriptingEngine] " + error);
}

void ScriptingEngine::registerEngineAPI()
{
    if (!luaState)
        return;

    auto* state = static_cast<sol::state*>(luaState);

    // Register safe utility functions for scripts
    (*state)["print"] = [](const std::string& msg) { Logger::getInstance().info("[Lua] " + msg); };

    (*state)["warn"] = [](const std::string& msg) {
        Logger::getInstance().warning("[Lua] " + msg);
    };

    (*state)["error"] = [](const std::string& msg) { Logger::getInstance().error("[Lua] " + msg); };

    // Register engine version info
    (*state)["Engine"] = state->create_table_with("name", "Fresh Voxel Engine", "version", "0.1.0",
                                                  "lua_enabled", true);

    Logger::getInstance().info("Registered Lua engine API");
}

void ScriptingEngine::registerInputManager(void* inputManager)
{
    if (!luaState || !inputManager)
        return;

    auto* state = static_cast<sol::state*>(luaState);
    auto* manager = static_cast<InputManager*>(inputManager);

    // Create Input table in Lua
    auto inputTable = state->create_table("Input");

    // Key state queries
    inputTable["IsKeyPressed"] = [manager](int keyCode) { return manager->isKeyPressed(keyCode); };

    inputTable["IsKeyJustPressed"] = [manager](int keyCode) {
        return manager->isKeyJustPressed(keyCode);
    };

    // Mouse button queries
    inputTable["IsMouseButtonPressed"] = [manager](int button) {
        return manager->isMouseButtonPressed(button);
    };

    inputTable["IsMouseButtonJustPressed"] = [manager](int button) {
        return manager->isMouseButtonJustPressed(button);
    };

    // Mouse position
    inputTable["GetMouseX"] = [manager]() { return manager->getMouseX(); };

    inputTable["GetMouseY"] = [manager]() { return manager->getMouseY(); };

    inputTable["GetMousePosition"] = [manager]() {
        auto pos = manager->getMousePosition();
        return sol::as_table(std::vector<float>{pos.x, pos.y});
    };

    // Mouse delta
    inputTable["GetMouseDelta"] = [manager]() {
        auto delta = manager->getMouseDelta();
        return sol::as_table(std::vector<float>{delta.x, delta.y});
    };

    // Action-based input
    inputTable["IsActionActive"] = [manager](int action) {
        return manager->isActionActive(static_cast<InputAction>(action));
    };

    inputTable["IsActionJustPressed"] = [manager](int action) {
        return manager->isActionJustPressed(static_cast<InputAction>(action));
    };

    // Input configuration
    inputTable["SetKeyBinding"] = [manager](int action, int keyCode) {
        manager->setKeyBinding(static_cast<InputAction>(action), keyCode);
    };

    inputTable["GetMouseSensitivity"] = [manager]() { return manager->getMouseSensitivity(); };

    inputTable["SetMouseSensitivity"] = [manager](float sensitivity) {
        manager->setMouseSensitivity(sensitivity);
    };

    // Input modes
    inputTable["GetInputMode"] = [manager]() { return static_cast<int>(manager->getInputMode()); };

    inputTable["SetInputMode"] = [manager](int mode, sol::optional<bool> temporary) {
        manager->setInputMode(static_cast<InputMode>(mode), temporary.value_or(false));
    };

    // Key codes for convenience (matching GLFW key codes)
    auto keysTable = state->create_table("Keys");
    keysTable["Space"] = 32;
    keysTable["W"] = 87;
    keysTable["A"] = 65;
    keysTable["S"] = 83;
    keysTable["D"] = 68;
    keysTable["E"] = 69;
    keysTable["Q"] = 81;
    keysTable["R"] = 82;
    keysTable["F"] = 70;
    keysTable["Escape"] = 256;
    keysTable["Enter"] = 257;
    keysTable["Tab"] = 258;
    keysTable["Backspace"] = 259;
    keysTable["LeftShift"] = 340;
    keysTable["LeftControl"] = 341;
    keysTable["LeftAlt"] = 342;
    keysTable["RightShift"] = 344;
    keysTable["RightControl"] = 345;
    keysTable["RightAlt"] = 346;

    // Mouse button codes
    auto mouseTable = state->create_table("Mouse");
    mouseTable["Left"] = 0;
    mouseTable["Right"] = 1;
    mouseTable["Middle"] = 2;

    // Input actions enum
    auto actionsTable = state->create_table("Actions");
    actionsTable["MoveForward"] = 0;
    actionsTable["MoveBackward"] = 1;
    actionsTable["MoveLeft"] = 2;
    actionsTable["MoveRight"] = 3;
    actionsTable["Jump"] = 4;
    actionsTable["Crouch"] = 5;
    actionsTable["Sprint"] = 6;
    actionsTable["Use"] = 7;
    actionsTable["Attack"] = 8;
    actionsTable["PlaceBlock"] = 9;
    actionsTable["BreakBlock"] = 10;
    actionsTable["OpenInventory"] = 11;
    actionsTable["OpenMenu"] = 12;
    actionsTable["ToggleEditor"] = 13;
    actionsTable["OpenChat"] = 14;

    // Input modes enum
    auto modesTable = state->create_table("InputModes");
    modesTable["GameMode"] = 0;
    modesTable["UIMode"] = 1;
    modesTable["BuildMode"] = 2;

    Logger::getInstance().info("Registered InputManager Lua bindings");
}

} // namespace scripting
} // namespace fresh

#else // FRESH_LUA_AVAILABLE not defined

    // Stub implementation when Lua is not available
    #include <iostream>

    #include "scripting/lua/ScriptingEngine.h"

namespace fresh
{
namespace scripting
{

ScriptingEngine::ScriptingEngine() : luaState(nullptr) {}
ScriptingEngine::~ScriptingEngine()
{
    shutdown();
}
bool ScriptingEngine::initialize()
{
    return true;
}
void ScriptingEngine::shutdown() {}
bool ScriptingEngine::loadScript(const std::string&)
{
    return false;
}
bool ScriptingEngine::executeScript(const std::string&)
{
    return false;
}
bool ScriptingEngine::callFunction(const std::string&)
{
    return false;
}
void ScriptingEngine::registerFunction(const std::string&, std::function<void()>) {}
void ScriptingEngine::setGlobal(const std::string&, int) {}
void ScriptingEngine::setGlobal(const std::string&, float) {}
void ScriptingEngine::setGlobal(const std::string&, const std::string&) {}
int ScriptingEngine::getGlobalInt(const std::string&)
{
    return 0;
}
float ScriptingEngine::getGlobalFloat(const std::string&)
{
    return 0.0f;
}
std::string ScriptingEngine::getGlobalString(const std::string&)
{
    return "";
}
bool ScriptingEngine::loadMod(const std::string&)
{
    return false;
}
std::vector<std::string> ScriptingEngine::getLoadedMods() const
{
    return {};
}
void ScriptingEngine::registerInputManager(void*) {}

void ScriptingEngine::reportError(const std::string& error)
{
    lastError = error;
    std::cerr << "[ScriptingEngine] " << error << std::endl;
}

} // namespace scripting
} // namespace fresh

#endif // FRESH_LUA_AVAILABLE
