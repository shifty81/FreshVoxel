# Week 4: Paste Preview & Undo/Redo Integration Design

**Date**: 2025-11-16  
**Task**: Complete Cut/Copy/Paste functionality with paste preview and undo/redo support  
**Reference**: IMMEDIATE_ACTION_PLAN.md Week 4 tasks

---

## Overview

This document outlines the design for completing Week 4 tasks:
1. **Paste Preview with Ghost Blocks** - Visual feedback before pasting
2. **Undo/Redo Integration** - All clipboard operations should be undoable

---

## 1. Paste Preview with Ghost Blocks

### Problem Statement

Currently, `pasteFromClipboard()` immediately places voxels without any preview. Users need to see where voxels will be placed before committing the paste operation.

### Solution Design

Add a **paste preview mode** that shows "ghost blocks" at the paste location:

#### New Methods in SelectionManager

```cpp
/**
 * @brief Enable paste preview mode
 * @param pastePos Position where paste will occur
 */
void enablePastePreview(const glm::ivec3& pastePos);

/**
 * @brief Update paste preview position
 * @param pastePos New position for preview
 */
void updatePastePreview(const glm::ivec3& pastePos);

/**
 * @brief Disable paste preview mode
 */
void disablePastePreview();

/**
 * @brief Check if paste preview is active
 * @return true if preview is showing
 */
bool isPastePreviewActive() const { return m_pastePreviewActive; }

/**
 * @brief Get paste preview data for rendering
 * @param positions Output vector of preview positions
 * @param types Output vector of preview voxel types
 * @return true if preview is active
 */
bool getPastePreviewData(std::vector<VoxelPosition>& positions,
                         std::vector<VoxelType>& types) const;
```

#### New Member Variables

```cpp
private:
    bool m_pastePreviewActive;
    glm::ivec3 m_pastePreviewPosition;
```

#### Preview Rendering

The `SelectionRenderer` should be enhanced to render paste preview:

```cpp
/**
 * @brief Render paste preview with ghost block effect
 * @param selectionManager Selection manager with preview data
 * @param alpha Transparency for ghost blocks (default 0.5)
 */
void renderPastePreview(SelectionManager* selectionManager, float alpha = 0.5f);
```

Ghost blocks should:
- Use semi-transparent rendering (50% opacity)
- Show the actual voxel colors/textures
- Highlight the bounds with a different color (e.g., green for valid, red for obstructed)

### Workflow

1. User triggers paste (Ctrl+V)
2. Paste preview mode is enabled at cursor/player position
3. User can:
   - Move cursor to adjust position → `updatePastePreview()`
   - Confirm paste (Left Click or Enter) → `pasteFromClipboard()` + `disablePastePreview()`
   - Cancel paste (Esc or Right Click) → `disablePastePreview()`
4. Preview is rendered every frame while active

### Implementation Priority

**High** - This is a core usability feature mentioned in Week 4 requirements.

---

## 2. Undo/Redo Integration

### Problem Statement

Currently, clipboard operations (cut, paste, move, delete) modify the world but don't record undo commands. This means:
- Users can't undo a paste operation
- Users can't undo a cut operation
- This breaks the expected editor workflow

### Solution Design

Integrate `SelectionManager` with `TerraformingSystem`'s command pattern:

#### Approach 1: Direct Integration (Recommended)

Make `SelectionManager` aware of `TerraformingSystem` and record commands:

```cpp
class SelectionManager
{
public:
    /**
     * @brief Set terraforming system for undo/redo integration
     * @param system Pointer to terraforming system
     */
    void setTerraformingSystem(TerraformingSystem* system);

private:
    TerraformingSystem* m_terraformingSystem; // For undo/redo
    
    /**
     * @brief Record a voxel change for undo/redo
     * @param pos Position of voxel
     * @param oldVoxel Previous voxel state
     * @param newVoxel New voxel state
     */
    void recordVoxelChange(const WorldPos& pos, const Voxel& oldVoxel, const Voxel& newVoxel);
};
```

#### Modified Methods

Each method that modifies the world should record commands:

