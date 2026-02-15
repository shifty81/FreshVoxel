# Editor and Runtime Separation Architecture

**Date:** 2026-01-12
**Status:** Architecture Design / Roadmap
**Goal:** Establish clear separation between editor tools and runtime game engine

---

## Overview

This document outlines the strategy for separating the Fresh Voxel Engine into distinct **Editor** and **Runtime** components while maintaining a unified development experience.

## Current Architecture

Currently, the Fresh Voxel Engine is a monolithic application that combines:
- **Core Runtime** (engine, rendering, physics, ECS)
- **Editor UI** (Win32 panels, tools, inspectors)
- **World Generation** (built-in generators)
- **Game Systems** (player, AI, gameplay)

All components are compiled into a single executable: `FreshVoxelEngine.exe`

## Target Architecture

### Component Separation

```
Fresh Engine Ecosystem
├── FreshRuntime.lib          # Core engine library
│   ├── Rendering (DirectX 11/12, OpenGL)
│   ├── Physics (2D/3D systems)
│   ├── ECS (Entity Component System)
│   ├── Audio (OpenAL)
│   ├── Scripting (Lua/C#)
│   ├── Asset Management
│   └── Plugin System
│
├── FreshEditor.exe           # Editor application
│   ├── Win32 Native UI
│   ├── Viewport Panel
│   ├── Scene Hierarchy
│   ├── Content Browser
│   ├── Inspector
│   ├── Terraforming Tools
│   └── Play-in-Editor
│
├── FreshGame.exe             # Standalone game runtime
│   ├── Links to FreshRuntime.lib
│   ├── No editor UI
│   ├── Optimized for distribution
│   └── Minimal dependencies
│
└── Plugins/                  # Plugin modules
    ├── worldgen/             # World generation plugins
    │   ├── terrain3d.dll
    │   ├── platformer2d.dll
    │   └── topdown2d.dll
    └── enginemode/           # Engine mode plugins
        ├── mode3d.dll
        ├── mode2d_platformer.dll
        └── mode2d_topdown.dll
```

### Benefits of Separation

1. **Faster Iteration**: Game runtime compiles faster without editor UI
2. **Smaller Distribution**: Ship games without editor dependencies
3. **Clearer Architecture**: Well-defined API boundaries
4. **Parallel Development**: Editor and runtime can evolve independently
5. **Plugin System**: Extend functionality without modifying core
6. **Multiple Frontends**: Could create different editors (ImGui, Qt, web-based)

## Plugin System Architecture

### World Generation Plugins

World generators are now first-class plugins:

```cpp
// IWorldGenerator interface (already exists)
class IWorldGenerator {
public:
    virtual WorldGeneratorMetadata getMetadata() const = 0;
    virtual void generateChunk(Chunk* chunk) = 0;
    virtual void setSeed(int seed) = 0;
    // ... other methods
};

// Plugin implementation (in separate DLL/SO)
class Terrain3DGenerator : public IWorldGenerator {
public:
    WorldGeneratorMetadata getMetadata() const override {
        return {
            "3D Terrain",          // name
            "terrain3d",           // id
            "Perlin noise terrain",// description
            "1.0.0",               // version
            "Fresh Engine",        // author
            false,                 // is2D
            true,                  // is3D
            {"terrain", "3d"}      // tags
        };
    }
    
    void generateChunk(Chunk* chunk) override {
        // Generation logic
    }
};

// Plugin registration (automatic via factory)
REGISTER_WORLD_GENERATOR("terrain3d", Terrain3DGenerator);
```

### Engine Mode Plugins

New plugin type for switching between 2D/3D rendering:

```cpp
// IEngineMode interface (new)
class IEngineMode {
public:
    virtual EngineModeMetadata getMetadata() const = 0;
    virtual bool initialize(IRenderContext*, EntityManager*) = 0;
    virtual void update(float deltaTime) = 0;
    virtual void render(IRenderContext*) = 0;
    virtual Camera* getCamera() = 0;
    virtual bool uses2DPhysics() const = 0;
    virtual bool uses3DPhysics() const = 0;
    // ... other methods
};

// Plugin implementation
class Mode3D : public IEngineMode {
public:
    EngineModeMetadata getMetadata() const override {
        return {
            "3D Voxel Mode",       // name
            "mode3d",              // id
            "Full 3D voxel rendering",
            "1.0.0",
            "Fresh Engine",
            true,                  // supportsVoxels
            false,                 // supportsSprites
            3                      // dimensions
        };
    }
    
    bool initialize(IRenderContext* renderer, EntityManager* em) override {
        // Setup 3D rendering pipeline
        // Configure perspective camera
        // Enable 3D physics
        return true;
    }
    
    void update(float deltaTime) override {
        // Update 3D game logic
    }
    
    void render(IRenderContext* renderer) override {
        // Render 3D voxel world
    }
    
    bool uses3DPhysics() const override { return true; }
    bool uses2DPhysics() const override { return false; }
};

// Plugin registration
REGISTER_ENGINE_MODE("mode3d", Mode3D);
```

