# Frequently Asked Questions (FAQ)

## General Questions

### What is Fresh Voxel Engine?

Fresh Voxel Engine is a modern, open-source voxel game engine built with C++20 supporting multiple graphics APIs (OpenGL, DirectX 11/12). It features procedural terrain generation, a powerful world editor, and supports creating voxel-based games and applications.

### What can I build with it?

- Minecraft-like games
- Voxel-based sandbox games
- Terrain visualization tools
- Procedural world generators
- Educational projects
- Game prototypes

### Is it free?

Yes! Fresh Voxel Engine is released under the MIT License, which means:
- ✅ Free to use
- ✅ Free to modify
- ✅ Free for commercial projects
- ✅ Open source

### What platforms are supported?

Fresh Voxel Engine is designed specifically for Windows:
- **Windows 10** (64-bit) or newer
- DirectX 12 support recommended
- DirectX 11 or OpenGL fallback available

### What are the system requirements?

**Minimum:**
- OpenGL 4.5+ capable GPU (or DirectX 11+ on Windows)
- 4-core CPU
- 8 GB RAM
- 2 GB storage

**Recommended:**
- Modern GPU (RTX 2060, RX 5700, or better)
- 8-core CPU
- 16 GB RAM
- 10 GB storage

## Getting Started

### How do I install it?

See [GETTING_STARTED.md](GETTING_STARTED.md) for detailed instructions:

```bash
git clone https://github.com/shifty81/fresh.git
cd fresh
mkdir build && cd build
cmake ..
make
./FreshVoxelEngine
```

### I'm getting build errors. What should I do?

**For Visual Studio 2022 users**: See [QUICKSTART_VISUAL.md](QUICKSTART_VISUAL.md) for step-by-step setup with troubleshooting.

**Common errors and solutions**:

1. **"'OpenGLRenderContext': undeclared identifier"**
   - Solution: Make sure GLEW is installed: `vcpkg install glew:x64-windows`

2. **"'std::make_unique': no matching overloaded function"**
   - Solution: Fixed in latest version. Update your code or add `#include <memory>`

