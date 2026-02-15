# Win32 Native GUI Development Plan

## Current Status

The Fresh Voxel Engine already has Win32 native UI components partially implemented:

### Existing Win32 UI Components
- ✅ `Win32Window` - Native Windows window management
- ✅ `Win32InputManager` - Native Windows input handling  
- ✅ `Win32Panel` - Base class for Win32 UI panels
- ✅ `Win32MenuBar` - Native Windows menu bar
- ✅ `Win32Toolbar` - Native Windows toolbar
- ✅ `Win32HUD` - Native heads-up display
- ✅ `Win32TreeView` - Native tree view control
- ✅ `Win32ListView` - Native list view control
- ✅ `WindowsThemeManager` - Windows theme integration
- ✅ `WindowsDialogManager` - Native file dialogs
- ✅ `WindowsTaskbarManager` - Taskbar integration
- ✅ `WindowsDPIManager` - High DPI support

### Problem with Current Build

The current CMakeLists.txt only includes Win32 UI components when:
1. ImGui is found AND
2. Building on Windows

This creates a dependency on ImGui even for Win32 native UI.

## Solution: Separate Win32 UI from ImGui

### Step 1: Update CMakeLists.txt

Create a separate section for Win32 native UI that doesn't depend on ImGui:

```cmake
# Add Windows-native UI on Windows (independent of ImGui)
if(WIN32)
    list(APPEND ENGINE_SOURCES
        # Core Windows components
        engine/core/Win32Window.cpp
        engine/input/Win32InputManager.cpp
        
        # Native Win32 UI panels and controls
        engine/ui/native/Win32Panel.cpp
        engine/ui/native/Win32MenuBar.cpp
        engine/ui/native/Win32Toolbar.cpp
        engine/ui/native/Win32HUD.cpp
        engine/ui/native/Win32TreeView.cpp
        engine/ui/native/Win32ListView.cpp
        
        # Windows integration features
        engine/ui/WindowsThemeManager.cpp
        engine/ui/WindowsDialogManager.cpp
        engine/ui/WindowsTaskbarManager.cpp
        engine/ui/WindowsCustomizationPanel.cpp
        engine/ui/WindowsJumpListManager.cpp
        engine/ui/WindowsToastManager.cpp
        engine/ui/WindowsDPIManager.cpp
    )
    
    # Define that we're using native Win32 UI
    add_definitions(-DFRESH_WIN32_UI)
    
    # Link Windows libraries
    target_link_libraries(${PROJECT_NAME} PRIVATE 
        comctl32    # Common controls
        user32      # User interface
        gdi32       # Graphics Device Interface
        shell32     # Shell integration
        ole32       # OLE/COM
        dwmapi      # Desktop Window Manager
    )
endif()

# Add ImGui-dependent UI sources only if ImGui is available
if(imgui_FOUND)
    list(APPEND ENGINE_SOURCES
        engine/editor/EditorManager.cpp
        engine/ui/ImGuiContext.cpp
        engine/ui/SceneHierarchyPanel.cpp
        # ... other ImGui panels
    )
    add_definitions(-DFRESH_IMGUI_AVAILABLE)
endif()
```

### Step 2: Create Win32-Only Main Menu

Create `engine/ui/native/Win32MainMenu.cpp`:

```cpp
#ifdef _WIN32
#include "ui/native/Win32MainMenu.h"
#include "ui/native/Win32MenuBar.h"
#include "core/Win32Window.h"

namespace fresh {

class Win32MainMenu : public Win32Panel {
public:
    Win32MainMenu() : m_menuBar(nullptr) {}
    
    bool create(Win32Window* window) {
        m_menuBar = std::make_unique<Win32MenuBar>();
        
        if (!m_menuBar->create(window->getHandle())) {
            return false;
        }
        
        setupMenus();
        return true;
    }
    
private:
    void setupMenus() {
        // File menu
        int fileMenu = m_menuBar->addMenu("File");
        m_menuBar->addMenuItem(fileMenu, "New World...", 
            [this]() { onNewWorld(); });
        m_menuBar->addMenuItem(fileMenu, "Load World...", 
            [this]() { onLoadWorld(); });
        m_menuBar->addSeparator(fileMenu);
        m_menuBar->addMenuItem(fileMenu, "Exit", 
            [this]() { onExit(); });
        
        // Settings menu
        int settingsMenu = m_menuBar->addMenu("Settings");
        m_menuBar->addMenuItem(settingsMenu, "Game Settings...",
            [this]() { onGameSettings(); });
        m_menuBar->addMenuItem(settingsMenu, "Graphics...",
            [this]() { onGraphicsSettings(); });
    }
    
    void onNewWorld() {
        // Show native Win32 dialog for new world
        // Use WindowsDialogManager for file dialogs
    }
    
    void onLoadWorld() {
        // Show native Win32 open file dialog
    }
    
    void onGameSettings() {
        // Open Win32 settings window
    }
    
    void onGraphicsSettings() {
        // Open Win32 graphics settings window
    }
    
    void onExit() {
        PostQuitMessage(0);
    }
    
    std::unique_ptr<Win32MenuBar> m_menuBar;
};

} // namespace fresh
#endif
```

