# Fresh Voxel Engine - Gameplay Demos & Examples

This directory contains information about the **actual playable gameplay demos** available in the Fresh Voxel Engine. Unlike traditional example files, all demos are **integrated directly into the main engine executable** and can be played immediately.

## 📚 Editor Guides

New to the Unreal Engine-inspired editor? Start here:

- **[Unreal Editor Quick Start](UNREAL_EDITOR_QUICKSTART.md)** - Quick answers to common questions:
  - How to customize the editor layout
  - How to use Lua scripting for interactive elements (Blueprint alternative)
  - How to sculpt terrain with voxel editing tools (Modeling Mode equivalent)

- **[Complete Workflow Guide](../docs/guides/UNREAL_ENGINE_INSPIRED_WORKFLOW.md)** - Comprehensive guide covering:
  - Core interface components
  - Layout customization examples
  - Interactive element scripting with Lua
  - Advanced terrain sculpting techniques

## 🎮 How to Use These Demos

All gameplay demos are accessible through the main `FreshVoxelEngine.exe` executable. Simply:

1. Build and run the engine: `FreshVoxelEngine.exe`
2. Use the main menu to select your demo scenario
3. Play and test the features interactively

**No separate compilation needed** - everything is built into one executable!

## 📋 Available Gameplay Demos

### Demo 1: Basic World Exploration
**What it demonstrates:** Core voxel engine functionality
- Procedural terrain generation with varied biomes
- Chunk streaming and LOD system
- First-person camera controls
- Physics-based player movement

**How to play:**
1. Launch FreshVoxelEngine.exe
2. Select "Create New World"
3. Enter world name and seed (try seed: 12345 for interesting terrain)
4. Press ESC when done to return to menu

**Controls:**
- WASD - Move
- Mouse - Look around
- Space - Jump
- Shift - Sprint
- ESC - Pause/Menu

### Demo 2: Block Interaction & Building
**What it demonstrates:** Voxel manipulation and interaction
- Raycast-based block selection
- Block placement and removal
- 13+ different voxel types
- Real-time mesh updates

**How to play:**
1. Create or load a world
2. Look at a block and interact with it
3. Left-click to break blocks
4. Right-click to place blocks
5. Use number keys 1-9 to select block types

**Block Types Available:**
- 1 - Stone
- 2 - Dirt
- 3 - Grass
- 4 - Sand
- 5 - Water
- 6 - Wood
- 7 - Leaves
- 8 - Glass
- 9 - Ore

### Demo 3: World Editor & Terraforming
**What it demonstrates:** Advanced world editing tools
- Brush-based terraforming
- Sphere and cube placement
- Terrain smoothing and flattening
- Undo/Redo system (up to 100 operations)

**How to play:**
1. Create or load a world
2. Press T to toggle the editor interface
3. Use the editor tools panel to select tools
4. Click and drag to terraform terrain

**Editor Tools:**
- Brush Tool - Paint terrain with selected block
- Sphere Tool - Create spherical structures
- Cube Tool - Create rectangular structures
- Flatten Tool - Level terrain to uniform height
- Smooth Tool - Smooth rough terrain
- Paint Tool - Change block types without destroying structure

### Demo 4: Procedural Generation Showcase
**What it demonstrates:** Advanced terrain generation
- Multiple biome types (plains, forests, mountains, deserts)
- Cave systems and underground structures
- Natural feature placement (trees, rocks, etc.)
- Seed-based reproducible worlds

**How to play:**
1. Create multiple worlds with different seeds
2. Explore to see different biome generation
3. Compare seed results:
   - Seed 0: Balanced terrain with all biomes
   - Seed 12345: Mountain-heavy landscape
   - Seed 99999: Flat plains with scattered features
   - Seed 424242: Dense forests with caves

### Demo 5: Performance & Optimization
**What it demonstrates:** Engine performance capabilities
- Chunk streaming performance
- Render distance adjustment
- FPS monitoring
- Memory usage tracking

**How to play:**
1. Load a world with large render distance
2. Press F3 to show debug overlay
3. Adjust settings to see performance impact
4. Monitor FPS, chunk count, and memory usage

**Performance Tips:**
- Reduce render distance for better FPS
- Close unused chunks automatically after distance
- Greedy meshing reduces triangle count by 90%
- Frustum culling hides off-screen chunks

### Demo 6: Save & Load System
**What it demonstrates:** World persistence
- World serialization and deserialization
- Save file management
- Chunk data compression
- Player state persistence

**How to play:**
1. Create a new world and make changes
2. Exit to main menu (worlds save automatically)
3. Select "Load Existing World"
4. Load your saved world - all changes preserved

**Save System Features:**
- Automatic save on exit
- Manual save with Ctrl+S
- Compressed chunk data
- Fast loading with streaming
- Saves stored in `saves/` directory

## 🛠️ Technical Architecture

### C++20 Core Engine
The engine uses modern C++20 features:
- **Modules** (where supported by compiler)
- **Concepts** for template constraints
- **Ranges** for efficient iteration
- **Coroutines** for async operations
- **std::format** for string formatting

