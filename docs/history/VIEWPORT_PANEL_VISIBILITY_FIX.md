# Viewport and Panel Visibility Fix

## Problem Summary

The GUI panels (Scene Hierarchy, Inspector, Console, Content Browser, Terraforming Panel, Status Bar) were not visible on startup or after generating a new world. Instead, the 3D viewport was displaying fullscreen, making it impossible to use the editor GUI to make changes.

## Root Cause

The `EditorManager::setVisible(bool)` method was only setting an internal boolean flag (`m_visible`) but not actually calling `ShowWindow()` or the `setVisible()` methods on the Win32 native panel windows. This meant that:

1. On startup, when `Engine::initialize()` called `m_editorManager->setVisible(true)`, the panels remained in their default state (hidden or visible depending on how they were created)
2. When entering play mode, `enterPlayMode()` called `setVisible(false)`, but the panels remained visible because no Win32 API calls were made to hide them
3. When exiting play mode, `exitPlayMode()` called `setVisible(true)`, but again no actual window visibility changes occurred

## Solution

### Changed Files

#### 1. `engine/editor/EditorManager.h`
- Changed `setVisible()` from an inline method to a declared method that will be implemented in the .cpp file

**Before:**
```cpp
void setVisible(bool visible)
{
    m_visible = visible;
}
```

**After:**
```cpp
void setVisible(bool visible);
```

#### 2. `engine/editor/EditorManager.cpp`
- Implemented `EditorManager::setVisible()` to actually show/hide Win32 native panels
- The implementation respects individual panel visibility flags (`m_showSceneHierarchy`, `m_showInspector`, etc.)
- Panels are only shown if BOTH the editor is visible AND the panel's individual flag is true
- Added logging to track visibility changes

**Implementation:**
```cpp
void EditorManager::setVisible(bool visible)
{
    m_visible = visible;
    
#ifdef _WIN32
    // Show or hide all Win32 native panels
    // Respect individual panel visibility flags
    
    if (m_nativeSceneHierarchy) {
        m_nativeSceneHierarchy->setVisible(visible && m_showSceneHierarchy);
    }
    
    if (m_nativeInspector) {
        m_nativeInspector->setVisible(visible && m_showInspector);
    }
    
    if (m_nativeContentBrowser) {
        m_nativeContentBrowser->setVisible(visible && m_showContentBrowser);
    }
    
    if (m_nativeConsole) {
        m_nativeConsole->setVisible(visible && m_showConsole);
    }
    
    if (m_nativeTerraformingPanel) {
        m_nativeTerraformingPanel->setVisible(visible);
    }
    
    if (m_statusBar) {
        m_statusBar->setVisible(visible);
    }
    
    LOG_INFO_C(visible ? "Editor panels shown" : "Editor panels hidden", "EditorManager");
#endif
}
```

## Expected Behavior After Fix

### On Startup (Editor Mode)
1. All GUI panels should be visible around the viewport
2. Scene Hierarchy panel (top-right)
3. Inspector panel (bottom-right)
4. Content Browser panel (bottom-left)
5. Console panel (bottom-center/right)
6. Terraforming Panel (left side) - only visible when a world is loaded
7. Status Bar (bottom)
8. Viewport panel (center) - showing the 3D world

### When Entering Play Mode
1. All GUI panels should hide (Scene Hierarchy, Inspector, Content Browser, Console, Terraforming Panel, Status Bar)
2. Only the viewport should remain visible, now fullscreen
3. HUD and Hotbar should appear (if enabled)

### When Exiting Play Mode
1. All GUI panels should reappear in their proper positions
2. HUD and Hotbar should hide
3. Viewport should return to its panel size (not fullscreen)

### When Generating a New World
1. GUI panels should remain visible throughout the process
2. Terraforming Panel should appear (if it wasn't visible before)
3. The viewport should display the new world within its panel area

## Testing Instructions

1. **Build the project:**
   ```powershell
   cmake --preset windows-release
   cmake --build build/release --config Release
   ```

2. **Run the application:**
   ```powershell
   .\build\release\Release\FreshVoxelEngine.exe
   ```

3. **Test startup visibility:**
   - Verify that all GUI panels are visible on startup
   - Check that the viewport is properly sized in the center
   - Verify you can see Scene Hierarchy, Inspector, Content Browser, and Console panels

4. **Test world generation:**
   - Use File → New World to generate a new world
   - Verify panels remain visible during and after world generation
   - Check that the Terraforming Panel appears after world creation

5. **Test play mode:**
   - Press the Play button in the toolbar (or use menu/console command)
   - Verify all GUI panels hide
   - Verify viewport goes fullscreen
   - Press ESC to exit play mode
   - Verify all GUI panels reappear

6. **Test panel toggles:**
   - Use Window menu to toggle individual panels (Scene Hierarchy, Inspector, etc.)
   - Verify each panel can be hidden and shown individually
   - Enter and exit play mode with some panels hidden
   - Verify the hidden panels remain hidden when returning to editor mode

## Related Files

- `engine/editor/EditorManager.cpp` - Main fix implementation
- `engine/editor/EditorManager.h` - Method declaration change
- `engine/core/Engine.cpp` - Calls `setVisible()` during mode changes
- `engine/ui/native/Win32Panel.cpp` - Base panel class with `setVisible()` implementation

## Notes

- This fix only affects Windows builds (when `_WIN32` is defined)
- The viewport panel remains visible at all times (needed for both editor and play mode)
- Individual panel visibility preferences are preserved across editor/play mode transitions
- The fix uses the existing Win32Panel::setVisible() method which properly calls ShowWindow()