### Step 3: Create Win32 Settings Dialog

Create `engine/ui/native/Win32SettingsDialog.cpp`:

```cpp
#ifdef _WIN32
#include "ui/native/Win32SettingsDialog.h"
#include <commctrl.h>

namespace fresh {

class Win32SettingsDialog : public Win32Panel {
public:
    struct Settings {
        float mouseSensitivity = 0.5f;
        int resolutionWidth = 1920;
        int resolutionHeight = 1080;
        bool fullscreen = false;
        bool vsync = true;
        float masterVolume = 1.0f;
        float musicVolume = 0.7f;
        float sfxVolume = 0.8f;
    };
    
    bool createDialog(HWND parent) {
        // Create a modal dialog window
        HWND dlg = CreateWindowEx(
            WS_EX_DLGMODALFRAME,
            L"#32770", // Dialog class
            L"Game Settings",
            WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
            CW_USEDEFAULT, CW_USEDEFAULT,
            500, 400,
            parent,
            nullptr,
            GetModuleHandle(nullptr),
            this
        );
        
        if (!dlg) return false;
        
        m_hwnd = dlg;
        createControls();
        centerWindow();
        
        return true;
    }
    
private:
    void createControls() {
        // Tab control for different settings categories
        HWND tabCtrl = CreateWindowEx(
            0, WC_TABCONTROL, L"",
            WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
            10, 10, 470, 340,
            m_hwnd, (HMENU)IDC_TAB_CONTROL,
            GetModuleHandle(nullptr), nullptr
        );
        
        // Add tabs
        TCITEM tie;
        tie.mask = TCIF_TEXT;
        
        tie.pszText = (LPWSTR)L"Controls";
        TabCtrl_InsertItem(tabCtrl, 0, &tie);
        
        tie.pszText = (LPWSTR)L"Graphics";
        TabCtrl_InsertItem(tabCtrl, 1, &tie);
        
        tie.pszText = (LPWSTR)L"Audio";
        TabCtrl_InsertItem(tabCtrl, 2, &tie);
        
        // Mouse sensitivity slider
        createSlider(L"Mouse Sensitivity:", 
                    20, 60, 300, 
                    IDC_MOUSE_SENSITIVITY);
        
        // Resolution dropdown
        createDropdown(L"Resolution:", 
                      20, 110, 300,
                      IDC_RESOLUTION);
        
        // OK and Cancel buttons
        CreateWindow(L"BUTTON", L"OK",
            WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
            300, 360, 80, 25,
            m_hwnd, (HMENU)IDOK,
            GetModuleHandle(nullptr), nullptr);
            
        CreateWindow(L"BUTTON", L"Cancel",
            WS_CHILD | WS_VISIBLE,
            390, 360, 80, 25,
            m_hwnd, (HMENU)IDCANCEL,
            GetModuleHandle(nullptr), nullptr);
    }
    
    void createSlider(const wchar_t* label, int x, int y, int width, int id) {
        // Label
        CreateWindow(L"STATIC", label,
            WS_CHILD | WS_VISIBLE,
            x, y, 150, 20,
            m_hwnd, nullptr,
            GetModuleHandle(nullptr), nullptr);
        
        // Trackbar (slider)
        HWND slider = CreateWindowEx(
            0, TRACKBAR_CLASS, L"",
            WS_CHILD | WS_VISIBLE | TBS_HORZ | TBS_AUTOTICKS,
            x, y + 25, width, 30,
            m_hwnd, (HMENU)id,
            GetModuleHandle(nullptr), nullptr
        );
        
        SendMessage(slider, TBM_SETRANGE, TRUE, MAKELONG(1, 100));
        SendMessage(slider, TBM_SETPOS, TRUE, 50); // Default to middle
    }
    
    void createDropdown(const wchar_t* label, int x, int y, int width, int id) {
        // Label
        CreateWindow(L"STATIC", label,
            WS_CHILD | WS_VISIBLE,
            x, y, 150, 20,
            m_hwnd, nullptr,
            GetModuleHandle(nullptr), nullptr);
        
        // Combobox
        HWND combo = CreateWindow(L"COMBOBOX", L"",
            WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
            x, y + 25, width, 200,
            m_hwnd, (HMENU)id,
            GetModuleHandle(nullptr), nullptr);
        
        // Add resolution options
        SendMessage(combo, CB_ADDSTRING, 0, (LPARAM)L"1280x720");
        SendMessage(combo, CB_ADDSTRING, 0, (LPARAM)L"1920x1080");
        SendMessage(combo, CB_ADDSTRING, 0, (LPARAM)L"2560x1440");
        SendMessage(combo, CB_ADDSTRING, 0, (LPARAM)L"3840x2160");
        SendMessage(combo, CB_SETCURSEL, 1, 0); // Default to 1080p
    }
    
    void centerWindow() {
        RECT rc;
        GetWindowRect(m_hwnd, &rc);
        
        int screenWidth = GetSystemMetrics(SM_CXSCREEN);
        int screenHeight = GetSystemMetrics(SM_CYSCREEN);
        
        int x = (screenWidth - (rc.right - rc.left)) / 2;
        int y = (screenHeight - (rc.bottom - rc.top)) / 2;
        
        SetWindowPos(m_hwnd, nullptr, x, y, 0, 0, 
                    SWP_NOSIZE | SWP_NOZORDER);
    }
    
    bool handleMessage(UINT msg, WPARAM wParam, LPARAM lParam, LRESULT& result) override {
        switch (msg) {
            case WM_COMMAND:
                if (LOWORD(wParam) == IDOK) {
                    // Save settings
                    applySettings();
                    DestroyWindow(m_hwnd);
                    return true;
                }
                if (LOWORD(wParam) == IDCANCEL) {
                    DestroyWindow(m_hwnd);
                    return true;
                }
                break;
        }
        return false;
    }
    
    void applySettings() {
        // Read control values and update settings
        // Then save to settings.cfg file
    }
    
    enum {
        IDC_TAB_CONTROL = 1001,
        IDC_MOUSE_SENSITIVITY = 1002,
        IDC_RESOLUTION = 1003,
    };
    
    Settings m_settings;
};

} // namespace fresh
#endif
```

