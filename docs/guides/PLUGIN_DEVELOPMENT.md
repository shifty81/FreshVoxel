# Plugin Development Guide

This guide explains how to create plugins for the Fresh Voxel Engine, extending its functionality without modifying core engine code.

## Table of Contents

1. [Introduction](#introduction)
2. [Plugin Types](#plugin-types)
3. [Creating a World Generation Plugin](#creating-a-world-generation-plugin)
4. [Creating an Engine Mode Plugin](#creating-an-engine-mode-plugin)
5. [Plugin Metadata](#plugin-metadata)
6. [Plugin Registration](#plugin-registration)
7. [Hot Reload](#hot-reload)
8. [Best Practices](#best-practices)

## Introduction

Fresh Engine uses a plugin architecture to allow extending functionality at runtime. Plugins are self-contained modules that implement specific interfaces and can be loaded/unloaded without recompiling the engine.

## Plugin Types

### World Generation Plugins

World generation plugins create procedural worlds. They implement the `IWorldGenerator` interface.

**Use cases:**
- Custom terrain generation algorithms
- Specialized biome systems
- Voxel-based structures (dungeons, cities)
- 2D level generation
- Procedural content generation

### Engine Mode Plugins

Engine mode plugins define how the engine renders and processes game logic. They implement the `IEngineMode` interface.

**Use cases:**
- 2D vs 3D rendering modes
- Custom rendering pipelines
- Specialized physics modes
- VR/AR modes
- Custom camera systems

## Creating a World Generation Plugin

### Step 1: Implement the Interface

Create a class that implements `IWorldGenerator`:

```cpp
#include "generation/IWorldGenerator.h"
#include "voxel/Chunk.h"

namespace myplugin {

class CustomTerrainGenerator : public fresh::IWorldGenerator
{
public:
    // Constructor
    CustomTerrainGenerator() : m_seed(0) {}
    
    // Required: Return plugin metadata
    fresh::WorldGeneratorMetadata getMetadata() const override
    {
        return {
            "Custom Terrain",           // name
            "custom_terrain",           // id (must be unique)
            "My custom terrain generation algorithm",  // description
            "1.0.0",                    // version
            "Your Name",                // author
            false,                      // is2D
            true,                       // is3D
            {"terrain", "custom", "3d"} // tags
        };
    }
    
    // Required: Generate a single chunk
    void generateChunk(fresh::Chunk* chunk) override
    {
        if (!chunk) return;
        
        // Get chunk world position
        int chunkX = chunk->getWorldPosition().x;
        int chunkZ = chunk->getWorldPosition().z;
        
        // Generate voxels
        for (int x = 0; x < 16; x++) {
            for (int z = 0; z < 16; z++) {
                int worldX = chunkX + x;
                int worldZ = chunkZ + z;
                
                // Your generation logic here
                int height = calculateHeight(worldX, worldZ);
                
                for (int y = 0; y < height && y < 256; y++) {
                    fresh::VoxelType type = getBlockType(y, height);
                    chunk->setVoxel(x, y, z, type);
                }
            }
        }
    }
    
    // Required: Set random seed
    void setSeed(int seed) override
    {
        m_seed = seed;
    }
    
    // Optional: Get height at position (for terrain queries)
    int getHeight(int x, int z) const override
    {
        return calculateHeight(x, z);
    }

private:
    int m_seed;
    
    int calculateHeight(int x, int z) const
    {
        // Your height calculation logic
        // Example: simple sine wave
        return static_cast<int>(64 + 20 * sin(x * 0.1f) * cos(z * 0.1f));
    }
    
    fresh::VoxelType getBlockType(int y, int surfaceHeight) const
    {
        if (y == surfaceHeight - 1) {
            return fresh::VoxelType::Grass;
        } else if (y >= surfaceHeight - 4) {
            return fresh::VoxelType::Dirt;
        } else {
            return fresh::VoxelType::Stone;
        }
    }
};

} // namespace myplugin
```

### Step 2: Register the Plugin

Register your generator with the factory:

```cpp
#include "generation/IWorldGenerator.h"

// In your plugin initialization function
void initializeMyPlugin()
{
    // Register with the factory
    fresh::WorldGeneratorFactory::registerGenerator(
        "custom_terrain",
        []() -> std::unique_ptr<fresh::IWorldGenerator> {
            return std::make_unique<myplugin::CustomTerrainGenerator>();
        }
    );
}
```

### Step 3: Build as Plugin

Add to CMakeLists.txt:

```cmake
# Plugin library
add_library(worldgen_custom SHARED
    CustomTerrainGenerator.cpp
    plugin_init.cpp
)

target_link_libraries(worldgen_custom
    FreshRuntime  # Link to engine runtime
)

# Install to plugins directory
install(TARGETS worldgen_custom
    LIBRARY DESTINATION plugins/worldgen
)
```

## Creating an Engine Mode Plugin

### Step 1: Implement the Interface

Create a class that implements `IEngineMode`:

```cpp
#include "core/IEngineMode.h"
#include "renderer/RenderContext.h"
#include "gameplay/Camera.h"
#include "ecs/EntityManager.h"

namespace myplugin {

class Custom3DMode : public fresh::IEngineMode
{
public:
    Custom3DMode() 
        : m_camera(nullptr)
        , m_entityManager(nullptr) 
    {}
    
    // Required: Return plugin metadata
    fresh::EngineModeMetadata getMetadata() const override
    {
        return {
            "Custom 3D Mode",           // name
            "custom3d",                 // id (must be unique)
            "Custom 3D rendering mode with special effects",
            "1.0.0",                    // version
            "Your Name",                // author
            true,                       // supportsVoxels
            false,                      // supportsSprites
            3                           // dimensions
        };
    }
    
    // Required: Initialize the mode
    bool initialize(fresh::IRenderContext* renderer, 
                   fresh::ecs::EntityManager* entityMgr) override
    {
        m_entityManager = entityMgr;
        
        // Create camera
        m_camera = std::make_unique<fresh::Camera>();
        m_camera->setPosition(glm::vec3(0.0f, 100.0f, 0.0f));
        m_camera->setPerspective(70.0f, 1280.0f / 720.0f, 0.1f, 1000.0f);
        
        // Setup rendering pipeline
        // ...
        
        return true;
    }
    
    // Required: Shutdown and cleanup
    void shutdown() override
    {
        m_camera.reset();
        m_entityManager = nullptr;
    }
    
    // Required: Update game logic
    void update(float deltaTime) override
    {
        // Update camera
        if (m_camera) {
            // Update camera movement
        }
        
        // Update 3D physics
        // ...
    }
    
    // Required: Render the scene
    void render(fresh::IRenderContext* renderer) override
    {
        if (!renderer || !m_camera) return;
        
        // Render 3D world
        // ...
    }
    
    // Required: Handle resize
    void onResize(int width, int height) override
    {
        if (m_camera) {
            float aspect = static_cast<float>(width) / static_cast<float>(height);
            m_camera->setPerspective(70.0f, aspect, 0.1f, 1000.0f);
        }
    }
    
    // Required: Get camera
    fresh::Camera* getCamera() override
    {
        return m_camera.get();
    }
    
    // Required: Physics queries
    bool uses2DPhysics() const override { return false; }
    bool uses3DPhysics() const override { return true; }
    
    // Required: Get gravity
    glm::vec3 getGravity() const override
    {
        return glm::vec3(0.0f, -9.81f, 0.0f);
    }

private:
    std::unique_ptr<fresh::Camera> m_camera;
    fresh::ecs::EntityManager* m_entityManager;
};

} // namespace myplugin
```

### Step 2: Register the Plugin

```cpp
void initializeMyEngineMode()
{
    fresh::EngineModeFactory::registerMode(
        "custom3d",
        []() -> std::unique_ptr<fresh::IEngineMode> {
            return std::make_unique<myplugin::Custom3DMode>();
        }
    );
}
```

## Plugin Metadata

Metadata provides information about your plugin that the engine can display to users.

### WorldGeneratorMetadata

```cpp
struct WorldGeneratorMetadata {
    std::string name;           // Display name (e.g., "Custom Terrain")
    std::string id;             // Unique ID (e.g., "custom_terrain")
    std::string description;    // Description for users
    std::string version;        // Semantic version (e.g., "1.0.0")
    std::string author;         // Your name or organization
    bool is2D;                  // True if generates 2D worlds
    bool is3D;                  // True if generates 3D worlds
    std::vector<std::string> tags;  // Tags for categorization
};
```

### EngineModeMetadata

```cpp
struct EngineModeMetadata {
    std::string name;           // Display name (e.g., "Custom 3D Mode")
    std::string id;             // Unique ID (e.g., "custom3d")
    std::string description;    // Description for users
    std::string version;        // Semantic version
    std::string author;         // Your name or organization
    bool supportsVoxels;        // True if renders voxels
    bool supportsSprites;       // True if renders sprites
    int dimensions;             // 2 for 2D, 3 for 3D
};
```

## Plugin Registration

### Automatic Registration

Plugins can be automatically registered at startup by placing them in the `plugins/` directory:

```
plugins/
├── worldgen/
│   ├── custom_terrain.dll
│   └── custom_terrain.json
└── enginemode/
    ├── custom3d.dll
    └── custom3d.json
```

### Manual Registration

Alternatively, register plugins manually in code:

```cpp
// In engine initialization
void initializePlugins()
{
    // World generation plugins
    fresh::WorldGeneratorFactory::registerGenerator("custom_terrain", 
        createCustomTerrain);
    
    // Engine mode plugins
    fresh::EngineModeFactory::registerMode("custom3d", 
        createCustomMode);
}
```

## Hot Reload

Plugins support hot reload during development (debug mode only).

### Enabling Hot Reload

```cpp
// Enable hot reload
engine.enablePluginHotReload(true);

// Reload specific plugin
engine.reloadPlugin("custom_terrain");

// Reload all plugins
engine.reloadAllPlugins();
```

### Hot Reload Limitations

- Only works in debug builds
- State is lost during reload
- Runtime must be restarted for production builds

## Best Practices

### Performance

1. **Cache calculations**: Store expensive computations
2. **Use noise libraries**: Don't reinvent the wheel
3. **Generate incrementally**: Don't block the main thread
4. **Profile your code**: Measure performance

### Memory Management

1. **Use smart pointers**: Prefer `std::unique_ptr` and `std::shared_ptr`
2. **Clean up resources**: Implement proper destructors
3. **Avoid memory leaks**: Use RAII patterns

### Error Handling

1. **Check for null pointers**: Always validate inputs
2. **Log errors**: Use the logging system
3. **Fail gracefully**: Return sensible defaults on error
4. **Document assumptions**: Comment your code

### Testing

1. **Unit test your plugin**: Test generation algorithms
2. **Integration test**: Test with the engine
3. **Performance test**: Measure generation speed
4. **Visual test**: Generate and inspect worlds

### Documentation

1. **Document your API**: Add comments
2. **Provide examples**: Include sample code
3. **Explain parameters**: Document all settings
4. **Show screenshots**: Include visual examples

## Example Plugins

See the `plugins/worldgen/` directory for example plugins:

- `terrain3d` - 3D Perlin noise terrain
- `platformer2d` - 2D side-scrolling terrain
- `topdown2d` - 2D overhead terrain
- `empty` - Blank world (template)

## Additional Resources

- [IWorldGenerator API Reference](../engine/generation/IWorldGenerator.h)
- [IEngineMode API Reference](../engine/core/IEngineMode.h)
- [Plugin System Architecture](../architecture/EDITOR_RUNTIME_SEPARATION.md#plugin-system-architecture)
- [World Generation Guide](WORLD_GENERATION.md)

## Support

For questions or issues:

1. Check the [FAQ](FAQ.md)
2. Search [existing issues](https://github.com/shifty81/fresh/issues)
3. Open a [new issue](https://github.com/shifty81/fresh/issues/new)
4. Join our [Discord](https://discord.gg/fresh) (if available)

---

**Happy plugin development!** 🎮
