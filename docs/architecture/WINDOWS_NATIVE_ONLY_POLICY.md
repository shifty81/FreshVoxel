# Windows Native UI Only - Official Policy

**Effective Date:** 2025-11-21  
**Status:** OFFICIAL PROJECT POLICY  
**Scope:** All UI development for Fresh Voxel Engine

---

## 🎯 Policy Statement

**Fresh Voxel Engine uses Windows Native Win32 UI ONLY.**

### What This Means

1. **ALL UI components must use native Windows Win32 API**
   - Menu bars, toolbars, dialogs, panels - everything uses Win32 controls
   - No cross-platform UI frameworks (ImGui, Qt, wxWidgets, etc.)
   - Direct Win32 API calls with proper error handling

2. **ImGui is DEPRECATED and will be removed**
   - ImGui dependency will be removed from vcpkg.json
   - All ImGui-based code will be deleted or migrated
   - `FRESH_IMGUI_AVAILABLE` compile definition will be removed

3. **Windows 10/11 is the ONLY supported platform**
   - No Linux support (archived)
   - No macOS support (archived)
   - Windows-exclusive features are encouraged (DPI awareness, themes, taskbar integration, etc.)

---

## ✅ Why Windows Native UI?

### Professional Appearance
- Native Windows 10/11 look and feel
- Automatic dark/light theme support
- Familiar to Windows gamers and developers
- Per-monitor DPI awareness built-in

### Performance
- Native controls rendered by Windows (no custom rendering)
- Lower memory footprint
- Better GPU utilization for game rendering
- No overhead from UI framework

### Windows Integration
- Jump lists for recent files
- Taskbar progress indicators
- Toast notifications
- System dialogs (file open/save)
- Window chrome integration

### Development Focus
- Single platform = faster development
- Fewer dependencies to manage
- No cross-platform compatibility issues
- Full access to Windows capabilities

---

## 🚫 What NOT to Use

### NEVER Use These UI Frameworks
- ❌ ImGui (Dear ImGui) - DEPRECATED in this project
- ❌ Qt - Overkill for our needs, licensing issues
- ❌ wxWidgets - Cross-platform focus we don't need
- ❌ GLFW UI - Not designed for comprehensive UI
- ❌ Any cross-platform UI library

### NEVER Target These Platforms for UI
- ❌ Linux - Use headless mode only for CI/testing
- ❌ macOS - Not supported
- ❌ Web (WASM) - Not a goal for this engine
- ❌ Mobile (iOS/Android) - Not a goal for this engine

---

## ✅ What TO Use

### Use These Win32 Components

#### Window Management
```cpp
#include <windows.h>

// Create windows with CreateWindowEx
HWND hwnd = CreateWindowExW(
    WS_EX_APPWINDOW,
    className,
    windowTitle,
    WS_OVERLAPPEDWINDOW,
    x, y, width, height,
    nullptr, nullptr, hInstance, nullptr
);
```

#### Menu Bars
```cpp
// Use native menu bar
HMENU hMenu = CreateMenu();
HMENU hFileMenu = CreatePopupMenu();
AppendMenuW(hFileMenu, MF_STRING, ID_FILE_NEW, L"&New");
AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hFileMenu, L"&File");
SetMenu(hwnd, hMenu);
```

#### Toolbars
```cpp
#include <commctrl.h>

HWND hToolbar = CreateWindowEx(
    0, TOOLBARCLASSNAME, nullptr,
    WS_CHILD | WS_VISIBLE | TBSTYLE_FLAT,
    0, 0, 0, 0,
    hwndParent, nullptr, hInstance, nullptr
);
```

#### Dialogs
```cpp
// Modal dialog
INT_PTR result = DialogBox(
    hInstance, 
    MAKEINTRESOURCE(IDD_SETTINGS),
    hwndParent,
    SettingsDialogProc
);

// Modeless dialog
HWND hDialog = CreateDialog(
    hInstance,
    MAKEINTRESOURCE(IDD_INSPECTOR),
    hwndParent,
    InspectorDialogProc
);
```

