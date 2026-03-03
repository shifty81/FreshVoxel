# Changelog

All notable changes to FreshVoxel Engine will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- **Physics Integration for Character System** — Phase 7 Weeks 13-14 implementation
  - `ClothSimulation`: Verlet-integration particle-based cloth physics for capes/clothing
    - Rectangular particle grid with structural (horizontal/vertical) and shear (diagonal) constraints
    - Configurable damping and constraint solver iterations
    - Pin/unpin particles, apply forces (gravity, wind), reset to initial state
    - 17 new tests for grid construction, constraint satisfaction, pinning, and hanging behavior
  - `RagdollSystem`: Skeleton-to-ragdoll conversion for death/hit physics
    - Converts HumanoidSkeleton bone hierarchy into Verlet-integrated rigid bodies
    - Distance constraints maintain bone-to-bone rest lengths
    - Activate/deactivate ragdoll at any world position
    - Apply impulses to specific bodies or radially from a point (explosion/hit)
    - Configurable gravity, damping, and constraint iterations
    - 17 new tests for initialization, activation, gravity, impulse, and constraint maintenance
  - `HitReactionSystem`: Procedural hit reaction system for character animations
    - Process hits with direction, force, and target bone
    - Compute relative hit direction from character orientation (Front, Back, Left, Right, Above, Below)
    - Decaying rotation offsets applied to skeleton bones
    - Force-scaled reactions capped at 30° maximum rotation
    - Multiple simultaneous reactions with configurable maximum
    - Global intensity multiplier for gameplay tuning
    - 21 new tests for hit processing, decay, direction computation, and bone offsets
  - 55 new tests total for physics integration

- **Dynamic Weapon/Tool Handling System** — Complete weapon attachment and procedural swing animations
  - `WeaponAttachment`: Attach weapons/tools to character bone attachment points (RightHand, LeftHand, BothHands, Back, Hip)
  - `WeaponDefinition`: Data-driven weapon definitions with grip type, weight, length, and voxel geometry
  - Grip types: OneHanded, TwoHanded, DualWield, Shield, Tool with slot compatibility validation
  - `calculateGripPosition()`: World-space grip calculation from bone transforms + weapon offsets
  - `generateSwing()`: Procedural swing animation generation (Slash, Overhead, Thrust, Chop, Sweep, Block)
  - Weight-based swing speed — heavier weapons produce slower, more powerful swings
  - Two-handed weapons automatically occupy both hand slots; detaching either releases both
  - `getTransformedWeaponVoxels()`: Weapon voxels follow bone animations in world space
  - Left-hand mirroring for off-hand swing animations
  - 45 new tests for attachment, compatibility, swing generation, voxel transforms, and edge cases

- **Atlas-style Editor Workflow** — Build, package, and live-test games from the editor
  - `GamePackager`: Packages game projects (world saves, assets, shaders, sounds, textures, config, Lua scripts) into distributable directories with a `package.json` manifest. Users can zip and share for play-testing.
  - `ClientLauncher`: Launches FreshClient/FreshServer/FreshRuntime from the editor as separate processes for live testing. Auto-saves the world before launch. Cross-platform (CreateProcess on Windows, fork+exec on Linux/macOS).
  - `EditorManager::buildPackage()`, `launchClient()`, `launchServer()` operations wired into the editor
  - FreshClient now accepts `--project`, `--world`, `--width`, `--height`, `--windowed` CLI arguments for editor integration

- **ViewportContext Rendering Pipeline** — Voxel world now renders through ViewportContext's render target
  - Per ENGINE.md: "Nothing renders unless a ViewportRenderTarget is bound"
  - Editor separates viewport rendering (voxel world) from editor composite (UI panels)
  - Falls back to legacy direct-render path when ViewportContext is not yet initialized
  - ViewportContext automatically wired to new worlds on creation

