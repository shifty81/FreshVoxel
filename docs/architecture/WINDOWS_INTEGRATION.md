# Windows-Native Integration Guide

## Overview

Fresh Voxel Engine now includes deep Windows integration features that provide a native Windows 10/11 experience while maintaining cross-platform compatibility. These features are automatically enabled on Windows and gracefully disabled on other platforms.

## Features

### 1. Windows Theme System

Automatically detects and applies Windows system themes to the editor interface.

**Supported Themes:**
- **Auto** - Automatically matches Windows dark/light mode setting
- **Light** - Windows light theme
- **Dark** - Windows dark theme (default)
- **High Contrast** - Windows high contrast mode
- **Classic** - Classic Windows theme
- **Custom** - User-defined custom theme

**Features:**
- Automatic dark mode detection
- Windows accent color integration
- Modern Windows 11 styling with rounded corners
- Real-time theme updates
- Custom theme editor with color pickers

**Access:** Settings → Windows Customization → Theme tab

### 2. Native Windows Dialogs

Replaces generic dialogs with Windows-native dialogs that match the system UI.

**Available Dialogs:**
- **Open File** - Native file picker with type filters and multi-select
- **Save File** - Native save dialog with filename and extension suggestions
- **Folder Browser** - Native folder selection dialog
- **Message Boxes** - Native message boxes with various button configurations

**Features:**
- Supports all standard file filters
- Multi-file selection
- Recent folders integration
- Native Windows icons and styling
- Keyboard shortcuts and navigation

**Access:** Settings → Windows Customization → Dialogs tab

### 3. Taskbar Integration

Provides Windows taskbar features for better system integration.

**Features:**
- **Progress Indicator** - Show progress on taskbar button (Normal, Error, Paused, Indeterminate)
- **Flash Taskbar** - Get user attention with taskbar flashing
- **Overlay Icons** - Display status icons on taskbar button
- **Jump Lists** - Recent files and quick actions (planned)

**Use Cases:**
- Show build/loading progress
- Notify users of important events
- Display current status (recording, processing, etc.)

**Access:** Settings → Windows Customization → Taskbar tab

### 4. Jump Lists

Recent files and quick actions accessible from the Windows taskbar.

**Features:**
- **Recent Worlds** - Quick access to recently opened worlds
- **Custom Tasks** - Quick actions like "Create New World"
- **Automatic Updates** - Jump list updates when worlds are opened/saved
- **Windows 7+** - Works on Windows 7, 8, 10, and 11

**Use Cases:**
- Quick access to recent projects
- Launch common tasks from taskbar
- Improve workflow efficiency

**Access:** Automatically managed, right-click taskbar icon to view

### 5. Windows Toast Notifications

Native toast notifications that appear in the Windows Action Center.

**Features:**
- **Multiple Types** - Info, Success, Warning, Error notifications
- **Rich Content** - Support for images and buttons (planned)
- **Action Center** - Notifications persist in Windows Action Center
- **Windows 10+** - Requires Windows 10 or later

**Use Cases:**
- Notify about world save completion
- Alert on build/generation errors
- Inform about important events

**Access:** Called programmatically when events occur

### 6. High DPI Support

Automatic per-monitor DPI awareness for crisp rendering on high-resolution displays.

**Features:**
- **Per-Monitor DPI V2** - Best quality on multi-monitor setups
- **Automatic Scaling** - UI scales correctly on 4K/5K displays
- **DPI Change Detection** - Updates when moving between monitors
- **Windows 8.1+** - Works on Windows 8.1, 10, and 11

**Use Cases:**
- Optimal display on high-DPI monitors
- Multi-monitor setups with different DPIs
- Laptop + external 4K monitor configurations

**Access:** Automatically enabled during engine initialization

## How to Use

### Opening Windows Customization Panel

1. **Via Menu:** Settings → Windows Customization... (Ctrl+W)
2. **Via Code:** Call `EditorManager::getWindowsCustomizationPanel()->setVisible(true)`

### Changing Themes

```cpp
// Get theme manager from editor
auto* themeManager = editorManager->getWindowsCustomizationPanel()
                                   ->getThemeManager();

// Apply a preset theme
themeManager->setTheme(WindowsTheme::Dark);

// Or create a custom theme
ThemeColors customColors;
customColors.background = 0xFF1E1E1EFF;
customColors.accent = 0xFF0078D4FF;
customColors.text = 0xFFE0E0E0FF;
themeManager->setCustomTheme(customColors);
themeManager->setTheme(WindowsTheme::Custom);
```

### Using Native Dialogs

```cpp
// Get dialog manager from editor
auto* dialogManager = editorManager->getWindowsCustomizationPanel()
                                    ->getDialogManager();

// Open file dialog
std::vector<FileFilter> filters = {
    {"Text Files", "*.txt"},
    {"All Files", "*.*"}
};
auto files = dialogManager->showOpenFileDialog("Select a file", filters);

// Save file dialog
auto savePath = dialogManager->showSaveFileDialog("Save as", "document.txt", filters);

// Message box
auto result = dialogManager->showMessageBox(
    "Confirmation",
    "Do you want to save changes?",
    MessageBoxButtons::YesNoCancel,
    MessageBoxIcon::Question
);
```

