# FreshVoxel Engine — Rules & Architecture Vision

> This document defines the non-negotiable rules and architecture vision for FreshVoxel.
> All contributors must read and follow these rules.

---

## 🧱 Scope

**FreshVoxel is a voxel-only engine.** It supports voxel game development from Project Zomboid-style isometric/2.5D to Minecraft-style full 3D.

- ✅ 3D voxel terrain with chunks, meshing, and streaming
- ✅ Isometric / 2.5D voxel views
- ✅ Full 3D perspective voxel views
- ✅ Mesh + skeleton characters living inside voxel worlds
- ❌ **No 2D gameplay systems** — A future **Fresh2D** fork will handle 2D games

---

## 🏗️ Architecture Rules

### Build Targets

| Target | Type | Purpose |
|--------|------|---------|
| `FreshEngine` | Static library | Shared voxel engine core (no editor code) |
| `FreshEditor` | Executable | Unreal-like editor (tools, viewports, gizmos) |
| `FreshClient` | Executable | Standalone game client (no editor junk) |
| `FreshServer` | Executable | Headless authoritative server |
| `FreshRuntime` | Executable | Runtime launcher with CLI options |

### Engine Layer Rules

1. **Engine must not include editor headers** — Engine must not know the editor exists
2. **Engine must not include client headers** — Engine provides capabilities, not workflows
3. **World rendering happens only via ViewportContext** — No rendering without explicit render target
4. **Input is never global** — Input routes through viewport-owned InputContext
5. **Cameras are viewport-owned** — Not scene-owned
6. **Play-In-Editor always clones the world** — Editor state is never mutated during play
7. **Editor never mutates runtime state** — Clean separation guaranteed
8. **Headless rendering must be supported** — For CI and dedicated servers

### Rendering Rules

1. Nothing renders unless a ViewportRenderTarget is bound
2. The main window backbuffer is for editor composite only
3. Each viewport has its own render target and swapchain
4. GUI panels never render voxel worlds directly — only ViewportContext does

### Voxel Rules

1. Voxel world update must be renderer-agnostic
2. Chunk mesh generation is separated from rendering
3. World generation is seed-driven and deterministic
4. World generation must not reference editor code

---

## 🎮 Play Modes

### Play-In-Editor (PIE)
- Clone the voxel world
- Clone the camera
- Run the game loop inside a viewport
- Route input to runtime systems
- Stop → destroy runtime state, restore editor state

### Play-In-Client (Standalone)
- Load the same saved world via PlaySnapshot
- Run the same game loop in a separate process
- No editor overhead, accurate performance testing

**Rule:** PIE and Client must execute the **same game code path**. No `if (isEditor)` in engine code.

---

## 🪟 Viewport System

The viewport is the atomic unit of the engine. Everything routes through ViewportContext:

```
OS Events
   ↓
ViewportContext
   ↓
InputContext ───► Systems
   ↓
Camera
   ↓
Scene / VoxelWorld
   ↓
ViewportRenderTarget
   ↓
DX11 / DX12 / Offscreen
```

### Viewport Types
| View | Camera | Use Case |
|------|--------|----------|
| Top-Down 2D | Orthographic | Zomboid-style |
| Isometric | Axonometric | 2.5D |
| Perspective 3D | Perspective | Minecraft-style |

---

## 🧍 Character System

Characters are **mesh + skeleton**, not voxel characters. They live inside voxel worlds.

- Skeleton-based animation with layered locomotion
- Hytale-scope movement: grounded, momentum-based, animation-driven
- Same system for players, NPCs, and monsters
- Voxel collision via capsule vs voxel AABB sweep

---

## 🤖 NPC & AI

- Behavior tree AI for intelligent NPC actions
- Navigation on voxel navgrid
- Schedules across chunks
- Simulation LOD (sleep when far)
- Deterministic updates

---

## 📁 Directory Structure

```
freshvoxel/
├── engine/          # Pure voxel engine (no editor, no UI chrome)
│   ├── core/        # Engine bootstrap, logging, config
│   ├── renderer/    # DirectX 11/12, OpenGL rendering
│   ├── voxel/       # Chunks, world, meshing
│   ├── generation/  # Procedural terrain, biomes
│   ├── physics/     # Voxel collision, character controller
│   ├── ai/          # NPC AI framework
│   ├── character/   # Skeleton, animation
│   └── ...
├── editor/          # Unreal-like editor executable
├── client/          # Standalone game client executable
├── server/          # Headless server executable
├── runtime/         # Runtime launcher executable
├── tests/           # Unit tests
├── shaders/         # HLSL shaders
├── docs/            # Documentation
└── tools/           # Development tools
```

---

## 🔀 Fresh2D Fork Boundary

### Shared (future submodule)
- `engine/core/` — Math, Jobs, IO, Logging

### FreshVoxel Only
- `engine/voxel/` — Chunks, meshing, streaming
- `engine/generation/` — Voxel world generation
- `engine/physics/` — Voxel collision
- Triplanar shaders
- Voxel tools

### Fresh2D Only (future fork)
- Tilemaps
- Sprite animation
- 2D physics

**Hard rule:** Gameplay systems must not leak into engine layers.

---

## 🔧 Build Scripts

All build scripts must log output to `logs/` directory:
- Timestamped log files
- CI must archive `logs/` as artifacts
- No silent scripts

---

*Last updated: 2026-02-17*
