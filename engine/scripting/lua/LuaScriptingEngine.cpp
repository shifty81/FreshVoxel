/**
 * @file LuaScriptingEngine.cpp
 * @brief Enhanced Lua scripting engine implementation using Sol2
 */

#include "scripting/lua/LuaScriptingEngine.h"

#ifdef FRESH_LUA_AVAILABLE
#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>
#include <fstream>
#include <iostream>
#include <sstream>
#include <sys/stat.h>

namespace fresh {
namespace scripting {

LuaScriptingEngine::LuaScriptingEngine() 
    : m_lua(nullptr)
    , m_initialized(false)
    , m_hotReloadEnabled(false) {
}

LuaScriptingEngine::~LuaScriptingEngine() {
    shutdown();
}

bool LuaScriptingEngine::initialize() {
    if (m_initialized) {
        return true;
    }

    try {
        // Create new Lua state with Sol2
        m_lua = new sol::state();
        
        // Open standard Lua libraries
        m_lua->open_libraries(
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

        // Register basic engine functions
        (*m_lua)["print"] = [](const std::string& msg) {
            std::cout << "[Lua] " << msg << std::endl;
        };

        (*m_lua)["log"] = [](const std::string& msg) {
            std::cout << "[Lua Log] " << msg << std::endl;
        };

        (*m_lua)["warn"] = [](const std::string& msg) {
            std::cerr << "[Lua Warning] " << msg << std::endl;
        };

        (*m_lua)["error"] = [](const std::string& msg) {
            std::cerr << "[Lua Error] " << msg << std::endl;
        };

        m_initialized = true;
        std::cout << "[LuaScriptingEngine] Initialized successfully with Sol2" << std::endl;
        return true;

    } catch (const std::exception& e) {
        reportError(std::string("Failed to initialize Lua: ") + e.what());
        return false;
    }
}

void LuaScriptingEngine::shutdown() {
    if (!m_initialized) {
        return;
    }

    m_loadedScripts.clear();
    m_loadedMods.clear();
    delete m_lua;
    m_lua = nullptr;
    m_initialized = false;
    
    std::cout << "[LuaScriptingEngine] Shutdown complete" << std::endl;
}

bool LuaScriptingEngine::loadScript(const std::string& filepath) {
    if (!m_initialized) {
        reportError("Scripting engine not initialized");
        return false;
    }

    try {
        // Load and execute the script file
        m_lua->script_file(filepath);
        
        // Track the script for hot-reload
        if (m_hotReloadEnabled) {
            ScriptInfo info;
            info.filepath = filepath;
            info.lastModified = getFileModificationTime(filepath);
            m_loadedScripts[filepath] = info;
        }
        
        std::cout << "[LuaScriptingEngine] Loaded script: " << filepath << std::endl;
        return true;

    } catch (const sol::error& e) {
        reportError(std::string("Failed to load script '") + filepath + "': " + e.what());
        return false;
    }
}

bool LuaScriptingEngine::executeScript(const std::string& code) {
    if (!m_initialized) {
        reportError("Scripting engine not initialized");
        return false;
    }

    try {
        m_lua->script(code);
        return true;
    } catch (const sol::error& e) {
        reportError(std::string("Failed to execute script: ") + e.what());
        return false;
    }
}

bool LuaScriptingEngine::reloadScript(const std::string& filepath) {
    if (!m_initialized) {
        reportError("Scripting engine not initialized");
        return false;
    }

    auto it = m_loadedScripts.find(filepath);
    if (it == m_loadedScripts.end()) {
        reportError(std::string("Script not loaded: ") + filepath);
        return false;
    }

    std::cout << "[LuaScriptingEngine] Reloading script: " << filepath << std::endl;
    return loadScript(filepath);
}

void LuaScriptingEngine::registerReflectedType(const std::string& typeName) {
    if (!m_initialized) {
        reportError("Scripting engine not initialized");
        return;
    }

    auto& registry = ReflectionRegistry::getInstance();
    auto typeInfo = registry.getType(typeName);
    
    if (!typeInfo) {
        reportError(std::string("Type not found in reflection registry: ") + typeName);
        return;
    }

    try {
        // Create a new user type in Lua
        auto userType = m_lua->new_usertype<void>(typeName);
        
        // Register all properties from reflection
        registerReflectedProperties(typeName, typeInfo);
        
        std::cout << "[LuaScriptingEngine] Registered reflected type: " << typeName << std::endl;
    } catch (const std::exception& e) {
        reportError(std::string("Failed to register type '") + typeName + "': " + e.what());
    }
}

void LuaScriptingEngine::registerAllReflectedTypes() {
    if (!m_initialized) {
        reportError("Scripting engine not initialized");
        return;
    }

    auto& registry = ReflectionRegistry::getInstance();
    const auto& types = registry.getAllTypes();
    
    std::cout << "[LuaScriptingEngine] Registering " << types.size() << " reflected types" << std::endl;
    
    for (const auto& [typeIndex, typeInfo] : types) {
        registerReflectedType(typeInfo->getName());
    }
}

void LuaScriptingEngine::registerReflectedProperties(const std::string& typeName, 
                                                     std::shared_ptr<TypeInfo> typeInfo) {
    // Note: Full property registration would require template specialization
    // for each property type. This is a simplified version that logs the properties.
    
    const auto& properties = typeInfo->getProperties();
    std::cout << "[LuaScriptingEngine] Type '" << typeName << "' has " 
              << properties.size() << " properties" << std::endl;
    
    // In a full implementation, we would register getters and setters for each property
    // based on the PropertyType enum. For now, we just acknowledge their existence.
}

void LuaScriptingEngine::registerFunction(const std::string& name, std::function<void()> func) {
    if (!m_initialized) {
        reportError("Scripting engine not initialized");
        return;
    }

    try {
        (*m_lua)[name] = func;
        std::cout << "[LuaScriptingEngine] Registered function: " << name << std::endl;
    } catch (const std::exception& e) {
        reportError(std::string("Failed to register function '") + name + "': " + e.what());
    }
}

void LuaScriptingEngine::setGlobal(const std::string& name, int value) {
    if (!m_initialized) return;
    (*m_lua)[name] = value;
}

void LuaScriptingEngine::setGlobal(const std::string& name, float value) {
    if (!m_initialized) return;
    (*m_lua)[name] = value;
}

void LuaScriptingEngine::setGlobal(const std::string& name, double value) {
    if (!m_initialized) return;
    (*m_lua)[name] = value;
}

void LuaScriptingEngine::setGlobal(const std::string& name, bool value) {
    if (!m_initialized) return;
    (*m_lua)[name] = value;
}

void LuaScriptingEngine::setGlobal(const std::string& name, const std::string& value) {
    if (!m_initialized) return;
    (*m_lua)[name] = value;
}

void LuaScriptingEngine::checkForScriptChanges() {
    if (!m_initialized || !m_hotReloadEnabled) {
        return;
    }

    for (auto& [filepath, info] : m_loadedScripts) {
        std::time_t currentModTime = getFileModificationTime(filepath);
        
        if (currentModTime != info.lastModified) {
            std::cout << "[LuaScriptingEngine] Detected change in: " << filepath << std::endl;
            reloadScript(filepath);
            info.lastModified = currentModTime;
        }
    }
}

bool LuaScriptingEngine::loadMod(const std::string& modDirectory) {
    if (!m_initialized) {
        reportError("Scripting engine not initialized");
        return false;
    }

    // Try to load mod.lua or init.lua
    std::vector<std::string> possibleFiles = {
        modDirectory + "/mod.lua",
        modDirectory + "/init.lua",
        modDirectory + "/main.lua"
    };

    for (const auto& filepath : possibleFiles) {
        std::ifstream file(filepath);
        if (file.good()) {
            file.close();
            
            if (loadScript(filepath)) {
                m_loadedMods.push_back(modDirectory);
                std::cout << "[LuaScriptingEngine] Loaded mod from: " << modDirectory << std::endl;
                return true;
            }
        }
    }

    reportError(std::string("No valid mod script found in: ") + modDirectory);
    return false;
}

void LuaScriptingEngine::reportError(const std::string& error) {
    m_lastError = error;
    std::cerr << "[LuaScriptingEngine Error] " << error << std::endl;
}

std::time_t LuaScriptingEngine::getFileModificationTime(const std::string& filepath) {
    struct stat fileInfo;
    if (stat(filepath.c_str(), &fileInfo) == 0) {
        return fileInfo.st_mtime;
    }
    return 0;
}

} // namespace scripting
} // namespace fresh

#else // FRESH_LUA_AVAILABLE not defined

// Stub implementation when Lua is not available
#include "scripting/lua/LuaScriptingEngine.h"
#include <iostream>

namespace fresh {
namespace scripting {

LuaScriptingEngine::LuaScriptingEngine() : m_lua(nullptr), m_initialized(false) {}
// Note: Destructor doesn't delete m_lua pointer because sol::state is incomplete type in stub build
// The pointer is never allocated in stub implementation (initialize() always returns false)
LuaScriptingEngine::~LuaScriptingEngine() {}

bool LuaScriptingEngine::initialize() {
    std::cout << "[LuaScriptingEngine] Lua support not compiled in (stub implementation)" << std::endl;
    return false;
}

void LuaScriptingEngine::shutdown() {}
bool LuaScriptingEngine::loadScript(const std::string&) { return false; }
bool LuaScriptingEngine::executeScript(const std::string&) { return false; }
bool LuaScriptingEngine::reloadScript(const std::string&) { return false; }

void LuaScriptingEngine::registerReflectedType(const std::string&) {}
void LuaScriptingEngine::registerAllReflectedTypes() {}
void LuaScriptingEngine::registerReflectedProperties(const std::string&, std::shared_ptr<TypeInfo>) {}

void LuaScriptingEngine::registerFunction(const std::string&, std::function<void()>) {}
void LuaScriptingEngine::setGlobal(const std::string&, int) {}
void LuaScriptingEngine::setGlobal(const std::string&, float) {}
void LuaScriptingEngine::setGlobal(const std::string&, double) {}
void LuaScriptingEngine::setGlobal(const std::string&, bool) {}
void LuaScriptingEngine::setGlobal(const std::string&, const std::string&) {}

void LuaScriptingEngine::checkForScriptChanges() {}
bool LuaScriptingEngine::loadMod(const std::string&) { return false; }
void LuaScriptingEngine::reportError(const std::string& error) {
    m_lastError = error;
}
std::time_t LuaScriptingEngine::getFileModificationTime(const std::string&) { return 0; }

} // namespace scripting
} // namespace fresh

#endif // FRESH_LUA_AVAILABLE
