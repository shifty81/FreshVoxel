# Layout Management System

This document describes the layout management system in the Fresh Voxel Engine editor, which allows users to save, load, and switch between different workspace configurations.

## Overview

The Layout Management system provides a way to organize editor panels based on different workflows. It supports predefined layouts and user-defined custom layouts, with automatic persistence across sessions.

## Features

### Predefined Layouts

The editor comes with three predefined workspace layouts:

1. **Default Layout**
   - All panels visible
   - Best for general game development
   - Panels: Scene Hierarchy, Inspector, Content Browser, Console, Tool Palette

2. **Minimal Layout**
   - Only essential editing panels
   - Best for focused voxel editing with minimal distractions
   - Panels: Inspector, Tool Palette

3. **Debugging Layout**
   - Console and debugging-focused panels
   - Best for troubleshooting and testing
   - Panels: Scene Hierarchy, Inspector, Console

### Custom Layouts

Users can save their current panel configuration as a custom layout:
- Save current panel visibility states
- Restore custom layouts later
- Persist across editor sessions

### Automatic Persistence

- Last used layout is automatically saved
- Layout configuration restored on editor startup
- Panel states persist across application restarts

## Usage

### Switching Layouts

1. Open the **Window** menu in the editor menu bar
2. Navigate to **Layout** submenu
3. Select a layout:
   - **Default** - All panels visible
   - **Minimal** - Essential panels only
   - **Debugging** - Debug-focused layout

### Saving Current Layout

1. Arrange panels as desired (show/hide via Window menu)
2. Open **Window > Layout > Save Layout**
3. Current panel configuration is saved to the active layout

### Resetting Layout

To restore the default layout:
1. Open **Window > Reset Layout**
2. All panels reset to the Default layout configuration

## Technical Details

### LayoutConfig Structure

Each layout is defined by a `LayoutConfig` structure:

```cpp
struct LayoutConfig {
    std::string name;               // Layout name
    bool showSceneHierarchy;        // Scene Hierarchy panel visibility
    bool showInspector;             // Inspector panel visibility
    bool showContentBrowser;        // Content Browser panel visibility
    bool showConsole;               // Console panel visibility
    bool showToolPalette;           // Tool Palette panel visibility
};
```

### Storage Format

Layouts are stored in `configs/layouts.ini` using a simple INI format:

```ini
[Current]
layout=Default

[MyCustomLayout]
showSceneHierarchy=1
showInspector=1
showContentBrowser=0
showConsole=1
showToolPalette=1
```

### API

#### LayoutManager Methods

```cpp
// Initialize the layout manager
bool initialize(const std::string& configPath = "configs/layouts.ini");

// Load a layout by name
bool loadLayout(const std::string& name, LayoutConfig& config);

// Save the current layout
bool saveLayout(const std::string& name, const LayoutConfig& config);

// Get predefined layout
bool getPredefinedLayout(const std::string& name, LayoutConfig& config);

// Get list of available layouts
std::vector<std::string> getAvailableLayouts() const;
```

#### EditorManager Methods

```cpp
// Load a workspace layout
void loadLayout(const std::string& name);

// Save the current workspace layout
void saveCurrentLayout(const std::string& name = "");

// Reset to default layout
void resetLayout();
```

## Architecture

### Components

1. **LayoutManager** (`engine/editor/LayoutManager.h`)
   - Core layout management logic
   - File I/O for persistence
   - Predefined layout definitions

2. **EditorManager** (`engine/editor/EditorManager.h`)
   - Integration point for layout system
   - Applies layouts to panel visibility states
   - Provides high-level layout operations

3. **EditorMenuBar** (`engine/ui/EditorMenuBar.h`)
   - User interface for layout operations
   - Window menu integration
   - Layout selection UI

### Data Flow

```
User Action (Menu)
    ↓
EditorMenuBar (callback)
    ↓
EditorManager (loadLayout/saveLayout)
    ↓
LayoutManager (file I/O)
    ↓
Panel Visibility Flags
    ↓
UI Update
```

## Configuration File

### Location

- Default: `configs/layouts.ini`
- Created automatically on first save
- Parent directories created if needed

### Format

INI-style configuration with sections for each layout:

```ini
# Current active layout
[Current]
layout=Default

# User-defined layout example
[MyWorkflow]
showSceneHierarchy=1
showInspector=1
showContentBrowser=0
showConsole=1
showToolPalette=0
```