#### Common Controls
```cpp
#include <commctrl.h>

// Tree view for scene hierarchy
HWND hTree = CreateWindowEx(
    0, WC_TREEVIEW, nullptr,
    WS_CHILD | WS_VISIBLE | TVS_HASLINES | TVS_LINESATROOT,
    x, y, width, height,
    hwndParent, nullptr, hInstance, nullptr
);

// List view for content browser
HWND hList = CreateWindowEx(
    0, WC_LISTVIEW, nullptr,
    WS_CHILD | WS_VISIBLE | LVS_REPORT,
    x, y, width, height,
    hwndParent, nullptr, hInstance, nullptr
);

// Property sheet for settings
PROPSHEETHEADER psh = { sizeof(PROPSHEETHEADER) };
psh.dwFlags = PSH_PROPSHEETPAGE | PSH_USEICONID;
psh.hwndParent = hwndParent;
// ... configure pages ...
PropertySheet(&psh);
```

### Use Native File Dialogs (NFD)
```cpp
#include <nfd.h>

// Native File Dialog Extended library wraps Win32 file dialogs
nfdchar_t* outPath = nullptr;
nfdresult_t result = NFD_OpenDialog("world", nullptr, &outPath);
if (result == NFD_OKAY) {
    // Use path
    free(outPath);
}
```

### Theme Integration
```cpp
// Use our UnrealStyleTheme helper
#include "ui/native/UnrealStyleTheme.h"

// Apply Unreal Engine-like dark theme
UnrealStyleTheme::apply(hwnd);
```

---

## 🔧 Migration Guide: Removing ImGui

### Step 1: Remove ImGui Dependency

**vcpkg.json** - Remove imgui:
```json
{
  "dependencies": [
    "glfw3",
    "glew",
    "glm",
    "openal-soft",
    "libvorbis",
    "luajit",
    "gtest",
    "nlohmann-json",
    "tinyobjloader",
    "nativefiledialog-extended"
    // REMOVED: "imgui"
  ]
}
```

### Step 2: Update CMakeLists.txt

Remove these sections from CMakeLists.txt:
```cmake
# REMOVE THIS SECTION:
# ImGui for UI (optional, will migrate to native Win32 UI fully)
find_package(imgui QUIET CONFIG)
if(imgui_FOUND)
    message(STATUS "ImGui found - UI panels will be available")
    set(IMGUI_AVAILABLE TRUE)
else()
    message(WARNING "ImGui not found. Some UI panels will not be available.")
    message(WARNING "  vcpkg install imgui:x64-windows")
    set(IMGUI_AVAILABLE FALSE)
endif()

# REMOVE THIS SECTION:
# Link ImGui (UI library)
if(IMGUI_AVAILABLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE imgui::imgui)
    target_compile_definitions(${PROJECT_NAME} PRIVATE FRESH_IMGUI_AVAILABLE)
    message(STATUS "ImGui enabled for UI panels")
endif()
```

### Step 3: Remove ImGui Header Files

Delete these files:
```bash
rm engine/ui/ImGuiContext.h
rm engine/ui/ImGuiContext.cpp
```

### Step 4: Remove ImGui-based Panel Files

Delete or migrate these files to Win32 equivalents:
```bash
# These may have ImGui fallback implementations
rm engine/ui/MainMenuPanel.h
rm engine/ui/MainMenuPanel.cpp

# Check these files for ImGui usage and remove fallback code
# - engine/ui/SettingsPanel.h
# - engine/ui/SettingsPanel.cpp
# - engine/ui/HotbarPanel.h
# - engine/ui/HotbarPanel.cpp
```

### Step 5: Update EditorManager

**engine/editor/EditorManager.cpp** - Remove all `#ifdef FRESH_IMGUI_AVAILABLE` blocks:

```cpp
// BEFORE (with ImGui fallback):
#ifdef FRESH_IMGUI_AVAILABLE
    // ImGui code...
#else
    // Native Win32 code...
#endif

// AFTER (Native Win32 only):
// Native Win32 code only - no conditional compilation
```

