# Voxel Building Tools Guide

This guide covers the new Enshrouded-inspired voxel building and terraforming tools added to the Fresh Voxel Engine.

## Overview

The engine now includes three specialized tools for voxel interaction:

1. **Construction Hammer** - Primary building tool with blueprints
2. **Pickaxe** - Mining and removal tool
3. **Rake** - Terrain leveling and smoothing tool

These tools provide intuitive and flexible building capabilities inspired by Enshrouded's voxel-based construction system.

## Tool System Architecture

### Base Classes

All tools inherit from the `VoxelTool` base class, which provides:
- Common tool interface
- Size/radius configuration
- World reference management
- Basic tool operations

### Tool Manager

The `ToolManager` class provides:
- Tool selection and switching
- Unified tool interface
- Tool cycling (keyboard shortcuts)
- Access to specific tools

## Tools Reference

### 1. Construction Hammer

**Purpose**: Primary building tool for constructing structures with pre-made blueprints or single-block placement.

**Features**:
- 7 blueprint types for quick construction
- Single-block placement for detailed work
- Seamless material blending
- No structural integrity constraints (creative freedom)

**Blueprint Types**:

| Blueprint | Description | Size |
|-----------|-------------|------|
| `SingleBlock` | Place individual voxels | 1×1×1 |
| `Wall` | Vertical wall | 3×5×1 (width×height×depth) |
| `Foundation` | Horizontal foundation | 3×1×3 |
| `Column` | Vertical column | 1×8×1 |
| `Floor` | Flat floor platform | 5×1×5 |
| `Stairs` | Simple staircase | 5 blocks diagonal |
| `Arch` | Simple arch structure | 5×5×1 |

**Usage Example**:
```cpp
// Create and initialize construction hammer
ConstructionHammer hammer;
hammer.initialize(world);

// Set blueprint type
hammer.setBlueprint(ConstructionHammer::Blueprint::Wall);

// Place wall at position
WorldPos pos(10, 64, 10);
hammer.useTool(pos, VoxelType::Stone);
```

**Key Methods**:
- `setBlueprint(Blueprint)` - Select blueprint type
- `getBlueprint()` - Get current blueprint
- `placeBlueprint(pos, voxelType)` - Place blueprint at position
- `useTool(pos, voxelType)` - Use hammer with current blueprint

### 2. Pickaxe

**Purpose**: Mining and removal tool for breaking blocks and terraforming.

**Features**:
- Single voxel removal
- Area removal (sphere-based)
- Creates rougher, rustic edges
- Protects bedrock from removal
- Configurable removal radius

**Usage Example**:
```cpp
// Create and initialize pickaxe
Pickaxe pickaxe;
pickaxe.initialize(world);

// Remove single voxel
WorldPos pos(10, 64, 10);
pickaxe.setSize(1);
pickaxe.useTool(pos, VoxelType::Air);

// Remove area (3 block radius)
pickaxe.setSize(3);
pickaxe.useTool(pos, VoxelType::Air);
```

**Key Methods**:
- `removeVoxel(pos)` - Remove single voxel
- `removeArea(pos, radius)` - Remove sphere of voxels
- `setSize(size)` - Set removal radius (1-10)
- `useTool(pos, voxelType)` - Remove voxel(s) at position

**Notes**:
- Will not remove bedrock blocks
- Will not remove air blocks (returns false)
- Area removal uses spherical selection

### 3. Rake

**Purpose**: Terrain leveling tool for creating flat surfaces and smooth terrain transitions.

**Features**:
- Flatten terrain to specific height
- Smooth terrain transitions
- Level ground at current height
- Create foundations for building
- Configurable area radius

**Usage Example**:
```cpp
// Create and initialize rake
Rake rake;
rake.initialize(world);

// Level ground at current position
WorldPos pos(10, 64, 10);
rake.setSize(5);  // 5 block radius
rake.useTool(pos, VoxelType::Grass);

// Flatten to specific height
rake.flattenToHeight(pos, 60, 5);

// Smooth terrain
rake.smoothTerrain(pos, 5);
```

**Key Methods**:
- `flattenToHeight(pos, targetHeight, radius)` - Flatten to specific height
- `smoothTerrain(pos, radius)` - Smooth by averaging heights
- `levelGround(pos)` - Level to current height
- `setSize(size)` - Set operation radius (1-10)
- `useTool(pos, voxelType)` - Level ground at position

**Notes**:
- Fills below target height with dirt
- Top layer becomes grass (or specified material)
- Removes blocks above target height
- Protects bedrock from removal

## Tool Manager Usage

### Basic Setup

```cpp
#include "interaction/ToolManager.h"

// Initialize tool manager
ToolManager toolManager;
toolManager.initialize(world);

// Tool manager starts with Construction Hammer selected
```

### Tool Selection

```cpp
// Select specific tool
toolManager.selectTool(VoxelToolType::ConstructionHammer);
toolManager.selectTool(VoxelToolType::Pickaxe);
toolManager.selectTool(VoxelToolType::Rake);

// Cycle through tools
toolManager.cycleNextTool();      // Hammer -> Pickaxe -> Rake -> Hammer
toolManager.cyclePreviousTool();  // Hammer -> Rake -> Pickaxe -> Hammer
```

### Using Tools

```cpp
// Use active tool
WorldPos pos(10, 64, 10);
toolManager.useActiveTool(pos, VoxelType::Stone);

// Get specific tools for advanced configuration
auto hammer = toolManager.getConstructionHammer();
if (hammer) {
    hammer->setBlueprint(ConstructionHammer::Blueprint::Foundation);
}

auto pickaxe = toolManager.getPickaxe();
if (pickaxe) {
    pickaxe->setSize(3);
}

auto rake = toolManager.getRake();
if (rake) {
    rake->setSize(5);
}
```

