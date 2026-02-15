# Native Win32 Toolbar Integration

## Summary

This document describes the integration of the native Win32 toolbar into the Fresh Voxel Engine window. This is the second step in the incremental migration from ImGui-based UI to Windows native Win32 UI components.

## What Was Changed

### 1. Win32Window Class Updates

**File: `engine/core/Win32Window.h`**
- Added forward declaration of `Win32Toolbar`
- Added `getToolbar()` method to get or create the toolbar instance
- Added `std::unique_ptr<Win32Toolbar> m_toolbar` member variable

**File: `engine/core/Win32Window.cpp`**
- Added `#include "ui/native/Win32Toolbar.h"`
- Implemented `getToolbar()` method that lazily creates the toolbar on first access
- Updated `WM_COMMAND` message handling to route toolbar commands to the toolbar handler

### 2. Engine Class Updates

**File: `engine/core/Engine.h`**
- Added `setupNativeToolbar()` private method declaration (Windows-only)

**File: `engine/core/Engine.cpp`**
- Added call to `setupNativeToolbar()` immediately after menu bar setup
- Implemented `setupNativeToolbar()` method that creates a basic toolbar with:
  - **File Operations**: New, Open, Save buttons
  - **Edit Operations**: Undo, Redo buttons
  - **View Operations**: Camera, Fullscreen buttons
  - Separators between button groups

## How It Works

### Toolbar Creation Flow

1. **Window Initialization**: After menu bar setup, `setupNativeToolbar()` is called
2. **Lazy Creation**: `setupNativeToolbar()` calls `m_window->getToolbar()`, which creates the toolbar
3. **Toolbar Population**: The method adds buttons with callbacks
4. **Auto-Positioning**: The toolbar automatically positions itself below the menu bar
5. **Message Routing**: Toolbar button clicks send `WM_COMMAND` messages
6. **Callback Execution**: `WindowProc` forwards to `Win32Toolbar::handleCommand()`, which executes callbacks

### Toolbar Command Flow

```
User clicks toolbar button
    ↓
Windows sends WM_COMMAND to window
    ↓
Win32Window::WindowProc receives WM_COMMAND
    ↓
Tries Win32MenuBar::handleCommand(commandId) first
    ↓
If not handled, tries Win32Toolbar::handleCommand(commandId)
    ↓
Win32Toolbar looks up callback for commandId
    ↓
Executes registered lambda function
```

## Usage Example

### Basic Toolbar Setup

```cpp
// In Engine initialization (Windows-only)
#ifdef _WIN32
    setupNativeToolbar();
#endif

void Engine::setupNativeToolbar()
{
    auto* toolbar = m_window->getToolbar();
    if (!toolbar) return;

    // Add buttons
    toolbar->addButton(1001, "New", iconHandle, [this]() {
        createNewWorld();
    });
    
    // Add separator
    toolbar->addSeparator();
}
```

### Adding Buttons with Icons

```cpp
// Load icon from resources
HICON icon = LoadIcon(GetModuleHandle(nullptr), MAKEINTRESOURCE(IDI_NEW));

// Add button with icon
toolbar->addButton(1001, "New", icon, [this]() {
    createNewWorld();
});
```

### Dynamic Toolbar Updates

```cpp
// Enable/disable buttons
toolbar->setButtonEnabled(1001, canCreateNew);

// Check/uncheck toggle buttons
toolbar->setButtonChecked(1007, isFullscreen);
```

## Benefits of Native Toolbar

1. **True Native Feel**: Uses Windows native toolbar control with proper styling
2. **Automatic Layout**: Windows handles button positioning and sizing
3. **Tooltips**: Automatically shows button text as tooltip on hover
4. **Keyboard Support**: Tab navigation works automatically
5. **High DPI**: Scales properly with Windows DPI settings
6. **Performance**: Native control, no per-frame redraw needed
7. **Theme Integration**: Respects Windows visual style and theme

## Icon Support

The toolbar supports icons via `HICON` handles. Icons can be loaded from:

1. **Embedded Resources**: Using `LoadIcon()` with resource IDs
2. **Icon Files**: Using `LoadImage()` to load .ico files
3. **Image Lists**: For more advanced scenarios

Example:
```cpp
// Load from resource
HICON icon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SAVE));

// Load from file
HICON icon = (HICON)LoadImage(NULL, L"save.ico", IMAGE_ICON, 16, 16, LR_LOADFROMFILE);

// Use in button
toolbar->addButton(1003, "Save", icon, saveCallback);
```