- **OpenGL Texture GPU Operations** — Complete implementation of Texture GPU operations
  - `createFromData()`: Uploads pixel data to GPU via glGenTextures/glTexImage2D
  - `createEmpty()`: Creates empty GPU textures for render targets
  - `setFilter()`: Updates GPU min/mag filter modes (Nearest, Linear, Bilinear, Trilinear)
  - `setWrap()`: Updates GPU wrap modes (Repeat, Clamp, Mirror)
  - `bind()`/`unbind()`: Texture binding to GPU texture units
  - `cleanup()`: Proper GPU resource deletion with glDeleteTextures
  - Full TextureFormat support: R8, RG8, RGB8, RGBA8, RGB16F, RGBA16F, RGB32F, RGBA32F, Depth24, Depth32F
  - Non-OpenGL fallback preserved for DirectX backends

- **ChunkStreamer Background Generation** — Background thread now pre-generates chunk data
  - Background thread pulls from generation queue and builds terrain + mesh data
  - Pre-generated chunks stored in ready cache for main thread to pick up
  - Condition variable notification for efficient thread wake-up
  - `VoxelWorld::generateChunkData()` for thread-safe terrain generation
  - Non-const `VoxelWorld::getChunks()` accessor for chunk insertion

- **22 new tests** for GamePackager (12 tests) and ClientLauncher (10 tests)

### Changed
- `Engine::renderEditor()` now routes voxel world rendering through ViewportContext when available
- `Engine::createNewWorld()` wires ViewportContext to the new world
- FreshClient updated with CLI argument parsing for editor integration

### Added
- **FreshVoxel Rebranding** — Project renamed from "Fresh" to "FreshVoxel" to reflect voxel-only focus
  - All project files, build scripts, and documentation updated
  - Scope locked to voxel games only (isometric/2.5D to full 3D)
  - Future Fresh2D fork planned for 2D game development
  - ENGINE.md added with engine rules and architecture vision
  - Logging added to setup-and-build.ps1 script
- **Win32TerraformingPanel** - Native Windows GUI for terraforming tools (v0.2.6)
  - Full tool selection UI with buttons for all 10 terraforming tools
  - Mode selection buttons (Place, Remove, Replace)
  - Material picker with 5+ common voxel types
  - Size control with +/- buttons and live display
  - Undo/Redo buttons with automatic enable/disable
  - Unreal-style dark theme integration
  - Auto-synchronization with TerraformingSystem
  - Replaces console-based EditorGUI with proper native GUI

- **Windows-Native Windowing System** - Complete migration to native Win32 window management
  - Replaced GLFW with native Win32Window on Windows for optimal platform integration
  - Native Win32 input handling with Win32InputManager
  - Native Win32 menu bar and toolbar support
  - ImGui editor panels integrated with Win32 windowing
  - GLFW retained only for Linux CI testing purposes
  - Per-monitor DPI awareness for crisp rendering on 4K/5K displays
  - Full Windows 10/11 theme integration
  - Jump lists and taskbar integration
  - Toast notifications support
  
- **Complete Cross-Platform Build System** - Production-ready builds on all platforms
  - Full CMake configuration for Windows, Linux, and macOS
  - Platform-specific dependency detection and linking
  - Cross-platform compiler flags (MSVC, GCC, Clang)
  - FRESH_GLEW_AVAILABLE definition for OpenGL extension loading
  - Comprehensive Linux and macOS build instructions in BUILD.md
  - Updated README.md with platform-specific quick start guides
  
- **Cross-Platform Support** - Engine now builds on Windows, Linux, and macOS
  - Platform detection and conditional compilation
  - OpenGL 4.5+ rendering backend for cross-platform support
  - Automatic graphics API selection based on platform
  - GLEW integration for OpenGL extension loading
  - Full OpenGL resource management (buffers, textures, shaders)
  
- **ImGui DirectX Rendering Backends** - Complete DirectX 11/12 support for ImGui
  - Implemented DirectX 11 ImGui rendering backend
  - Implemented DirectX 12 ImGui rendering backend
  - Added SRV descriptor heap to DirectX12RenderContext for ImGui
  - Added public getters to access DirectX device/context for ImGui integration
  - Full support for ImGui initialization, frame rendering, and cleanup
  - OpenGL ImGui backend already functional

