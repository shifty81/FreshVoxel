# Fresh Voxel Engine - GUI System Guide

**Comprehensive guide to the Fresh Editor Windows Native GUI implementation**

## Table of Contents

1. [Overview](#overview)
2. [Architecture](#architecture)
3. [Windows Native GUI](#windows-native-gui)
4. [Editor Panels](#editor-panels)
5. [Dark Theme System](#dark-theme-system)
6. [Transform Gizmo](#transform-gizmo)
7. [Toolbar and Menu Bar](#toolbar-and-menu-bar)
8. [Quick Start](#quick-start)
9. [Current Status](#current-status)
10. [Keyboard Shortcuts](#keyboard-shortcuts)

---

## Overview

The Fresh Voxel Engine features a professional Unreal Engine-like editor with **Windows Native GUI**:
- **Windows Native UI**: All UI uses native Win32 controls
- **Native Window Management**: Win32 window management, menu bars, and toolbars
- **Native Editor Panels**: Tree views, list views, property grids using Win32 controls
- **DirectX Rendering**: Full integration with DirectX 11/12 graphics pipeline
- **Dark Theme**: Professional Unreal-style dark theme

### Key Features

- ✅ Native Windows window management
- ✅ Native menu bar with File, Edit, View, Tools, Help menus
- ✅ Native toolbar with icon buttons
- ✅ Native Win32 panels: Scene Hierarchy, Inspector, Content Browser, Console
- ✅ Transform Gizmo for object manipulation (Move/Rotate/Scale)
- ✅ Selection system for voxel and entity selection
- ✅ Unreal-style dark theme
- ✅ High DPI support
- ✅ Keyboard shortcuts (W/E/R for gizmo modes)

---

## Architecture

### Windows Native UI System

The Fresh Editor uses **native Windows controls exclusively**:

```
┌─────────────────────────────────────────┐
│     Native Windows Window (Win32)       │
│  ┌────────────┐  ┌──────────────────┐  │
│  │Menu Bar    │  │ Toolbar          │  │
│  │(HMENU)     │  │ (Native)         │  │
│  └────────────┘  └──────────────────┘  │
│  ┌─────────────────────────────────┐   │
│  │  Main Viewport (DirectX)        │   │
│  │  ┌─────────┐  ┌──────────────┐  │   │
│  │  │Hierarchy│  │ Inspector    │  │   │
│  │  │TreeView │  │ PropGrid     │  │   │
│  │  └─────────┘  └──────────────┘  │   │
│  │  ┌──────────────────────────┐   │   │
│  │  │ Content Browser ListView │   │   │
│  │  └──────────────────────────┘   │   │
│  └─────────────────────────────────┘   │
└─────────────────────────────────────────┘
```

### Why Native Win32 Only?

1. **Native Windows Controls**: Best OS integration, familiar UX, native performance
2. **Professional Appearance**: Unreal-style theme applied to native controls
3. **DirectX Integration**: Seamless rendering with DirectX 11/12
4. **Smaller Binary**: No third-party UI library dependency

---

## Windows Native GUI

### Win32Window Class

The `Win32Window` class provides native window management:

**Features:**
- Native window creation and management
- DirectX 11/12 swap chain integration
- High DPI awareness (per-monitor DPI v2)
- Native message handling
- Window resizing, maximizing, minimizing
- Full-screen support

**Key Methods:**
```cpp
class Win32Window {
    bool initialize(bool useOpenGL = false);
    void pollEvents();
    void swapBuffers();
    bool shouldClose() const;
    HWND getHandle() const;
    void setTitle(const std::string& title);
};
```

### Win32InputManager Class

Native Windows input handling:

**Features:**
- Keyboard input (WM_KEYDOWN, WM_KEYUP)
- Mouse input (WM_MOUSEMOVE, WM_LBUTTONDOWN, etc.)
- Mouse wheel (WM_MOUSEWHEEL)
- Raw input support for high-precision mouse

**Key Methods:**
```cpp
class Win32InputManager {
    bool isKeyPressed(int keyCode) const;
    bool isMouseButtonPressed(int button) const;
    glm::vec2 getMousePosition() const;
    glm::vec2 getMouseDelta() const;
};
```

### Build Configuration

Native Win32 UI is enabled by default on Windows:

```cmake
if(WIN32)
    list(APPEND ENGINE_SOURCES
        engine/ui/native/Win32Panel.cpp
        engine/ui/native/Win32MenuBar.cpp
        engine/ui/native/Win32Toolbar.cpp
        engine/ui/native/Win32ConsolePanel.cpp
        engine/ui/native/Win32InspectorPanel.cpp
        engine/ui/native/Win32SceneHierarchyPanel.cpp
        engine/ui/native/Win32ContentBrowserPanel.cpp
    )
    add_definitions(-DFRESH_WIN32_UI)
endif()
```

---

## Editor Panels

### Win32SceneHierarchyPanel

**File**: `engine/ui/native/Win32SceneHierarchyPanel.h`

Displays world entities in a native tree control:

```cpp
class Win32SceneHierarchyPanel {
    bool create(HWND parent, VoxelWorld* world);
    void refresh();
    void selectEntity(Entity* entity);
    Entity* getSelectedEntity() const;
};
```

**Features:**
- Native tree view control (SysTreeView32)
- Drag-and-drop support
- Right-click context menu
- Search/filter
- Multi-selection

### Win32InspectorPanel

**File**: `engine/ui/native/Win32InspectorPanel.h`

Property editor for selected entities:

```cpp
class Win32InspectorPanel {
    bool create(HWND parent);
    void setEntity(Entity* entity);
    void setVoxel(Voxel* voxel);
};
```

**Features:**
- Native property grid
- Transform properties (Position, Rotation, Scale)
- Component properties
- Material properties
- Real-time editing

### Win32ContentBrowserPanel

**File**: `engine/ui/native/Win32ContentBrowserPanel.h`

Asset browser with native list view:

```cpp
class Win32ContentBrowserPanel {
    bool create(HWND parent, const std::string& assetPath);
    void refresh();
    Asset* getSelectedAsset() const;
};
```

**Features:**
- Native list view control (icon/list/details views)
- Folder navigation
- Asset thumbnails
- Asset import
- Drag-and-drop to scene

### Win32ConsolePanel

**File**: `engine/ui/native/Win32ConsolePanel.h`

Log viewer with native list control:

```cpp
class Win32ConsolePanel {
    bool create(HWND parent);
    void addMessage(const std::string& message, LogLevel level);
    void clear();
};
```

**Features:**
- Color-coded messages (Info, Warning, Error)
- Filtering by level
- Search functionality
- Timestamps
- Copy to clipboard

### Win32TerraformingPanel

**File**: `engine/ui/native/Win32TerraformingPanel.h`

**NEW in v0.2.6** - Native terraforming tool panel with full UI controls:

```cpp
class Win32TerraformingPanel {
    bool initialize(HWND parent, WorldEditor* worldEditor);
    void update();  // Call after terraforming state changes
};
```

**Features:**
- Tool selection buttons (10 tools):
  - Single Block, Brush, Sphere, Filled Sphere
  - Cube, Filled Cube, Line
  - Flatten, Smooth, Paint
- Mode selection (Place, Remove, Replace)
- Material picker with 5+ common voxel types
- Size control with +/- buttons and live display
- Undo/Redo buttons with automatic enable/disable
- Unreal-style dark theme integration
- Native Win32 controls for best performance
- Auto-synchronizes with TerraformingSystem

**Usage:**
```cpp
// Initialize with world editor
Win32TerraformingPanel* panel = editorManager->getTerraformingPanel();
if (panel) {
    // Panel is automatically created and positioned
    // Updates happen automatically when terraforming changes
    panel->update();  // Optional: force refresh
}
```

**Integration:**
- Replaces console-based EditorGUI output
- Seamlessly integrates with EditorManager
- Coordinates with other Win32 panels
- Full keyboard shortcut support via WorldEditor

### Win32StatusBar

**File**: `engine/ui/native/Win32StatusBar.h`

Native status bar component for displaying editor state information:

```cpp
class Win32StatusBar {
    bool create(HWND parent, int numPanes = 3, const PaneConfig* paneConfigs = nullptr);
    void setPaneText(int paneIndex, const std::wstring& text);
    void setStatusText(const std::wstring& text);
    void setCursorPosition(float x, float y, float z = 0.0f);
    void setSelectionInfo(int count, const std::wstring& type);
    void setFPS(float fps);
    void updateLayout();
};
```

**Features:**
- Multi-pane status bar with customizable layout
- Dark theme styling matching Unreal Engine
- Automatic resize handling
- Pre-built methods for common status information:
  - Ready/idle status text
  - Cursor position (X, Y, Z coordinates)
  - Selection info (count and type)
  - FPS display
  - Memory usage display

**Usage:**
```cpp
// Create status bar with 4 panes
Win32StatusBar statusBar;
statusBar.create(hwnd, 4);

// Update status information
statusBar.setStatusText(L"Ready");
statusBar.setCursorPosition(100.0f, 50.0f, 25.0f);
statusBar.setSelectionInfo(5, L"entities");
statusBar.setFPS(60.0f);
```

---

## Dark Theme System

### Unreal-Style Theme

The editor uses a professional dark theme inspired by Unreal Engine:

**File**: `engine/ui/native/UnrealStyleTheme.h`

**Primary Colors:**
```cpp
// Background colors
WindowBackground:  RGB(37, 37, 38)    // Main background
PanelBackground:   RGB(45, 45, 48)    // Panel background
 
// Text colors
Text:              RGB(241, 241, 241) // Primary text
TextDisabled:      RGB(128, 128, 128) // Disabled text

// Accent colors
AccentBlue:        RGB(51, 153, 255)  // Blue accent
AccentHover:       RGB(75, 175, 255)  // Lighter blue
AccentActive:      RGB(25, 128, 230)  // Darker blue
```

### Applying the Theme

```cpp
#include "ui/native/UnrealStyleTheme.h"

// Apply theme to window
WindowsThemeManager themeManager;
themeManager.initialize();
themeManager.applyToWindow(hwnd);

// Apply theme colors
HBRUSH bgBrush = CreateSolidBrush(UnrealStyleTheme::WindowBackground);
SetClassLongPtr(hwnd, GCLP_HBRBACKGROUND, (LONG_PTR)bgBrush);
```

### Theme Helper Functions

The theme provides several helper functions for consistent styling:

```cpp
// Get pre-configured fonts
HFONT font = UnrealStyleTheme::GetFont();          // Normal UI font
HFONT boldFont = UnrealStyleTheme::GetBoldFont();  // Bold for section headers
HFONT titleFont = UnrealStyleTheme::GetTitleFont(); // Title font for panel headers
HFONT monoFont = UnrealStyleTheme::GetMonospaceFont(); // For console/code

// Get cached brushes
HBRUSH panelBg = UnrealStyleTheme::GetPanelBackgroundBrush();
HBRUSH darkBg = UnrealStyleTheme::GetDarkBackgroundBrush();
HBRUSH inputBg = UnrealStyleTheme::GetInputBackgroundBrush();

// Draw themed UI elements
UnrealStyleTheme::DrawPanelTitleBar(hdc, rect, L"Inspector", true);
UnrealStyleTheme::DrawSectionHeader(hdc, rect, L"Transform", false);
UnrealStyleTheme::DrawSeparator(hdc, x1, y1, x2, y2);

// Color helpers
COLORREF hover = UnrealStyleTheme::GetHoverColor(baseColor);
COLORREF pressed = UnrealStyleTheme::GetPressedColor(baseColor);
COLORREF blended = UnrealStyleTheme::BlendColors(color1, color2, 0.5f);
```

---

## Transform Gizmo

### Overview

The Transform Gizmo allows visual manipulation of objects in the 3D viewport:

**Modes:**
- **Translate** (W key): Move objects along X/Y/Z axes
- **Rotate** (E key): Rotate objects around X/Y/Z axes
- **Scale** (R key): Scale objects uniformly or per-axis

### TransformGizmo Class

**File**: `engine/editor/TransformGizmo.h`

```cpp
class TransformGizmo {
    enum class Mode { Translate, Rotate, Scale };
    
    void setMode(Mode mode);
    Mode getMode() const;
    
    void setTarget(Entity* entity);
    void render(const Camera& camera);
    bool handleInput(const InputManager& input, const Camera& camera);
};
```

### Implementation Details

**Rendering:**
- Uses DebugRenderer for visualization
- Color-coded axes: Red (X), Green (Y), Blue (Z)
- Highlights on hover
- Scales with camera distance

**Interaction:**
- Ray casting for selection
- Dragging for manipulation
- Snap to grid (hold Ctrl)
- Local/World space toggle

---

## Toolbar and Menu Bar

### Native Menu Bar

**File**: `engine/ui/native/Win32MenuBar.h`

Native Windows menu bar with standard menus:

**Structure:**
```
File
├── New World          (Ctrl+N)
├── Open World         (Ctrl+O)
├── Save World         (Ctrl+S)
├── Save World As      (Ctrl+Shift+S)
├── Import Assets      
├── Export Selection   
├── ─────────────────
├── Recent Worlds      >
├── ─────────────────
└── Exit               (Alt+F4)

Edit
├── Undo               (Ctrl+Z)
├── Redo               (Ctrl+Y)
├── ─────────────────
├── Cut                (Ctrl+X)
├── Copy               (Ctrl+C)
├── Paste              (Ctrl+V)
├── Delete             (Del)
├── ─────────────────
├── Select All         (Ctrl+A)
└── Preferences        

View
├── Scene Hierarchy
├── Inspector
├── Content Browser
├── Console
├── ─────────────────
├── Toolbar
├── Status Bar
└── Layout             >

Tools
├── Transform Gizmo    >
│   ├── Move (W)
│   ├── Rotate (E)
│   └── Scale (R)
├── Terraforming       >
├── Voxel Editor       
└── Asset Manager      

Help
├── Documentation
├── Keyboard Shortcuts
├── ─────────────────
├── Report Bug
└── About
```

### Native Toolbar

**File**: `engine/ui/native/Win32Toolbar.h`

Native Windows toolbar with icon buttons:

**Buttons:**
- New, Open, Save (File operations)
- Undo, Redo (Edit operations)
- Move, Rotate, Scale (Transform gizmo)
- Play, Pause, Stop (Playmode)
- Build (Project build)

**Implementation:**
```cpp
class Win32Toolbar {
    bool create(HWND parent);
    void addButton(int id, const wchar_t* icon, const wchar_t* tooltip);
    void setButtonState(int id, bool enabled);
    void setToolCallback(std::function<void(Tool)> callback);
};
```

---

## Quick Start

### 1. Build the Engine

```bash
cmake -DUSE_WIN32_NATIVE=ON \
      -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake \
      -S . -B build
cmake --build build --config Release
```

### 2. Run the Editor

```bash
cd build/Release
./FreshVoxelEngine.exe
```

### 3. First Time Setup

1. The editor opens with default layout
2. File > New World to create a world
3. Use toolbar or keyboard shortcuts to select tools
4. Use Scene Hierarchy to manage entities
5. Use Inspector to edit properties

### 4. Basic Workflow

**Creating Objects:**
1. Right-click in Scene Hierarchy
2. Select "Create Entity" or use voxel tools
3. Select the new entity
4. Edit properties in Inspector

**Transforming Objects:**
1. Select an entity in viewport or hierarchy
2. Press W (Move), E (Rotate), or R (Scale)
3. Drag gizmo handles to transform
4. Hold Ctrl for grid snapping

**Saving:**
1. File > Save World (Ctrl+S)
2. Choose location and name
3. World saves automatically on exit

---

## Current Status

### ✅ Completed Features

1. **Windows Native UI** ✅
   - Native Win32 window management
   - Native menu bar (HMENU)
   - Native toolbar
   - DirectX integration
   - High DPI support

2. **Native Editor Panels** ✅
   - Win32SceneHierarchyPanel (TreeView)
   - Win32InspectorPanel (Property grid)
   - Win32ContentBrowserPanel (ListView)
   - Win32ConsolePanel (ListBox)
   - Win32TerraformingPanel (NEW in v0.2.6) ✨
     - Full tool selection UI
     - Mode and material pickers
     - Size controls
     - Undo/Redo integration

3. **Transform Gizmo** ✅
   - Rendering with DebugRenderer
   - Keyboard shortcuts (W/E/R)
   - Toolbar integration
   - Bidirectional sync

4. **Dark Theme** ✅
   - Unreal-style color scheme
   - Applied to native controls
   - Customizable colors

5. **Selection System** ✅
   - SelectionManager
   - SelectionRenderer
   - Box selection
   - Multi-selection

### 🔮 Future Enhancements

1. **Layout Management** (1 week)
   - Save/Load workspace layouts
   - Predefined layouts (Default, Minimal, Debugging)
   - Layout switcher in UI

2. **Camera Controls** (1 week)
   - Orthographic views (Top, Front, Side)
   - Focus on Selection (F key)
   - Frame Selection in View
   - Camera bookmarks

3. **Editor Settings Dialog** (1 week)
   - Auto-save interval
   - Grid settings
   - Snap settings
   - Theme customization
   - Font size

4. **Asset Preview System** (1-2 weeks)
   - Zoom/pan for textures
   - 3D model preview with rotation
   - Material preview sphere
   - Audio playback controls

---

## Keyboard Shortcuts

### General
- `Ctrl+N` - New World
- `Ctrl+O` - Open World
- `Ctrl+S` - Save World
- `Ctrl+Shift+S` - Save World As

### Edit
- `Ctrl+Z` - Undo
- `Ctrl+Y` or `Ctrl+Shift+Z` - Redo
- `Ctrl+X` - Cut
- `Ctrl+C` - Copy
- `Ctrl+V` - Paste
- `Del` - Delete Selection

### Transform Gizmo
- `W` - Translate Mode
- `E` - Rotate Mode
- `R` - Scale Mode
- `Ctrl` (hold) - Snap to Grid

### Camera
- `F` - Focus on Selection (planned)
- `Alt+LMB` - Orbit Camera
- `Alt+MMB` - Pan Camera
- `Alt+RMB` - Zoom Camera

### Viewport
- `Space` - Play/Pause
- `Esc` - Stop Playing

---

## Related Documentation

- [UI Architecture](../architecture/UI_ARCHITECTURE.md)
- [Input System](../guides/INPUT_SYSTEM.md)
- [Windows Native UI Quick Start](../getting-started/QUICK_START_WIN32_UI.md)
- [Development Tools](../community/DEVELOPMENT_TOOLS.md)

---

## Troubleshooting

### GUI Not Showing

1. Check Win32Window initialization:
   ```cpp
   if (!window->initialize()) {
       LOG_ERROR("Failed to initialize Win32Window");
   }
   ```

2. Verify DirectX device is created
3. Check window handle is valid

### Toolbar Buttons Not Working

1. Verify callback is set:
   ```cpp
   toolbar->setToolCallback([this](Tool tool) { ... });
   ```

2. Check button IDs match
3. Enable debug logging

### Gizmo Not Visible

1. Ensure entity is selected
2. Check gizmo is enabled
3. Verify DebugRenderer is initialized
4. Check camera is properly positioned

### High DPI Issues

1. Verify DPI awareness is set in manifest
2. Check DPI scaling in Win32Window
3. Update to latest Windows SDK

---

**Last Updated**: 2025-11-19  
**Version**: 2.0  
**Status**: Production Ready - Windows Native UI Only