### .NET 9 Bindings (Optional)
C# developers can use .NET 9 bindings:
```csharp
using FreshEngine.Managed;

var engine = new Engine();
if (engine.Initialize())
{
    engine.WindowTitle = "My Voxel Game";
    engine.Run();
}
```

See [../dotnet/README.md](../dotnet/README.md) for .NET 9 integration guide.

### Lua Scripting Support
Lua scripting is available for modding and gameplay logic:
```lua
-- Example: Custom block behavior
function onBlockPlaced(x, y, z, blockType)
    print("Block placed at: " .. x .. ", " .. y .. ", " .. z)
end
```

See [../LUA_INTEGRATION_GUIDE.md](../LUA_INTEGRATION_GUIDE.md) for Lua setup.

### Python Tools
Python scripts for asset generation and tooling:
- `tools/generate_test_sounds.py` - Audio file generation
- Additional tools coming soon

## 🎯 Creating Your Own Gameplay

### Option 1: C++ Development
Modify the engine directly in C++20:

1. Add your gameplay systems to `engine/gameplay/`
2. Integrate with existing systems
3. Build with CMake
4. Test in the main executable

### Option 2: .NET 9 Development (Recommended)
Write gameplay in C# with managed bindings:

1. Reference `FreshEngine.Managed.dll`
2. Write your game logic in C#
3. Use the native engine as backend
4. Easier debugging and faster iteration

### Option 3: Lua Scripting
Create gameplay mods with Lua:

1. Write Lua scripts in `scripts/` directory
2. Hook into engine events
3. Define custom behaviors
4. Hot-reload during development

### Option 4: Python Tools
Create development tools in Python:

1. Add scripts to `tools/` directory
2. Use for asset generation and processing
3. Integrate with build pipeline
4. Extend the development workflow

## 📖 API Documentation

For detailed API documentation, see:

- **C++ API**: [../docs/api/README.md](../docs/api/README.md)
- **.NET API**: [../dotnet/README.md](../dotnet/README.md)
- **Lua API**: [../LUA_INTEGRATION_GUIDE.md](../LUA_INTEGRATION_GUIDE.md)
- **Architecture**: [../ARCHITECTURE.md](../ARCHITECTURE.md)

## 🔧 Building from Source

### Prerequisites
- **Windows 10/11** (x64)
- **Visual Studio 2022** with C++ and .NET workloads
- **CMake 3.20+**
- **.NET 9 SDK** (for managed bindings)
- **Python 3.8+** (for tools)

### Build Commands
```batch
# Automated build (recommended)
.\setup-and-build.ps1

# Or manual build
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build --config Release
```

See [../BUILD.md](../BUILD.md) for complete build instructions.

## 🎓 Learning Path

### Beginner (Start Here!)
1. **Demo 1** - Learn basic controls and movement
2. **Demo 2** - Understand block interaction
3. **Demo 6** - Learn about world persistence

### Intermediate
4. **Demo 3** - Master world editing tools
5. **Demo 4** - Understand procedural generation
6. **Demo 5** - Optimize performance

### Advanced
7. Modify C++ source code
8. Create .NET gameplay extensions
9. Write Lua mods
10. Develop Python tools

## 🆘 Getting Help

- 📖 **Documentation**: [../DOCUMENTATION_INDEX.md](../DOCUMENTATION_INDEX.md)
- 💬 **Discussions**: [GitHub Discussions](https://github.com/shifty81/fresh/discussions)
- 🐛 **Issues**: [Report a Bug](https://github.com/shifty81/fresh/issues)
- 🤝 **Contributing**: [../CONTRIBUTING.md](../CONTRIBUTING.md)

## 📊 Engine Capabilities

Current engine features demonstrated in gameplay:

| Feature | Status | Demo |
|---------|--------|------|
| Procedural Terrain | ✅ Complete | Demo 1, 4 |
| Player Movement | ✅ Complete | Demo 1 |
| Block Interaction | ✅ Complete | Demo 2 |
| World Editor | ✅ Complete | Demo 3 |
| Save/Load System | ✅ Complete | Demo 6 |
| Physics System | ✅ Complete | Demo 1 |
| Chunk Streaming | ✅ Complete | Demo 1, 5 |
| DirectX 11/12 | ✅ Complete | All |
| OpenGL | ✅ Complete | All |
| .NET 9 Bindings | 🟨 Framework | - |
| Lua Scripting | 🟨 Framework | - |
| Python Tools | 🟨 Basic | - |
| Multiplayer | 🔄 Planned | - |
| Audio System | ✅ Complete | - |

**Legend:**
- ✅ Complete and tested
- 🟨 Framework in place, needs expansion
- 🔄 Planned for future release

## 🚀 Next Steps

After exploring the demos:

1. **Read the Documentation** - Understand the architecture
2. **Modify the Source** - Make your own changes
3. **Create Mods** - Use Lua scripting
4. **Develop Tools** - Write Python utilities
5. **Contribute** - Submit improvements via PR

---

**Ready to play?** Launch `FreshVoxelEngine.exe` and start exploring!

**Want to develop?** Choose your language: C++20, C# (.NET 9), Lua, or Python!