### Step 4: Update Engine.cpp to Use Win32 UI

```cpp
#ifdef FRESH_WIN32_UI
    #include "ui/native/Win32MainMenu.h"
    #include "ui/native/Win32SettingsDialog.h"
#endif

bool Engine::initialize() {
#ifdef _WIN32
    // Create Win32 window
    m_window = std::make_unique<Win32Window>(1920, 1080, "Fresh Voxel Engine");
    m_window->initialize(false); // Use DirectX
    
    // Create Win32 input manager
    m_inputManager = std::make_unique<Win32InputManager>();
    m_inputManager->initialize(m_window.get());
    
    // Create Win32 main menu
    m_mainMenu = std::make_unique<Win32MainMenu>();
    m_mainMenu->create(m_window.get());
    
    // Create Win32 HUD
    m_hud = std::make_unique<Win32HUD>();
    m_hud->initialize(m_window->getHandle());
#else
    // Fallback for non-Windows platforms
    Logger::getInstance().error("Windows-only build - cannot run on this platform");
    return false;
#endif
    
    return true;
}
```

## Next Steps

1. ✅ Document Win32 UI plan
2. [ ] Update CMakeLists.txt to separate Win32 UI from ImGui
3. [ ] Implement Win32MainMenu class
4. [ ] Implement Win32SettingsDialog class
5. [ ] Integrate Win32 UI into Engine class
6. [ ] Test on Windows 10/11
7. [ ] Add Win32 world creation dialog
8. [ ] Add Win32 world loading dialog
9. [ ] Polish Win32 UI theming

## Benefits of Pure Win32 UI

1. **No ImGui dependency** - Smaller binary, faster startup
2. **Native Windows look** - Perfect integration with Windows 10/11
3. **Better performance** - Direct Win32 rendering, no intermediate framework
4. **Windows features** - Easy access to taskbar, jump lists, notifications
5. **Professional appearance** - Looks like a real Windows application

## Testing on Windows

To test this on Windows:

```cmd
cd fresh
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
build\Release\FreshVoxelEngine.exe
```

The application should now show a native Windows menu bar and use Win32 dialogs for all UI operations.