**deleteSelected()**:
```cpp
void SelectionManager::deleteSelected(VoxelWorld* world)
{
    if (world == nullptr || m_selection.isEmpty()) {
        return;
    }
    
    // Record undo commands
    if (m_terraformingSystem) {
        m_terraformingSystem->beginCommandGroup(); // Group all deletions
    }
    
    // Delete all selected voxels
    for (size_t i = 0; i < m_selection.positions.size(); ++i) {
        const auto& pos = m_selection.positions[i];
        Voxel oldVoxel(m_selection.types[i]);
        Voxel newVoxel(VoxelType::Air);
        
        // Record command before changing
        if (m_terraformingSystem) {
            m_terraformingSystem->recordCommand(pos.toWorldPos(), oldVoxel, newVoxel);
        }
        
        world->setVoxel(pos.toWorldPos(), newVoxel);
    }
    
    if (m_terraformingSystem) {
        m_terraformingSystem->endCommandGroup();
    }
    
    Logger::getInstance().info("Deleted " + std::to_string(m_selection.size()) + " selected voxels", "SelectionManager");
    clearSelection();
}
```

**pasteFromClipboard()**:
```cpp
void SelectionManager::pasteFromClipboard(const glm::ivec3& pastePos, VoxelWorld* world)
{
    if (world == nullptr || m_clipboard.isEmpty()) {
        return;
    }
    
    // Calculate offset
    glm::ivec3 offset = pastePos - m_clipboard.boundsMin;
    
    // Record undo commands
    if (m_terraformingSystem) {
        m_terraformingSystem->beginCommandGroup();
    }
    
    // Place all voxels from clipboard
    for (size_t i = 0; i < m_clipboard.positions.size(); ++i) {
        VoxelPosition newPos(
            m_clipboard.positions[i].x + offset.x,
            m_clipboard.positions[i].y + offset.y,
            m_clipboard.positions[i].z + offset.z
        );
        
        // Get old voxel before replacing
        Voxel oldVoxel = world->getVoxel(newPos.toWorldPos());
        Voxel newVoxel(m_clipboard.types[i]);
        
        // Record command
        if (m_terraformingSystem) {
            m_terraformingSystem->recordCommand(newPos.toWorldPos(), oldVoxel, newVoxel);
        }
        
        world->setVoxel(newPos.toWorldPos(), newVoxel);
    }
    
    if (m_terraformingSystem) {
        m_terraformingSystem->endCommandGroup();
    }
    
    Logger::getInstance().info("Pasted " + std::to_string(m_clipboard.size()) + " voxels", "SelectionManager");
}
```

#### Command Grouping

Need to add command grouping to `TerraformingSystem`:

```cpp
class TerraformingSystem
{
public:
    /**
     * @brief Begin a command group (for multi-voxel operations)
     */
    void beginCommandGroup();
    
    /**
     * @brief End command group and push to undo stack
     */
    void endCommandGroup();
    
    /**
     * @brief Record a single command (can be part of a group)
     */
    void recordCommand(const WorldPos& pos, const Voxel& oldVoxel, const Voxel& newVoxel);

private:
    std::vector<TerraformCommand> m_currentGroup;
    bool m_inCommandGroup;
};
```

#### Approach 2: Indirect Integration (Alternative)

Have `EditorManager` handle undo/redo coordination:
- EditorManager owns both SelectionManager and TerraformingSystem
- SelectionManager methods return change lists
- EditorManager records commands before applying changes

**Pros**: Better separation of concerns  
**Cons**: More complex, slower due to double world queries

**Decision**: Use **Approach 1** for better performance and simpler code.

### Implementation Priority

**High** - This is explicitly required in Week 4 tasks.

---

## 3. Integration with EditorManager

Update `EditorManager::initialize()` to connect systems:

```cpp
// After creating SelectionManager
m_selectionManager = std::make_unique<SelectionManager>();
if (worldEditor && worldEditor->getTerraformingSystem()) {
    m_selectionManager->setTerraformingSystem(worldEditor->getTerraformingSystem());
}
```

Update paste callback to use preview:

```cpp
m_menuBar->setPasteCallback([this, world]() {
    if (m_selectionManager && m_selectionManager->hasClipboardData()) {
        // Enable paste preview at player/cursor position
        glm::ivec3 pastePos(0, 64, 0); // TODO: Get from camera/cursor
        m_selectionManager->enablePastePreview(pastePos);
        LOG_INFO_C("Paste preview enabled - Click to place, Esc to cancel", "EditorManager");
    } else {
        LOG_INFO_C("Clipboard is empty", "EditorManager");
    }
});
```

---

## 4. Testing Strategy

### Unit Tests to Add

