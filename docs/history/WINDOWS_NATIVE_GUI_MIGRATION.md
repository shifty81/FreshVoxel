# Windows Native GUI Migration Guide

## Overview

This document describes the migration from GLFW-based windowing to fully native Win32 windowing for the Fresh Voxel Engine. This change makes the GUI "totally Windows native" by replacing the cross-platform GLFW library with direct Win32 API calls.

## What Changed

### Previous Architecture (GLFW-based)
- **Windowing**: GLFW library (cross-platform)
- **Input**: GLFW input callbacks
- **ImGui Backend**: ImGui_ImplGlfw

### New Architecture (Win32-native)
- **Windowing**: Native Win32 API (`CreateWindowEx`, `WndProc`)
- **Input**: Native Win32 message handling (`WM_KEYDOWN`, `WM_MOUSEMOVE`, etc.)
- **ImGui Backend**: ImGui_ImplWin32

## New Classes

### Win32Window
**Location**: `engine/core/Win32Window.h`, `engine/core/Win32Window.cpp`

Replacement for the GLFW-based `Window` class. Provides:
- Native Win32 window creation
- Window message handling (WndProc)
- Input event callbacks
- OpenGL context support (optional)
- DirectX integration (native)

**Key Methods**:
```cpp
bool initialize(bool useOpenGL = false);
void pollEvents();
bool shouldClose() const;
HWND getHandle() const;
void setKeyCallback(KeyCallback callback);
void setMouseMoveCallback(MouseMoveCallback callback);
void setMouseButtonCallback(MouseButtonCallback callback);
```

### Win32InputManager
**Location**: `engine/input/Win32InputManager.h`, `engine/input/Win32InputManager.cpp`

Replacement for the GLFW-based `InputManager` class. Provides:
- Win32 virtual key code handling
- Mouse capture using Win32 APIs
- Action-based input system
- Compatible interface with existing InputManager

**Key Methods**:
```cpp
void initialize(Win32Window* window);
void processKeyEvent(WPARAM vkCode, bool isDown);
void processMouseMovement(int xpos, int ypos);
void processMouseButton(int button, bool isDown);
bool isActionActive(InputAction action) const;
void setCursorMode(bool captured);
```

### Updated ImGuiContext
**Location**: `engine/ui/ImGuiContext.h`, `engine/ui/ImGuiContext.cpp`

Now supports both GLFW and Win32 backends:

**New Method**:
```cpp
#ifdef _WIN32
bool initializeWin32(Win32Window* window, IRenderContext* renderContext);
#endif
```

## Integration Guide

### Step 1: Window Creation

**Old GLFW Code**:
```cpp
#include "core/Window.h"

std::unique_ptr<Window> window = std::make_unique<Window>(1920, 1080, "Fresh Voxel Engine");
window->initialize(false); // false for DirectX
```

**New Win32 Code**:
```cpp
#ifdef _WIN32
    #include "core/Win32Window.h"
    std::unique_ptr<Win32Window> window = std::make_unique<Win32Window>(1920, 1080, "Fresh Voxel Engine");
    window->initialize(false); // false for DirectX
#else
    #include "core/Window.h"
    std::unique_ptr<Window> window = std::make_unique<Window>(1920, 1080, "Fresh Voxel Engine");
    window->initialize(false);
#endif
```

### Step 2: Input Manager Setup

**Old GLFW Code**:
```cpp
#include "input/InputManager.h"

std::unique_ptr<InputManager> inputManager = std::make_unique<InputManager>();
inputManager->initialize(window->getHandle());
```

**New Win32 Code**:
```cpp
#ifdef _WIN32
    #include "input/Win32InputManager.h"
    std::unique_ptr<Win32InputManager> inputManager = std::make_unique<Win32InputManager>();
    inputManager->initialize(window.get());
    
    // Set up input callbacks
    window->setKeyCallback([&inputManager](int vk, bool isDown) {
        inputManager->processKeyEvent(vk, isDown);
    });
    
    window->setMouseMoveCallback([&inputManager](int x, int y) {
        inputManager->processMouseMovement(x, y);
    });
    
    window->setMouseButtonCallback([&inputManager](int button, bool isDown) {
        inputManager->processMouseButton(button, isDown);
    });
#else
    #include "input/InputManager.h"
    std::unique_ptr<InputManager> inputManager = std::make_unique<InputManager>();
    inputManager->initialize(window->getHandle());
#endif
```

