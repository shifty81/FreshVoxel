# Fresh Voxel Engine - Showcase Demo

This document describes the **Showcase Demo** - a curated demonstration of Fresh Voxel Engine's capabilities using OpenGL rendering (the default graphics API on Windows).

## 🎮 Quick Start

### Running the Showcase Demo

1. **Build and run the engine:**
   ```batch
   cd build
   Release\FreshVoxelEngine.exe
   ```

2. **Create the demo world:**
   - Select **"Create New World"** from the main menu
   - Enter world name: **"ShowcaseDemo"**
   - Enter seed: **12345** (recommended for interesting terrain)
   - Press **Enter** to create

3. **Explore the world:**
   - Use **WASD** to move around
   - **Mouse** to look around
   - **Space** to jump
   - **Left-click** to break blocks
   - **Right-click** to place blocks
   - Press **T** to toggle the world editor

## 🌟 What This Demo Showcases

### 1. Graphics Rendering System (OpenGL)
**What you'll see:**
- OpenGL as the default graphics API on Windows
- Real-time voxel rendering with greedy meshing optimization
- Smooth 60+ FPS performance with proper chunk streaming
- Textured block faces with 16x16, 32x32, and 64x64 texture support
- Face culling (only visible faces are rendered)

**Try this:**
- Press **F3** to show performance metrics
- Watch FPS counter remain steady even with many chunks loaded
- Observe how chunks load/unload as you move around

### 2. Procedural Terrain Generation
**What you'll see:**
- Perlin noise-based terrain generation
- Multiple biome types (plains, forests, mountains, deserts)
- Natural height variation and interesting landforms
- Cave systems underground
- Seed-based reproducible worlds

**Recommended seeds to try:**
- **12345** - Mountain-heavy landscape with varied terrain
- **0** - Balanced terrain with all biomes
- **99999** - Flat plains with scattered features
- **424242** - Dense forests with cave systems

**Try this:**
- Create multiple worlds with different seeds
- Explore to discover caves and different biomes
- Note how the same seed always generates the same world

### 3. Block Types & Textures
**What you'll see:**
- 13+ different voxel types, each with unique textures
- Generic textures included (stone, dirt, grass, sand, water, wood, leaves, etc.)
- Real-time texture loading and rendering

**Available block types (press number keys to select):**
1. **Stone** - Gray rock texture
2. **Dirt** - Brown soil texture
3. **Grass** - Green top, dirt sides
4. **Sand** - Yellow/tan beach texture
5. **Water** - Blue translucent texture
6. **Wood** - Brown log texture
7. **Leaves** - Green foliage texture
8. **Glass** - Transparent texture
9. **Ore** - Stone with colored veins

**Try this:**
- Press **1-9** to select different block types
- Place blocks with **Right-click**
- Break blocks with **Left-click**
- Build simple structures to test different textures

### 4. Physics & Collision System
**What you'll see:**
- Gravity-based player movement
- Collision detection with voxel world
- Smooth character controller
- Jump mechanics with proper physics
- Sprint and crouch functionality

**Try this:**
- Jump off a high cliff and experience gravity
- Try to walk through blocks (collision prevents it)
- Sprint with **Shift** to move faster
- Crouch with **Ctrl** to reduce height

### 5. Chunk Streaming & LOD
**What you'll see:**
- Dynamic chunk loading as you move
- Chunks unload when you move away
- Efficient memory management
- Greedy meshing reduces triangle count by ~90%

**Try this:**
- Walk in one direction and watch new chunks load ahead
- Press **F3** to see chunk count
- Observe how chunks far behind you unload automatically
- Note the smooth performance even with many chunks

### 6. World Editor & Terraforming Tools
**What you'll see:**
- Professional ImGui-based editor interface
- Multiple terraforming tools
- Real-time terrain modification
- Undo/Redo system (up to 100 operations)

