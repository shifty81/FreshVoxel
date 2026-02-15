# Lua Scripting Integration Guide

## Overview

The Fresh Voxel Engine includes a prepared scripting framework that requires Lua library integration to be functional. This guide explains how to complete the integration.

**Note:** Lua integration is **optional**. The engine will build and run without Lua installed - Lua-dependent features like asset management Lua configurations will simply use stub implementations. This allows developers to build and test the engine locally without needing to set up the Lua environment.

## Current Status

✅ **Implemented:**
- ScriptingEngine class structure
- API design for script execution
- Object registration framework
- Mod loading architecture

⚠️ **Requires Integration:**
- Lua library installation
- C++ binding library (LuaBridge or sol2)
- Implementation of stub methods

## Integration Steps

### Step 1: Install Lua Library

#### Windows (Recommended: vcpkg)

```batch
# Using vcpkg
vcpkg install lua:x64-windows
```

#### Windows (Manual)

Download prebuilt binaries from:
- https://www.lua.org/download.html
- Or build from source using Visual Studio

### Step 2: Choose Binding Library

#### Option A: LuaBridge3 (Recommended)
**Pros:**
- Simple, clean API
- Header-only
- Good documentation
- Lightweight

**Installation:**
```bash
git clone https://github.com/kunitoki/LuaBridge3.git external/LuaBridge3
```

**Usage Example:**
```cpp
#include <LuaBridge/LuaBridge.h>

lua_State* L = luaL_newstate();
luaL_openlibs(L);

// Register a class
luabridge::getGlobalNamespace(L)
    .beginClass<Entity>("Entity")
    .addConstructor<void(*)(void)>()
    .addFunction("getId", &Entity::getId)
    .endClass();

// Register a function
luabridge::getGlobalNamespace(L)
    .addFunction("createEntity", &createEntity);
```

#### Option B: sol2
**Pros:**
- Modern C++20 support
- Very comprehensive features
- Excellent error messages

**Installation:**
```bash
git clone https://github.com/ThePhD/sol2.git external/sol2
```

**Usage Example:**
```cpp
#include <sol/sol.hpp>

sol::state lua;
lua.open_libraries(sol::lib::base, sol::lib::package);

// Register a class
lua.new_usertype<Entity>("Entity",
    "getId", &Entity::getId
);

// Register a function
lua["createEntity"] = &createEntity;
```

### Step 3: Update CMakeLists.txt

Add to your `CMakeLists.txt`:

```cmake
# Find Lua
find_package(Lua REQUIRED)

# Include Lua headers
target_include_directories(${PROJECT_NAME}
    PRIVATE
    ${LUA_INCLUDE_DIR}
)

# Link Lua library
target_link_libraries(${PROJECT_NAME}
    PRIVATE
    ${LUA_LIBRARIES}
)

# If using LuaBridge (header-only)
target_include_directories(${PROJECT_NAME}
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/external/LuaBridge3/Source
)

# If using sol2 (header-only)
target_include_directories(${PROJECT_NAME}
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/external/sol2/include
)

# Add scripting source files
set(ENGINE_SOURCES
    ${ENGINE_SOURCES}
    engine/scripting/lua/ScriptingEngine.cpp
)

set(ENGINE_HEADERS
    ${ENGINE_HEADERS}
    engine/scripting/lua/ScriptingEngine.h
)
```

### Step 4: Implement ScriptingEngine Methods

Edit `engine/scripting/lua/ScriptingEngine.cpp`:

#### Initialize Method (with LuaBridge)
```cpp
bool ScriptingEngine::initialize() {
    luaState = luaL_newstate();
    if (!luaState) {
        reportError("Failed to create Lua state");
        return false;
    }
    
    // Open standard libraries
    luaL_openlibs((lua_State*)luaState);
    
    // Register engine classes
    registerEngineClasses();
    
    return true;
}
```

#### Execute Script
```cpp
bool ScriptingEngine::executeScript(const std::string& code) {
    lua_State* L = (lua_State*)luaState;
    
    if (luaL_dostring(L, code.c_str()) != LUA_OK) {
        reportError(lua_tostring(L, -1));
        lua_pop(L, 1);
        return false;
    }
    
    return true;
}
```