### Changed
- **Build System Improvements** - Enhanced cross-platform compatibility
  - DirectX code wrapped in platform-specific conditionals
  - DirectX libraries only linked on Windows
  - OpenGL and GLEW libraries linked on Linux and macOS
  - Test configuration updated with required source files
  - Fixed unused variable warnings with [[maybe_unused]] attribute
  
- **Removed Windows-Only Restriction** - Engine is now truly cross-platform
  - Updated CMakeLists.txt to support multiple platforms
  - DirectX backends available on Windows
  - OpenGL backend available on all platforms
  - Conditional compilation for platform-specific features
  - Smart dependency detection and linking
  - Platform-specific installation instructions

### Fixed
- **ImGui Win32 Backend OpenGL Support** - Fixed ImGui initialization failure with Win32 and OpenGL
  - Added OpenGL case to `initializeWin32` graphics backend switch statement
  - Matches GLFW backend implementation for consistency
  - Fixes error: "[ERROR][ImGuiContext] Unsupported graphics API for ImGui with Win32"
  - Enables editor UI when using Win32 window with OpenGL renderer
- **EditorManager Initialization** - Fixed engine initialization failure
  - EditorManager can now be initialized before a world is created
  - Made world and worldEditor optional parameters (can be nullptr)
  - World-dependent panels are deferred until world is available
  - MainMenuPanel and SettingsPanel initialize without requiring a world
  - Fixes error: "[ERROR][EditorManager] Invalid parameters for EditorManager"
- Compilation errors on Linux (DirectX-specific code)
- Linking errors in test suite (missing VoxelWorld.cpp and ModularAssetSystem.cpp)
- Unused variable warnings in IKSolver.cpp and Engine.cpp

### Tested
- ✅ Linux build with GCC 13.3.0 (Ubuntu 24.04)
- ✅ All 156 unit tests passing on Linux
- ✅ OpenGL rendering backend functional

### Documentation
- Updated BUILD.md with comprehensive Linux and macOS instructions
- Updated README.md to highlight cross-platform support
- Added platform comparison table
- Added quick build instructions for each platform

### Added
- Comprehensive project documentation
  - CONTRIBUTING.md guide
  - CODE_OF_CONDUCT.md
  - ARCHITECTURE.md with system diagrams
  - LICENSE (MIT)
  - CHANGELOG.md
- GitHub issue templates
- GitHub pull request template
- CI/CD workflow with GitHub Actions
- Example code and tutorials

## [0.2.7] - 2026-03-01

### Added
- **AtlasForge Networking Merge** — Production-ready networking systems ported from AtlasForge engine
  - **NetContext** — Packet-based networking with lockstep/rollback, P2P support, and input recording
  - **NetHardening** — Connection hardening: timeout, reconnect, bandwidth throttling, heartbeat, packet loss simulation
  - **QoSScheduler** — Priority-based packet scheduling with congestion detection
  - **Replication** — Component replication rules: server-to-client, on-change, manual, reliable/unreliable
  - CRC32 packet checksums and validation
- **Blueprint Serialization** — Save/load blueprint graphs in text format
- **Dialogue Condition Evaluation** — Variable-based expression evaluation (==, !=, >, <, >=, <=)
- **LLM Assistant Panel** — AI-powered developer assistant integrated into editor

### Changed
- **Networking upgraded from 10% → 45%** — AtlasForge merge added packet layer, hardening, QoS, and replication
- **GameServer.processMessage()** — Was stub; now handles Connect, Disconnect, SectorChange, EntityUpdate, ChatMessage
- **SectorServer.broadcastToPlayers()** — Was empty; now serializes messages for delivery

### Optimized
- **ReplicationManager** — Linear scan replaced with unordered_map for O(1) rule lookups
- **QoSScheduler** — Dirty flag avoids redundant sorts on consecutive dequeues
- **GameServer** — Client→sector mapping avoids O(n) sector iteration on SectorChange

### Verified
- ✅ All 22 engine subsystems properly initialized and wired in Engine.cpp
- ✅ All 40+ include headers resolve — no broken references
- ✅ Shutdown cleanup verified — all unique_ptrs properly reset

## [0.2.0] - 2025-11-03

