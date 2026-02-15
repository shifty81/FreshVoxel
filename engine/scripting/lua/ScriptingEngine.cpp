/**
 * @file ScriptingEngine.cpp
 * @brief Lua scripting engine implementation using Sol2
 *
 * This implementation uses Sol2 for type-safe C++ to Lua bindings.
 * Sol2 handles most of the low-level Lua C API details and provides
 * a modern C++ interface.
 */

#include "scripting/lua/ScriptingEngine.h"

#ifdef FRESH_LUA_AVAILABLE
#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

#include <fstream>
#include <iostream>
#include <sstream>
#include <filesystem>

namespace fresh
{
namespace scripting
{

ScriptingEngine::ScriptingEngine() : lua(nullptr) {}

ScriptingEngine::~ScriptingEngine()
{
    shutdown();
}

bool ScriptingEngine::initialize()
{
    try {
        // Create new Lua state
        lua = new sol::state();
        
        // Open standard Lua libraries
        setupStandardLibraries();
        
        std::cout << "[ScriptingEngine] Initialized successfully with Sol2" << std::endl;
        return true;
    }
    catch (const std::exception& e) {
        reportError(std::string("Failed to initialize Lua: ") + e.what());
        return false;
    }
}

void ScriptingEngine::setupStandardLibraries()
{
    if (!lua) return;

    // Open standard Lua libraries
    lua->open_libraries(
        sol::lib::base,
        sol::lib::package,
        sol::lib::coroutine,
        sol::lib::string,
        sol::lib::os,
        sol::lib::math,
        sol::lib::table,
        sol::lib::debug,
        sol::lib::bit32,
        sol::lib::io,
        sol::lib::utf8
    );
}

void ScriptingEngine::shutdown()
{
    if (lua) {
        delete lua;
        lua = nullptr;
        std::cout << "[ScriptingEngine] Shutdown complete" << std::endl;
    }
}

bool ScriptingEngine::loadScript(const std::string& filepath)
{
    if (!lua) {
        reportError("Lua state not initialized");
        return false;
    }

    // Check if file exists
    if (!std::filesystem::exists(filepath)) {
        reportError("Script file not found: " + filepath);
        return false;
    }

    try {
        // Load and execute the script file
        lua->script_file(filepath);
        std::cout << "[ScriptingEngine] Loaded script: " << filepath << std::endl;
        return true;
    }
    catch (const sol::error& e) {
        reportError(std::string("Error loading script: ") + e.what());
        return false;
    }
}

bool ScriptingEngine::executeScript(const std::string& code)
{
    if (!lua) {
        reportError("Lua state not initialized");
        return false;
    }

    try {
        // Execute the Lua code
        lua->script(code);
        return true;
    }
    catch (const sol::error& e) {
        reportError(std::string("Error executing script: ") + e.what());
        return false;
    }
}

bool ScriptingEngine::callFunction(const std::string& functionName)
{
    if (!lua) {
        reportError("Lua state not initialized");
        return false;
    }

    try {
        // Get the function from global scope
        sol::optional<sol::function> func = (*lua)[functionName];
        
        if (!func) {
            reportError("Function not found: " + functionName);
            return false;
        }

        // Call the function with no arguments
        sol::function_result result = func.value()();
        
        // Check if there were any errors
        if (!result.valid()) {
            sol::error err = result;
            reportError(std::string("Error calling function: ") + err.what());
            return false;
        }

        return true;
    }
    catch (const sol::error& e) {
        reportError(std::string("Error calling function: ") + e.what());
        return false;
    }
}

void ScriptingEngine::registerFunction(const std::string& name, std::function<void()> func)
{
    if (!lua) {
        reportError("Lua state not initialized");
        return;
    }

    try {
        // Register the function as a global
        lua->set_function(name, func);
    }
    catch (const sol::error& e) {
        reportError(std::string("Error registering function: ") + e.what());
    }
}

void ScriptingEngine::setGlobal(const std::string& name, int value)
{
    if (!lua) {
        reportError("Lua state not initialized");
        return;
    }

    (*lua)[name] = value;
}

void ScriptingEngine::setGlobal(const std::string& name, float value)
{
    if (!lua) {
        reportError("Lua state not initialized");
        return;
    }

    (*lua)[name] = value;
}

void ScriptingEngine::setGlobal(const std::string& name, const std::string& value)
{
    if (!lua) {
        reportError("Lua state not initialized");
        return;
    }

    (*lua)[name] = value;
}

int ScriptingEngine::getGlobalInt(const std::string& name)
{
    if (!lua) {
        reportError("Lua state not initialized");
        return 0;
    }

    try {
        sol::optional<int> value = (*lua)[name];
        return value.value_or(0);
    }
    catch (const sol::error& e) {
        reportError(std::string("Error getting global: ") + e.what());
        return 0;
    }
}

float ScriptingEngine::getGlobalFloat(const std::string& name)
{
    if (!lua) {
        reportError("Lua state not initialized");
        return 0.0f;
    }

    try {
        sol::optional<float> value = (*lua)[name];
        return value.value_or(0.0f);
    }
    catch (const sol::error& e) {
        reportError(std::string("Error getting global: ") + e.what());
        return 0.0f;
    }
}

std::string ScriptingEngine::getGlobalString(const std::string& name)
{
    if (!lua) {
        reportError("Lua state not initialized");
        return "";
    }

    try {
        sol::optional<std::string> value = (*lua)[name];
        return value.value_or("");
    }
    catch (const sol::error& e) {
        reportError(std::string("Error getting global: ") + e.what());
        return "";
    }
}

bool ScriptingEngine::loadMod(const std::string& modDirectory)
{
    if (!lua) {
        reportError("Lua state not initialized");
        return false;
    }

    try {
        // Look for init.lua or mod.lua in the mod directory
        std::filesystem::path modPath(modDirectory);
        std::filesystem::path initScript;

        if (std::filesystem::exists(modPath / "init.lua")) {
            initScript = modPath / "init.lua";
        }
        else if (std::filesystem::exists(modPath / "mod.lua")) {
            initScript = modPath / "mod.lua";
        }
        else {
            reportError("No init.lua or mod.lua found in: " + modDirectory);
            return false;
        }

        // Load and execute the mod script
        if (loadScript(initScript.string())) {
            loadedMods.push_back(modDirectory);
            std::cout << "[ScriptingEngine] Loaded mod: " << modDirectory << std::endl;
            return true;
        }

        return false;
    }
    catch (const std::exception& e) {
        reportError(std::string("Error loading mod: ") + e.what());
        return false;
    }
}

std::vector<std::string> ScriptingEngine::getLoadedMods() const
{
    return loadedMods;
}

void ScriptingEngine::registerInputManager(void* inputManager)
{
    // This is a placeholder - actual implementation would register
    // the input manager with proper type bindings
    (void)inputManager;
}

sol::state* ScriptingEngine::getState()
{
    return lua;
}

void ScriptingEngine::reportError(const std::string& error)
{
    lastError = error;
    std::cerr << "[ScriptingEngine Error] " << error << std::endl;
}

} // namespace scripting
} // namespace fresh

