# Fresh Voxel Engine

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++20](https://img.shields.io/badge/C++-20-blue.svg)](https://en.cppreference.com/w/cpp/20)
[![.NET 9](https://img.shields.io/badge/.NET-9.0-512BD4.svg)](https://dotnet.microsoft.com/)
[![CMake](https://img.shields.io/badge/CMake-3.20--3.31-064F8C.svg)](https://cmake.org/)
[![Platform](https://img.shields.io/badge/Platform-Windows%2010%2F11-0078D4.svg)](https://github.com/shifty81/fresh)
[![Graphics](https://img.shields.io/badge/Graphics-DirectX%2012%2F11-00B2FF.svg)](https://github.com/shifty81/fresh)
[![Editor](https://img.shields.io/badge/Editor-Unreal--like-orange.svg)](https://github.com/shifty81/fresh)

A professional **Windows-exclusive** game editor and development platform built with **C++20**, featuring an **Unreal Engine-like editor-first workflow**, **DirectX 12/11** rendering, voxel worlds with **pluggable generation**, **.NET 9 bindings**, **Lua scripting**, and comprehensive Windows-native integration.

> 🎨 **Editor-First Design** - Fresh is a game editor that happens to support voxel worlds, not just a voxel engine. Start with the editor, create scenes, and use world generation as a plugin.
> 📚 **New to Fresh?** See [DOCUMENTATION.md](DOCUMENTATION.md) for complete documentation guide.
> 🎮 **Windows Gaming Platform** - Exclusive focus on Windows 10/11 with DirectX and professional game development tools.

---

## ✨ Features

### Editor-First Game Development Platform
- **🎨 Editor-First Workflow** - Start with editor, create/open scenes, not a traditional "game with menu"
- **🎬 Scene-Based Development** - Unreal-style scenes, not just "worlds" - edit, save, test cycle
- **🔌 Pluggable World Generation** - World generators are plugins, not the main feature
- **🎮 Multiple Scene Types** - 3D voxel, 2D platformer, 2D top-down, empty/manual scenes
- **🪟 Windows-Native Editor** - Professional Win32 UI with menus, toolbars, panels, and dialogs
- **⚡ DirectX 12/11 Rendering** - Modern graphics API with hardware acceleration
- **🎯 Visual Studio 2022 Optimized** - Seamless integration with Microsoft's flagship IDE
- **💻 High DPI Support** - Per-monitor DPI awareness for crisp rendering on 4K/5K displays
- **🚀 AVX2 Optimized** - SIMD instructions for 15-30% performance improvement

### Core Engine
- **C++20** - Modern C++ with strong type safety and performance
- **.NET 9 Bindings** - Optional C# scripting support for rapid development
- **Lua Scripting** - Optional scripting engine for gameplay and modding
- **CMake Build System** - Professional build configuration with Visual Studio 2022 focus
- **Windows SDK Integration** - Full access to Windows platform capabilities
- **Compiler Optimizations** - Whole program optimization, link-time code generation, fast floating-point

### Voxel World System (Pluggable)
- **Plugin-Based Generation** - World generators are optional plugins, not core features
- **Built-In Generators** - 3D Terrain, 2D Platformer, 2D Top-Down, Empty/Manual
- **Custom Generators** - Create your own world generators via plugin interface
- **Chunk-Based World** - Efficient 16×256×16 chunk system
- **13+ Voxel Types** - Stone, Dirt, Grass, Sand, Water, Wood, and more
- **Dynamic Streaming** - World loads around player as they explore
- **Greedy Meshing** - Optimized rendering with face culling

### Editor & Tools (Unreal Engine-Like Experience)
- **Professional Game Editor** - Comprehensive native Windows editor for complete game development
- **Windows Native Windowing** - Native Win32 window management with native menu bar and toolbar support
- **Windows Native UI** - All editor panels use native Windows controls (tree views, list views, property grids)
- **Windows Native Integration** - Native Windows 10/11 themes, dialogs, and taskbar features
- **Terraforming Tools** - Brush, sphere, cube, flatten, smooth, paint for world building
- **Voxel Building Tools** - Enshrouded-inspired building system with specialized tools
  - **Construction Hammer** - Quick building with 7 blueprint types (walls, foundations, columns, floors, stairs, arches)
  - **Pickaxe** - Mining and removal with area operations and bedrock protection
  - **Rake** - Terrain leveling, flattening, and smoothing for foundations
  - See [VOXEL_TOOLS.md](docs/guides/VOXEL_TOOLS.md) for complete guide
- **NPC Creation System** - Easy-to-use interface for adding NPCs (villagers, traders, enemies)
  - Visual NPC placement in world editor
  - Dropdown behavior selection (Friendly, Trader, Hostile, Patrol, Guard, etc.)
  - Configure AI behaviors without coding
  - Use existing character, AI, and behavior tree systems
- **Scene Hierarchy** - Visual world structure and entity browser (native tree control)
- **Content Browser** - Complete asset management system (native list/icon view)
- **Entity Inspector** - Edit properties of selected NPCs and objects (native property grid)
- **Undo/Redo** - Up to 100 operations in history
- **Hot-Reload** - Shader and script changes update in real-time
- **Play-in-Editor** - Test gameplay instantly without leaving editor

### Gameplay & NPC Systems
- **First-Person Controller** - Smooth WASD movement with physics
- **NPC System** - Villagers, wandering traders, enemies with AI behaviors
  - Behavior Tree AI for intelligent NPC actions
  - Easy editor integration for non-programmers
  - Configurable behaviors: Friendly, Trader, Hostile, Patrol, Guard, Wander
- **Block Interaction** - Place and remove blocks with mouse
- **Collision Detection** - Physics-based player and NPC movement
- **Camera System** - Free-look FPS camera
- **Main Menu** - Create/load worlds with save system

### Additional Systems
- **Audio Engine** - OpenAL-based audio with WAV support
- **Asset Management** - Modular asset system with texture loading
- **Physics System** - Gravity, collision, and raycasting for players and NPCs
- **AI Framework** - Behavior tree system for NPC intelligence
- **Character System** - Voxel character rendering with IK and animation
- **Networking** - Framework in place for multiplayer (stub implementation)

---

## 🚀 Quick Start

### Prerequisites

**Windows 10/11 Only** - Fresh Voxel Engine is exclusively designed for Windows gaming and development.

- **Windows 10/11** (x64) - Required
- **Visual Studio 2022** with "Desktop development with C++" - Required
- **CMake 3.20-3.31** (stable releases only) - Required
  - ⚠️ **Avoid**: CMake 4.x or pre-release versions (may cause build failures)
  - Download: https://cmake.org/download/
- **Git for Windows** - Required
- **.NET 9 SDK** (optional, for C# scripting bindings)
- **Windows 10 SDK** (included with Visual Studio) - For DirectX support

### Build and Run

#### Automated Build (Recommended)

```powershell
# Clone repository
git clone https://github.com/shifty81/fresh.git
cd fresh

# Run automated setup script (installs dependencies, builds project)
.\setup-and-build.ps1
```

This script handles everything: vcpkg setup, dependency installation, CMake generation, and building (10-20 minutes first run).

**See [docs/getting-started/AUTOMATED_BUILD.md](docs/getting-started/AUTOMATED_BUILD.md) for detailed options.**

#### Manual Build

```batch
# 1. Clone repository
git clone https://github.com/shifty81/fresh.git
cd fresh

# 2. Set up vcpkg (one-time setup)
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
bootstrap-vcpkg.bat
cd ..

# 3. Generate Visual Studio solution
generate_vs2022.bat

# 4. Build
cd build
cmake --build . --config Release

# 5. Run
Release\FreshEditor.exe     # Full editor
Release\FreshClient.exe     # Player runtime
Release\FreshServer.exe     # Headless server
Release\FreshRuntime.exe    # Runtime launcher
```

**For complete step-by-step instructions, see [docs/getting-started/BUILD.md](docs/getting-started/BUILD.md).**

---

## 🎨 Editor-First Workflow (Project-Based)

Fresh uses an **Unreal Engine-style project-based workflow**. You start by creating a project, then add content:

### Creating Your First Project

1. **Launch the Editor**
   ```bash
   Release\FreshVoxelEngine.exe
   ```
   The editor opens immediately with full UI

2. **Create a New Project**
   - Click `File > New Project` (or press `Ctrl+N`)
   - Enter project name (e.g., "MyGame")
   - Choose template:
     - **Blank** - Empty project, build from scratch
     - **3D Voxel** - Minecraft-style game ready
     - **2D Platformer** - Terraria-style side-scroller
     - **2D Top-Down** - Zelda-style overhead view
   - Click "Create"

3. **Add a Level (Optional)**
   - For Blank projects, add content manually or create a level:
   - Click `File > New Level` (or press `Ctrl+Shift+N`)
   - Choose world generator:
     - **3D Terrain** - Procedural terrain with biomes
     - **2D Platformer** - Side-scrolling world
     - **2D Top-Down** - Overhead world
     - **Empty** - Blank canvas for manual building
   - Configure seed and click "Create"

4. **Edit Your Content**
   - Use terraforming tools (Brush, Sphere, Flatten, Paint)
   - Place NPCs and entities
   - Build structures with Construction Hammer
   - Arrange objects with Transform Gizmo

5. **Test Your Project**
   - Click the **Play** button (or press `F5`)
   - Editor hides, gameplay starts
   - Test your creation
   - Press `ESC` to return to editor

6. **Save Your Project**
   - `File > Save All` (or `Ctrl+Shift+A`)
   - Project and all content saved

**See [docs/architecture/PROJECT_WORKFLOW.md](docs/architecture/PROJECT_WORKFLOW.md) for complete project guide.**

---

## 🎮 Controls

### Editor Mode (Default)

**Camera Controls:**

**Mouse Controls:**
- **Mouse Movement** - Look around (first-person camera with improved sensitivity)
- **Left Click** - Break blocks
- **Right Click** - Place blocks
- **Editor UI** - Click, drag, and interact with all editor panels

- **Mouse Movement** - Look around (free-look camera)
- **Right Mouse + Drag** - Rotate camera (Unreal-style)
- **WASD** - Move in camera direction
- **Space** - Fly up (ascend)
- **Ctrl** - Fly down (descend)
- **Shift** - Move faster
- **F5** - Enter Play Mode

**Editor Tools:**
- **Left Click** - Select objects
- **Right Click** - Place blocks / Context menu
- **Click Panels** - Interact with Scene Hierarchy, Inspector, Content Browser
- **Transform Gizmo** - Move/rotate/scale selected objects
- **Terraforming Tools** - Brush, Sphere, Flatten, Paint from toolbar

### Play Mode (Testing)

**Movement:**
- **WASD** - Move (FPS-style)
- **Space** - Jump
- **Shift** - Sprint
- **Ctrl** - Crouch
- **ESC** - Exit Play Mode (return to editor)

**Interaction:**
- **Left Click** - Break blocks
- **Right Click** - Place blocks
- **Physics enabled** - Gravity and collision active

**See [docs/guides/CONTROLS.md](docs/guides/CONTROLS.md) for complete controls reference.**

### Getting Started with the Editor

1. **Launch** - Run FreshVoxelEngine.exe
2. **Main Menu** - Choose "Create New World" or "Load Existing World"
3. **Explore** - Fly around in free-camera mode to explore procedurally generated terrain
4. **Build** - Place and remove blocks with mouse clicks
5. **Add NPCs** - Use the Entity menu to spawn villagers, traders, or enemies
6. **Configure NPCs** - Select an NPC and use the Inspector panel to:
   - Choose behavior type (Friendly, Trader, Hostile, Patrol, Guard, Wander)
   - Set patrol routes or guard positions
   - Configure trading inventory (for traders)
   - Set hostility and combat parameters (for enemies)
7. **Edit** - Use the always-visible editor toolbar and panels for advanced features
8. **Test** - Click **Play** in the editor toolbar to test gameplay with normal physics and active NPCs

**See [examples/README.md](examples/README.md) for 6 playable in-engine demo scenarios.**

**Try the Showcase Demo**: Create a world named "ShowcaseDemo" with seed **12345** to see all engine capabilities! See [examples/DEMO_SHOWCASE.md](examples/DEMO_SHOWCASE.md) for the complete guided demo.

---

## 🎨 Development Languages & Tools

Fresh Voxel Engine supports multiple development approaches for Windows:

### Primary: C++20 Core Engine
- **Modern C++ features**: Concepts, ranges, coroutines
- **High performance**: Direct hardware access
- **Full engine control**: All features available
- **Build with**: CMake + Visual Studio 2022

### Optional: .NET 9 C# Bindings
- **Managed development**: Memory safety, easier debugging
- **Rapid iteration**: Hot reload, faster compile times
- **Rich ecosystem**: NuGet packages, LINQ, async/await
- **Interop**: P/Invoke to native engine
- **See**: [docs/architecture/DOTNET_INTEGRATION.md](docs/architecture/DOTNET_INTEGRATION.md)

### Optional: Lua Scripting
- **Gameplay modding**: Custom behaviors and logic
- **Hot reload**: Change scripts without recompiling
- **Easy learning**: Simple syntax for non-programmers
- **Event system**: Hook into engine events
- **See**: [docs/guides/LUA_INTEGRATION_GUIDE.md](docs/guides/LUA_INTEGRATION_GUIDE.md)

### Tools: Python 3.8+
- **Asset pipeline**: Texture atlas generation
- **Build verification**: Environment checking
- **World analysis**: Debug saved worlds
- **Cross-platform**: Works on Windows, Linux, macOS
- **See**: [tools/README.md](tools/README.md)

**Choose your language based on your needs:**
- Performance-critical code → C++20
- Game logic & rapid development → .NET 9 C#
- Modding & scripting → Lua
- Tools & automation → Python

---

## 📁 Project Structure

Atlas-style modular architecture — the engine ships as multiple executables linked against a shared core library:

| Binary         | Purpose                              |
|----------------|--------------------------------------|
| `FreshEditor`  | Full editor + runtime (superset)     |
| `FreshClient`  | Player runtime                       |
| `FreshServer`  | Headless authoritative server        |
| `FreshRuntime` | Runtime launcher with CLI options    |

```
fresh/
├── CMakeLists.txt          # Root build — add_subdirectory() for each target
├── vcpkg.json              # Dependency manifest
├── generate_vs2022.bat     # Visual Studio generator
├── setup-and-build.ps1     # Automated setup script
│
├── engine/                 # Core engine library (FreshEngine)
│   ├── CMakeLists.txt      # Builds FreshEngine as STATIC library
│   ├── core/               # Engine bootstrap, logging, config, window
│   ├── renderer/           # DirectX 11/12, OpenGL rendering
│   │   └── backends/       # Platform-specific render backends
│   ├── voxel/              # Voxel world and chunks
│   ├── generation/         # Procedural terrain generation
│   ├── physics/            # Physics and collision
│   ├── editor/             # Editor systems (GUI, selection, tools)
│   ├── ui/                 # UI framework and native Win32 panels
│   │   └── native/         # Win32 native controls
│   ├── gameplay/           # Player, camera, time, weather
│   ├── audio/              # Audio engine
│   ├── character/          # Voxel character system
│   ├── assets/             # Asset management
│   ├── ecs/                # Entity/Component/System
│   ├── ai/                 # AI system, behavior trees
│   ├── input/              # Input management
│   ├── interaction/        # Raycasting, voxel tools
│   ├── networking/         # Client/server networking
│   ├── scripting/          # Event system, Lua bindings
│   ├── serialization/      # World serialization
│   ├── rpg/                # RPG systems (inventory, crafting)
│   ├── galaxy/             # Galaxy generation
│   ├── voxelship/          # Voxel structures
│   └── devtools/           # Debug tools, profiler
│
├── editor/                 # Editor executable
│   ├── CMakeLists.txt
│   └── main.cpp
│
├── client/                 # Player runtime executable
│   ├── CMakeLists.txt
│   └── main.cpp
│
├── server/                 # Headless server executable
│   ├── CMakeLists.txt
│   └── main.cpp
│
├── runtime/                # Runtime launcher executable
│   ├── CMakeLists.txt
│   └── main.cpp
│
├── tests/                  # Unit tests
├── include/                # Legacy headers (backward compatibility)
├── src/                    # Legacy sources (backward compatibility)
├── shaders/                # HLSL shaders for DirectX
├── textures/               # Placeholder textures (42 blocks + 11 UI)
├── sounds/                 # Audio files
├── Assets/                 # Game assets
├── examples/               # In-engine gameplay demo documentation
├── docs/                   # Detailed documentation
├── dotnet/                 # .NET 9 C# bindings
└── tools/                  # Python development tools
```

---

## 📖 Documentation

### Getting Started
- [README.md](README.md) - This file
- [docs/getting-started/GETTING_STARTED.md](docs/getting-started/GETTING_STARTED.md) - Detailed setup guide
- [docs/getting-started/BUILD.md](docs/getting-started/BUILD.md) - Complete build instructions
- [docs/getting-started/AUTOMATED_BUILD.md](docs/getting-started/AUTOMATED_BUILD.md) - Automated setup documentation

### Features & Systems
- [docs/guides/CONTROLS.md](docs/guides/CONTROLS.md) - Complete control reference
- [docs/editor/EDITOR_INTEGRATION.md](docs/editor/EDITOR_INTEGRATION.md) - Editor usage guide
- [docs/guides/TERRAFORMING.md](docs/guides/TERRAFORMING.md) - Terraforming tools
- [docs/guides/VOXEL_TOOLS.md](docs/guides/VOXEL_TOOLS.md) - Voxel building tools (Construction Hammer, Pickaxe, Rake)
- [docs/guides/ADVANCED_MESH_GENERATION.md](docs/guides/ADVANCED_MESH_GENERATION.md) - Advanced mesh generation (marching cubes)
- [docs/guides/INPUT_SYSTEM.md](docs/guides/INPUT_SYSTEM.md) - Input system documentation
- [docs/guides/AUDIO_SYSTEM.md](docs/guides/AUDIO_SYSTEM.md) - Audio engine guide
- [docs/guides/LOGGING.md](docs/guides/LOGGING.md) - Logging system
- [textures/USAGE.md](textures/USAGE.md) - Texture system guide

### Development
- [docs/architecture/ARCHITECTURE.md](docs/architecture/ARCHITECTURE.md) - System architecture
- [docs/community/CONTRIBUTING.md](docs/community/CONTRIBUTING.md) - Contribution guidelines
- [docs/community/CPP_GUIDELINES.md](docs/community/CPP_GUIDELINES.md) - C++ coding standards
- [docs/community/DEVELOPMENT_TOOLS.md](docs/community/DEVELOPMENT_TOOLS.md) - Dev tools guide

### Status & Planning
- [docs/status/PROJECT_STATUS.md](docs/status/PROJECT_STATUS.md) - Current status
- [docs/status/ROADMAP.md](docs/status/ROADMAP.md) - Development roadmap
- [CHANGELOG.md](CHANGELOG.md) - Version history

### Advanced
- [docs/architecture/DOTNET_INTEGRATION.md](docs/architecture/DOTNET_INTEGRATION.md) - .NET 9 bindings
- [docs/guides/LUA_INTEGRATION_GUIDE.md](docs/guides/LUA_INTEGRATION_GUIDE.md) - Lua scripting
- [docs/architecture/WINDOWS_INTEGRATION.md](docs/architecture/WINDOWS_INTEGRATION.md) - Windows-native features (Jump Lists, DPI, etc.)
- [docs/editor/GUI_GUIDE.md](docs/editor/GUI_GUIDE.md) - Complete GUI system guide
- [docs/testing/TESTING.md](docs/testing/TESTING.md) - Testing guide
- [docs/WORKFLOWS.md](docs/WORKFLOWS.md) - CI/CD workflows

---

## 🎨 Graphics APIs

Fresh Engine uses DirectX as the primary graphics API for Windows gaming:

### DirectX Rendering (Primary)
- **DirectX 12** - Primary renderer for Windows 10/11, cutting-edge performance
- **DirectX 11** - Excellent compatibility fallback for wider hardware support
- **Native Windows integration** - Optimal performance and features

### OpenGL (Optional)
- **OpenGL 4.5+** - Available for compatibility and testing
- **GLEW** - For extension loading

The engine automatically selects **DirectX 12** as the primary renderer on Windows 10/11, with automatic fallback to DirectX 11 if needed. OpenGL is available as an optional backend.

---

## 🔷 .NET 9 Support (Optional)

Optional C# bindings for managed code development on Windows:

```batch
cd dotnet
dotnet build -c Release
```

**See [DOTNET_INTEGRATION.md](DOTNET_INTEGRATION.md) and [dotnet/README.md](dotnet/README.md) for details.**

---

## ⚠️ Current Status

### Platform Support
- **Windows 10/11 (x64)** ✅ **EXCLUSIVE PLATFORM** - Full support with DirectX 12/11
- **Linux** ❌ Not supported (archived)
- **macOS** ❌ Not supported (archived)

### Graphics
- **DirectX 12** - Production ready ✅ Full voxel rendering (Windows 10/11)
- **DirectX 11** - Production ready ✅ Full voxel rendering (Windows compatibility)
- **OpenGL** - Available ✅ Optional compatibility backend

### Editor & NPC Features
- **Unreal-like Editor** - ✅ Professional native Windows editor with Win32 UI components
- **NPC System** - ✅ Integrated with character, AI, and behavior tree systems
- **NPC Editor Integration** - 🚧 In development - Easy dropdown-based behavior configuration
- **Visual NPC Placement** - 🚧 In development - Click-to-place NPCs in world editor
- **Behavior Types** - 🚧 In development - Friendly, Trader, Hostile, Patrol, Guard, Wander

### Optional Features
- **Lua Scripting** - Framework exists (~40% complete), needs Sol2 integration
- **Networking** - Framework only (~10% complete), multiplayer not functional
- **Audio** - WAV support (OGG/Vorbis planned)

### Testing
- **Code Coverage** - ~30% (core systems well-tested)
- **Platform Testing** - Windows 10/11 exclusive

**For current status, see [PROJECT_STATUS.md](PROJECT_STATUS.md).**

---

## 👥 Contributing

We welcome contributions to the Windows-exclusive gaming platform! Please see [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

Ways to contribute:
- 🐛 Report bugs via [Issues](https://github.com/shifty81/fresh/issues)
- ✨ Suggest features for Windows gaming
- 📝 Improve documentation
- 🔧 Submit pull requests
- 💬 Help others in [Discussions](https://github.com/shifty81/fresh/discussions)
- 🎮 Test the NPC editor and behavior systems

Please read [CODE_OF_CONDUCT.md](CODE_OF_CONDUCT.md) before contributing.

---

## 📜 License

This project is licensed under the MIT License - see [LICENSE](LICENSE) file for details.

---

## 🙏 Acknowledgments

- Microsoft - DirectX API and Visual Studio 2022
- Unreal Engine - Editor inspiration
- Minecraft - Voxel game inspiration
- Windows gaming community

---

**Ready to build your voxel game on Windows? Get started with [GETTING_STARTED.md](GETTING_STARTED.md)!**

**Build games with the Unreal-like editor - No coding required for NPCs and gameplay!**