### Added
- **Modular Asset System** - Complete asset pack support with auto-discovery
  - Auto-scan asset directory for packs
  - JSON manifest-based asset definitions
  - 7 placement rules (Random, Clustered, Grid, NearWater, OnHills, InCaves, RiverSide)
  - Biome-specific asset placement
  - Smart placement with distance enforcement
  - Asset pack template generation
- **Enhanced Core Systems**
  - Memory management with pool allocators
  - Event system for decoupled communication
  - Scene graph with transformations
  - Spatial hashing for collision optimization
  - Hot-reloading for resources

### Completed
- All 10 core engine systems (100% foundation)
- Player controller with first-person camera
- Chunk streaming for infinite worlds
- Enhanced physics with comprehensive collision detection
- Input management with rebindable controls
- Resource management with caching
- Audio engine framework (ready for backend)
- Scripting engine framework (ready for backend)

### Documentation
- Created comprehensive documentation suite
  - ROADMAP.md - Development roadmap
  - COMPLETE_STATUS.md - Implementation status
  - GETTING_STARTED.md - Quick start guide
  - CPP_GUIDELINES.md - C++ development standards
  - IMPLEMENTATION_SUMMARY.md - Feature summary
  - docs/TERRAFORMING.md - Terraforming guide

## [0.1.0] - 2025-10-15

### Added
- **Core Engine Foundation**
  - CMake-based build system
  - Engine initialization and main loop
  - Window management with GLFW
  
- **Multi-API Rendering System**
  - OpenGL 4.5+ renderer implementation
  - DirectX 11 renderer (Windows)
  - DirectX 12 renderer (Windows)
  - ShaderManager with API-agnostic support
  - Basic vertex and fragment shaders

- **Voxel World System**
  - Chunk-based world (16x256x16 voxels)
  - 13 voxel types (Air, Stone, Dirt, Grass, Sand, Water, Wood, Leaves, Coal, Iron, Gold, Diamond, Bedrock)
  - Face culling optimization
  - Greedy meshing algorithm

- **Procedural Generation**
  - Perlin noise implementation
  - Multi-octave fractal noise
  - TerrainGenerator with biomes
  - 3D noise for cave generation
  - Height-based biome selection

- **World Editing System**
  - TerraformingSystem with 10 tools
    - Single Block, Brush, Sphere, Filled Sphere
    - Cube, Filled Cube, Line
    - Flatten, Smooth, Paint
  - Undo/Redo system (100 operations)
  - WorldEditor integration
  - EditorGUI framework (console-based)

- **Serialization**
  - WorldSerializer (binary format)
  - Chunk compression
  - World save/load functionality

- **UI Systems**
  - Main Menu (console-based)
  - Create/Load world functionality
  - World management

- **Foundation Systems**
  - PhysicsSystem foundation
  - RaycastSystem foundation
  - AISystem framework with behavior trees

### Technical Details
- C++20 standard
- Modern CMake configuration
- Windows-native with DirectX 12 support
- Modular architecture
- Clean code structure

## Release Types

### Major Releases (X.0.0)
- Significant architectural changes
- Breaking API changes
- Major new features

### Minor Releases (0.X.0)
- New features
- Non-breaking API additions
- Significant improvements

### Patch Releases (0.0.X)
- Bug fixes
- Performance improvements
- Documentation updates
- Minor tweaks

## Upcoming Features

See [ROADMAP.md](ROADMAP.md) for detailed future plans.

### Next Release (0.3.0) - Planned
- ImGui integration for UI
- Complete rendering pipeline for all backends
- Inventory system implementation
- Resource gathering mechanics
- Day/night cycle
- Weather system foundation

### Future Releases
- Multiplayer networking (0.4.0)
- Profession system (0.5.0)
- Advanced AI and NPCs (0.6.0)
- Audio system integration (0.7.0)
- Full release (1.0.0)

## Links

- [Repository](https://github.com/shifty81/fresh)
- [Issues](https://github.com/shifty81/fresh/issues)
- [Discussions](https://github.com/shifty81/fresh/discussions)
- [Contributing Guide](CONTRIBUTING.md)

---

**Note**: This project is in active development. Features and APIs may change between releases.
