# Voxel Selection System

## Overview

The Voxel Selection System provides comprehensive tools for selecting, manipulating, and managing groups of voxels in the Fresh Voxel Engine editor. This system is a critical foundation for Cut/Copy/Paste operations and advanced editing workflows.

## Components

### 1. SelectionManager

**Location:** `engine/editor/SelectionManager.h`, `engine/editor/SelectionManager.cpp`

The SelectionManager handles all voxel selection operations, including:
- Box selection with mouse drag
- Selection storage and queries
- Clipboard operations (Cut/Copy/Paste)
- Selection manipulation (move, delete)

#### Key Features

**Selection Operations:**
```cpp
// Start a new box selection
void startSelection(const glm::vec3& worldPos);

// Update selection as mouse drags
void updateSelection(const glm::vec3& worldPos);

// Finalize selection and populate voxel list
void finalizeSelection(VoxelWorld* world);

// Clear current selection
void clearSelection();
```

**Query Methods:**
```cpp
// Check if there's an active selection
bool hasSelection() const;

// Get selection size
size_t getSelectionSize() const;

// Get selected voxels
const std::vector<VoxelPosition>& getSelectedVoxels() const;

// Get selection bounds for rendering
bool getSelectionBounds(glm::ivec3& min, glm::ivec3& max) const;
```

**Manipulation:**
```cpp
// Delete all selected voxels
void deleteSelected(VoxelWorld* world);

// Move selection by delta
void moveSelection(const glm::ivec3& delta, VoxelWorld* world);
```

**Clipboard Operations:**
```cpp
// Copy selection to clipboard
void copyToClipboard(VoxelWorld* world);

// Cut selection (copy and delete)
void cutToClipboard(VoxelWorld* world);

// Check if clipboard has data
bool hasClipboardData() const;

// Paste from clipboard at position
void pasteFromClipboard(const glm::ivec3& pastePos, VoxelWorld* world);
```

### 2. SelectionRenderer

**Location:** `engine/editor/SelectionRenderer.h`, `engine/editor/SelectionRenderer.cpp`

The SelectionRenderer provides visual feedback for selections using wireframe rendering.

#### Features

- **Bounding Box:** Draws a wireframe box around the entire selection
- **Individual Voxels:** For small selections (< 100 voxels), draws individual voxel outlines
- **Customizable Color:** Light blue with transparency by default
- **Performance:** Automatically adjusts detail based on selection size

```cpp
// Initialize with debug renderer
bool initialize(devtools::DebugRenderer* debugRenderer);

// Render selection visualization
void render(const SelectionManager* selectionManager);

// Customize appearance
void setSelectionColor(const glm::vec4& color);
void setEnabled(bool enabled);
```

### 3. Data Structures

#### VoxelPosition
```cpp
struct VoxelPosition {
    int x, y, z;
    
    VoxelPosition(int x, int y, int z);
    WorldPos toWorldPos() const;
};
```

#### VoxelSelection
```cpp
struct VoxelSelection {
    std::vector<VoxelPosition> positions;  // Selected voxel positions
    std::vector<VoxelType> types;          // Voxel types at each position
    glm::ivec3 boundsMin;                  // Selection bounding box min
    glm::ivec3 boundsMax;                  // Selection bounding box max
    
    void clear();
    bool isEmpty() const;
    size_t size() const;
};
```

## Integration with EditorManager

The SelectionManager is fully integrated with the EditorManager and connected to the menu system:

### Keyboard Shortcuts

| Shortcut | Action | Callback |
|----------|--------|----------|
| Ctrl+C | Copy | `m_menuBar->setCopyCallback()` |
| Ctrl+X | Cut | `m_menuBar->setCutCallback()` |
| Ctrl+V | Paste | `m_menuBar->setPasteCallback()` |
| Delete | Delete Selection | `m_menuBar->setDeleteCallback()` |
| Esc | Clear Selection | (To be implemented) |

### Access from Code

```cpp
// Get selection manager from editor manager
EditorManager* editorManager = ...;
SelectionManager* selectionManager = editorManager->getSelectionManager();

// Get selection renderer
SelectionRenderer* selectionRenderer = editorManager->getSelectionRenderer();
```

## Usage Examples

### Example 1: Manual Selection

```cpp
// Start selection at a world position
glm::vec3 startPos(10.0f, 64.0f, 10.0f);
selectionManager->startSelection(startPos);

// Update as user drags (in game loop)
glm::vec3 currentPos(15.0f, 68.0f, 15.0f);
selectionManager->updateSelection(currentPos);

// Finalize when user releases mouse
selectionManager->finalizeSelection(world);

// Check what was selected
size_t count = selectionManager->getSelectionSize();
std::cout << "Selected " << count << " voxels" << std::endl;
```

### Example 2: Copy and Paste

```cpp
// Copy current selection
if (selectionManager->hasSelection()) {
    selectionManager->copyToClipboard(world);
}

// Paste at a new location
if (selectionManager->hasClipboardData()) {
    glm::ivec3 pastePos(20, 64, 20);
    selectionManager->pasteFromClipboard(pastePos, world);
}
```

### Example 3: Move Selection

```cpp
// Move selection up by 5 blocks
if (selectionManager->hasSelection()) {
    glm::ivec3 delta(0, 5, 0);
    selectionManager->moveSelection(delta, world);
}
```

### Example 4: Delete Selection