### Using Taskbar Features

```cpp
// Get taskbar manager from editor
auto* taskbarManager = editorManager->getWindowsCustomizationPanel()
                                     ->getTaskbarManager();

// Show progress
taskbarManager->setProgressState(TaskbarProgressState::Normal);
taskbarManager->setProgressValue(50, 100); // 50%

// Flash taskbar for attention
taskbarManager->flashTaskbar(true); // Flash until window gets focus

// Show error state
taskbarManager->setProgressState(TaskbarProgressState::Error);

// Clear progress
taskbarManager->setProgressState(TaskbarProgressState::NoProgress);
```

### Using Jump Lists

```cpp
// Get jump list manager
WindowsJumpListManager jumpListManager;
jumpListManager.initialize("FreshVoxelEngine.VoxelEditor.1");

// Add a recent world
jumpListManager.addRecentWorld(
    "My Awesome World",
    "C:/Users/Username/Documents/FreshWorlds/awesome.world",
    "A world with mountains and caves"
);

// Add custom tasks
jumpListManager.addTask(
    "Create New World",
    "--new-world",
    "Create a new voxel world"
);

jumpListManager.addTask(
    "Open Documentation",
    "--help",
    "View documentation"
);

// Update the jump list
jumpListManager.updateJumpList();
```

### Using Toast Notifications

```cpp
// Get toast manager
WindowsToastManager toastManager;
toastManager.initialize("FreshVoxelEngine.VoxelEditor.1");

// Show a simple notification
toastManager.showToast(
    "World Saved",
    "Your world has been saved successfully!",
    WindowsToastManager::ToastType::Success
);

// Show error notification
toastManager.showToast(
    "Build Failed",
    "Failed to generate world meshes. Check the console for details.",
    WindowsToastManager::ToastType::Error
);

// Show notification with buttons (future)
std::vector<WindowsToastManager::ToastButton> buttons = {
    {"Open World", "--open-world"},
    {"Dismiss", ""}
};
toastManager.showToastWithButtons(
    "Generation Complete",
    "World generation finished successfully!",
    buttons,
    [](const std::string& args) {
        // Handle button click
        if (args == "--open-world") {
            // Open the world
        }
    },
    WindowsToastManager::ToastType::Success
);
```

### Using High DPI Support

```cpp
// Initialize DPI manager at startup
WindowsDPIManager dpiManager;
dpiManager.initialize(WindowsDPIManager::DPIAwarenessMode::PerMonitorAwareV2);

// Get DPI scale for current window
void* hwnd = getWindowHandle();
float dpiScale = dpiManager.getDPIScaleForWindow(hwnd);
LOG_INFO("Current DPI scale: " + std::to_string(dpiScale)); // e.g., 1.5 for 144 DPI

// Convert logical pixels to physical pixels for rendering
int logicalWidth = 800;
int physicalWidth = WindowsDPIManager::logicalToPhysical(logicalWidth, dpiScale);

// Convert physical pixels back to logical for hit testing
int physicalX = 1200;
int logicalX = WindowsDPIManager::physicalToLogical(physicalX, dpiScale);

// Get system DPI
unsigned int systemDPI = dpiManager.getSystemDPI();
LOG_INFO("System DPI: " + std::to_string(systemDPI)); // e.g., 144

// The DPI manager automatically handles:
// - Per-monitor DPI awareness
// - DPI changes when moving windows between monitors
// - Proper scaling for all UI elements
```

## Technical Details

### Architecture

All Windows-native features are:
- Conditionally compiled using `#ifdef _WIN32`
- Integrated into `EditorManager` for easy access
- Built on top of Win32 API and COM interfaces
- Compatible with Windows 10 and Windows 11

### File Organization

```
engine/ui/
├── WindowsThemeManager.h          # Theme detection and management
├── WindowsDialogManager.h         # Native dialog wrappers
├── WindowsTaskbarManager.h        # Taskbar integration
├── WindowsJumpListManager.h       # Jump list integration
├── WindowsToastManager.h          # Toast notifications
├── WindowsDPIManager.h            # High DPI awareness
└── WindowsCustomizationPanel.h    # UI for all Windows features

engine/ui/
├── WindowsThemeManager.cpp
├── WindowsDialogManager.cpp
├── WindowsTaskbarManager.cpp
├── WindowsJumpListManager.cpp
├── WindowsToastManager.cpp
├── WindowsDPIManager.cpp
└── WindowsCustomizationPanel.cpp
```

### Dependencies

- **Windows API** - Core Win32 functions
- **dwmapi.lib** - Desktop Window Manager API for theme detection
- **shell32.lib** - Shell API for dialogs and jump lists
- **propsys.lib** - Property system for jump list metadata
- **shcore.lib** - Shell core for DPI awareness APIs
- **COM** - Component Object Model for taskbar and notification integration
- **Windows 10 SDK** - For toast notification APIs (Windows.UI.Notifications)

