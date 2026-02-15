# Fresh Engine UI Architecture

## Overview

The Fresh Engine uses **Windows Native UI (Win32)** exclusively for its user interface. This provides the best Windows integration, native look and feel, and optimal performance.

**ImGui has been removed** - the engine now uses only Windows native GUI components.

## Windows-Native UI System

### Windows-Native Components

The engine includes a complete suite of native Win32 UI components:

**Location**: `engine/ui/native/` and `engine/ui/native/`

**Components**:
- `Win32MenuBar` - Native Windows menu bar (HMENU)
- `Win32Toolbar` - Native toolbar with buttons
- `Win32Panel` - Base panel class for UI containers
- `Win32HUD` - Head-up display overlay
- `Win32TreeView` - Hierarchical tree control
- `Win32ListView` - List/grid view control
- `Win32ConsolePanel` - Console output window
- `Win32ContentBrowserPanel` - Asset browser with native list/tree views
- `Win32SceneHierarchyPanel` - Scene hierarchy with native tree control
- `Win32InspectorPanel` - Property inspector with native controls
- `Win32SettingsDialog` - Settings dialog with native widgets

**Theme System**:
- `UnrealStyleTheme.h` - Unreal Engine-inspired color palette for Win32
- `WindowsThemeManager` - Manages native Windows theming
- `WindowsDialogManager` - Native file dialogs and message boxes
- `WindowsTaskbarManager` - Windows taskbar integration
- `WindowsCustomizationPanel` - Custom theme editor

### Native Win32 Features

```cpp
// Example: Creating a native menu bar
#include "ui/native/Win32MenuBar.h"
#include "ui/native/UnrealStyleTheme.h"

Win32MenuBar menuBar;
menuBar.create(hwnd);

int fileMenu = menuBar.addMenu("File");
menuBar.addMenuItem(fileMenu, "New", []() { /* ... */ });
menuBar.addMenuItem(fileMenu, "Open", []() { /* ... */ });

// Apply Unreal-style colors
HBRUSH bgBrush = CreateSolidBrush(UnrealStyleTheme::WindowBackground);
```

### Build Configuration

Native Win32 UI is enabled by default on Windows:

```cmake
# From CMakeLists.txt
if(WIN32)
    list(APPEND ENGINE_SOURCES
        engine/ui/native/Win32Panel.cpp
        engine/ui/native/Win32MenuBar.cpp
        engine/ui/native/Win32Toolbar.cpp
        engine/ui/native/Win32ConsolePanel.cpp
        engine/ui/native/Win32InspectorPanel.cpp
        engine/ui/native/Win32SceneHierarchyPanel.cpp
        engine/ui/native/Win32ContentBrowserPanel.cpp
        # ... etc
    )
    add_definitions(-DFRESH_WIN32_UI)
endif()
```

**Compilation Flags**:
- `FRESH_WIN32_UI` - Enables native Win32 UI (default on Windows)
- Uses native Windows APIs: `windows.h`, `commctrl.h`, `dwmapi.h`

## Editor UI Architecture

### Current Implementation

```
Fresh Engine Editor (Windows)
│
└─ Native Win32 UI (Primary & Only)
    ├─ Win32Window (HWND)
    ├─ Win32MenuBar (HMENU) ◄─── Tools menu, File menu, etc.
    ├─ Win32Toolbar (Native buttons)
    ├─ Win32HUD (Overlay)
    ├─ Win32ConsolePanel (Native list control)
    ├─ Win32SceneHierarchyPanel (Native tree control)
    ├─ Win32ContentBrowserPanel (Native list/icon view)
    ├─ Win32InspectorPanel (Native property grid)
    ├─ WindowsThemeManager ◄───── UnrealStyleTheme
    └─ Native panels and controls
```

### Why Windows Native UI Only?

1. **Best Windows Integration**:
   - Native look and feel
   - OS-level features (taskbar, themes, jump lists)
   - System dialogs (file open/save)
   - Better performance

2. **Professional Appearance**:
   - Unreal-style theming via UnrealStyleTheme.h
   - Native window chrome
   - System-integrated menus and toolbars

3. **Simplified Codebase**:
   - Single UI system to maintain
   - No cross-platform abstraction needed
   - Direct Windows API usage
   - Smaller binary size (no ImGui dependency)

## Technical Details

### Win32 Native Controls

```cpp
// HWND-based window
Win32Window* window = new Win32Window();
window->create("Fresh Engine Editor", 1920, 1080);

// HMENU-based menu bar
Win32MenuBar* menuBar = new Win32MenuBar();
menuBar->create(window->getHandle());

// Apply Unreal theme
WindowsThemeManager* themeManager = new WindowsThemeManager();
themeManager->initialize();
themeManager->applyToWindows();  // Apply to native controls
```

### Editor Panel Usage

```cpp
// Scene Hierarchy Panel
Win32SceneHierarchyPanel* hierarchyPanel = new Win32SceneHierarchyPanel();
hierarchyPanel->create(hwnd, world);

// Content Browser Panel
Win32ContentBrowserPanel* contentBrowser = new Win32ContentBrowserPanel();
contentBrowser->create(hwnd, "assets/");

// Inspector Panel
Win32InspectorPanel* inspector = new Win32InspectorPanel();
inspector->create(hwnd);
inspector->setSelectedObject(entity);
```

## Conclusion

The Fresh Engine uses **native Windows GUI exclusively**:
- Native Win32 components for all UI (menus, toolbars, panels, dialogs)
- UnrealStyleTheme for professional dark appearance
- No ImGui dependency - pure Windows application
- Optimal Windows integration and performance

All UI is implemented using native Windows controls for the best user experience on Windows 10/11.