### Step 6: Remove Type Aliases

Update these header files to remove ImGui compatibility aliases:

**engine/ui/ConsolePanel.h:**
```cpp
#pragma once
#include "ui/native/Win32ConsolePanel.h"

namespace fresh
{
    // Direct typedef - no ImGui fallback
    using ConsolePanel = Win32ConsolePanel;
}
```

Do the same for:
- `engine/ui/SceneHierarchyPanel.h`
- `engine/ui/InspectorPanel.h`
- `engine/ui/ContentBrowserPanel.h`

### Step 7: Update Documentation

Update all documentation to remove references to ImGui:
- README.md
- docs/editor/GUI_GUIDE.md
- docs/editor/EDITOR_FEATURES_STATUS.md
- docs/getting-started/BUILD.md
- Any file mentioning "ImGui-based panels"

Change "ImGui-based" → "Native Win32"

### Step 8: Clean Build

```powershell
# Remove build directory
Remove-Item -Recurse -Force build

# Remove vcpkg packages
Remove-Item -Recurse -Force vcpkg/installed

# Regenerate build files (without ImGui)
.\generate_vs2022.bat

# Build
cd build
cmake --build . --config Release
```

---

## 📋 Implementation Checklist for New UI Features

When adding ANY new UI feature, follow this checklist:

### ✅ Required for All UI Components

1. **Use Win32 API directly**
   - [ ] Include `<windows.h>` and relevant headers
   - [ ] Use Win32 window classes (BUTTON, EDIT, LISTBOX, etc.)
   - [ ] Handle WM_* messages in window procedure

2. **Apply Unreal-style theme**
   - [ ] Use `UnrealStyleTheme::apply(hwnd)` for dark theme
   - [ ] Set appropriate background/foreground colors
   - [ ] Use proper fonts (Segoe UI, size 9)

3. **Handle DPI awareness**
   - [ ] Use `GetDpiForWindow()` to get current DPI
   - [ ] Scale sizes/fonts appropriately
   - [ ] Listen for WM_DPICHANGED messages

4. **Proper resource management**
   - [ ] Delete GDI objects (brushes, pens, fonts)
   - [ ] Release device contexts
   - [ ] Destroy child windows on parent destroy

5. **Error handling**
   - [ ] Check return values from Win32 API calls
   - [ ] Log errors with `LOG_ERROR_C()`
   - [ ] Provide fallback behavior

### ❌ Never Do This

1. **Don't use ImGui** - Not allowed under any circumstances
2. **Don't use `#ifdef _WIN32`** for UI - We're Windows-only, no need for conditionals
3. **Don't use cross-platform wrappers** - Direct Win32 API only
4. **Don't ignore DPI** - Always scale for high-DPI displays
5. **Don't hardcode colors** - Use UnrealStyleTheme constants

---

## 🎨 UI Style Guide

### Color Scheme (Unreal Engine Dark Theme)

```cpp
namespace UnrealStyleTheme {
    // Background colors
    constexpr COLORREF DARK_BG = RGB(42, 42, 42);          // Main background
    constexpr COLORREF DARKER_BG = RGB(32, 32, 32);        // Darker sections
    constexpr COLORREF PANEL_BG = RGB(51, 51, 51);         // Panel background
    
    // Text colors
    constexpr COLORREF TEXT = RGB(210, 210, 210);          // Normal text
    constexpr COLORREF TEXT_DISABLED = RGB(128, 128, 128); // Disabled text
    constexpr COLORREF TEXT_HIGHLIGHT = RGB(255, 255, 255);// Selected text
    
    // Accent colors
    constexpr COLORREF ACCENT = RGB(0, 122, 204);          // Blue accent
    constexpr COLORREF ACCENT_HOVER = RGB(30, 144, 255);   // Hover state
    constexpr COLORREF ACCENT_PRESSED = RGB(0, 100, 180);  // Pressed state
    
    // Status colors
    constexpr COLORREF SUCCESS = RGB(76, 175, 80);         // Green
    constexpr COLORREF WARNING = RGB(255, 152, 0);         // Orange
    constexpr COLORREF ERROR = RGB(244, 67, 54);           // Red
}
```