## API Boundaries

### Runtime Public API

The runtime library exposes a clean C++ API:

```cpp
namespace fresh {

// Core engine interface
class IEngine {
public:
    virtual bool initialize() = 0;
    virtual void shutdown() = 0;
    virtual void update(float deltaTime) = 0;
    virtual void render() = 0;
    
    virtual EntityManager* getEntityManager() = 0;
    virtual IRenderContext* getRenderer() = 0;
    virtual PhysicsSystem* getPhysics() = 0;
    virtual AudioEngine* getAudio() = 0;
    
    virtual bool loadScene(const std::string& path) = 0;
    virtual bool saveScene(const std::string& path) = 0;
    
    virtual void setEngineMode(const std::string& modeId) = 0;
    virtual IEngineMode* getEngineMode() = 0;
};

// Factory for creating engine instances
std::unique_ptr<IEngine> createEngine(const EngineConfig& config);

} // namespace fresh
```

### Editor API

The editor calls into the runtime API:

```cpp
// Editor main loop
void EditorApplication::run() {
    // Create runtime engine instance
    auto engine = fresh::createEngine(config);
    if (!engine->initialize()) {
        // Handle error
        return;
    }
    
    // Create editor UI
    EditorUI ui(engine.get());
    
    // Main loop
    while (running) {
        // Process editor input
        ui.processInput();
        
        // Update engine (if in play mode)
        if (playMode) {
            engine->update(deltaTime);
        }
        
        // Render to viewport
        engine->render();
        
        // Render editor UI on top
        ui.render();
        
        // Present
        swapBuffers();
    }
    
    engine->shutdown();
}
```

## Migration Strategy

### Phase 1: Plugin System (Current)
- ✅ Enhance IWorldGenerator with metadata
- ✅ Create IEngineMode interface
- ✅ Create plugin directory structure
- ⏳ Implement world generation plugins
- ⏳ Implement engine mode plugins
- ⏳ Add plugin hot-reload support

### Phase 2: Library Separation (Future)
- Create FreshRuntime static library target
- Move editor-specific code to separate directory
- Define clean API boundary (IEngine interface)
- Ensure runtime has no Win32 UI dependencies
- Create stub implementations for testing

### Phase 3: Editor Application (Future)
- Create FreshEditor executable target
- Link against FreshRuntime library
- Move all Win32 UI code to editor
- Implement viewport rendering to runtime
- Add Play-in-Editor functionality

### Phase 4: Standalone Runtime (Future)
- Create FreshGame executable target
- Minimal UI (splash screen, settings)
- Optimized build (no debug symbols, smaller binary)
- Package system for distribution
- Installer/launcher

### Phase 5: Plugin SDK (Future)
- Document plugin API
- Create plugin templates
- Add plugin verification
- Implement plugin marketplace/sharing
- Hot-reload in editor

## Build System Changes

### Current CMakeLists.txt Structure
```cmake
project(FreshVoxelEngine)

# All sources in one list
set(ENGINE_SOURCES
    engine/main.cpp
    engine/core/Engine.cpp
    # ... all sources ...
    engine/editor/EditorManager.cpp
    engine/ui/native/Win32*.cpp
)

# Single executable
add_executable(FreshVoxelEngine ${ENGINE_SOURCES})
```

### Future CMakeLists.txt Structure
```cmake
project(FreshEngine VERSION 0.2.0)

# Runtime library (no editor UI)
add_subdirectory(runtime)

# Editor application
add_subdirectory(editor)

# Standalone game runtime
add_subdirectory(game)

# Plugins
add_subdirectory(plugins/worldgen)
add_subdirectory(plugins/enginemode)

# Tests
add_subdirectory(tests)
```