### Boolean Values

- `1` or `true` = Panel visible
- `0` or `false` = Panel hidden

## Extension Points

### Adding New Panels

To add a new panel to the layout system:

1. Add visibility flag to `LayoutConfig` struct
2. Update `LayoutManager::saveToFile()` to persist the flag
3. Update `LayoutManager::loadFromFile()` to read the flag
4. Update `EditorManager::loadLayout()` to apply the flag
5. Update predefined layouts in `initializePredefinedLayouts()`

Example:
```cpp
// In LayoutConfig
struct LayoutConfig {
    // ... existing fields ...
    bool showMyNewPanel;  // Add new field
};

// In LayoutManager::initializePredefinedLayouts()
defaultLayout.showMyNewPanel = true;
minimalLayout.showMyNewPanel = false;
debuggingLayout.showMyNewPanel = true;

// In LayoutManager::saveToFile()
file << "showMyNewPanel=" << (config.showMyNewPanel ? "1" : "0") << "\n";

// In LayoutManager::loadFromFile()
if (key == "showMyNewPanel") {
    currentConfig.showMyNewPanel = boolValue;
}

// In EditorManager::loadLayout()
m_showMyNewPanel = config.showMyNewPanel;
```

### Custom Layout Names

Users can save layouts with custom names by modifying `EditorManager::saveCurrentLayout()`:

```cpp
// Save with custom name
void saveLayoutWithName(const std::string& name) {
    LayoutConfig config;
    config.showSceneHierarchy = m_showSceneHierarchy;
    // ... set other fields ...
    m_layoutManager->saveLayout(name, config);
}
```

## Future Enhancements

Potential improvements for the layout system:

1. **Panel Positions & Sizes**
   - Save panel docking positions
   - Persist panel sizes
   - Restore exact workspace arrangement

2. **Layout Import/Export**
   - Export layouts to share with team
   - Import layouts from file
   - Layout presets marketplace

3. **Keyboard Shortcuts**
   - Quick-switch between layouts (F1-F4)
   - Save layout hotkey (Ctrl+Shift+L)

4. **Layout Templates**
   - More predefined layouts (Animation, Lighting, Testing)
   - Role-based layouts (Programmer, Artist, Designer)

5. **Cloud Sync**
   - Sync layouts across machines
   - Team-shared layouts
   - Version control for layouts

## Troubleshooting

### Layout Not Saving

**Problem:** Layout changes don't persist after restart

**Solutions:**
- Ensure `configs/` directory is writable
- Check file permissions on `configs/layouts.ini`
- Verify disk space availability
- Check console for error messages

### Layout File Corrupted

**Problem:** Layout file is unreadable or contains errors

**Solutions:**
- Delete `configs/layouts.ini` to reset
- Editor will recreate with default layouts
- Manually fix INI syntax if possible

### Panel Not Responding to Layout

**Problem:** A panel doesn't show/hide when changing layouts

**Solutions:**
- Verify panel visibility flag in EditorManager
- Check menu bar callback is connected
- Ensure LayoutManager is initialized
- Review console logs for errors

## Examples

### Example 1: Creating a Custom Workflow

```cpp
// In your code
EditorManager* editor = getEditorManager();

// Configure panels for your workflow
editor->toggleSceneHierarchy();  // Show
editor->toggleInspector();        // Show
editor->toggleContentBrowser();   // Hide
editor->toggleConsole();          // Hide
editor->toggleToolPalette();      // Show

// Save as "VoxelEditing" layout
editor->saveCurrentLayout("VoxelEditing");
```

### Example 2: Switching Layouts Programmatically

```cpp
// Switch to debugging layout
editor->loadLayout("Debugging");

// Do some debugging work...

// Switch back to default
editor->loadLayout("Default");
```

### Example 3: Getting Available Layouts

```cpp
LayoutManager* layoutMgr = editor->getLayoutManager();
auto layouts = layoutMgr->getAvailableLayouts();

std::cout << "Available layouts:\n";
for (const auto& name : layouts) {
    std::cout << "  - " << name << "\n";
}
```

## Related Documentation

- [Editor Integration Guide](EDITOR_INTEGRATION.md)
- [GUI Guide](GUI_GUIDE.md)
- [Editor Features Status](EDITOR_FEATURES_STATUS.md)

---

*Last Updated: 2025-11-19*
