# Viewport Rendering Fix for Scene Generation

## Problem Statement

When generating a new scene in the Fresh Voxel Engine, the world was rendering behind the GUI at full screen instead of being constrained to the viewport panel. Users could see the world being generated as full screen behind the GUI panels, indicating that the DirectX swap chain was not properly configured to render to the viewport child window.

Additionally, there was a need to verify that viewport scaling works correctly when the window is resized to match the user's screen resolution.

## Root Cause Analysis

The issue occurred because:

1. **Timing Issue**: During world creation in `createNewWorld()`, the world object and chunks were created before the viewport swap chain was properly configured in `initializeGameSystems()`.

2. **Invalid Dimensions**: The viewport panel dimensions might be invalid (zero or negative) when the world is first created, causing the swap chain recreation to fail silently and fall back to rendering to the main window.

3. **Z-Order Management**: The viewport panel and UI panels could have incorrect Z-ordering, causing the viewport to render on top of UI panels or vice versa.

## Changes Made

### 1. Pre-Creation Viewport Validation (`engine/core/Engine.cpp`)

Added validation BEFORE world creation to ensure the viewport has valid dimensions:

```cpp
// In Engine::createNewWorld()
#ifdef _WIN32
    // CRITICAL FIX: Ensure viewport panel has valid dimensions BEFORE creating world
    if (m_editorManager && m_editorManager->getViewportPanel() && m_renderer) {
        auto* viewportPanel = m_editorManager->getViewportPanel();
        int vpWidth = viewportPanel->getWidth();
        int vpHeight = viewportPanel->getHeight();
        
        // Validate viewport dimensions before proceeding
        if (vpWidth <= 0 || vpHeight <= 0) {
            // Try to fix by triggering a window resize event
            // This recalculates all panel dimensions including viewport
            m_editorManager->onWindowResize(clientWidth, clientHeight);
        }
    }
#endif
```

**Benefits:**
- Detects invalid viewport dimensions before world creation begins
- Automatically attempts to fix by triggering a layout recalculation
- Logs clear error messages if dimensions cannot be fixed
- Prevents silent failure that leads to full-screen rendering

### 2. Enhanced Logging in Swap Chain Recreation (`engine/core/Engine.cpp`)

Added detailed logging during viewport swap chain recreation in `initializeGameSystems()`:

```cpp
LOG_INFO_C("Viewport panel handle: " + std::to_string(reinterpret_cast<uintptr_t>(viewportHwnd)), "Engine");
LOG_INFO_C("Configuring viewport swap chain after world creation - viewport size: " + 
           std::to_string(vpWidth) + "x" + std::to_string(vpHeight), "Engine");
std::cout << "Setting up viewport rendering: " << vpWidth << "x" << vpHeight << std::endl;
// ... more detailed logging during swap chain setup
std::cout << "✓ Viewport swap chain configured: " << vpWidth << "x" << vpHeight << std::endl;
```

**Benefits:**
- Provides clear visibility into viewport setup process
- Helps diagnose issues during world creation
- Shows when viewport swap chain is successfully configured
- Makes it easy to verify the fix is working

### 3. Z-Order Management Fix (`engine/editor/EditorManager.cpp`)

Updated `ensurePanelsOnTop()` to explicitly manage Z-order:

```cpp
void EditorManager::ensurePanelsOnTop()
{
#ifdef _WIN32
    // Ensure proper Z-order: Viewport at bottom, UI panels on top
    // First, explicitly position viewport at the bottom of Z-order
    if (m_viewportPanel && m_viewportPanel->getHandle()) {
        SetWindowPos(m_viewportPanel->getHandle(), HWND_BOTTOM, 0, 0, 0, 0, 
                    SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    }
    
    // Then bring all UI panels to the front
    // ... (existing code for each panel)
#endif
}
```

**Benefits:**
- Viewport is explicitly positioned at HWND_BOTTOM (back of Z-order)
- UI panels are brought to HWND_TOP (front of Z-order)
- Ensures viewport renders behind UI panels as intended
- Prevents UI panels from being obscured by the viewport

### 4. Explicit Z-Order Update After Swap Chain Creation (`engine/core/Engine.cpp`)

Added a call to `ensurePanelsOnTop()` after successful swap chain recreation:

```cpp
// In Engine::initializeGameSystems()
if (m_renderer->recreateSwapChain(vpWidth, vpHeight)) {
    // ... swap chain successfully created
    
    // Ensure viewport panel is visible and at correct Z-order
    if (m_editorManager) {
        m_editorManager->ensurePanelsOnTop();
        LOG_INFO_C("Ensured proper panel Z-order after viewport swap chain creation", "Engine");
    }
}
```

**Benefits:**
- Guarantees correct Z-order immediately after swap chain creation
- Prevents race conditions or timing issues with panel layering
- Ensures UI is always visible after world creation

## Testing Instructions

### Manual Testing Steps

1. **Launch the Application**
   ```
   Release\FreshVoxelEngine.exe
   ```

2. **Create a New Scene**
   - Click `File > New Scene` (or `Ctrl+N`)
   - Choose any scene type (e.g., "3D Voxel")
   - Enter a world name and seed
   - Click "Create"

