# Archived Features and Cross-Platform Support

## Overview

As of this release, Fresh Voxel Engine has transitioned to a **Windows-exclusive gaming platform** focused on providing the best possible game development experience with DirectX rendering and an Unreal Engine-like editor.

## Archived Components

### Documentation Reorganization (November 2025)

On 2025-11-19, the project documentation was comprehensively reorganized:

**Changes Made:**
- 120+ markdown files moved from root to organized `docs/` structure
- 69 historical implementation summaries moved to `docs/history/`
- Created comprehensive guides: `GUI_GUIDE.md`, `GUI_TESTING_GUIDE.md`
- Created new master index: `DOCUMENTATION.md` (replaces `DOCUMENTATION_INDEX.md`)
- Updated all cross-references in README.md

**Old Documentation Index:**
- `DOCUMENTATION_INDEX.md` → archived to `docs/history/DOCUMENTATION_INDEX_OLD.md`

**New Documentation Structure:**
- See [DOCUMENTATION.md](DOCUMENTATION.md) for current documentation navigation
- Historical documents preserved in `docs/history/` for reference

### Cross-Platform Build Scripts

The following build scripts for Linux and macOS have been archived to `archived/cross-platform/`:

- `setup-and-build.sh` - Automated build script for Linux/macOS
- `tools/validate_vcpkg_baseline.sh` - Cross-platform vcpkg validation
- `asset_packs/create_pack.sh` - Asset pack creation script for Unix systems

These scripts are preserved for historical reference but are no longer maintained or tested.

### Platform Support

**Archived Platforms:**
- **Linux** (Ubuntu, Debian, Fedora, RHEL) - Previously supported with OpenGL 4.5+
- **macOS** (macOS 12.0+) - Previously supported with native OpenGL framework

**Current Platform:**
- **Windows 10/11 (x64)** - Exclusive platform with full DirectX 12/11 support

## Rationale for Windows-Only Focus

The decision to focus exclusively on Windows was made for the following reasons:

1. **Development Focus** - Concentrated effort on a single platform allows for:
   - Better DirectX integration and optimization
   - Native Windows features (themes, dialogs, taskbar integration)
   - More polished Unreal Engine-like editor experience
   
2. **Learning and Testing** - The primary developer is learning on Windows and cannot currently test on other platforms

3. **Gaming Market** - Windows is the dominant gaming platform:
   - DirectX 12/11 provides cutting-edge graphics capabilities
   - Visual Studio 2022 offers superior C++ development tools
   - Windows 10/11 SDK enables platform-specific optimizations

4. **Editor Development** - Professional game development tools work best with:
   - Native Windows UI integration
   - DirectX for real-time preview
   - Visual Studio debugging and profiling

## Future Cross-Platform Support

Cross-platform support may be reconsidered in the future if:
- Community contributors want to maintain Linux/macOS ports
- The project reaches sufficient maturity on Windows
- Resources become available for multi-platform testing

## Accessing Archived Files

Archived cross-platform build scripts can be found in:
```
archived/cross-platform/
├── setup-and-build.sh
├── validate_vcpkg_baseline.sh
└── create_pack.sh
```

These files are preserved as-is but are not guaranteed to work with future versions of the engine.

## Migration Path (For Existing Linux/macOS Users)

If you were previously using Fresh on Linux or macOS:

1. **Option 1: Use Windows** - Install Windows 10/11 for the best experience
2. **Option 2: Wine/Proton** - May work but is not officially supported
3. **Option 3: Fork the Project** - Feel free to maintain a cross-platform fork
4. **Option 4: Wait** - Cross-platform support may return in the future

## Questions or Concerns?

If you have questions about the Windows-only transition:
- Open an issue: https://github.com/shifty81/fresh/issues
- Join discussions: https://github.com/shifty81/fresh/discussions

---

## Historical Documents

All implementation summaries, PR summaries, and historical documentation from previous development sessions are preserved in `docs/history/` including:

- GUI implementation summaries (12 files)
- Windows Native UI migration documents (15 files)
- Weekly sprint summaries (4 files)
- Feature implementation notes (20+ files)
- PR summaries (6 files)
- Bug fix reports (5+ files)

To browse historical documents:
```bash
ls docs/history/
```

---

### Voxel-Only Scope Alignment (February 2026)

On 2026-02-16, the project was fully aligned to the **voxel-only engine scope** as defined in [ENGINE.md](ENGINE.md). All systems not relevant to voxel game development were moved to the `archived/` directory.

**Rationale:**
FreshVoxel is a voxel-only engine. A future **Fresh2D** fork will handle 2D game development. All 2D gameplay systems, galaxy generation, dialogue systems, and other non-voxel features have been archived.

#### 2D Systems (`archived/2d-systems/`)

All 2D gameplay systems have been archived — these belong in the future Fresh2D fork:

- **ECS Components:** Transform2D, Sprite2D, Camera2D, Collider2D, RigidBody2D, Animation2D
- **ECS Systems:** Physics2D, Collision2D, Animation2D, Camera2D
- **Generation:** World2DGenerator (platformer/top-down world generation)
- **Tests:** Transform2D, Collider2D, RigidBody2D tests
- **Documentation:** `2D_ENGINE_SYSTEMS.md`

**Source code decoupled:** The `TerrainGenerator` was updated to remove its 2D generator dependency. Only 3D voxel terrain generation is now supported.

#### Galaxy System (`archived/galaxy/`)

The galaxy generation system (space sectors, asteroids, stations, trading) has been archived — it is not relevant to voxel game development:

- `GalaxySector.h/cpp` - Galaxy sector data structures
- `GalaxyGenerator.h/cpp` - Procedural galaxy generation
- `docs/ENHANCED_GALAXY_GENERATION.md` - Galaxy documentation
- `tests/GalaxyGeneratorEnhancedTest.cpp` - Galaxy tests

**Source code decoupled:** The networking `SectorServer` was updated to remove its galaxy dependency.

#### Dialogue System (`archived/dialogue/`)

The dialogue editor and dialogue system from the .NET integration have been archived:

- `dotnet/DialogueEditor/` - Windows Forms dialogue graph editor
- `dotnet/DialogueSystem/` - Runtime dialogue system
- `examples/dialogue/` - Lua dialogue scripts

#### Python Scripting (`archived/python-scripting/`)

Python scripting bindings (stub implementation) have been archived — Lua is the supported scripting language:

- `PythonBindings.h/cpp` - Empty Python binding stubs

#### ImGui Panels (`archived/imgui-panels/`)

Previously archived ImGui-based editor panels (replaced by native Win32 UI).

#### Other Archived Files

- `newdirection.txt` - Planning conversation (moved to `archived/`)
- `upload/` - Issue tracking notes (moved to `archived/upload/`)

### Accessing Archived Files

```
archived/
├── 2d-systems/           # 2D ECS, generation, tests, docs
│   ├── ecs/              # 2D components and systems
│   ├── generation/       # World2DGenerator
│   ├── tests/ecs/        # 2D unit tests
│   └── docs/architecture/# 2D documentation
├── cross-platform/       # Linux/macOS build scripts
├── dialogue/             # Dialogue editor and system
│   ├── dotnet/           # .NET dialogue tools
│   └── examples/         # Lua dialogue examples
├── galaxy/               # Galaxy generation system
│   ├── docs/             # Galaxy documentation
│   └── tests/            # Galaxy tests
├── imgui-panels/         # ImGui editor panels
├── python-scripting/     # Python binding stubs
├── upload/               # Issue tracking notes
└── newdirection.txt      # Planning conversation

```

---

**Last Updated:** 2026-02-16
**Archived Versions:**
- Linux/macOS support prior to Windows-only transition
- Documentation structure prior to 2025-11-19 reorganization
- 2D systems, galaxy, dialogue, Python scripting prior to voxel-only scope alignment