## Integration with Editor

The tools can be integrated into the editor UI:

```cpp
// In your editor class
class WorldEditor {
private:
    ToolManager m_toolManager;
    
public:
    void initialize(VoxelWorld* world) {
        m_toolManager.initialize(world);
    }
    
    void handleToolSelection(int toolIndex) {
        switch (toolIndex) {
            case 0: m_toolManager.selectTool(VoxelToolType::ConstructionHammer); break;
            case 1: m_toolManager.selectTool(VoxelToolType::Pickaxe); break;
            case 2: m_toolManager.selectTool(VoxelToolType::Rake); break;
        }
    }
    
    void handleMouseClick(const WorldPos& pos, VoxelType material) {
        m_toolManager.useActiveTool(pos, material);
    }
};
```

## Keyboard Shortcuts (Suggested)

Add these keyboard shortcuts to your input handler:

| Key | Action |
|-----|--------|
| `1` | Select Construction Hammer |
| `2` | Select Pickaxe |
| `3` | Select Rake |
| `Tab` | Cycle to next tool |
| `B` | Cycle blueprint (Construction Hammer) |
| `[` / `]` | Decrease/increase tool size |

## Building Examples

### Simple House Foundation

```cpp
ToolManager tools;
tools.initialize(world);

// Select rake and level ground
tools.selectTool(VoxelToolType::Rake);
auto rake = tools.getRake();
rake->setSize(8);
rake->levelGround(WorldPos(0, 60, 0));

// Select hammer and place foundation
tools.selectTool(VoxelToolType::ConstructionHammer);
auto hammer = tools.getConstructionHammer();
hammer->setBlueprint(ConstructionHammer::Blueprint::Foundation);

// Place multiple foundation pieces
for (int x = -5; x <= 5; x += 3) {
    for (int z = -5; z <= 5; z += 3) {
        hammer->useTool(WorldPos(x, 60, z), VoxelType::Cobblestone);
    }
}

// Add walls
hammer->setBlueprint(ConstructionHammer::Blueprint::Wall);
hammer->useTool(WorldPos(-5, 61, -5), VoxelType::Wood);
hammer->useTool(WorldPos(5, 61, -5), VoxelType::Wood);
```

### Mining Operation

```cpp
ToolManager tools;
tools.initialize(world);

// Select pickaxe with area removal
tools.selectTool(VoxelToolType::Pickaxe);
auto pickaxe = tools.getPickaxe();
pickaxe->setSize(3);

// Create tunnel
for (int z = 0; z < 20; z += 3) {
    pickaxe->useTool(WorldPos(10, 65, z), VoxelType::Air);
}
```

### Terrain Sculpting

```cpp
ToolManager tools;
tools.initialize(world);

// Select rake for smoothing
tools.selectTool(VoxelToolType::Rake);
auto rake = tools.getRake();
rake->setSize(6);

// Smooth rough terrain
for (int x = 0; x < 30; x += 6) {
    for (int z = 0; z < 30; z += 6) {
        rake->smoothTerrain(WorldPos(x, 60, z), 6);
    }
}
```

## Performance Considerations

### Tool Size Impact

- Larger tool sizes affect more voxels per operation
- Area operations (Pickaxe, Rake) scale with radius³
- Consider limiting max tool size to 10 blocks
- Use smaller sizes for detailed work

### Mesh Regeneration

- Each voxel change triggers chunk mesh regeneration
- Group operations when possible
- Consider implementing operation batching for large builds

### Memory Usage

- Tools maintain minimal state (world reference + config)
- ToolManager holds one instance of each tool
- No significant memory overhead

## Advanced Features

### Custom Blueprints

To add custom blueprints, extend the `ConstructionHammer` class:

```cpp
class CustomHammer : public ConstructionHammer {
public:
    enum class CustomBlueprint {
        Tower,
        Bridge,
        Dome
    };
    
    void placeCustomBlueprint(const WorldPos& pos, 
                             VoxelType voxelType,
                             CustomBlueprint blueprint) {
        // Implement custom structure placement
    }
};
```

### Tool Persistence

Save/load tool settings with world data:

```cpp
struct ToolSettings {
    VoxelToolType activeType;
    int hammerBlueprintIndex;
    int pickaxeSize;
    int rakeSize;
};

void saveToolSettings(const ToolManager& manager);
void loadToolSettings(ToolManager& manager);
```

## Troubleshooting

### Tool Not Working

1. Verify world pointer is valid: `tools.initialize(world)`
2. Check tool is selected: `tools.getActiveToolType()`
3. Ensure position is within loaded chunks
4. Verify material type is valid for construction

### Unexpected Behavior

1. **Bedrock removal**: Pickaxe intentionally protects bedrock
2. **Tool size limits**: Size is clamped to 1-10 range
3. **Blueprint orientation**: Current implementation uses fixed orientation
4. **Chunk boundaries**: Operations may span multiple chunks

## Future Enhancements

Potential future additions:
- Directional blueprint placement
- Rotatable structures
- Copy/paste regions
- Undo/redo for tool operations (integrate with TerraformingSystem)
- Material blending and transitions
- Advanced mesh generation (marching cubes)
- LOD system for distant voxels
- Custom blueprint designer UI

## See Also

- [Editor Integration Guide](../editor/EDITOR_INTEGRATION.md)
- [Terraforming System](TERRAFORMING.md)
- [VoxelWorld API Reference](../../docs/api/VoxelWorld.md)
- [Input System](INPUT_SYSTEM.md)

## API Reference

For complete API documentation, see:
- `engine/interaction/VoxelTool.h`
- `engine/interaction/ToolManager.h`
- Unit tests: `tests/interaction/VoxelToolTests.cpp`