**Try this:**
1. Press **T** to open the editor
2. Select the **Brush Tool**
3. Click and drag to paint terrain
4. Try other tools: Sphere, Cube, Flatten, Smooth
5. Press **Ctrl+Z** to undo, **Ctrl+Y** to redo

**Editor Tools Available:**
- **Brush Tool** - Paint terrain with selected block type
- **Sphere Tool** - Create spherical structures
- **Cube Tool** - Create rectangular structures
- **Flatten Tool** - Level terrain to uniform height
- **Smooth Tool** - Smooth rough terrain edges
- **Paint Tool** - Change block types without destroying structure

### 7. Input System
**What you'll see:**
- Responsive keyboard and mouse controls
- Multiple input modes (Game Mode vs Editor Mode)
- Customizable key bindings
- Mouse capture for first-person camera

**Try this:**
- **ESC** - Toggle between menu and game
- **T** - Toggle editor interface
- **F3** - Toggle debug overlay
- **Tab** - Show/hide UI panels
- Mouse cursor is captured during gameplay for smooth camera control

### 8. Save & Load System
**What you'll see:**
- Automatic world saving
- Persistent world state
- Compressed chunk data storage
- Fast loading with streaming

**Try this:**
1. Create a new world and make changes (build something)
2. Press **ESC** to exit to main menu (auto-saves)
3. Select **"Load Existing World"**
4. Load your world - all changes are preserved
5. Worlds are saved in the `saves/` directory

## 🎯 Demo Walkthrough (5 Minutes)

Follow this quick walkthrough to see all major features:

### Minute 1: Terrain Exploration
1. Create world with seed **12345**
2. Look around with mouse
3. Walk forward with **W** and explore the landscape
4. Jump with **Space** to navigate terrain

### Minute 2: Block Interaction
1. Look at a block on the ground
2. Break it with **Left-click**
3. Press **1** to select Stone
4. Place a stone block with **Right-click**
5. Try placing several blocks to build something

### Minute 3: Different Block Types
1. Press **2** (Dirt), place some blocks
2. Press **3** (Grass), place some blocks
3. Press **6** (Wood), place some blocks
4. Press **7** (Leaves), place some blocks
5. Observe different textures on each block type

### Minute 4: World Editor
1. Press **T** to open editor
2. Select **Sphere Tool**
3. Choose a block type (e.g., Stone)
4. Click to place a sphere structure
5. Press **Ctrl+Z** to undo
6. Try **Smooth Tool** on rough terrain

### Minute 5: Performance & Features
1. Press **F3** to show debug info
2. Walk around and watch chunk loading
3. Observe FPS counter (should be 60+ FPS)
4. Press **ESC** to exit (auto-saves world)

## 📊 Technical Details

### Graphics Rendering
- **API**: OpenGL 4.5+ (default on Windows)
- **Fallback**: DirectX 11/12 available
- **Shader Language**: GLSL for OpenGL
- **Vertex Buffer**: Optimized with greedy meshing
- **Texture Atlas**: 16x16, 32x32, 64x64 support
- **Draw Calls**: Minimized via batching

### Performance Metrics
- **Target FPS**: 60+ (typically 100-200 on modern hardware)
- **Chunk Size**: 16×256×16 voxels
- **Initial Load**: ~7×7 chunks (49 chunks total)
- **Streaming Distance**: Configurable (default: 3 chunks)
- **Triangle Reduction**: ~90% via greedy meshing

### Asset Pipeline
- **Textures**: PNG format, multiple resolutions
- **Shaders**: GLSL vertex and fragment shaders
- **Audio**: WAV format (OpenAL backend)
- **World Data**: Binary format with compression

### Memory Management
- **Chunk Pooling**: Reuse allocated chunks
- **Automatic Unloading**: Distant chunks freed
- **Texture Streaming**: Load on demand
- **Memory Tracking**: F3 overlay shows usage

## 🔧 Customization

### Changing Graphics API
The engine defaults to OpenGL on Windows. To use DirectX instead:

1. **Edit**: `engine/renderer/GraphicsAPI.h`
2. **Find**: `selectBestGraphicsAPI()` function
3. **Change**: Return `GraphicsAPI::DirectX12` instead of `OpenGL`
4. **Rebuild**: `cmake --build build --config Release`

**Note**: OpenGL is recommended as it has complete rendering implementation.

### Custom Terrain Seeds
Try these interesting seeds:
- **42** - Coastal terrain with beaches
- **1337** - Extreme mountains
- **2048** - Flat creative mode
- **9999** - Balanced exploration
- **100000** - Random terrain features

### Adjusting Render Distance
1. Press **T** to open editor
2. Look for **Render Settings** panel
3. Adjust **Chunk Radius** slider
4. Note: Higher values impact performance

## 🐛 Troubleshooting

### Low FPS / Performance Issues
- **Solution 1**: Reduce render distance in settings
- **Solution 2**: Close other applications
- **Solution 3**: Update graphics drivers
- **Solution 4**: Try DirectX backend instead of OpenGL

### Textures Not Loading
- **Check**: Ensure `textures/` directory exists
- **Check**: Verify texture files in `textures/blocks/`
- **Solution**: Rebuild project to copy textures to build directory

### World Won't Save
- **Check**: Ensure `saves/` directory has write permissions
- **Solution**: Run as administrator (Windows)
- **Check**: Disk space available

### Editor Not Opening
- **Solution**: Press **T** key (not Tab)
- **Check**: Ensure ImGui was built with the engine
- **Rebuild**: `cmake --build build --config Release`

## 📚 Learn More

### Documentation
- **Architecture**: [../ARCHITECTURE.md](../ARCHITECTURE.md)
- **Build Guide**: [../BUILD.md](../BUILD.md)
- **Controls**: [../CONTROLS.md](../CONTROLS.md)
- **Features**: [../FEATURES.md](../FEATURES.md)

### Development
- **C++ Guidelines**: [../docs/CPP_GUIDELINES.md](../docs/CPP_GUIDELINES.md)
- **Contributing**: [../CONTRIBUTING.md](../CONTRIBUTING.md)
- **API Reference**: [../docs/api/README.md](../docs/api/README.md)

### Advanced Topics
- **.NET Integration**: [../DOTNET_INTEGRATION.md](../DOTNET_INTEGRATION.md)
- **Lua Scripting**: [../LUA_INTEGRATION_GUIDE.md](../LUA_INTEGRATION_GUIDE.md)
- **Testing**: [../TESTING.md](../TESTING.md)

## 🎓 Next Steps

After exploring the Showcase Demo:

1. **Read All Demos**: Check [README.md](README.md) for 6 more demos
2. **Modify Source**: Start developing your own features
3. **Create Worlds**: Experiment with different seeds
4. **Build Structures**: Use the editor to create complex builds
5. **Contribute**: Submit your improvements via GitHub

## 🌟 Summary of Capabilities

This demo showcases Fresh Voxel Engine's current capabilities:

✅ **OpenGL Rendering** (default on Windows)  
✅ **Procedural Terrain Generation** with multiple biomes  
✅ **13+ Block Types** with generic textures  
✅ **Physics & Collision** system  
✅ **Chunk Streaming** & dynamic loading  
✅ **World Editor** with terraforming tools  
✅ **Save/Load System** with world persistence  
✅ **Input System** with responsive controls  
✅ **Performance Optimizations** (greedy meshing, culling)  
✅ **Asset Pipeline** (textures, shaders, audio)  

**Version**: 0.2.5  
**Graphics API**: OpenGL 4.5+ (default)  
**Platform**: Windows 10/11, Linux, macOS  
**Build System**: CMake + vcpkg  
**Language**: C++20  

---

**Ready to explore?** Launch FreshVoxelEngine.exe and create world "ShowcaseDemo" with seed 12345!