#else // FRESH_LUA_AVAILABLE not defined

// Stub implementation when Lua is not available
#include <iostream>

namespace fresh
{
namespace scripting
{

ScriptingEngine::ScriptingEngine() : lua(nullptr) {}
// Note: Destructor doesn't delete lua pointer because sol::state is incomplete type in stub build
// The pointer is never allocated in stub implementation (initialize() always returns false)
ScriptingEngine::~ScriptingEngine() {}

bool ScriptingEngine::initialize() {
    std::cout << "[ScriptingEngine] Lua support not compiled in (stub implementation)" << std::endl;
    return false;
}

void ScriptingEngine::shutdown() {}
void ScriptingEngine::setupStandardLibraries() {}
bool ScriptingEngine::loadScript(const std::string&) { return false; }
bool ScriptingEngine::executeScript(const std::string&) { return false; }
bool ScriptingEngine::loadMod(const std::string&) { return false; }

void ScriptingEngine::registerFunction(const std::string&, std::function<void()>) {}

void ScriptingEngine::setGlobal(const std::string&, int) {}
void ScriptingEngine::setGlobal(const std::string&, float) {}
void ScriptingEngine::setGlobal(const std::string&, const std::string&) {}

int ScriptingEngine::getGlobalInt(const std::string&) { return 0; }
float ScriptingEngine::getGlobalFloat(const std::string&) { return 0.0f; }
std::string ScriptingEngine::getGlobalString(const std::string&) { return ""; }

std::vector<std::string> ScriptingEngine::getLoadedMods() const { return {}; }
void ScriptingEngine::registerInputManager(void*) {}

void ScriptingEngine::reportError(const std::string& error) {
    lastError = error;
    std::cerr << "[ScriptingEngine Error] " << error << std::endl;
}

sol::state* ScriptingEngine::getState() { return nullptr; }

} // namespace scripting
} // namespace fresh

#endif // FRESH_LUA_AVAILABLE
