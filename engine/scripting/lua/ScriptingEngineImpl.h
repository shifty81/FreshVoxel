#pragma once
#include "scripting/lua/ScriptingEngine.h"

#ifdef SOL_ALL_SAFETIES_ON
#include <sol/sol.hpp>
#else
#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>
#endif

namespace fresh
{
namespace scripting
{

// Template implementations for type registration
template <typename T>
void ScriptingEngine::registerClass(const std::string& className)
{
    if (!lua) {
        reportError("Cannot register class: Lua state not initialized");
        return;
    }

    // Sol2 automatically creates a usertype for C++ classes
    // This is a basic registration - users should extend this for their specific classes
    lua->new_usertype<T>(className);
}

template <typename T>
void ScriptingEngine::registerObject(const std::string& name, T* object)
{
    if (!lua) {
        reportError("Cannot register object: Lua state not initialized");
        return;
    }

    // Register the object as a global variable in Lua
    (*lua)[name] = object;
}

} // namespace scripting
} // namespace fresh