All dependencies are standard Windows libraries included with the Windows SDK.

### Platform Compatibility

| Feature | Windows 7 | Windows 8.1 | Windows 10 | Windows 11 | Linux | macOS |
|---------|-----------|-------------|-----------|-----------|-------|-------|
| Theme Manager | ❌ | ❌ | ✅ | ✅ | ❌ | ❌ |
| Native Dialogs | ✅ | ✅ | ✅ | ✅ | ❌ | ❌ |
| Taskbar Integration | ✅ | ✅ | ✅ | ✅ | ❌ | ❌ |
| Jump Lists | ✅ | ✅ | ✅ | ✅ | ❌ | ❌ |
| Toast Notifications | ❌ | ❌ | ✅ | ✅ | ❌ | ❌ |
| High DPI Support | ❌ | ✅ | ✅ | ✅ | ❌ | ❌ |

On non-Windows platforms, these features are automatically disabled at compile time, and the engine falls back to cross-platform alternatives (ImGui dialogs, etc.).

## Customization Examples

### Custom Theme Example

Create a custom theme with your brand colors:

```cpp
WindowsThemeManager themeManager;
themeManager.initialize();

ThemeColors brandTheme;
brandTheme.background = 0xFF2D2D30FF;  // Dark gray
brandTheme.foreground = 0xFF3E3E42FF;  // Medium gray
brandTheme.accent = 0xFFFF6B35FF;      // Brand orange
brandTheme.text = 0xFFFFFFFFFF;        // White
brandTheme.textDisabled = 0xFF808080FF; // Gray
brandTheme.border = 0xFF555555FF;      // Border gray
brandTheme.highlight = 0xFFCC5529FF;   // Darker orange
brandTheme.shadow = 0xFF000000FF;      // Black

themeManager.setCustomTheme(brandTheme);
themeManager.setTheme(WindowsTheme::Custom);
themeManager.applyToImGui();
```

### Progress Tracking Example

Show progress during a long operation:

```cpp
void performLongOperation(WindowsTaskbarManager* taskbar) {
    taskbar->setProgressState(TaskbarProgressState::Normal);
    
    for (int i = 0; i <= 100; i++) {
        // Do work...
        taskbar->setProgressValue(i, 100);
        
        if (errorOccurred) {
            taskbar->setProgressState(TaskbarProgressState::Error);
            taskbar->flashTaskbar(true);
            return;
        }
    }
    
    taskbar->setProgressState(TaskbarProgressState::NoProgress);
}
```

## Recently Implemented

These features have been recently added to Fresh Voxel Engine:

1. ✅ **Jump Lists** - Recent files and quick actions in taskbar menu (IMPLEMENTED)
2. ✅ **Toast Notifications** - Windows 10/11 toast notifications (BASIC IMPLEMENTATION)
3. ✅ **High DPI Support** - Per-monitor DPI awareness for crisp rendering (IMPLEMENTED)
4. ✅ **AVX2 Optimizations** - SIMD instructions for improved performance
5. ✅ **Enhanced Compiler Flags** - Whole program optimization, fast floating-point

## Future Enhancements

Planned features for future releases:

1. **Full WinRT Toast Support** - Rich interactive toast notifications with images and buttons
2. **Thumbnail Toolbar** - Playback controls in taskbar preview
3. **Live Tiles** - Dynamic Start menu tile (Windows 10)
4. **Share Contract** - Windows Share integration
5. **Windows Hello** - Biometric authentication integration
6. **Clipboard Integration** - Rich clipboard with world/voxel data
7. **DirectStorage** - Ultra-fast asset loading on Windows 11

## Troubleshooting

### Theme not applying

**Problem:** Windows theme changes don't reflect in the application

**Solution:** 
1. Restart the application
2. Manually set theme to "Auto" in Windows Customization panel
3. Check Windows Settings → Personalization → Colors

### Dialogs not appearing

**Problem:** Native dialogs don't show up

**Solution:**
1. Ensure application has focus
2. Check if dialogs are being blocked by antivirus/security software
3. Verify window handle is valid in dialog manager initialization

### Taskbar features not working

**Problem:** Progress indicator or flash not visible

**Solution:**
1. Check taskbar settings (Windows Settings → Personalization → Taskbar)
2. Ensure taskbar is not hidden
3. Try running application as administrator

## Contributing

To contribute to Windows-native integration features:

1. Follow the existing pattern of conditional compilation (`#ifdef _WIN32`)
2. Ensure features gracefully degrade on non-Windows platforms
3. Test on both Windows 10 and Windows 11
4. Document any new Windows API dependencies
5. Add examples to this guide

## See Also

- [ARCHITECTURE.md](ARCHITECTURE.md) - Overall system architecture
- [CONTRIBUTING.md](CONTRIBUTING.md) - Contribution guidelines
- [BUILD.md](BUILD.md) - Build instructions
- [Microsoft Docs - Windows App Development](https://docs.microsoft.com/en-us/windows/apps/)