#### Call Function
```cpp
bool ScriptingEngine::callFunction(const std::string& functionName) {
    lua_State* L = (lua_State*)luaState;
    
    lua_getglobal(L, functionName.c_str());
    if (!lua_isfunction(L, -1)) {
        reportError("Function not found: " + functionName);
        lua_pop(L, 1);
        return false;
    }
    
    if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
        reportError(lua_tostring(L, -1));
        lua_pop(L, 1);
        return false;
    }
    
    return true;
}
```

### Step 5: Register Engine Classes

Create a helper function to register all engine classes:

```cpp
void ScriptingEngine::registerEngineClasses() {
    lua_State* L = (lua_State*)luaState;
    
    // Register Entity
    luabridge::getGlobalNamespace(L)
        .beginClass<ecs::Entity>("Entity")
        .addConstructor<void(*)(void)>()
        .addFunction("getId", &ecs::Entity::getId)
        .addFunction("isValid", &ecs::Entity::isValid)
        .endClass();
    
    // Register PhysicsComponent
    luabridge::getGlobalNamespace(L)
        .beginClass<physics::PhysicsComponent>("PhysicsComponent")
        .addConstructor<void(*)(void)>()
        .addData("position", &physics::PhysicsComponent::position)
        .addData("velocity", &physics::PhysicsComponent::velocity)
        .addData("mass", &physics::PhysicsComponent::mass)
        .endClass();
    
    // Register Inventory
    luabridge::getGlobalNamespace(L)
        .beginClass<rpg::Inventory>("Inventory")
        .addConstructor<void(*)(float)>()
        .addFunction("addResource", &rpg::Inventory::addResource)
        .addFunction("removeResource", &rpg::Inventory::removeResource)
        .addFunction("getResourceAmount", &rpg::Inventory::getResourceAmount)
        .addFunction("getCapacity", &rpg::Inventory::getCapacity)
        .endClass();
    
    // Register global functions
    luabridge::getGlobalNamespace(L)
        .addFunction("print", [](const std::string& msg) {
            std::cout << "[Lua] " << msg << std::endl;
        })
        .addFunction("warn", [](const std::string& msg) {
            std::cerr << "[Lua Warning] " << msg << std::endl;
        });
}
```

### Step 6: Implement Mod Loading

```cpp
bool ScriptingEngine::loadMod(const std::string& modDirectory) {
    // Load mod manifest
    std::string manifestPath = modDirectory + "/mod.json";
    // Parse JSON (use nlohmann/json or similar)
    
    // Load main script
    std::string initScript = modDirectory + "/init.lua";
    if (!loadScript(initScript)) {
        reportError("Failed to load mod: " + modDirectory);
        return false;
    }
    
    // Call mod initialization
    callFunction("onModLoad");
    
    loadedMods.push_back(modDirectory);
    return true;
}
```

## Example Lua Scripts

### Basic Script (`test.lua`)
```lua
-- Access engine objects
print("Hello from Lua!")

-- Create entity (assuming EntityManager is registered)
local entity = Entity.new()
print("Created entity with ID: " .. entity:getId())

-- Modify physics
local physics = PhysicsComponent.new()
physics.position = Vector3.new(0, 10, 0)
physics.velocity = Vector3.new(1, 0, 0)
physics.mass = 100.0
```

### Mod Example (`mods/example/init.lua`)
```lua
-- Mod initialization
function onModLoad()
    print("Example mod loaded!")
    registerCommands()
end

function registerCommands()
    -- Register custom commands
    registerCommand("spawn_ship", function(args)
        local x = tonumber(args[1]) or 0
        local y = tonumber(args[2]) or 0
        local z = tonumber(args[3]) or 0
        
        -- Spawn ship at position
        spawnShip(x, y, z)
    end)
end

function spawnShip(x, y, z)
    local entity = createEntity()
    local physics = addPhysicsComponent(entity)
    physics.position = Vector3.new(x, y, z)
    
    print("Spawned ship at: " .. x .. ", " .. y .. ", " .. z)
end
```

### Event Handler Example
```lua
-- Subscribe to events
events.subscribe("onPlayerSpawn", function(player)
    print("Player spawned: " .. player:getName())
    
    -- Give starting resources
    local inventory = player:getInventory()
    inventory:addResource(ResourceType.Iron, 100)
    inventory:addResource(ResourceType.Credits, 1000)
end)

events.subscribe("onAsteroidMined", function(asteroid, player)
    local resourceType = asteroid:getResourceType()
    local amount = asteroid:getResourceAmount()
    
    print("Mined " .. amount .. " of " .. resourceType)
    
    -- Add bonus resources
    player:getInventory():addResource(resourceType, amount * 1.1)
end)
```

