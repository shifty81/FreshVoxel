# Completing the Lua Integration

This guide will help you complete the Lua scripting integration in Fresh Voxel Engine.

## Current Status

✅ **What's Ready:**
- Lua 5.4 library installed and linked
- Sol2 binding library downloaded (`external/sol2/`)
- Production implementation written (`ScriptingEngineImpl.cpp`)
- Build system ready to switch implementations
- Test infrastructure in place

⚠️ **What's Needed:**
- Switch from stub to production implementation
- Expose engine classes to Lua
- Test the integration

---

## Step 1: Verify Sol2 is Available

Check that Sol2 was downloaded:

```bash
ls external/sol2/include/sol/sol.hpp
```

If missing, download it:

```bash
git clone --depth 1 --branch v3.3.0 https://github.com/ThePhD/sol2.git external/sol2
```

---

## Step 2: Update CMakeLists.txt to Use Production Implementation

The CMakeLists.txt needs a small update to use the Sol2 implementation.

Find the line around line 313 in `CMakeLists.txt`:

```cmake
set(ENGINE_SOURCES
    engine/main.cpp
    engine/core/Engine.cpp
    ...
    engine/devtools/DevToolsManager.cpp
    engine/scripting/lua/ScriptingEngine.cpp   # ← Change this line
)
```

**Option A: Conditional Based on Sol2 Availability** (Recommended)

Replace the fixed line with a variable:

```cmake
# Before the ENGINE_SOURCES list (around line 250)
# Determine which Lua implementation to use
if(Lua_FOUND AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/external/sol2/include")
    set(LUA_IMPL_FILE engine/scripting/lua/ScriptingEngineImpl.cpp)
    message(STATUS "Using production Lua implementation with Sol2")
else()
    set(LUA_IMPL_FILE engine/scripting/lua/ScriptingEngine.cpp)
    message(STATUS "Using stub Lua implementation")
endif()

# Then in ENGINE_SOURCES:
set(ENGINE_SOURCES
    engine/main.cpp
    ...
    engine/devtools/DevToolsManager.cpp
    ${LUA_IMPL_FILE}  # ← Use variable instead
)
```

**Option B: Direct Switch** (Simpler)

Just change the filename:

```cmake
set(ENGINE_SOURCES
    ...
    engine/scripting/lua/ScriptingEngineImpl.cpp  # ← Changed
)
```

---

## Step 3: Rebuild the Project

```bash
cd build
rm -rf *  # Clean build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . -j$(nproc)
```

**Expected Output:**
```
-- Sol2 found - using production Lua integration
-- Lua enabled for scripting and modding
```

If you see errors about Sol2, check that `external/sol2/include/sol/sol.hpp` exists.

---

## Step 4: Test Basic Lua Functionality

Create a test script `scripts/test.lua`:

```lua
-- Test script
print("Hello from Lua!")
print("Engine: " .. Engine.name)
print("Version: " .. Engine.version)

-- Simple math test
x = 5 + 3
print("5 + 3 = " .. x)

-- Define a function
function greet(name)
    print("Hello, " .. name .. "!")
end

greet("World")
```

Test it from C++:

```cpp
#include "scripting/lua/ScriptingEngine.h"

int main() {
    fresh::scripting::ScriptingEngine engine;
    
    if (!engine.initialize()) {
        std::cerr << "Failed to initialize Lua\n";
        return 1;
    }
    
    // Load and run script
    if (engine.loadScript("scripts/test.lua")) {
        std::cout << "Script executed successfully\n";
    }
    
    // Test global variables
    engine.setGlobal("player_health", 100);
    int health = engine.getGlobalInt("player_health");
    std::cout << "Player health: " << health << "\n";
    
    engine.shutdown();
    return 0;
}
```

---

## Step 5: Expose Engine Classes to Lua

Add to `ScriptingEngineImpl.cpp` in the `registerEngineAPI()` function:

```cpp
void ScriptingEngine::registerEngineAPI() {
    if (!luaState) return;
    
    auto* state = static_cast<sol::state*>(luaState);
    
    // Existing utility functions...
    
    // Register Vector3 (example)
    state->new_usertype<glm::vec3>("Vector3",
        sol::constructors<glm::vec3(), glm::vec3(float, float, float)>(),
        "x", &glm::vec3::x,
        "y", &glm::vec3::y,
        "z", &glm::vec3::z,
        "length", [](const glm::vec3& v) { return glm::length(v); }
    );
    
    // Register Entity (example)
    // state->new_usertype<ecs::Entity>("Entity",
    //     sol::constructors<ecs::Entity()>(),
    //     "getId", &ecs::Entity::getId,
    //     "isValid", &ecs::Entity::isValid
    // );
    
    // Register game functions
    (*state)["spawnEntity"] = [](float x, float y, float z) {
        // Implement entity spawning
        Logger::getInstance().info("Spawning entity at (" + 
            std::to_string(x) + ", " + 
            std::to_string(y) + ", " + 
            std::to_string(z) + ")");
    };
    
    Logger::getInstance().info("Registered Lua engine API");
}
```

---

## Step 6: Create a Mod System

### Mod Structure

```
mods/
└── example_mod/
    ├── init.lua          # Mod entry point
    ├── mod.json          # Mod metadata (optional)
    └── scripts/
        └── items.lua     # Mod content
```

### Example Mod (`mods/example_mod/init.lua`):

