#pragma once

#include "core/Reflection.h"
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <ctime>

// Forward declare sol types to avoid including the full header
namespace sol {
    class state;
}

namespace fresh {
namespace scripting {

/**
 * @brief Enhanced Lua scripting engine with Sol2 integration
 * 
 * This implementation provides:
 * - Modern C++ Lua bindings via Sol2
 * - Automatic reflection-based class registration
 * - Hot-reload support for scripts
 * - Script component system for entities
 * - Comprehensive error handling
 */
class LuaScriptingEngine {
public:
    LuaScriptingEngine();
    ~LuaScriptingEngine();

    // Lifecycle
    bool initialize();
    void shutdown();
    [[nodiscard]] bool isInitialized() const noexcept { return m_initialized; }

    // Script execution
    bool loadScript(const std::string& filepath);
    bool executeScript(const std::string& code);
    bool reloadScript(const std::string& filepath);
    
    // Function calls
    template<typename... Args>
    bool callFunction(const std::string& functionName, Args&&... args);
    
    template<typename Ret, typename... Args>
    Ret callFunctionWithReturn(const std::string& functionName, Args&&... args);

    // Automatic reflection-based registration
    void registerReflectedType(const std::string& typeName);
    void registerAllReflectedTypes();

    // Manual object registration
    template<typename T>
    void registerClass(const std::string& className);

    template<typename T>
    void registerObject(const std::string& name, T* object);

    void registerFunction(const std::string& name, std::function<void()> func);

    // Global variable access
    void setGlobal(const std::string& name, int value);
    void setGlobal(const std::string& name, float value);
    void setGlobal(const std::string& name, double value);
    void setGlobal(const std::string& name, bool value);
    void setGlobal(const std::string& name, const std::string& value);

    template<typename T>
    T getGlobal(const std::string& name);

    // Hot-reload support
    void enableHotReload(bool enable) noexcept { m_hotReloadEnabled = enable; }
    [[nodiscard]] bool isHotReloadEnabled() const noexcept { return m_hotReloadEnabled; }
    void checkForScriptChanges();

    // Mod loading system
    bool loadMod(const std::string& modDirectory);
    [[nodiscard]] const std::vector<std::string>& getLoadedMods() const noexcept { return m_loadedMods; }

    // Error handling
    [[nodiscard]] const std::string& getLastError() const noexcept { return m_lastError; }
    [[nodiscard]] bool hasError() const noexcept { return !m_lastError.empty(); }
    void clearError() noexcept { m_lastError.clear(); }

    // Direct Lua state access (for advanced users)
    sol::state* getLuaState() { return m_lua; }

private:
    sol::state* m_lua;
    bool m_initialized = false;
    bool m_hotReloadEnabled = false;
    std::string m_lastError;
    std::vector<std::string> m_loadedMods;
    
    // Script file tracking for hot-reload
    struct ScriptInfo {
        std::string filepath;
        std::time_t lastModified;
    };
    std::map<std::string, ScriptInfo> m_loadedScripts;

    void reportError(const std::string& error);
    std::time_t getFileModificationTime(const std::string& filepath);
    
    // Helper for registering reflected properties
    void registerReflectedProperties(const std::string& typeName, 
                                     std::shared_ptr<TypeInfo> typeInfo);
};

// Template implementations
#ifdef FRESH_LUA_AVAILABLE
// Only compile template implementations when Sol2 is available
template<typename... Args>
bool LuaScriptingEngine::callFunction(const std::string& functionName, Args&&... args) {
    if (!m_initialized) {
        reportError("Scripting engine not initialized");
        return false;
    }

    try {
        (*m_lua)[functionName](std::forward<Args>(args)...);
        return true;
    } catch (const std::exception& e) {
        reportError(std::string("Error calling function '") + functionName + "': " + e.what());
        return false;
    }
}

template<typename Ret, typename... Args>
Ret LuaScriptingEngine::callFunctionWithReturn(const std::string& functionName, Args&&... args) {
    if (!m_initialized) {
        reportError("Scripting engine not initialized");
        return Ret();
    }

    try {
        return (*m_lua)[functionName](std::forward<Args>(args)...);
    } catch (const std::exception& e) {
        reportError(std::string("Error calling function '") + functionName + "': " + e.what());
        return Ret();
    }
}

template<typename T>
void LuaScriptingEngine::registerClass(const std::string& className) {
    // Implementation in .cpp file using Sol2
    // This is a placeholder for custom class registration
    (void)className; // Suppress unused parameter warning
}

template<typename T>
void LuaScriptingEngine::registerObject(const std::string& name, T* object) {
    if (!m_initialized || !object) return;
    
    try {
        (*m_lua)[name] = object;
    } catch (const std::exception& e) {
        reportError(std::string("Error registering object '") + name + "': " + e.what());
    }
}

template<typename T>
T LuaScriptingEngine::getGlobal(const std::string& name) {
    if (!m_initialized) {
        reportError("Scripting engine not initialized");
        return T();
    }

    try {
        return (*m_lua)[name];
    } catch (const std::exception& e) {
        reportError(std::string("Error getting global '") + name + "': " + e.what());
        return T();
    }
}

#else // FRESH_LUA_AVAILABLE not defined

// Stub implementations when Lua/Sol2 is not available
template<typename... Args>
bool LuaScriptingEngine::callFunction(const std::string& functionName, Args&&... args) {
    (void)functionName;
    (void)sizeof...(args); // Suppress unused parameter warning
    reportError("Lua support not compiled in");
    return false;
}

template<typename Ret, typename... Args>
Ret LuaScriptingEngine::callFunctionWithReturn(const std::string& functionName, Args&&... args) {
    (void)functionName;
    (void)sizeof...(args); // Suppress unused parameter warning
    reportError("Lua support not compiled in");
    return Ret();
}

template<typename T>
void LuaScriptingEngine::registerClass(const std::string& className) {
    (void)className;
    // Stub implementation - do nothing
}

template<typename T>
void LuaScriptingEngine::registerObject(const std::string& name, T* object) {
    (void)name;
    (void)object;
    // Stub implementation - do nothing
}

template<typename T>
T LuaScriptingEngine::getGlobal(const std::string& name) {
    (void)name;
    reportError("Lua support not compiled in");
    return T();
}

#endif // FRESH_LUA_AVAILABLE

} // namespace scripting
} // namespace fresh