## Testing

### Unit Tests

```cpp
// Test script execution
TEST(ScriptingEngine, ExecuteScript) {
    ScriptingEngine engine;
    ASSERT_TRUE(engine.initialize());
    
    EXPECT_TRUE(engine.executeScript("x = 5 + 3"));
    EXPECT_EQ(engine.getGlobalInt("x"), 8);
    
    engine.shutdown();
}

// Test function call
TEST(ScriptingEngine, CallFunction) {
    ScriptingEngine engine;
    engine.initialize();
    
    engine.executeScript("function add(a, b) return a + b end");
    // Test calling with parameters (requires parameter support)
    
    engine.shutdown();
}
```

### Integration Test

```cpp
int main() {
    ScriptingEngine scripting;
    
    if (!scripting.initialize()) {
        std::cerr << "Failed to initialize scripting\n";
        return 1;
    }
    
    // Load mod
    if (scripting.loadMod("mods/example")) {
        std::cout << "Mod loaded successfully\n";
    }
    
    // Execute script
    scripting.loadScript("scripts/test.lua");
    
    // Call function
    scripting.callFunction("onUpdate");
    
    scripting.shutdown();
    return 0;
}
```

## API Reference

### Core Functions (exposed to Lua)

#### Entity Management
```lua
entity = Entity.new()
id = entity:getId()
valid = entity:isValid()
```

#### Physics
```lua
physics = PhysicsComponent.new()
physics.position = Vector3.new(x, y, z)
physics.velocity = Vector3.new(vx, vy, vz)
physics.mass = 1000.0
physics:addForce(Vector3.new(fx, fy, fz))
```

#### Inventory
```lua
inventory = Inventory.new(capacity)
inventory:addResource(ResourceType.Iron, amount)
inventory:removeResource(ResourceType.Iron, amount)
amount = inventory:getResourceAmount(ResourceType.Iron)
```

#### Galaxy Generation
```lua
generator = GalaxyGenerator.new(seed)
sector = generator:generateSector(x, y)
asteroids = sector:getAsteroids()
stations = sector:getStations()
```

## Performance Considerations

1. **Minimize Lua <-> C++ calls**: Each call has overhead
2. **Batch operations**: Pass arrays instead of individual items
3. **Cache Lua functions**: Don't look up functions every frame
4. **Use LuaJIT**: For better performance (optional)
5. **Profile scripts**: Use debug profiler for hot paths

## Security Considerations

1. **Sandbox untrusted scripts**: Use `lua_newstate()` with custom allocator
2. **Limit script execution time**: Use `lua_sethook()` with instruction count
3. **Disable dangerous functions**: Remove `os.execute()`, `io.popen()`, etc.
4. **Validate script inputs**: Check all parameters from Lua
5. **Use whitelist approach**: Only expose necessary API

## Debugging

### Enable Lua Debug Output
```cpp
lua_pushcfunction(L, [](lua_State* L) {
    const char* msg = lua_tostring(L, 1);
    std::cout << "[Lua Debug] " << msg << std::endl;
    return 0;
});
lua_setglobal(L, "debug_print");
```

### Error Handling
```lua
local status, err = pcall(function()
    -- Risky operation
    doSomethingDangerous()
end)

if not status then
    print("Error: " .. tostring(err))
end
```

## Next Steps

1. Install Lua library
2. Choose binding library (LuaBridge3 recommended)
3. Implement TODOs in ScriptingEngine.cpp
4. Test with example scripts
5. Create mod examples
6. Document Lua API for modders

## Resources

- **Lua Official**: https://www.lua.org/
- **LuaBridge**: https://github.com/kunitoki/LuaBridge3
- **sol2**: https://github.com/ThePhD/sol2
- **Lua Users Wiki**: http://lua-users.org/wiki/
- **Programming in Lua**: https://www.lua.org/pil/

## Support

For issues with Lua integration, check:
1. Lua version compatibility (5.3+ recommended)
2. Binding library documentation
3. Compiler C++20 support
4. Platform-specific build issues