### Step 3: ImGui Initialization

**Old GLFW Code**:
```cpp
ImGuiContext imguiContext;
imguiContext.initialize(window.get(), renderContext.get());
```

**New Win32 Code**:
```cpp
ImGuiContext imguiContext;
#ifdef _WIN32
    imguiContext.initializeWin32(window.get(), renderContext.get());
#else
    imguiContext.initialize(window.get(), renderContext.get());
#endif
```

### Step 4: Main Loop

**Old GLFW Code**:
```cpp
while (!window->shouldClose()) {
    window->pollEvents();
    
    // ... game logic ...
    
    window->swapBuffers(); // For OpenGL
}
```

**New Win32 Code** (same interface):
```cpp
while (!window->shouldClose()) {
    window->pollEvents();
    
    // ... game logic ...
    
    window->swapBuffers(); // For OpenGL (if used)
}
```

## Key Differences

### Virtual Key Codes
Win32 uses virtual key codes (`VK_*` constants) instead of GLFW key codes:

| GLFW | Win32 |
|------|-------|
| `GLFW_KEY_W` | `'W'` |
| `GLFW_KEY_SPACE` | `VK_SPACE` |
| `GLFW_KEY_ESCAPE` | `VK_ESCAPE` |
| `GLFW_KEY_LEFT_CONTROL` | `VK_CONTROL` |
| `GLFW_KEY_LEFT_SHIFT` | `VK_SHIFT` |
| `GLFW_MOUSE_BUTTON_LEFT` | `VK_LBUTTON` |
| `GLFW_MOUSE_BUTTON_RIGHT` | `VK_RBUTTON` |

### Window Handle
- GLFW: `GLFWwindow*`
- Win32: `HWND`

### Native Window Handle
Both provide access to the native handle:
```cpp
// GLFW
void* hwnd = window->getNativeWindowHandle();

// Win32
HWND hwnd = window->getHandle();
// or
void* hwnd = window->getNativeWindowHandle();
```

## Benefits of Win32-Native Approach

1. **Fully Native**: No dependency on cross-platform libraries
2. **Better Integration**: Direct access to all Windows features
3. **Smaller Binary**: No GLFW library needed
4. **Windows-Specific Features**: Can easily add Windows-only features
5. **Better Performance**: Direct Win32 API calls without abstraction layer
6. **ImGui Win32 Backend**: Better integration with ImGui's native Windows support

## Backward Compatibility

The GLFW code paths are still present in the codebase for reference, but on Windows, the Win32-native path is now preferred. To use GLFW, simply don't define the Win32 conditional compilation paths.

## Future Enhancements

With native Win32 windowing, we can now easily add:

1. **Custom Window Styles**: Native Windows 11 rounded corners, mica effects
2. **Title Bar Customization**: Custom title bar with Windows controls
3. **Snap Layouts**: Windows 11 snap layout support
4. **Drag and Drop**: Native Windows drag-and-drop for files
5. **System Tray**: Add system tray icon and menu
6. **Multiple Windows**: Easier to manage multiple windows natively

## Troubleshooting

### ImGui Input Not Working
Make sure the ImGui Win32 message handler is called first in `WindowProc`:
```cpp
if (ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam)) {
    return true;
}
```

### Mouse Not Captured
Ensure cursor capture is set correctly:
```cpp
inputManager->setCursorMode(true); // Capture for game mode
```

### Window Not Creating
Check that:
1. Window class is registered successfully
2. `CreateWindowEx` returns a valid HWND
3. Window style flags are correct

## References

- [Win32 Window Classes](https://docs.microsoft.com/en-us/windows/win32/winmsg/about-window-classes)
- [Win32 Window Messages](https://docs.microsoft.com/en-us/windows/win32/winmsg/about-messages-and-message-queues)
- [ImGui Win32 Backend](https://github.com/ocornut/imgui/blob/master/backends/imgui_impl_win32.cpp)
- [Virtual Key Codes](https://docs.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes)