1. **Paste Preview Tests** (SelectionManagerTests.cpp)
   - `EnablePastePreview_SetsPreviewActive`
   - `UpdatePastePreview_UpdatesPosition`
   - `DisablePastePreview_ClearsPreview`
   - `GetPastePreviewData_ReturnsCorrectPositions`
   - `GetPastePreviewData_WithOffset_CalculatesCorrectly`

2. **Undo/Redo Integration Tests**
   - `DeleteSelected_WithTerraformingSystem_RecordsUndoCommand`
   - `PasteFromClipboard_WithTerraformingSystem_RecordsUndoCommand`
   - `CutToClipboard_WithTerraformingSystem_RecordsUndoCommand`
   - `MoveSelection_WithTerraformingSystem_RecordsUndoCommand`
   - `UndoAfterPaste_RestoresOriginalVoxels`
   - `RedoAfterUndo_ReappliesPaste`

3. **SelectionRenderer Tests**
   - `RenderPastePreview_WithActivePreview_Renders`
   - `RenderPastePreview_WithoutPreview_DoesNothing`
   - `RenderPastePreview_UsesGhostTransparency`

### Integration Tests

1. **Full Paste Workflow**
   - Select voxels → Copy → Enable preview → Move preview → Confirm paste → Undo → Redo
2. **Cut-Paste Workflow**
   - Select voxels → Cut → Preview → Paste → Undo (should restore cut voxels)
3. **Large Selection Paste**
   - Test with 1000+ voxels to ensure performance

---

## 5. Implementation Order

### Phase 1: Undo/Redo Integration (3-4 hours)
1. Add `beginCommandGroup()` / `endCommandGroup()` to TerraformingSystem
2. Add `setTerraformingSystem()` to SelectionManager  
3. Modify `deleteSelected()` to record commands
4. Modify `pasteFromClipboard()` to record commands
5. Modify `cutToClipboard()` to record commands
6. Modify `moveSelection()` to record commands
7. Update EditorManager to connect systems
8. Write and run undo/redo tests

### Phase 2: Paste Preview (3-4 hours)
1. Add preview state variables to SelectionManager
2. Implement `enablePastePreview()` / `updatePastePreview()` / `disablePastePreview()`
3. Implement `getPastePreviewData()`
4. Add `renderPastePreview()` to SelectionRenderer
5. Update EditorManager paste workflow
6. Write and run preview tests

### Phase 3: Polish & Documentation (1-2 hours)
1. Test complete workflows
2. Update documentation
3. Add usage examples
4. Run full test suite
5. Code review cleanup

**Total Estimated Time**: 7-10 hours

---

## 6. Success Criteria

✅ **Paste preview works**:
- User can see ghost blocks before pasting
- Preview updates with cursor movement
- Preview can be confirmed or cancelled

✅ **Undo/redo works**:
- All clipboard operations record undo commands
- Undo restores original state
- Redo reapplies changes
- Command grouping works for multi-voxel operations

✅ **Tests pass**:
- All existing tests still pass (316 tests)
- New tests added and passing (~15 new tests)
- Total: ~331 tests passing

✅ **Documentation complete**:
- Implementation documented
- Usage examples added
- Week 4 marked complete in IMMEDIATE_ACTION_PLAN.md

---

## 7. Future Enhancements (Post-Week 4)

These are **not** required for Week 4 completion:

1. **Collision Detection in Preview** - Show red if paste location is obstructed
2. **Preview Rotation** - Rotate preview before pasting (R key)
3. **Multi-Step Undo** - Undo multiple operations at once (Ctrl+Shift+Z)
4. **Paste History** - Multiple clipboard slots (Ctrl+Shift+V to choose)
5. **Preview Animation** - Pulse or fade ghost blocks for visibility

---

## References

- [IMMEDIATE_ACTION_PLAN.md](/IMMEDIATE_ACTION_PLAN.md) - Week 4 requirements
- [SELECTION_SYSTEM_TESTING_SUMMARY.md](/SELECTION_SYSTEM_TESTING_SUMMARY.md) - Previous work
- [TerraformingSystem.h](/engine/editor/TerraformingSystem.h) - Undo/redo implementation
- [SelectionManager.h](/engine/editor/SelectionManager.h) - Current implementation

---

**Document Status**: ✅ Complete  
**Implementation Status**: 🚧 Ready to implement  
**Priority**: HIGH - Week 4 Sprint 1 deliverable