```cmake
# runtime/CMakeLists.txt
add_library(FreshRuntime STATIC
    engine/core/Engine.cpp
    engine/renderer/*.cpp
    engine/physics/*.cpp
    engine/ecs/*.cpp
    # ... runtime sources only ...
)

target_link_libraries(FreshRuntime
    glfw glm OpenAL nlohmann_json sol2
    # ... runtime dependencies ...
)
```

```cmake
# editor/CMakeLists.txt
add_executable(FreshEditor
    engine/main_editor.cpp
    engine/EditorApplication.cpp
    engine/ui/native/*.cpp
    # ... editor sources only ...
)

target_link_libraries(FreshEditor
    FreshRuntime
    # ... editor-specific dependencies ...
)
```

```cmake
# game/CMakeLists.txt
add_executable(FreshGame
    engine/main_game.cpp
    engine/GameApplication.cpp
    # ... minimal game runtime ...
)

target_link_libraries(FreshGame
    FreshRuntime
    # ... runtime dependencies only ...
)
```

## Viewport Integration

### Current Implementation
- EditorManager creates Win32ViewportPanel
- Engine renders directly to viewport HWND
- Works, but tightly coupled

### Future Implementation
- Editor provides viewport HWND to runtime
- Runtime exposes rendering interface
- Clean separation of concerns

```cpp
// Runtime provides rendering interface
class IEngine {
public:
    virtual void setViewportWindow(void* hwnd) = 0;
    virtual void resizeViewport(int width, int height) = 0;
    virtual void renderToViewport() = 0;
};

// Editor uses interface
void EditorManager::initialize() {
    // Create viewport panel
    m_viewport = std::make_unique<Win32ViewportPanel>();
    m_viewport->create(parentHwnd, x, y, width, height);
    
    // Give viewport to runtime
    m_engine->setViewportWindow(m_viewport->getHandle());
}

void EditorManager::render() {
    // Runtime renders to viewport
    m_engine->renderToViewport();
    
    // Editor renders UI panels on top
    renderEditorUI();
}
```

## Backward Compatibility

During migration, maintain backward compatibility:
- Keep current monolithic build as default
- Add CMake options for new build targets
- Gradual migration of components
- Deprecation warnings for old APIs

```cmake
option(FRESH_BUILD_MODULAR "Build as separate runtime and editor" OFF)

if(FRESH_BUILD_MODULAR)
    # New modular build system
    add_subdirectory(runtime)
    add_subdirectory(editor)
else()
    # Legacy monolithic build
    add_executable(FreshVoxelEngine ${ALL_SOURCES})
endif()
```

## Testing Strategy

- **Unit Tests**: Test runtime library in isolation
- **Integration Tests**: Test editor-runtime interaction
- **Plugin Tests**: Test plugin loading/unloading
- **Performance Tests**: Ensure separation doesn't hurt performance
- **Distribution Tests**: Test standalone game builds

## Documentation Updates

- Update build instructions for new structure
- Create plugin development guide
- Document runtime API
- Update architecture diagrams
- Create migration guide for existing projects

## Timeline

- **Phase 1** (Current): Plugin system foundation
- **Phase 2** (Months 1-2): Library separation
- **Phase 3** (Months 3-4): Editor application
- **Phase 4** (Months 5-6): Standalone runtime
- **Phase 5** (Months 7-8): Plugin SDK

## Risks and Mitigations

### Risk: Breaking Existing Code
**Mitigation**: Maintain backward compatibility, gradual migration

### Risk: Performance Overhead
**Mitigation**: Profile before/after, optimize hot paths

### Risk: Increased Complexity
**Mitigation**: Clear documentation, good abstractions

### Risk: Longer Build Times
**Mitigation**: Incremental builds, parallel compilation

## Conclusion

Separating the editor and runtime is essential for:
1. Professional game engine architecture
2. Faster development iteration
3. Smaller game distributions
4. Plugin ecosystem growth
5. Long-term maintainability

The plugin system (Phase 1) provides immediate value while laying the foundation for full separation in future phases.

---

## Next Steps

1. ✅ Create plugin directory structure
2. ✅ Enhance IWorldGenerator interface
3. ✅ Create IEngineMode interface
4. ⏳ Implement world generation plugins
5. ⏳ Implement engine mode plugins
6. ⏳ Add plugin loading system
7. ⏳ Add plugin hot-reload
8. ⏳ Document plugin API
9. ⏳ Begin Phase 2 planning