## Current Implementation

The toolbar is created with text-only buttons (no icons) as a proof of concept. Button IDs start at 5000 to avoid conflicts with menu IDs (which start at 1000).

### Button IDs

- 5001: New
- 5002: Open  
- 5003: Save
- 5004: Undo
- 5005: Redo
- 5006: Camera
- 5007: Fullscreen

## Integration with Menu Bar

The toolbar and menu bar work together seamlessly:

1. Both use the same `WM_COMMAND` message system
2. Menu bar is checked first, then toolbar
3. Same functionality can be exposed in both (e.g., File > New and New button)
4. Callbacks can be shared or separate

## Visual Appearance

On Windows:
- Toolbar appears directly below the menu bar
- Uses Windows standard toolbar styling (flat buttons)
- Buttons show text labels (when no icons provided)
- Separators create visual grouping
- Hover effects show button is clickable
- Pressed state shows when button is clicked

## Future Enhancements

1. **Add Icons**: Create or load 16x16 icons for all buttons
2. **Dropdown Buttons**: Add buttons with dropdown menus (e.g., New -> New 3D World, New 2D World)
3. **Toggle Buttons**: Add checkable buttons for modes (e.g., Edit mode, Play mode)
4. **Status Text**: Show status in the toolbar area
5. **Customization**: Allow users to customize toolbar layout
6. **Icon Themes**: Support light/dark icon sets based on Windows theme

## Testing

To test the native toolbar:

1. Build the project on Windows (requires Visual Studio 2022)
2. Run the executable
3. The window should display a toolbar below the menu bar
4. Click toolbar buttons to see log messages
5. Verify tooltips appear on hover
6. Test keyboard navigation (Tab key)
7. Verify toolbar respects Windows theme

## Related Components

### Current Native UI Components
- ✅ **Win32MenuBar** - Native menu bar (completed)
- ✅ **Win32Toolbar** - Native toolbar (completed)
- ⬜ **Win32Panel** - Base panel class (created, not yet used)
- ⬜ **Win32TreeView** - Tree control (created, not yet used)
- ⬜ **Win32ListView** - List control (created, not yet used)

### Next Steps
Following the incremental migration plan:

1. ✅ Native Menu Bar
2. ✅ Native Toolbar (Current)
3. ⬜ Console Panel (Simple panel migration)
4. ⬜ Settings Panel
5. ⬜ Content Browser (Complex panel)
6. ⬜ Scene Hierarchy (Using Win32TreeView)
7. ⬜ Inspector Panel

## Technical Notes

### Common Controls Library

The toolbar uses the Windows Common Controls library (comctl32.lib), which is automatically linked via pragma comment in Win32Toolbar.cpp:

```cpp
#pragma comment(lib, "comctl32.lib")
```

### Image List

The toolbar creates an image list for icons:
- 16x16 pixel icons
- 32-bit color with alpha channel
- Initial capacity of 10 icons

### Button Structure

Each button uses the `TBBUTTON` structure:
```cpp
TBBUTTON button = {};
button.iBitmap = imageIndex;    // Icon index in image list
button.idCommand = id;          // Command ID
button.fsState = TBSTATE_ENABLED; // Button state
button.fsStyle = BTNS_BUTTON;   // Button style
button.iString = text;          // Button text/tooltip
```

### Auto-Sizing

The toolbar automatically adjusts its size to fit buttons via `TB_AUTOSIZE` message, called after adding buttons or separators.

## Compatibility

- **Windows 10** (v1809+): Fully supported
- **Windows 11**: Fully supported with modern rounded buttons
- **Non-Windows platforms**: Not applicable (guarded by `#ifdef _WIN32`)

## Related Files

- `engine/core/Win32Window.h` - Window class header
- `engine/core/Win32Window.cpp` - Window class implementation
- `engine/core/Engine.h` - Engine class header
- `engine/core/Engine.cpp` - Engine class implementation
- `engine/ui/native/Win32Toolbar.h` - Toolbar class header
- `engine/ui/native/Win32Toolbar.cpp` - Toolbar class implementation
- `WINDOWS_NATIVE_UI_MIGRATION_PLAN.md` - Overall migration plan
- `NATIVE_MENU_BAR_INTEGRATION.md` - Menu bar integration details

---

**Status**: ✅ Complete and ready for testing  
**Version**: 1.0.0  
**Date**: 2025-11-16
