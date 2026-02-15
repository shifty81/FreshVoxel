# Native Win32 Menu Bar Integration

## Summary

This document describes the integration of the native Win32 menu bar into the Fresh Voxel Engine window. This is the first step in the incremental migration from ImGui-based UI to Windows native Win32 UI components.

## What Was Changed

### 1. Win32Window Class Updates

**File: `engine/core/Win32Window.h`**
- Added forward declaration of `Win32MenuBar`
- Added `#include <memory>` for smart pointers
- Added `getMenuBar()` method to get or create the menu bar instance
- Added `std::unique_ptr<Win32MenuBar> m_menuBar` member variable

**File: `engine/core/Win32Window.cpp`**
- Added `#include "ui/native/Win32MenuBar.h"`
- Implemented `getMenuBar()` method that lazily creates the menu bar on first access
- Added `WM_COMMAND` message handling in `WindowProc` to route menu commands to the menu bar

### 2. Engine Class Updates

**File: `engine/core/Engine.h`**
- Added `setupNativeMenuBar()` private method declaration (Windows-only, guarded by `#ifdef _WIN32`)

**File: `engine/core/Engine.cpp`**
- Added call to `setupNativeMenuBar()` immediately after window creation (Windows-only)
- Implemented `setupNativeMenuBar()` method that creates a basic menu structure:
  - **File Menu**: New World, Open World, Save World, Exit
  - **Edit Menu**: Undo, Redo, Settings
  - **View Menu**: Toggle Fullscreen, Reset Camera
  - **Help Menu**: Documentation, About

## How It Works

### Menu Bar Creation Flow

1. **Window Initialization**: When `Engine::initialize()` is called, it creates the Win32Window
2. **Menu Setup**: On Windows, `setupNativeMenuBar()` is called immediately after window creation
3. **Lazy Creation**: `setupNativeMenuBar()` calls `m_window->getMenuBar()`, which creates the menu bar on first access
4. **Menu Population**: The method adds menus and menu items with lambda callbacks
5. **Message Routing**: When user clicks a menu item, Windows sends `WM_COMMAND` message
6. **Callback Execution**: `WindowProc` receives `WM_COMMAND`, forwards to `Win32MenuBar::handleCommand()`, which executes the registered lambda

### Menu Command Flow

```
User clicks menu item
    ↓
Windows sends WM_COMMAND to window
    ↓
Win32Window::WindowProc receives WM_COMMAND
    ↓
Calls Win32MenuBar::handleCommand(commandId)
    ↓
Win32MenuBar looks up callback for commandId
    ↓
Executes registered lambda function
```

## Usage Example

### Basic Menu Bar Setup

```cpp
// In Engine initialization (Windows-only)
#ifdef _WIN32
    setupNativeMenuBar();
#endif

void Engine::setupNativeMenuBar()
{
    auto* menuBar = m_window->getMenuBar();
    if (!menuBar) return;

    // Create a menu
    int fileMenu = menuBar->addMenu("File");
    
    // Add menu items with callbacks
    menuBar->addMenuItem(fileMenu, "Exit", [this]() {
        m_running = false;
    });
}
```

### Adding Submenus

```cpp
int fileMenu = menuBar->addMenu("File");
int recentSubmenu = menuBar->addSubmenu(fileMenu, "Recent Files");
menuBar->addMenuItem(recentSubmenu, "world1.world", [this]() {
    loadWorld("world1");
});
```

### Dynamic Menu Updates

```cpp
// Enable/disable menu items
menuBar->setItemEnabled(saveItemId, hasUnsavedChanges);

// Check/uncheck menu items
menuBar->setItemChecked(fullscreenItemId, isFullscreen);
```

## Benefits of Native Menu Bar

1. **True Native Feel**: Uses Windows native menu rendering and behavior
2. **Keyboard Shortcuts**: Alt+F for File menu, etc., work automatically
3. **Accessibility**: Screen readers and other accessibility tools work properly
4. **Consistency**: Matches other Windows applications
5. **No Runtime Overhead**: Menu bar is created once, not redrawn every frame like ImGui
6. **Integration**: Works seamlessly with Windows DPI scaling, themes, etc.

## Current Limitations

The menu items currently only log their actions. Future work will:

1. Connect File menu items to actual world management dialogs
2. Implement undo/redo functionality
3. Add settings dialog integration
4. Implement fullscreen toggle
5. Add more sophisticated menu structures

## Testing

To test the native menu bar:

1. Build the project on Windows (requires Visual Studio 2022)
2. Run the executable
3. The window should display a native menu bar at the top
4. Click menu items to see log messages in console/log file
5. Test keyboard shortcuts (Alt+F, Alt+E, Alt+V, Alt+H)
6. Verify menu bar respects Windows theme (light/dark mode)

## Next Steps

Following the incremental migration plan:

1. ✅ **Native Menu Bar** (Current - Complete)
2. 🔄 **Native Toolbar** (Next)
3. ⬜ **Console Panel** (Simple panel)
4. ⬜ **Settings Panel**
5. ⬜ **Content Browser** (Complex panel)
6. ⬜ **Scene Hierarchy**
7. ⬜ **Inspector Panel**

## Related Files

- `engine/core/Win32Window.h` - Window class header
- `engine/core/Win32Window.cpp` - Window class implementation
- `engine/core/Engine.h` - Engine class header
- `engine/core/Engine.cpp` - Engine class implementation
- `engine/ui/native/Win32MenuBar.h` - Menu bar class header
- `engine/ui/native/Win32MenuBar.cpp` - Menu bar class implementation
- `WINDOWS_NATIVE_UI_MIGRATION_PLAN.md` - Overall migration plan

## Compatibility

- **Windows 10** (v1809+): Fully supported
- **Windows 11**: Fully supported with rounded corners and modern styling
- **Non-Windows platforms**: Not applicable (guarded by `#ifdef _WIN32`)

## Technical Notes

### Memory Management

The menu bar is managed via `std::unique_ptr` in the Win32Window class, ensuring proper cleanup when the window is destroyed.

### Thread Safety

All menu operations must be performed on the main/UI thread. The current implementation ensures this by only calling menu methods during initialization and message handling.

### Menu IDs

Menu IDs are automatically assigned by Win32MenuBar starting from 1000 and incrementing. This avoids conflicts with system menu IDs.

---

**Status**: ✅ Complete and ready for testing  
**Version**: 1.0.0  
**Date**: 2025-11-16