3. **"Windows 10/11 SDK Required"**
   - Solution: Fresh Voxel Engine requires Windows 10 SDK (version 10.0.19041.0 or later) for native UI
   - Install via Visual Studio Installer → Individual Components → Windows 10 SDK (10.0.19041.0) or newer
   - Windows 11 SDK is also supported
   - See [VISUAL_STUDIO_SETUP.md](VISUAL_STUDIO_SETUP.md#common-compilation-errors)

4. **"Unable to start program ... ALL_BUILD access is denied"**
   - Solution: Set `FreshVoxelEngine` as startup project (right-click in Solution Explorer)
   - Detailed guide: [FIXING_ALL_BUILD_ERROR.md](FIXING_ALL_BUILD_ERROR.md)

**General troubleshooting**:
1. Check [DEVELOPER_SETUP.md](DEVELOPER_SETUP.md) for prerequisites
2. Check [VISUAL_STUDIO_SETUP.md](VISUAL_STUDIO_SETUP.md) for Visual Studio-specific issues
3. See [Troubleshooting](#troubleshooting) section below
4. Search existing [GitHub Issues](https://github.com/shifty81/fresh/issues)
5. Create a new issue if not found

### How do I create my first world?

Follow [Tutorial 1: Your First Voxel World](docs/tutorials/tutorial_01_first_world.md) for a step-by-step guide.

Basic steps:
1. Initialize engine and window
2. Create VoxelWorld
3. Generate terrain with TerrainGenerator
4. Add player and camera
5. Run main loop

### Where can I find examples?

Check the [examples/](examples/) directory for:
- Basic setup
- Terrain generation
- Player controller
- World editing
- Asset loading

## Features

### Does it support multiplayer?

Multiplayer is planned for v0.4.0 (Q1 2026). The architecture supports it, but implementation is not yet complete.

### Can I create custom voxel types?

Yes! Edit `engine/voxel/VoxelTypes.h` to add new voxel types. The engine supports up to 256 types.

### How does terrain generation work?

The engine uses multi-octave Perlin noise for realistic terrain:
- Height maps for surface
- 3D noise for caves
- Biome-based block selection
- Seed-based reproducibility

See [TerrainGenerator](docs/tutorials/tutorial_01_first_world.md#step-4-generate-initial-chunks) for details.

### What's the world size limit?

Technically unlimited! The engine uses chunk streaming:
- Chunks are 16×256×16 voxels
- Loaded dynamically around player
- Only visible chunks in memory
- Limited by available storage

### Can I import custom models?

Yes, through the modular asset system:
- Supports OBJ, FBX, GLTF
- Create asset packs with JSON manifests
- Auto-discovery on engine start
- See [ModularAssetSystem](docs/api/README.md)

### Does it have physics?

Yes, the physics system includes:
- Gravity
- Collision detection (AABB, sphere, ray)
- Voxel-world collision
- Spatial partitioning
- More physics features coming in future updates

## Development

### How can I contribute?

See [CONTRIBUTING.md](CONTRIBUTING.md) for detailed guidelines:

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Submit a pull request

We welcome:
- Code contributions
- Documentation improvements
- Bug reports
- Feature suggestions
- Asset creation

### What coding standards do you follow?

See [CPP_GUIDELINES.md](CPP_GUIDELINES.md) for complete guidelines:
- C++20 standard
- 4-space indentation
- PascalCase for classes
- camelCase for functions
- RAII for resource management
- Modern C++ practices

### How do I run tests?

Test infrastructure is under development. When available:

```bash
cd build
ctest --output-on-failure
```

### Can I use this in a commercial project?

Yes! The MIT License allows commercial use. You can:
- Sell games built with it
- Modify and sell the engine
- Use it in proprietary projects
- No royalties required

Just keep the license notice in your project.

### How do I add a new feature?

1. Check [ROADMAP.md](ROADMAP.md) to see if it's planned
2. Open an issue to discuss the feature
3. Fork and create a feature branch
4. Implement the feature
5. Add tests and documentation
6. Submit a pull request

## Technical Questions

### Which graphics APIs are supported?

Fresh Voxel Engine supports multiple graphics APIs:
- **OpenGL 4.5+** - Cross-platform support
- **DirectX 11** - Windows (excellent compatibility)
- **DirectX 12** - Windows 10+ (modern, high performance)

The engine automatically selects the best API for your platform.

### How is performance?

On modern hardware:
- 60+ FPS at 1080p
- Hundreds of chunks loaded
- Smooth chunk streaming
- Optimized mesh generation

Performance optimizations:
- Greedy meshing (90% face reduction)
- Frustum culling
- Chunk streaming
- Spatial partitioning

### What about memory usage?

Typical memory usage:
- ~200-500 KB per chunk (with mesh)
- ~200 chunks loaded = ~100 MB
- Texture atlas: ~64 MB
- Total: ~200-500 MB typical

Memory management features:
- Object pooling
- Smart pointers
- Custom allocators
- Chunk streaming limits

### How does chunk streaming work?

1. Track player position
2. Calculate required chunks (circular pattern)
3. Priority queue by distance
4. Background thread generates chunks
5. Upload to GPU when ready
6. Unload distant chunks

See [ChunkStreamer](engine/voxel/ChunkStreamer.h) for implementation.

### Can I use a different renderer?

The architecture is designed to be modular with a unified `IRenderContext` interface. The engine currently supports:
- OpenGL 4.5+
- DirectX 11
- DirectX 12

You can extend support by implementing the `IRenderContext` interface for other graphics APIs.

## Troubleshooting

### Visual Studio Build & Runtime Errors

For comprehensive Visual Studio troubleshooting, see:
- **[QUICKSTART_VISUAL.md](QUICKSTART_VISUAL.md)** - Complete visual setup and troubleshooting guide
- **[FIXING_ALL_BUILD_ERROR.md](FIXING_ALL_BUILD_ERROR.md)** - Fix "ALL_BUILD access denied" error
- **[VISUAL_STUDIO_SETUP.md](VISUAL_STUDIO_SETUP.md)** - Detailed VS2022 setup and troubleshooting

**Quick fixes for common VS errors**:
- **ALL_BUILD access denied**: Right-click `FreshVoxelEngine` → Set as Startup Project
- **Windows SDK missing**: Install Windows 10/11 SDK via Visual Studio Installer
- **std::make_unique error**: Update to latest code (fixed) or add `#include <memory>`

**Note**: ImGui has been removed from Fresh Voxel Engine. The engine uses Windows Native Win32 UI exclusively. See [IMGUI_DEPRECATION_NOTICE.md](../history/IMGUI_DEPRECATION_NOTICE.md).

### Black screen / nothing renders

**Common causes:**
- Shaders not found
- Graphics API initialization errors
- Camera position issues

**Solutions:**
1. Check console for errors
2. Ensure shaders are in `build/shaders/`
3. Check that your GPU supports OpenGL 4.5+ (or DirectX 11+ on Windows)
4. Check camera position (should be above ground)

### "Failed to initialize renderer"

**Solutions:**
1. Update GPU drivers
2. Verify GPU supports OpenGL 4.5+ (or DirectX 11+ on Windows)
3. Check console output for specific error messages

### Low FPS

**Solutions:**
1. Build in Release mode:
   ```bash
   cmake .. -DCMAKE_BUILD_TYPE=Release
   ```
2. Disable validation layers
3. Reduce chunk count
4. Check GPU usage in task manager

### Memory leaks

**Solutions:**
1. Build with debug symbols
2. Run with Valgrind:
   ```bash
   valgrind --leak-check=full ./FreshVoxelEngine
   ```
3. Check for missing smart pointer usage
4. Verify proper cleanup in destructors

### Build errors with C++20

**Solution:**
```bash
cmake .. -DCMAKE_CXX_STANDARD=20 -DCMAKE_CXX_STANDARD_REQUIRED=ON
```

Ensure compiler supports C++20:
- GCC 10+
- Clang 10+
- MSVC 2019 16.11+

## Community

### Where can I get help?

- 📖 [Documentation](docs/)
- 💬 [GitHub Discussions](https://github.com/shifty81/fresh/discussions)
- 🐛 [GitHub Issues](https://github.com/shifty81/fresh/issues)
- 📧 Email: support@freshvoxelengine.org
- 💬 Discord: Coming soon!

### How do I report a bug?

1. Check existing issues
2. Use the bug report template
3. Include:
   - Description
   - Steps to reproduce
   - Environment (OS, GPU, etc.)
   - Error messages/logs
   - Screenshots if applicable

See [Bug Report Template](.github/ISSUE_TEMPLATE/bug_report.md)

### Can I request features?

Yes! Use the feature request template:
1. Describe the feature
2. Explain use cases
3. Suggest implementation ideas
4. Discuss benefits

See [Feature Request Template](.github/ISSUE_TEMPLATE/feature_request.md)

### Is there a Discord/forum?

Discord server is planned for future. Currently use:
- GitHub Discussions for questions
- GitHub Issues for bugs/features

### How do I stay updated?

- ⭐ Star the repository
- 👁️ Watch for releases
- 📰 Check [CHANGELOG.md](CHANGELOG.md)
- 📊 View [PROJECT_STATUS.md](PROJECT_STATUS.md)

## License & Legal

### Can I modify the engine?

Yes! The MIT License allows you to:
- Modify the source code
- Create derivative works
- Distribute modified versions

Just keep the original license notice.

### Do I need to credit the engine?

Not required, but appreciated! You can:
- Mention in credits
- Link to the repository
- Use the logo (when available)

### Can I sell games made with it?

Absolutely! No royalties or fees required.

### What about assets I create?

Assets you create are yours:
- Your ownership
- Your license choice
- Your distribution rights

## Future Plans

### When is version 1.0?

Target: Mid 2026

See [ROADMAP.md](ROADMAP.md) for detailed timeline.

### What features are coming next?

**v0.3.0 (Dec 2025):**
- Inventory system
- Resource gathering
- Crafting
- Day/night cycle

**v0.4.0 (Q1 2026):**
- Multiplayer networking
- Chat system
- Lobby system

See full roadmap in [ROADMAP.md](ROADMAP.md).

### Will you support VR?

VR support is being considered for post-1.0 release.

### Mobile support?

Not currently planned. The engine targets desktop platforms with OpenGL 4.5+ or DirectX 11+ support.

## Additional Resources

- 📖 [Getting Started](GETTING_STARTED.md)
- 🏗️ [Architecture](ARCHITECTURE.md)
- 📚 [Tutorials](docs/tutorials/)
- 🔧 [Developer Setup](DEVELOPER_SETUP.md)
- 🗺️ [Roadmap](ROADMAP.md)
- 📝 [Contributing](CONTRIBUTING.md)

---

**Didn't find your answer?**

- Ask in [GitHub Discussions](https://github.com/shifty81/fresh/discussions)
- Create an [Issue](https://github.com/shifty81/fresh/issues)
- Email: support@freshvoxelengine.org

We're here to help! 🚀
