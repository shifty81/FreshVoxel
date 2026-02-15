# Viewport Z-Order Fix Summary

## Problem Description

After selecting "New 3D World" from the menu, the viewport would fill the entire screen and cover all UI panels (Scene Hierarchy, Inspector, Console, Content Browser, etc.). The panels would briefly appear for a fraction of a second, then disappear, leaving only the viewport visible.

**Screenshot Reference:** 23.PNG shows the viewport filling the entire application window with no visible UI panels.

## Root Cause Analysis

The issue was caused by **incorrect Z-order of child windows**. In Windows Win32 API, when multiple child windows are created within a parent window, the most recently created window appears **on top** of all previously created windows in the Z-order (stacking order).

In the original code:
1. UI panels (Scene Hierarchy, Inspector, Console, etc.) were created first
2. Viewport panel was created **last** (line 605-613 in original code)
3. This placed the viewport at the **top** of the Z-order, covering all other panels
4. When panels were positioned using `SetWindowPos(..., SWP_NOZORDER, ...)`, they maintained their creation order, keeping viewport on top

## Solution Implemented

### 1. Reordered Panel Creation (Lines 536-545)

The viewport panel is now created **FIRST**, before any UI panels:

```cpp
// Create native Viewport Panel FIRST (central 3D view)
// IMPORTANT: Create viewport first so it's at the bottom of the Z-order
m_viewportPanel = std::make_unique<Win32ViewportPanel>();
if (m_viewportPanel->create(hwnd, viewportX, viewportY, viewportWidth, viewportHeight)) {
    LOG_INFO_C("Native Win32 Viewport Panel created...", "EditorManager");
}
```

This ensures the viewport is at the **bottom** of the Z-order, allowing UI panels created afterwards to appear on top.

### 2. Explicit Z-Order Management

Added explicit Z-order control using `SetWindowPos(hwnd, HWND_TOP, ...)` to bring all UI panels to the front in three critical locations:

#### a) After Initial Panel Creation (Lines 627-650)
After all panels are created in `EditorManager::initialize()`, explicitly bring UI panels to front:

```cpp
// Ensure proper Z-order: UI panels should be on top of the viewport
if (m_nativeTerraformingPanel && m_nativeTerraformingPanel->getHandle()) {
    SetWindowPos(m_nativeTerraformingPanel->getHandle(), HWND_TOP, 0, 0, 0, 0, 
                SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
}
// ... repeated for all UI panels
```

#### b) After World Update (Lines 1077-1100)
When `updateWorld()` is called after creating a new world, ensure panels stay visible:

```cpp
// Ensure proper Z-order: UI panels should be on top of the viewport
// Bring all UI panels to the front to ensure they're visible above the viewport
if (m_nativeTerraformingPanel && m_nativeTerraformingPanel->getHandle()) {
    SetWindowPos(m_nativeTerraformingPanel->getHandle(), HWND_TOP, 0, 0, 0, 0, 
                SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
}
// ... repeated for all UI panels
```

#### c) After Window Resize (Lines 1984-2008)
In `onWindowResize()`, maintain proper Z-order after window size changes:

```cpp
// Ensure proper Z-order after resize: UI panels should be on top of the viewport
// This prevents viewport from covering panels after window resize
if (m_nativeTerraformingPanel && m_nativeTerraformingPanel->getHandle()) {
    SetWindowPos(m_nativeTerraformingPanel->getHandle(), HWND_TOP, 0, 0, 0, 0, 
                SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
}
// ... repeated for all UI panels
```

## Technical Details

### SetWindowPos Parameters
- `HWND_TOP`: Places the window at the top of the Z-order (above all non-topmost windows)
- `SWP_NOMOVE`: Retains current position (no X, Y change)
- `SWP_NOSIZE`: Retains current size (no width, height change)
- `SWP_NOACTIVATE`: Does not activate the window (doesn't steal focus)

### Affected Panels
All UI panels are brought to the front:
1. Terraforming Panel (left side)
2. Scene Hierarchy Panel (right top)
3. Inspector Panel (right bottom)
4. Content Browser Panel (bottom left)
5. Console Panel (bottom right)
6. Status Bar (bottom of window)

The viewport remains at the bottom of the Z-order, visible only in the central area between the UI panels.

## Testing Recommendations

To verify the fix works correctly:

1. **Initial Startup**: Launch the application and verify all UI panels are visible
2. **New World Creation**: Select "File > New > New 3D World" and verify:
   - Scene Hierarchy appears and remains visible
   - Terraforming Panel appears (if created after world initialization)
   - All other panels remain visible
   - Viewport is visible only in the central area, not covering panels
3. **Window Resize**: Resize the application window and verify all panels remain visible
4. **Multiple World Operations**: Create multiple worlds, load/save worlds, and verify panels stay visible

## Future Considerations

When adding new UI panels to the editor:
1. Create viewport panel **first** in the initialization sequence
2. Create new UI panels **after** the viewport
3. Add the new panel to the `ensurePanelsOnTop()` helper function (only one location to update!)
4. Ensure panels use appropriate window styles: `WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN`

### Refactoring Benefits
By extracting the Z-order management into `ensurePanelsOnTop()`:
- **Single source of truth**: All Z-order logic in one place
- **Easy maintenance**: Adding a new panel requires updating only the helper function
- **Reduced duplication**: From 18 `SetWindowPos` calls to 6
- **Better readability**: Intent is clear with `ensurePanelsOnTop()` call

## Files Modified

- `engine/editor/EditorManager.cpp`:
  - Line 536-545: Moved viewport creation to be first
  - Line 627: Call to `ensurePanelsOnTop()` after initialization
  - Line 1053: Call to `ensurePanelsOnTop()` in updateWorld()
  - Line 1937: Call to `ensurePanelsOnTop()` in onWindowResize()
  - Line 1940-1968: Implementation of `ensurePanelsOnTop()` helper function
  - Removed duplicate viewport creation code that was after all other panels

- `engine/editor/EditorManager.h`:
  - Line 560-568: Added `ensurePanelsOnTop()` private method declaration with documentation

## Related Issues

This fix addresses the specific issue where:
- Viewport appears to "fill entire screen"
- Scene Hierarchy "briefly appears then disappears"
- UI panels are not visible after creating a new 3D world

The panels were never actually hidden or destroyed - they were simply **behind** the viewport in Z-order, making them invisible to the user.