```cpp
// Delete all selected voxels
if (selectionManager->hasSelection()) {
    selectionManager->deleteSelected(world);
}
```

## Implementation Details

### Selection Algorithm

The box selection algorithm:
1. User clicks and drags to define a 3D bounding box
2. On finalization, the system iterates through all voxels in the box
3. Only solid voxels (not air) are added to the selection
4. Voxel types are stored for clipboard operations

### Clipboard Storage

The clipboard stores:
- **Positions:** Relative positions of each voxel
- **Types:** Voxel type for each position
- **Bounds:** Original bounding box for reference

When pasting, positions are recalculated relative to the paste location.

### Performance Considerations

- **Large Selections:** Only the bounding box is rendered (no individual voxels)
- **Small Selections:** Individual voxel outlines provide precise feedback
- **Memory:** Selection data is stored efficiently using vectors
- **Rendering:** Uses existing DebugRenderer for minimal overhead

## Future Enhancements

### Planned Features (Week 1-2)

1. **Mouse Input Integration**
   - Detect drag operations
   - Start/update/finalize selection automatically
   - Visual feedback during dragging

2. **Keyboard Shortcuts**
   - Implement key bindings in InputManager
   - Connect to selection operations
   - Add undo/redo support

3. **Selection Modes**
   - Box select (current)
   - Brush select (paint selection)
   - Wand select (select similar/connected voxels)

4. **Enhanced Paste**
   - Use cursor/camera position for paste location
   - Ghost block preview before confirming
   - Collision detection option

### Future Enhancements (Week 3+)

1. **Selection Filters**
   - Select by voxel type
   - Select connected regions
   - Invert selection

2. **Advanced Operations**
   - Rotate selection
   - Scale selection
   - Mirror/flip selection

3. **Selection Persistence**
   - Save selections to file
   - Load named selections
   - Selection library

4. **Multi-Selection**
   - Add/subtract from selection
   - Combine selections
   - Selection history

## Testing

### Manual Testing Checklist

- [ ] Start box selection with mouse
- [ ] Update selection while dragging
- [ ] Finalize selection
- [ ] Verify correct voxels selected
- [ ] Copy selection to clipboard
- [ ] Paste selection at new location
- [ ] Cut selection (copy + delete)
- [ ] Move selection with arrow keys
- [ ] Delete selection
- [ ] Clear selection
- [ ] Visualize selection with wireframe
- [ ] Test with small selection (< 100 voxels)
- [ ] Test with large selection (> 1000 voxels)

### Edge Cases

- Empty selection
- Single voxel selection
- Selection across chunk boundaries
- Very large selections (performance)
- Selection with only air blocks
- Paste outside world bounds

## Troubleshooting

### Selection not visible
- Check if SelectionRenderer is enabled: `selectionRenderer->setEnabled(true)`
- Verify DebugRenderer is enabled: `debugRenderer->setEnabled(true)`
- Ensure EditorManager::render() is being called

### Paste not working
- Check if clipboard has data: `selectionManager->hasClipboardData()`
- Verify paste position is valid
- Check VoxelWorld is not null

### Performance issues with large selections
- Large selections only render bounding box
- Consider adding selection size limit
- Optimize buildBoxSelection() for very large volumes

## API Reference

### SelectionManager Methods

| Method | Description | Parameters | Returns |
|--------|-------------|------------|---------|
| `startSelection` | Begin box selection | `worldPos` | void |
| `updateSelection` | Update selection bounds | `worldPos` | void |
| `finalizeSelection` | Complete selection | `world` | void |
| `clearSelection` | Clear current selection | - | void |
| `hasSelection` | Check if selection exists | - | bool |
| `getSelectionSize` | Get voxel count | - | size_t |
| `getSelectedVoxels` | Get voxel list | - | const vector& |
| `getSelectionBounds` | Get min/max bounds | `min, max` (out) | bool |
| `deleteSelected` | Delete selected voxels | `world` | void |
| `moveSelection` | Move by delta | `delta, world` | void |
| `copyToClipboard` | Copy to clipboard | `world` | void |
| `cutToClipboard` | Cut to clipboard | `world` | void |
| `hasClipboardData` | Check clipboard | - | bool |
| `pasteFromClipboard` | Paste at position | `pastePos, world` | void |

### SelectionRenderer Methods

| Method | Description | Parameters | Returns |
|--------|-------------|------------|---------|
| `initialize` | Setup renderer | `debugRenderer` | bool |
| `render` | Render selection | `selectionManager` | void |
| `setSelectionColor` | Set color | `color` | void |
| `getSelectionColor` | Get color | - | const vec4& |
| `setEnabled` | Enable/disable | `enabled` | void |
| `isEnabled` | Check enabled | - | bool |

## Contributing

When modifying the selection system:
1. Maintain backward compatibility
2. Add tests for new features
3. Update this documentation
4. Consider performance impact
5. Follow the existing code style

## See Also

- [IMMEDIATE_ACTION_PLAN.md](IMMEDIATE_ACTION_PLAN.md) - Development roadmap
- [COMPREHENSIVE_ROADMAP.md](COMPREHENSIVE_ROADMAP.md) - Long-term planning
- Editor integration: `engine/editor/EditorManager.h`
- Debug rendering: `engine/devtools/DebugRenderer.h`
- Voxel world: `engine/voxel/VoxelWorld.h`