### Font Standards

```cpp
// Default UI font
HFONT hFont = CreateFont(
    -MulDiv(9, dpi, 72),  // 9pt size, scaled for DPI
    0, 0, 0,
    FW_NORMAL,
    FALSE, FALSE, FALSE,
    DEFAULT_CHARSET,
    OUT_DEFAULT_PRECIS,
    CLIP_DEFAULT_PRECIS,
    CLEARTYPE_QUALITY,
    DEFAULT_PITCH | FF_DONTCARE,
    L"Segoe UI"
);
```

---

## 📚 Reference Resources

### Microsoft Documentation
- [Win32 API Reference](https://docs.microsoft.com/en-us/windows/win32/api/)
- [Common Controls](https://docs.microsoft.com/en-us/windows/win32/controls/common-controls-intro)
- [Dialog Boxes](https://docs.microsoft.com/en-us/windows/win32/dlgbox/dialog-boxes)
- [Menus](https://docs.microsoft.com/en-us/windows/win32/menurc/menus)
- [High DPI](https://docs.microsoft.com/en-us/windows/win32/hidpi/high-dpi-desktop-application-development-on-windows)

### Fresh Engine Examples
- `engine/ui/native/Win32MenuBar.h` - Menu bar implementation
- `engine/ui/native/Win32Toolbar.h` - Toolbar implementation
- `engine/ui/native/Win32Panel.h` - Base panel class
- `engine/ui/native/UnrealStyleTheme.h` - Theme constants
- `engine/core/Win32Window.cpp` - Main window implementation

### Community Resources
- [Windows UI Cookbook (CodeProject)](https://www.codeproject.com/KB/windows/)
- [Win32 Programming Tutorial](http://www.winprog.org/tutorial/)

---

## 🔒 Enforcement

### Code Review Requirements

All pull requests with UI changes MUST:
1. ✅ Use Win32 API exclusively
2. ✅ No ImGui or cross-platform UI frameworks
3. ✅ Apply UnrealStyleTheme
4. ✅ Handle DPI properly
5. ✅ Include error handling
6. ✅ Follow this policy document

### Automated Checks

CI/CD will reject PRs that:
- ❌ Import `imgui.h` or any ImGui headers
- ❌ Use ImGui namespace or functions
- ❌ Add cross-platform UI dependencies
- ❌ Use `#ifdef _WIN32` for UI code (we're Windows-only)

---

## ❓ FAQ

### Q: What about cross-platform support?
**A:** Fresh Voxel Engine is Windows-exclusive by design. Linux/macOS support was archived. Native Win32 UI provides the best Windows gaming experience.

### Q: Can I use ImGui for debugging/profiling overlays?
**A:** No. Use Win32 dialogs or console output for debugging. Consider Windows Performance Analyzer for profiling.

### Q: What if ImGui is easier for rapid prototyping?
**A:** Use Win32 from the start. Our Win32Panel base class and helper functions make Win32 development fast. Prototyping with ImGui creates technical debt.

### Q: How do I contribute UI improvements?
**A:** Read this policy, study existing Win32 code in `engine/ui/native/`, and follow the implementation checklist. Ask in Discussions if unsure.

### Q: Will ImGui support ever return?
**A:** No. This is a permanent architectural decision. Windows Native UI is the future of this project.

---

## 📝 Change Log

### 2025-11-21 - Policy Established
- Created official Windows Native UI Only policy
- Documented ImGui deprecation and removal process
- Established guidelines for all future UI development
- Created migration checklist for removing ImGui

---

## ✅ Sign-Off

By contributing to Fresh Voxel Engine, you agree to follow this Windows Native UI Only policy.

**Policy Owner:** Project Maintainers  
**Last Updated:** 2025-11-21  
**Status:** ACTIVE & ENFORCED

---

*For questions or clarification, open a GitHub Discussion.*
