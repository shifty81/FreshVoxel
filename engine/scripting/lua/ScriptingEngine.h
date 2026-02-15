#pragma once
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

// Forward declare sol types
namespace sol {
    class state;
}

namespace fresh
{
namespace scripting
{

/**
 * @brief Scripting engine for Lua integration using Sol2
 *
 * This implementation uses Sol2 for modern C++ Lua bindings.
 * Sol2 provides type-safe, easy-to-use C++ to Lua binding.
 */
class ScriptingEngine
{
public:
    ScriptingEngine();
    ~ScriptingEngine();

    // Lifecycle
    bool initialize();
    void shutdown();

    // Script execution
    bool loadScript(const std::string& filepath);
    bool executeScript(const std::string& code);
    bool callFunction(const std::string& functionName);

    // Object registration (for Lua access)
    template <typename T>
    void registerClass(const std::string& className);

    template <typename T>
    void registerObject(const std::string& name, T* object);

    void registerFunction(const std::string& name, std::function<void()> func);

    // Global variable access
    void setGlobal(const std::string& name, int value);
    void setGlobal(const std::string& name, float value);
    void setGlobal(const std::string& name, const std::string& value);

    int getGlobalInt(const std::string& name);
    float getGlobalFloat(const std::string& name);
    std::string getGlobalString(const std::string& name);

    // Mod loading system
    bool loadMod(const std::string& modDirectory);
    std::vector<std::string> getLoadedMods() const;

    // Input system registration
    void registerInputManager(void* inputManager);

    // Get the underlying sol::state for advanced usage
    sol::state* getState();

    // Error handling
    std::string getLastError() const
    {
        return lastError;
    }

private:
    // Lua state using sol2
    // Note: Raw pointer used instead of std::unique_ptr to avoid requiring
    // complete type definition of sol::state in the header file
    sol::state* lua;

    std::string lastError;
    std::vector<std::string> loadedMods;

    void reportError(const std::string& error);
    void setupStandardLibraries();
};

// Template implementations are in a separate header to avoid circular dependencies
// Include ScriptingEngineImpl.h after defining types you want to register

} // namespace scripting
} // namespace fresh