```lua
-- Example mod initialization
print("Loading Example Mod")

-- Mod metadata
Mod = {
    name = "Example Mod",
    version = "1.0.0",
    author = "Your Name"
}

-- Called when mod loads
function onModLoad()
    print("Example Mod: Initializing...")
    
    -- Register custom items
    registerItems()
    
    -- Register custom events
    registerEvents()
    
    print("Example Mod: Ready!")
end

function registerItems()
    -- Define a custom item
    -- createItem({
    --     id = "example:super_pickaxe",
    --     name = "Super Pickaxe",
    --     durability = 1000,
    --     mining_speed = 5.0
    -- })
end

function registerEvents()
    -- Listen for game events
    -- events.subscribe("onPlayerSpawn", function(player)
    --     print("Player spawned: " .. player:getName())
    -- end)
end

-- World generation rules (high-level)
function customBiomeRules(x, z)
    -- Define custom biome placement
    -- Return biome type based on coordinates
    return "plains"
end
```

### Load Mods from C++:

```cpp
ScriptingEngine engine;
engine.initialize();

// Load all mods
std::vector<std::string> mods = {"mods/example_mod", "mods/another_mod"};
for (const auto& mod : mods) {
    if (engine.loadMod(mod)) {
        std::cout << "Loaded mod: " << mod << "\n";
    }
}

// List loaded mods
for (const auto& mod : engine.getLoadedMods()) {
    std::cout << "Active mod: " << mod << "\n";
}
```

---

## Step 7: Add Hot-Reloading (Optional)

For development, add script hot-reloading:

```cpp
class ScriptWatcher {
    std::map<std::string, std::time_t> lastModified;
    
public:
    bool hasChanged(const std::string& filepath) {
        auto current = std::filesystem::last_write_time(filepath);
        auto& last = lastModified[filepath];
        
        if (current != last) {
            last = current;
            return true;
        }
        return false;
    }
};

// In your game loop:
ScriptWatcher watcher;
if (watcher.hasChanged("scripts/game_logic.lua")) {
    engine.loadScript("scripts/game_logic.lua");
    std::cout << "Reloaded script\n";
}
```

---

## Step 8: Create Unit Tests for Lua Integration

Add to `tests/scripting/LuaIntegrationTests.cpp`:

```cpp
#include <gtest/gtest.h>
#include "scripting/lua/ScriptingEngine.h"

using namespace fresh::scripting;

TEST(LuaIntegrationTest, Initialize_Success) {
    ScriptingEngine engine;
    EXPECT_TRUE(engine.initialize());
    engine.shutdown();
}

TEST(LuaIntegrationTest, ExecuteScript_ValidCode_Success) {
    ScriptingEngine engine;
    engine.initialize();
    
    EXPECT_TRUE(engine.executeScript("x = 5 + 3"));
    EXPECT_EQ(engine.getGlobalInt("x"), 8);
    
    engine.shutdown();
}

TEST(LuaIntegrationTest, CallFunction_ValidFunction_Success) {
    ScriptingEngine engine;
    engine.initialize();
    
    engine.executeScript("function test() return 42 end");
    EXPECT_TRUE(engine.callFunction("test"));
    
    engine.shutdown();
}

TEST(LuaIntegrationTest, RegisterFunction_CanCall_FromLua) {
    ScriptingEngine engine;
    engine.initialize();
    
    bool called = false;
    engine.registerFunction("testFunc", [&called]() {
        called = true;
    });
    
    engine.executeScript("testFunc()");
    EXPECT_TRUE(called);
    
    engine.shutdown();
}
```

Add to CMakeLists.txt test sources:

```cmake
set(TEST_SOURCES
    ...
    tests/scripting/LuaIntegrationTests.cpp
)
```

---

## Troubleshooting

### Sol2 Header Not Found

**Error:** `fatal error: sol/sol.hpp: No such file or directory`

**Solution:**
```bash
# Ensure Sol2 is in the right place
ls external/sol2/include/sol/sol.hpp

# Or re-download
rm -rf external/sol2
git clone --depth 1 --branch v3.3.0 https://github.com/ThePhD/sol2.git external/sol2
```

### Linker Errors with Lua

**Error:** `undefined reference to lua_*`

**Solution:** Ensure Lua is linked:
```cmake
target_link_libraries(${PROJECT_NAME} PRIVATE ${LUA_LIBRARIES})
```

### Runtime Error: "Lua state not initialized"

**Solution:** Call `initialize()` before using the engine:
```cpp
engine.initialize();  // Must call first!
engine.loadScript("test.lua");
```

---

## Performance Considerations

Following your architecture guidelines:

### ✅ Keep in C++ (High Performance)
- Voxel data structures and algorithms
- Mesh generation (Marching Cubes, etc.)
- Physics and collision detection
- Rendering API calls
- Core memory management

### ✅ Use Lua For (Flexibility)
- Game logic and rules
- Entity AI and behavior
- High-level world generation rules
- Item and recipe definitions
- Modding and customization
- Event handlers

### Profile the Lua ↔ C++ Boundary
- Minimize frequent cross-language calls
- Batch operations when possible
- Cache Lua function references
- Use coroutines for async tasks

---

## Next Steps

1. ✅ Switch to Sol2 implementation
2. ✅ Test basic Lua functionality
3. 🔄 Expose engine classes incrementally
4. 🔄 Create example mods
5. 🔄 Add hot-reloading
6. 🔄 Write comprehensive tests
7. 🔄 Document the Lua API for modders

## Resources

- [Sol2 Documentation](https://sol2.readthedocs.io/)
- [Lua 5.4 Reference](https://www.lua.org/manual/5.4/)
- `LUA_INTEGRATION_GUIDE.md` in this repo
- `CURRENT_STATUS.md` for overall project status
