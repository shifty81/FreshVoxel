# Fresh Voxel Engine - Project Roadmap & Status Assessment

**Last Updated**: April 2026  
**Engine Version**: 0.2.7 (Alpha)

This document provides an honest assessment of what is implemented, what is partially working, and what needs to be completed for the engine to be fully functional — particularly around viewport rendering, play mode, and editor UI integration.

---

## Table of Contents

0. [Phase 0: WPF Editor Layer, .vox Import & Cell Shading (NEW — Highest Priority)](#phase-0-wpf-editor-layer-vox-import--cell-shading)
1. [Critical Issues](#critical-issues)
2. [Viewport & Rendering Status](#viewport--rendering-status)
3. [Play Mode Status](#play-mode-status)
4. [Editor UI Wiring Audit](#editor-ui-wiring-audit)
5. [System-by-System Status](#system-by-system-status)
6. [Priority Roadmap](#priority-roadmap)
7. [Architecture Notes](#architecture-notes)

---

## Phase 0: WPF Editor Layer, .vox Import & Cell Shading

> **Priority: HIGHEST — These changes transform the project from a Win32-native prototype into a professional-grade Unreal-style editor with WPF-driven UI, .vox asset import, and Borderlands cell shading.**

### 0-A  Engine → DLL Export Layer

**Status**: ❌ Not done — engine currently builds as an `.exe` only.

The C++ engine must be compiled as `FreshVoxelEngine.dll` so that the WPF host process can load and drive it via P/Invoke.

| Task | Detail |
|------|--------|
| Add `SHARED` CMake target | Add `FreshVoxelEngine_dll` target alongside the existing `.exe`; set `WINDOWS_EXPORT_ALL_SYMBOLS ON` |
| Create `engine/core/EngineAPI.h` | Flat C-linkage (`extern "C"`) exports with `FRESH_API __declspec(dllexport)` |
| Implement `engine/core/EngineAPI.cpp` | Implement all C exports: `Engine_Create/Destroy/Initialize/Run/Shutdown`, `Engine_SetViewportWindow`, `Engine_ResizeViewport`, `Engine_SetEditorMode`, `Engine_Tick`, `Engine_LoadVoxFile`, `Editor_*` command dispatchers |
| Extend `dotnet/EngineInterop.cs` | Add P/Invoke signatures for every new C export |
| Add `Engine_SetTickCallback` | Lets WPF host call `Tick(deltaMs)` from its own render loop instead of `Engine_Run` blocking the thread |

**Design principle**: The WPF process is the host. It owns the window and message loop. It calls `Engine_Tick` every frame from a `CompositionTarget.Rendering` or `DispatcherTimer` callback. The C++ engine renders into a child HWND provided by WPF's `HwndHost`.

---

### 0-B  WPF Editor Application

**Status**: ❌ Not done — `dotnet/` contains only a class library; no WPF project exists.

Create `dotnet/FreshEditor.WPF/` — a full WPF application (`net9.0-windows`, `<UseWPF>true</UseWPF>`).

#### Window Layout  (mirrors Unreal Engine)

```
┌──────────────────────────────────────────────────────────────────┐
│  Menu Bar (File / Edit / View / World / Build / Play / Help)     │
├──────────────────────────────────────────────────────────────────┤
│  Toolbar (New · Open · Save │ Play ▶ · Simulate · Stop │ Modes) │
├──────┬───────────────────────────────────┬───────────────────────┤
│Place │  3-D Viewport (HwndHost→DX11/12)  │  Scene Outliner       │
│Panel │  ┌─────────────────────────────┐  │  (TreeView + search)  │
│(220) │  │  C++ engine renders here    │  ├───────────────────────┤
│      │  │  via setViewportWindow(hwnd)│  │  Details / Inspector  │
│      │  └─────────────────────────────┘  │  (property grid)      │
│      │  Viewport toolbar overlay (cam)   │                       │
├──────┴──────────────────────┬────────────┴───────────────────────┤
│  Content Browser (assets,   │  Output Log / Console              │
│  .vox files, textures, …)   │  (filtering, color-coded)          │
├─────────────────────────────┴───────────────────────────────────┤
│  Status Bar                                                      │
└─────────────────────────────────────────────────────────────────┘
```

#### Key WPF Components to Build

| Component | WPF Type | Notes |
|-----------|----------|-------|
| `MainWindow.xaml` | `Window` | `DockPanel`-based outer shell; hosts all regions |
| `ViewportHost.cs` | `HwndHost` subclass | `BuildWindowCore` creates a Win32 child window; passes HWND to `Engine_SetViewportWindow`; forwards `WM_SIZE` to `Engine_ResizeViewport` |
| `ViewportControl.xaml` | `UserControl` | Wraps `ViewportHost`; shows camera-speed overlay, gizmo mode buttons, view-mode dropdown |
| `SceneOutlinerControl.xaml` | `UserControl` | Binds to `SceneViewModel` (ObservableCollection of entity nodes); supports drag-reorder, search, multi-select |
| `DetailsControl.xaml` | `UserControl` | Reflection-driven property grid; shows component properties for selected entity; editable fields write back via `Engine_SetComponentProperty` P/Invoke |
| `ContentBrowserControl.xaml` | `UserControl` | File-system tree + thumbnail grid; supports drag-drop onto viewport; double-click imports `.vox` files |
| `OutputLogControl.xaml` | `UserControl` | Virtualized `ListView`; subscribes to engine log callback (`Engine_SetLogCallback`) |
| `PlacementPanel.xaml` | `UserControl` | Categorized tabs of placeable actors (Lights, Volumes, Voxel Structures, etc.) |
| `MainMenuBar.xaml` | `Menu` | All existing `Win32MenuBar` commands forwarded to engine via P/Invoke |
| `Toolbar.xaml` | `ToolBar` | Play / Simulate / Stop buttons; build buttons; mode toggles |
| `EditorViewModel.cs` | `INotifyPropertyChanged` | Central data context; exposes play-mode state, selected entities, undo/redo availability |
| `DockingManager` | Use `AvalonDock` NuGet | Provides Unreal-style floating/dockable panel layout with layout save/restore |

#### Interaction Model (mirroring Unreal)

- **Left-click viewport**: select entity → populate Details panel
- **Right-click + WASD**: fly-through camera (relay raw input to engine via P/Invoke)
- **Alt + LMB**: orbit around selection pivot
- **F**: frame/focus selection  
- **W / E / R**: translate / rotate / scale gizmo (hot-key forwarded to engine)
- **Ctrl+Z / Ctrl+Y**: undo/redo via `Engine_Undo` / `Engine_Redo`
- **Drag asset from Content Browser**: spawn entity at viewport cursor hit-point

#### Frame Loop Integration

```csharp
// ViewportControl.xaml.cs
CompositionTarget.Rendering += (_, _) =>
{
    double ms = _stopwatch.Elapsed.TotalMilliseconds - _lastMs;
    _lastMs = _stopwatch.Elapsed.TotalMilliseconds;
    NativeMethods.Engine_Tick((float)ms);   // drives C++ render loop one frame
};
```

---

### 0-C  .vox File Import System (MagicaVoxel Format)

**Status**: ❌ Not implemented anywhere in the project.

MagicaVoxel `.vox` is a chunked binary format. The engine already has a voxel world and chunk system; the importer maps `.vox` data directly onto it.

#### C++ Parser (`engine/assets/vox/`)

| File | Purpose |
|------|---------|
| `VoxFileParser.h/.cpp` | Binary reader for `.vox` chunks: `MAIN`, `SIZE`, `XYZI`, `RGBA`, `nTRN`, `nGRP`, `nSHP` (multi-model support) |
| `VoxImporter.h/.cpp` | Converts parsed data into engine's `VoxelTypeInfo` palette + populates a `VoxelStructureComponent` or seeds a `VoxelWorld` region |
| `VoxExporter.h/.cpp` | Write a world region back to `.vox` for round-trip editing |

**Binary format summary**:
```
4 bytes  magic   "VOX "
4 bytes  version 150
MAIN chunk {
  SIZE chunk  { sx, sy, sz  }          // dimensions
  XYZI chunk  { numVoxels; x,y,z,i[] } // voxel positions + palette index
  RGBA chunk  { 256 × RGBA32 }         // color palette
  nTRN/nGRP/nSHP                       // multi-model scene graph (v150+)
}
```

#### C Export API additions

```c
// Load .vox into a new VoxelStructureComponent at world position
FRESH_API bool Engine_LoadVoxFile(void* engine, const char* path, int worldX, int worldY, int worldZ);

// Return palette color for voxel type index
FRESH_API void Engine_GetVoxPaletteColor(void* engine, int idx, float* r, float* g, float* b);
```

#### WPF Integration

- Content Browser recognizes `.vox` extension; shows voxel thumbnail (render-to-texture preview)
- Drag a `.vox` file onto the viewport → `Engine_LoadVoxFile` places the structure at the ray-cast hit point
- Import dialog lets user choose: **Replace World Origin**, **Spawn as Entity**, or **Append to Selection**

---

### 0-D  Borderlands-Style Cell Shading

**Status**: ❌ Not implemented — shaders are plain diffuse-only (no lighting bands, no outlines).

Both HLSL (`shaders/voxel.hlsl`) and GLSL (`shaders/voxel.frag`) must be upgraded.

#### Core Technique

1. **Quantized/toon lighting** — clamp `NdotL` to discrete steps (4–5 bands) instead of linear diffuse
2. **Rim/specular highlight** — single sharp Phong highlight with hard step at threshold
3. **Outline pass** — inverted-hull geometry pass OR screen-space edge detection from depth+normals
4. **Color style** — high saturation, slightly desaturated darks, bright specular, dark ink outlines (≈ 2–3 px at 1080 p)

#### HLSL Upgrade (`shaders/voxel_cell.hlsl`)

```hlsl
// ---- Toon lighting ----
float NdotL = saturate(dot(normal, lightDir));
float toonLight = 
    NdotL > 0.75 ? 1.0   :   // full lit
    NdotL > 0.40 ? 0.65  :   // mid
    NdotL > 0.15 ? 0.35  :   // shadow
                   0.15  ;   // deep shadow

// ---- Rim light ----
float rim = 1.0 - saturate(dot(viewDir, normal));
float rimBand = step(0.6, rim) * 0.4;

// ---- Final color ----
float3 baseColor   = voxelColor;          // per-block color from palette CB
float3 litColor    = baseColor * (toonLight + rimBand);
float3 inkOutline  = step(edgeFactor, 0.3) * float3(0,0,0); // from edge pass
return float4(litColor + inkOutline, 1.0);
```

#### Outline Geometry Pass (DX11)

Add a second draw call per chunk with back-face culling **inverted** (`D3D11_CULL_FRONT`) and normals extruded along vertex normal by `outlineThickness`. The outline pass uses a flat black shader. This gives the thick-ink Borderlands look even on voxel hard edges.

#### New Shader Constant Buffer additions

```hlsl
cbuffer CellShadingParams : register(b1)
{
    float3 voxelColor;       // per-draw call palette color
    float  outlineThickness; // world-space outline extrusion (e.g. 0.04)
    float3 lightDir;         // directional light in world space
    float  rimThreshold;     // default 0.6
    float4 shadowColor;      // dark-tone color for deep shadow (rgba)
}
```

#### GLSL Upgrade (`shaders/voxel_cell.frag / voxel_outline.frag`)

Mirror the same toon-band logic and add a separate `voxel_outline.vert` that extrudes verts along normals with `gl_Position` nudging.

#### Integration Points

- `ShaderManager` loads `voxel_cell.hlsl` (DX11) and `voxel_cell.frag/.vert` (OpenGL)
- `IRenderContext` gets `setCellShadingEnabled(bool)` and `setCellShadingParams(...)` virtual methods
- New editor panel section **"Rendering Style"** in WPF Details pane with sliders for band count, outline thickness, rim intensity, shadow color

---

### 0-E  Low-Poly Voxel Rendering Best Practices — Missing Items Audit

| Best Practice | Current State | Action Required |
|--------------|--------------|-----------------|
| Greedy meshing | ✅ Implemented (`MeshGenerator`) | No change |
| Face culling (neighbor-aware) | ✅ Implemented | No change |
| Texture atlas | ⚠️ Atlas generator exists but GPU bind missing | Fix `Texture.cpp` TODOs — GPU upload/bind |
| Ambient occlusion | ❌ Missing | Add per-vertex AO in `MeshGenerator`: sample 8 corners per face, encode as vertex color channel |
| Frustum culling | ❌ Not confirmed | Add AABB frustum test before adding chunk to draw list in `Engine::renderVoxelWorld` |
| Level of Detail (LOD) | ❌ Missing | Add LOD0 (full greedy mesh) and LOD1 (merged flat mesh, 1/4 res) for chunks > 128 m from camera |
| Chunk mesh caching | ✅ Dirty-flagging exists | Extend: upload VB/IB to GPU once, only re-upload on dirty chunks |
| Transparent sort | ❌ Missing | Collect transparent (water/glass) chunks, sort back-to-front, render in second pass |
| GPU instancing | ❌ Not used | Use instancing for repeated small structures (voxel ships, prefabs) |
| Block color palette | ❌ Color hardcoded in shader | Move palette to a 256-entry constant buffer; index from voxel type; enables cell shading per-block color |
| .vox scene graph | ❌ Missing | Implement with `nTRN/nGRP/nSHP` chunk parsing (multi-model .vox scenes) |

---

### 0-F  Complete Missing Editor Systems (pre-WPF migration)

The following C++ editor systems must be complete before the WPF layer can fully replace Win32 panels, because WPF calls into them via P/Invoke:

| System | Gap | Fix |
|--------|-----|-----|
| `Engine_SetComponentProperty` C API | Missing | Add generic property setter callable from WPF Inspector |
| `Engine_GetSceneEntities` C API | Missing | Return entity list as JSON string for WPF Outliner binding |
| `Engine_RaycastViewport` C API | Missing | Convert viewport pixel → world ray → hit entity ID; used for WPF click-to-select |
| Selection highlight render | ⚠️ Partial (`SelectionRenderer`) | Wire to cell shading outline pass (use outline thickness = 0 with colored outline for selection) |
| `Engine_SetLogCallback` C API | Missing | Function pointer that WPF Output Log subscribes to instead of reading a file |
| Prefab system | ❌ Missing | Serialize entity + components to JSON; re-instantiate from Content Browser drag |
| Multi-select in viewport | ❌ Missing | Rubber-band selection rectangle (box select) in `SelectionManager` |

---

### Phase 0 Checklist

- [ ] **0-A** Build `FreshVoxelEngine.dll` from CMake; add `EngineAPI.h/.cpp` C exports
- [ ] **0-A** Add `Engine_SetTickCallback`, `Engine_SetViewportWindow`, `Engine_ResizeViewport` exports
- [ ] **0-A** Add `Engine_GetSceneEntities`, `Engine_RaycastViewport`, `Engine_SetComponentProperty`, `Engine_SetLogCallback` exports
- [ ] **0-B** Create `dotnet/FreshEditor.WPF/` WPF application project (net9.0-windows)
- [ ] **0-B** Implement `ViewportHost : HwndHost` — passes HWND to engine DLL
- [ ] **0-B** Build `MainWindow.xaml` with AvalonDock docking layout matching Unreal panel structure
- [ ] **0-B** Implement `SceneOutlinerControl`, `DetailsControl`, `ContentBrowserControl`, `OutputLogControl`, `PlacementPanel`
- [ ] **0-B** Implement `EditorViewModel` with play/stop, undo/redo, entity selection state
- [ ] **0-B** Wire `CompositionTarget.Rendering` → `Engine_Tick` for per-frame drive
- [ ] **0-B** Port all Win32 menu/toolbar commands to WPF menu + keybindings
- [ ] **0-C** Create `engine/assets/vox/VoxFileParser.h/.cpp` (MAIN/SIZE/XYZI/RGBA chunks)
- [ ] **0-C** Create `engine/assets/vox/VoxImporter.h/.cpp` — populate VoxelWorld from parsed data
- [ ] **0-C** Create `engine/assets/vox/VoxExporter.h/.cpp` — round-trip export
- [ ] **0-C** Add `Engine_LoadVoxFile` C export; integrate with Content Browser drag-drop in WPF
- [ ] **0-D** Write `shaders/voxel_cell.hlsl` with toon bands, rim light, inverted-hull outline pass
- [ ] **0-D** Write `shaders/voxel_cell.frag/.vert` and `shaders/voxel_outline.frag/.vert` for OpenGL
- [ ] **0-D** Add `CellShadingParams` constant buffer (b1) and palette color CB (b2)
- [ ] **0-D** Add `setCellShadingEnabled/Params` to `IRenderContext` and both DX11/GL backends
- [ ] **0-D** Add "Rendering Style" section to WPF Details panel with live shader parameter sliders
- [ ] **0-E** Fix `Texture.cpp` GPU upload/bind TODOs (6 items) for texture atlas
- [ ] **0-E** Add per-vertex ambient occlusion in `MeshGenerator`
- [ ] **0-E** Add AABB frustum culling in render loop
- [ ] **0-E** Add LOD0/LOD1 mesh generation and switch in render loop
- [ ] **0-E** Add transparent block second-pass render with back-to-front sort
- [ ] **0-E** Move block color data to per-draw-call constant buffer (palette)
- [ ] **0-F** Implement rubber-band multi-select in `SelectionManager`
- [ ] **0-F** Implement prefab save/load (JSON serialization of entity + components)
- [ ] **0-F** Wire selection highlight to cell shading outline color pass

---

---

## Critical Issues

### 🔴 Viewport Blank During World Generation

**Problem**: When creating a new world, the viewport shows nothing during chunk generation because `m_isGeneratingWorld = true` blocks `renderEditor()` from calling `renderVoxelWorld()`.

**Location**: `engine/core/Engine.cpp` lines 555-584 (`createNewWorld()`)

**Root Cause**: Chunk generation is synchronous and blocks the render loop. During generation (which loads 49 chunks = 7x7 radius), the `m_isGeneratingWorld` flag prevents any 3D rendering, resulting in a blank/frozen viewport.

**Fix Required**:
- [ ] Move chunk generation to a background thread
- [ ] Render already-loaded chunks while generation continues
- [ ] Add a loading progress indicator to the viewport
- [ ] Consider async chunk loading (load nearest chunks first, expand outward)

### 🔴 Swap Chain Initialization Timing

**Problem**: Viewport swap chain may fail to initialize if the viewport panel has zero dimensions when `setViewportWindow()` + `recreateSwapChain()` are called.

**Location**: `engine/core/Engine.cpp` lines 393-464

**Current Mitigation**: The engine retries swap chain creation every frame in `updateEditor()`, but this retry can be fragile.

**Fix Required**:
- [ ] Add explicit "viewport ready" signal from Win32ViewportPanel after first valid resize
- [ ] Guarantee minimum viewport size before attempting swap chain creation
- [ ] Add user-visible error message if swap chain fails after N retries

### 🔴 Scripting System Not Wired

**Problem**: The Lua scripting system (Sol2/LuaJIT) has the framework but the backend is mostly stubs. Scripts cannot actually be loaded or executed in the current state.

**Location**: `engine/scripting/lua/LuaScriptingEngine.cpp`, `engine/scripting/EventSystem.cpp`

**Fix Required**:
- [ ] Wire Sol2 bindings to actual engine APIs (entity creation, world manipulation)
- [ ] Implement script file loading and execution
- [ ] Connect EventSystem to scripting backend
- [ ] Add script reload capability for development iteration

---

## Viewport & Rendering Status

### DirectX 11 Backend ✅ Fully Functional
| Feature | Status | Notes |
|---------|--------|-------|
| Device/swap chain creation | ✅ Working | FLIP_DISCARD mode |
| Viewport window targeting | ✅ Working | `setViewportWindow()` + `recreateSwapChain()` |
| Voxel world rendering | ✅ Working | Per-chunk mesh generation, index buffers |
| View/projection matrices | ✅ Working | Camera integration |
| Shader compilation | ✅ Working | HLSL vertex + pixel shaders |
| Resize handling | ✅ Working | Swap chain recreation on resize |
| Texture rendering | ❌ Not implemented | `Texture.cpp` has 6+ TODOs |
| Material system | ❌ Not implemented | `ModelLoader.cpp` material processing missing |

### DirectX 12 Backend ⚠️ Partially Functional
| Feature | Status | Notes |
|---------|--------|-------|
| Device/command queue | ✅ Working | Pipeline state objects created |
| Swap chain | ✅ Working | FLIP_DISCARD mode |
| Voxel rendering | ⚠️ Partially working | Implementation exists but less tested |
| Shader compilation | ❌ Stub only | `createShader()` returns empty shader object |
| Texture upload | ❌ Not implemented | "Not yet implemented" warning |

### OpenGL Backend ✅ Fully Functional
| Feature | Status | Notes |
|---------|--------|-------|
| Context creation | ✅ Working | OpenGL 4.5+ |
| Voxel rendering | ✅ Working | glClear + render calls |
| Shader compilation | ✅ Working | GLSL shaders |
| Viewport integration | ⚠️ N/A | OpenGL doesn't use Win32 viewport targeting |

### What Needs Work for Viewport
- [ ] **Texture GPU operations** (6 TODOs in `Texture.cpp`): `bindTexture()`, `unbindTexture()`, GPU upload
- [ ] **DX12 shader compilation**: Currently returns empty stubs
- [ ] **Material processing**: `ModelLoader.cpp` doesn't process .mtl files
- [ ] **Async world generation**: To prevent viewport blanking during new world creation
- [ ] **Loading screen**: Show progress indicator during world generation

---

## Play Mode Status

### Current Flow ✅ Implemented
1. `enterPlayMode()` → Creates separate `GamePlayWindow`
2. Redirects renderer to game window via `setViewportWindow()` + `recreateSwapChain()`
3. Hides editor panels, shows game window
4. `exitPlayMode()` → Closes game window, restores editor viewport

### Game Systems Active During Play Mode ✅ Working
| System | Status | Notes |
|--------|--------|-------|
| Player movement (WASD) | ✅ Working | With sprint, crouch, edge prevention |
| Camera (mouse look) | ✅ Working | First-person with sensitivity |
| Gravity & physics | ✅ Working | Terminal velocity, grounding checks |
| Collision detection | ✅ Working | Voxel-based bounding box (8 samples) |
| Block placement/removal | ✅ Working | Mouse click interaction |
| AI system (NPC behaviors) | ✅ Working | Behavior trees update |
| Day/night cycle | ✅ Working | TimeManager updates |
| Stamina system | ✅ Working | Regen/consumption |

### What Needs Work for Play Mode
- [ ] **Inventory system in play mode**: Hotbar displays but item usage may not be fully connected
- [ ] **NPC interaction UI**: Trading/dialogue prompts need play-mode integration
- [ ] **Audio in play mode**: Sound effects for actions (mining, placing, footsteps)
- [ ] **Pause menu**: No in-game pause menu exists
- [ ] **HUD completeness**: Health/stamina bars display but may need polish
- [ ] **Save/load during gameplay**: Auto-save and manual save from play mode

---

## Editor UI Wiring Audit

### Fully Implemented (21/22 methods) ✅
| Method | Status |
|--------|--------|
| `undo()` / `redo()` | ✅ Connected to TerraformingSystem |
| `cut()` / `copy()` / `paste()` | ✅ Connected to SelectionManager |
| `deleteSelection()` | ✅ Connected to SelectionManager |
| `selectAll()` / `deselectAll()` | ✅ Connected to SceneHierarchy |
| `saveWorld()` / `saveWorldAs()` | ✅ WorldSerializer + Windows dialogs |
| `loadWorld()` | ✅ WorldSerializer + Windows dialogs |
| `newWorld()` | ✅ NativeMainMenu create dialog |
| `newProject()` / `openProject()` | ✅ ProjectManager + FileDialogManager |
| `showSettings()` | ✅ Win32SettingsDialog |
| `showEditorSettings()` | ✅ EditorSettingsDialog |
| `showImportAssets()` | ✅ ContentBrowser import dialog |
| `launchDialogueEditor()` | ✅ Launches external .NET app |
| `loadLayout()` / `saveCurrentLayout()` / `resetLayout()` | ✅ LayoutManager |
| `setCameraViewMode()` | ✅ CameraController |
| `focusOnSelection()` / `frameSelection()` | ✅ CameraController + SelectionManager |
| `toggleSceneHierarchy/Inspector/ContentBrowser/Console` | ✅ Panel visibility |
| `enterPlayMode()` / `exitPlayMode()` | ✅ Engine callbacks |

### Not Implemented
| Method | Status | Reason |
|--------|--------|--------|
| `showEngineConfig()` | ❌ Intentionally disabled | No EngineConfigPanel UI exists |

### New Addition
| Method | Status |
|--------|--------|
| `toggleLLMAssistant()` | ✅ New - Toggles AI assistant panel |

---

## System-by-System Status

### Core Engine ✅ 90% Complete
- [x] Game loop with proper timing (hybrid sleep + spin-wait)
- [x] Editor/play mode separation
- [x] Window management (Win32)
- [x] Input handling (keyboard, mouse)
- [x] Logger system with listeners
- [x] Resource manager
- [x] Scene serialization (JSON)
- [ ] Hot-reload for scripts/assets

### Voxel System ✅ 80% Complete
- [x] 16×256×16 chunk system
- [x] Greedy meshing with face culling
- [x] Infinite chunk streaming
- [x] Block type system (40+ types)
- [x] Chunk dirty flagging and regeneration
- [ ] Transparent block rendering (water, glass)
- [ ] Block lighting system
- [ ] Water physics simulation
- [ ] Level-of-detail for distant chunks

### Terrain Generation ✅ 75% Complete
- [x] Perlin/Simplex noise generation
- [x] Heightmap terrain
- [x] Multi-biome support
- [x] Tree/structure placement
- [ ] Cave generation
- [ ] Ore distribution per biome
- [ ] Custom generation plugins
- [ ] Async generation (currently blocks render)

### Editor ✅ 85% Complete
- [x] Unreal-style layout with all major panels
- [x] Terraforming tools (10 tool types)
- [x] Construction hammer (7 blueprints)
- [x] Scene hierarchy with tree view
- [x] Inspector with property editing
- [x] Content browser
- [x] Console with command input
- [x] Transform gizmo (move/rotate/scale)
- [x] Undo/redo (100-step history)
- [x] Layout persistence
- [x] AI Assistant panel (NEW)
- [ ] Multi-select in viewport
- [ ] Prefab system
- [ ] Drag-and-drop asset placement

### ECS (Entity-Component-System) ✅ 85% Complete
- [x] Entity creation/destruction
- [x] Component registration/retrieval
- [x] 2D physics/collision components
- [x] Animation, sprite, camera components
- [x] Health, movement, combat components
- [ ] 3D physics integration with ECS
- [ ] Spatial partitioning optimization

### AI System ⚠️ 60% Complete
- [x] Behavior tree framework (Sequence, Selector nodes)
- [x] Wander and Follow behaviors
- [x] NPC type system (Friendly, Hostile, Trader, etc.)
- [x] AISystem update loop
- [ ] Pathfinding (A* or navigation mesh)
- [ ] Line-of-sight detection
- [ ] Advanced behaviors (flee, investigate, alert others)
- [x] LLM-powered assistant for developers (NEW)

### Scripting ⚠️ 40% Complete
- [x] LuaJIT/Sol2 dependency configured
- [x] ScriptingEngine interface defined
- [x] EventSystem shell
- [ ] **Sol2 bindings to engine APIs** (CRITICAL)
- [ ] Script file loading and execution
- [ ] Script hot-reload
- [ ] Python bindings beyond stubs

### Audio ✅ 95% Complete
- [x] OpenAL integration
- [x] WAV file loading
- [x] Vorbis (OGG) codec support
- [x] 3D positional audio
- [x] Listener management
- [ ] Music streaming
- [ ] Audio effects processing

### Networking ⚠️ 45% Complete
- [x] Socket infrastructure
- [x] Server accept connections
- [x] NetworkMessage binary protocol (10 message types)
- [x] **NetContext** — Packet-based networking with lockstep/rollback, P2P (from AtlasForge)
- [x] **NetHardening** — Connection hardening: timeout, reconnect, bandwidth, heartbeat (from AtlasForge)
- [x] **QoSScheduler** — Priority-based packet scheduling with congestion detection (from AtlasForge)
- [x] **Replication** — Component replication rules (from AtlasForge)
- [x] GameServer.processMessage() — Connect, Disconnect, SectorChange, EntityUpdate, ChatMessage
- [ ] TCP/UDP transport integration with NetContext
- [ ] Player synchronization and client prediction
- [ ] Chunk streaming over network
- [ ] Chat system UI and lobby system

### RPG Systems ✅ 75% Complete
- [x] Inventory system
- [x] Crafting system
- [x] Trading system framework
- [x] Faction component
- [x] Progression component
- [x] Loot system
- [ ] Quest system
- [ ] Dialogue tree execution
- [ ] Experience/leveling

### Physics ✅ 70% Complete
- [x] Custom collision detection
- [x] Gravity and velocity
- [x] Grounding checks
- [x] Player collision (voxel-based)
- [ ] Rigid body dynamics
- [ ] Spatial partitioning (optimization)
- [ ] Projectile physics

---

## Priority Roadmap

### Phase 1: Core Playability (Immediate)
These items are needed for the engine to be usable for basic game testing:

1. **🔴 Fix viewport blank during world generation**
   - Move chunk generation to background thread
   - Render progressively loaded chunks
   
2. **🔴 Ensure swap chain initializes reliably**
   - Add viewport ready signaling
   - Minimum size enforcement
   
3. **🟠 Complete texture GPU operations**
   - Implement `Texture.cpp` GPU upload/bind
   - Enable textured voxel rendering

4. **🟠 Add block lighting system**
   - Ambient occlusion per-vertex
   - Sunlight propagation
   - Torch/light block emission

### Phase 2: Scripting & Interactivity (High Priority)
5. **🔴 Wire Lua scripting backend**
   - Sol2 bindings for entity, world, input APIs
   - Script file loading from project directory
   - Enable `onUpdate()`, `onInit()` lifecycle hooks

6. **🟠 NPC interaction system**
   - Dialogue display in play mode
   - Trading UI
   - Pathfinding (A* on voxel grid)

7. **🟠 Complete play mode experience**
   - Pause menu
   - In-game save/load
   - Sound effects for core actions

### Phase 3: Polish & Tools (Medium Priority)
8. **🟡 Editor improvements**
   - Multi-select in 3D viewport
   - Prefab save/load system
   - Asset drag-and-drop

9. **🟡 DX12 shader compilation**
   - Implement `createShader()` properly
   - HLSL compilation for DX12 pipeline

10. **🟡 Transparent block rendering**
    - Water, glass, leaves transparency
    - Proper depth sorting

### Phase 4: Advanced Features (Future)
11. Multiplayer networking protocol
12. Advanced terrain generation (caves, custom plugins)
13. Quest system implementation
14. Level-of-detail for distant chunks
15. Audio effects and music streaming

---

## Architecture Notes

### Render Pipeline Flow
```
Engine::run()
  └─ render()
       ├─ renderEditor() [editor mode]
       │    ├─ setViewport()
       │    ├─ DX11/DX12/GL: renderVoxelWorld(world, player)
       │    ├─ EditorManager::render() [overlay UI]
       │    └─ endFrame() [present]
       │
       └─ renderGamePlayWindow() [play mode]
            ├─ Separate window swap chain
            ├─ renderVoxelWorld(world, player)
            └─ HUD overlay
```

### Panel Layout (Unreal Engine Style)
```
┌─────────────────────────────────────────────────────┐
│  Menu Bar + Toolbar (80px)                           │
├──────┬────────────────────────────┬──────────────────┤
│ Left │    Center Viewport         │  Right (350px)   │
│ Tool │    (DirectX renders here)  │ ┌──────────────┐ │
│ Panel│                            │ │ Scene Hier.  │ │
│ 220px│                            │ │ (40% height) │ │
│      │                            │ ├──────────────┤ │
│      │                            │ │ Inspector    │ │
│      │                            │ │ (60% height) │ │
├──────┴──────────────┬─────────────┴──────────────────┤
│ Content Browser     │ Console / AI Assistant          │
│ (700px, 220px)      │ (dynamic width, 220px)          │
├─────────────────────┴────────────────────────────────┤
│ Status Bar                                            │
└──────────────────────────────────────────────────────┘
```

### Key Files for Each System
| System | Header | Implementation |
|--------|--------|----------------|
| Engine Loop | `engine/core/Engine.h` | `engine/core/Engine.cpp` |
| Viewport | `engine/ui/native/Win32ViewportPanel.h` | `engine/ui/native/Win32ViewportPanel.cpp` |
| DX11 Renderer | `engine/renderer/backends/DirectX11RenderContext.h` | `engine/renderer/backends/DirectX11RenderContext.cpp` |
| Editor Manager | `engine/editor/EditorManager.h` | `engine/editor/EditorManager.cpp` |
| Player | `engine/gameplay/Player.h` | `engine/gameplay/Player.cpp` |
| Voxel World | `engine/voxel/VoxelWorld.h` | `engine/voxel/VoxelWorld.cpp` |
| AI/LLM | `engine/ai/LLMClient.h` | `engine/ai/LLMClient.cpp` |
| Lua Scripting | `engine/scripting/lua/LuaScriptingEngine.h` | `engine/scripting/lua/LuaScriptingEngine.cpp` |