3. **Verify Viewport Rendering**
   - ✅ World should render ONLY within the center viewport panel
   - ✅ GUI panels (left, right, bottom) should remain visible
   - ✅ World should NOT render full screen behind the GUI
   - ✅ Console should show: "✓ Viewport swap chain configured: [width]x[height]"

4. **Test Window Resizing**
   - Resize the window to different sizes
   - Try maximizing and restoring the window
   - Verify:
     - ✅ Viewport scales correctly with window size
     - ✅ World continues to render only in viewport panel
     - ✅ GUI panels adjust size appropriately
     - ✅ No full-screen bleeding occurs

5. **Test Multiple Scene Creations**
   - Create several scenes in a row
   - Switch between them
   - Verify viewport rendering remains correct each time

### Expected Log Output

During world creation, you should see log messages like:

```
Pre-world-creation viewport check: 800x600
Configuring viewport swap chain after world creation - viewport size: 800x600
Setting up viewport rendering: 800x600
Setting viewport window handle in renderer...
Viewport window handle set successfully. Recreating swap chain...
DirectX swap chain successfully recreated for viewport after world creation
✓ Viewport swap chain configured: 800x600
Ensured proper panel Z-order after viewport swap chain creation
```

### Error Scenarios to Test

1. **Invalid Viewport Dimensions**
   - If you see: "CRITICAL: Invalid viewport dimensions before world creation"
   - The fix should automatically trigger a resize
   - Verify the next log shows valid dimensions

2. **Swap Chain Recreation Failure**
   - If you see: "ERROR: Failed to create viewport swap chain"
   - This indicates a deeper issue (e.g., DirectX driver problem)
   - Check that viewport panel handle is valid in logs

## Technical Details

### Viewport Swap Chain Flow

1. **Initial Setup** (during Engine::initialize()):
   - EditorManager creates viewport panel with initial dimensions
   - Engine sets viewport window handle in renderer
   - Swap chain is created for viewport child window

2. **World Creation** (during Engine::createNewWorld()):
   - **NEW**: Validate viewport dimensions before world creation
   - **NEW**: Attempt to fix if dimensions are invalid
   - Create VoxelWorld and generate chunks
   - Call initializeGameSystems()

3. **Game Systems Initialization** (during Engine::initializeGameSystems()):
   - **ENHANCED**: Log viewport handle and dimensions
   - Recreate swap chain for viewport (ensures correct target)
   - **NEW**: Call ensurePanelsOnTop() after successful recreation
   - Set editor visible and refresh panels

4. **Window Resize** (during Engine::update()):
   - Detect window resize
   - Update panel layouts via EditorManager::onWindowResize()
   - Recreate swap chain if viewport dimensions changed
   - Update camera aspect ratio

### DirectX Swap Chain Behavior

- **viewportHwnd** in DirectX11RenderContext is initially `nullptr`
- When `nullptr`, swap chain uses main window (full screen)
- When set to viewport child window HWND, swap chain uses viewport
- Swap chain must be recreated whenever:
  - Viewport window handle changes
  - Viewport dimensions change
  - World is created/loaded

### Z-Order in Win32 Child Windows

- Child windows are layered (Z-order)
- HWND_BOTTOM = back of Z-order (rendered first, behind others)
- HWND_TOP = front of Z-order (rendered last, on top of others)
- Viewport should be at HWND_BOTTOM
- UI panels should be at HWND_TOP

## Verification Checklist

- [ ] Application builds successfully
- [ ] Application launches without errors
- [ ] Creating a new scene shows world in viewport only (not full screen)
- [ ] GUI panels remain visible during and after world creation
- [ ] Console shows "✓ Viewport swap chain configured" message
- [ ] Window resize correctly scales viewport and panels
- [ ] Maximizing and restoring window works correctly
- [ ] Multiple scene creations work consistently
- [ ] No errors or warnings in console during normal operation

## Known Limitations

1. **Windows-Only**: This fix is specific to Win32 windowing and DirectX rendering
2. **DirectX 11/12**: Fix applies to DirectX render contexts only
3. **Real-Time Generation**: During chunk generation, the render loop continues to run

## Future Improvements

1. **Async World Generation**: Generate chunks in background thread to prevent UI blocking
2. **Progress Dialog**: Show progress dialog during world generation instead of rendering
3. **Viewport Visibility**: Optionally hide viewport during generation to prevent rendering attempts
4. **Dimension Caching**: Cache valid viewport dimensions to use as fallback

## Related Files Modified

- `engine/core/Engine.cpp` - Pre-creation validation and enhanced logging
- `engine/editor/EditorManager.cpp` - Z-order management fix

## Related Memory Notes

This fix builds on several previous fixes documented in the repository memories:

- **viewport swap chain validation**: "Always validate viewport dimensions (>0) before creating DirectX swap chain"
- **swap chain recreation order**: "DirectX viewport swap chain must be recreated BEFORE calling EditorManager::setVisible(true)"
- **panel refresh after resize**: "Call refreshAllPanels() after onWindowResize"
- **Win32 child windows visibility**: "Main window must use WS_CLIPCHILDREN style"
