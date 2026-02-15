# Windows Native UI Migration Roadmap
## Complete ImGui Replacement Strategy

> **Document Version:** 1.0.0  
> **Last Updated:** 2025-11-18  
> **Purpose:** Comprehensive plan to replace ImGui with Windows Native UI

---

## 📋 Executive Summary

This document outlines the complete strategy for migrating the Fresh Voxel Engine editor from ImGui to Windows Native UI using Win32 API with custom controls and modern theming.

### Current State
- **UI Framework:** ImGui (Dear ImGui)
- **Total UI Code:** ~10,500 lines across 15+ UI components
- **Dependencies:** imgui, imgui backends (win32, dx11, dx12, opengl3)
- **Benefits of ImGui:** Immediate mode, easy to use, cross-platform
- **Limitations:** Not native Windows look/feel, limited accessibility

### Target State
- **UI Framework:** Win32 API with custom controls
- **Rendering:** GDI+/Direct2D for custom controls
- **Theme:** Windows 11 dark mode with Unreal Engine styling
- **Benefits:** Native performance, Windows integration, professional appearance

---

## 🎯 Migration Strategy

### Phase 1: Foundation & Architecture (Weeks 1-2)

#### 1.1 Create Native UI Framework
- [ ] **NativeUIContext** - Replace ImGuiContext
  - Window management system
  - Message pump integration
  - Control factory system
  - Theme manager integration

- [ ] **NativeControl Base Class** - Foundation for all controls
  ```cpp
  class NativeControl {
      HWND m_hwnd;
      WNDPROC m_originalProc;
      virtual void onCreate();
      virtual void onPaint(HDC hdc);
      virtual void onResize(int width, int height);
  };
  ```

- [ ] **Dark Theme Engine**
  - Windows 11 dark mode integration
  - Custom control rendering with dark colors
  - Unreal Engine color palette
  - DPI awareness

#### 1.2 Control Library
Core controls needed (in order of priority):

1. **Window/Dialog** - Basic container
2. **Button** - Primary interaction
3. **Label/Text** - Display text
4. **TextBox** - Text input
5. **ComboBox** - Dropdown selection
6. **ListBox** - List selection
7. **TreeView** - Hierarchical data
8. **ListView** - Tabular data
9. **PropertyGrid** - Key-value editing
10. **TabControl** - Tabbed interface
11. **Toolbar** - Tool buttons
12. **MenuBar** - Application menu
13. **StatusBar** - Status information
14. **Splitter** - Resizable panels
15. **ScrollBar** - Scrolling support

---

## 📦 Component Migration Priority

### Critical Path (Phase 2-3, Weeks 3-6)

#### Priority 1: Main Menu (Week 3)
**Files to migrate:**
- `engine/ui/MainMenuPanel.cpp/h`

**Native controls needed:**
- Dialog window
- Buttons (Create New World, Load World, Exit)
- Text labels
- Modal dialogs

**Complexity:** LOW  
**Estimated effort:** 2-3 days  
**Impact:** HIGH (first user interaction)

#### Priority 2: World Creation Dialog (Week 3)
**Files to migrate:**
- `engine/ui/MainMenuPanel.cpp` (renderNewWorldDialog, renderLoadWorldDialog)

**Native controls needed:**
- Modal dialog
- TextBox (world name, seed)
- Radio buttons (world type, 2D style)
- Buttons (Create, Cancel, Load)

**Complexity:** MEDIUM  
**Estimated effort:** 2-3 days  
**Impact:** HIGH (critical workflow)

#### Priority 3: Editor Menu Bar (Week 4)
**Files to migrate:**
- `engine/ui/EditorMenuBar.cpp/h`

**Native controls needed:**
- Native menu bar (already partially implemented in Win32MenuBar)
- Menu items with shortcuts
- Separators

**Complexity:** LOW (already have Win32MenuBar)  
**Estimated effort:** 2 days  
**Impact:** HIGH (primary navigation)

#### Priority 4: Editor Toolbar (Week 4)
**Files to migrate:**
- `engine/ui/EditorToolbar.cpp/h`

**Native controls needed:**
- Toolbar control
- Buttons with icons
- Tooltips

**Complexity:** MEDIUM  
**Estimated effort:** 3 days  
**Impact:** HIGH (quick access tools)

---

### Secondary Components (Phase 4-5, Weeks 7-10)

#### Priority 5: Scene Hierarchy Panel (Week 7)
**Files to migrate:**
- `engine/ui/SceneHierarchyPanel.cpp/h`

**Native controls needed:**
- Dockable window
- TreeView control
- Context menu

**Complexity:** HIGH  
**Estimated effort:** 5 days  
**Impact:** MEDIUM (scene navigation)

#### Priority 6: Inspector Panel (Week 8)
**Files to migrate:**
- `engine/ui/InspectorPanel.cpp/h`

**Native controls needed:**
- Dockable window
- Property grid
- Various input controls (text, numbers, colors, etc.)

**Complexity:** VERY HIGH  
**Estimated effort:** 7 days  
**Impact:** HIGH (object editing)

#### Priority 7: Content Browser Panel (Week 9)
**Files to migrate:**
- `engine/ui/ContentBrowserPanel.cpp/h`

**Native controls needed:**
- Dockable window
- ListView (icon view, list view)
- Path navigation
- Preview pane

**Complexity:** HIGH  
**Estimated effort:** 5 days  
**Impact:** MEDIUM (asset management)

#### Priority 8: Console Panel (Week 10)
**Files to migrate:**
- `engine/ui/ConsolePanel.cpp/h`

**Native controls needed:**
- Dockable window
- Rich text control
- Command input
- Filtering

**Complexity:** MEDIUM  
**Estimated effort:** 3 days  
**Impact:** MEDIUM (debugging)

---

### Tertiary Components (Phase 6-7, Weeks 11-14)

#### Priority 9: Settings Panels (Week 11)
**Files to migrate:**
- `engine/ui/SettingsPanel.cpp/h`
- `engine/ui/EngineConfigPanel.cpp/h`
- `engine/ui/WindowsCustomizationPanel.cpp/h`

**Native controls needed:**
- Dialog window
- TabControl
- Various input controls

**Complexity:** MEDIUM  
**Estimated effort:** 4 days  
**Impact:** LOW (occasional use)

#### Priority 10: Tool Palettes (Week 12)
**Files to migrate:**
- `engine/ui/VoxelToolPalette.cpp/h`
- `engine/ui/HotbarPanel.cpp/h`

**Native controls needed:**
- Floating toolbar
- Icon buttons
- Tooltips

**Complexity:** MEDIUM  
**Estimated effort:** 3 days  
**Impact:** MEDIUM (tool selection)

#### Priority 11: Editor Manager (Weeks 13-14)
**Files to migrate:**
- `engine/editor/EditorManager.cpp/h`

**Native controls needed:**
- Window docking system
- Layout serialization
- Panel management

**Complexity:** VERY HIGH  
**Estimated effort:** 10 days  
**Impact:** HIGH (overall editor framework)

---

## 🏗️ Technical Architecture

### 1. Native UI System Architecture

```
┌─────────────────────────────────────────────────────┐
│                  Application Layer                  │
│         (EditorManager, Game Logic, etc.)           │
└──────────────────────┬──────────────────────────────┘
                       │
┌──────────────────────▼──────────────────────────────┐
│              Native UI Framework                    │
│  ┌──────────────┐  ┌──────────────┐  ┌───────────┐ │
│  │ NativeUI     │  │ Control      │  │  Theme    │ │
│  │ Context      │  │ Factory      │  │  Manager  │ │
│  └──────────────┘  └──────────────┘  └───────────┘ │
└──────────────────────┬──────────────────────────────┘
                       │
┌──────────────────────▼──────────────────────────────┐
│              Native Controls Library                │
│  ┌────────┐ ┌────────┐ ┌─────────┐ ┌─────────────┐ │
│  │ Window │ │ Button │ │ TextBox │ │ PropertyGrid│ │
│  └────────┘ └────────┘ └─────────┘ └─────────────┘ │
│  ┌────────┐ ┌────────┐ ┌─────────┐ ┌─────────────┐ │
│  │TreeView│ │ListView│ │ Toolbar │ │   MenuBar   │ │
│  └────────┘ └────────┘ └─────────┘ └─────────────┘ │
└──────────────────────┬──────────────────────────────┘
                       │
┌──────────────────────▼──────────────────────────────┐
│             Win32 API / Direct2D                    │
│  ┌────────────┐  ┌──────────────┐  ┌─────────────┐ │
│  │ HWND       │  │ GDI+/D2D     │  │  Messages   │ │
│  │ Management │  │ Rendering    │  │  WM_*       │ │
│  └────────────┘  └──────────────┘  └─────────────┘ │
└─────────────────────────────────────────────────────┘
```

### 2. Control Creation Pattern

```cpp
// Factory pattern for control creation
class NativeControlFactory {
public:
    static std::unique_ptr<NativeButton> createButton(
        HWND parent, 
        int x, int y, 
        int width, int height,
        const std::wstring& text);
    
    static std::unique_ptr<NativeTextBox> createTextBox(
        HWND parent,
        int x, int y,
        int width, int height,
        const std::wstring& placeholder);
    
    // ... other controls
};
```

### 3. Event System

```cpp
// Event callback system
class NativeControl {
public:
    using ClickCallback = std::function<void()>;
    using TextChangedCallback = std::function<void(const std::wstring&)>;
    
    void setOnClick(ClickCallback callback) { m_onClick = callback; }
    void setOnTextChanged(TextChangedCallback callback) { m_onTextChanged = callback; }
    
protected:
    ClickCallback m_onClick;
    TextChangedCallback m_onTextChanged;
};
```

---

## 🎨 Dark Theme Implementation

### Windows 11 Dark Mode Integration

```cpp
// Enable Windows 11 dark mode for window
void enableDarkMode(HWND hwnd) {
    BOOL value = TRUE;
    DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, 
                         &value, sizeof(value));
}

// Custom control rendering with dark theme
void renderDarkButton(HDC hdc, RECT rect, bool hovered, bool pressed) {
    // Background color (Unreal blue)
    COLORREF bgColor = hovered ? RGB(71, 122, 179) : RGB(51, 89, 140);
    if (pressed) bgColor = RGB(46, 82, 128);
    
    // Fill background
    HBRUSH brush = CreateSolidBrush(bgColor);
    FillRect(hdc, &rect, brush);
    DeleteObject(brush);
    
    // Draw text (bright white)
    SetTextColor(hdc, RGB(242, 244, 249));
    SetBkMode(hdc, TRANSPARENT);
    // ... draw text centered
}
```

### Color Palette (from Unreal theme)

```cpp
namespace UnrealColors {
    // Backgrounds
    constexpr COLORREF WindowBg = RGB(32, 32, 32);        // #202020
    constexpr COLORREF ChildBg = RGB(37, 37, 38);         // #252526
    constexpr COLORREF MenuBarBg = RGB(45, 45, 48);       // #2D2D30
    
    // Interactive
    constexpr COLORREF ButtonNormal = RGB(0, 122, 204);   // #007ACC
    constexpr COLORREF ButtonHover = RGB(28, 151, 234);   // #1C97EA
    constexpr COLORREF ButtonActive = RGB(0, 98, 163);    // #0062A3
    
    // Text
    constexpr COLORREF TextNormal = RGB(241, 241, 241);   // #F1F1F1
    constexpr COLORREF TextDisabled = RGB(108, 108, 108); // #6C6C6C
}
```

---

## 📅 Detailed Timeline

### **Month 1: Foundation (Weeks 1-4)**
- Week 1: Architecture design, NativeUIContext, base control class
- Week 2: Core controls (Window, Button, Label, TextBox)
- Week 3: Main Menu + World Creation Dialog migration
- Week 4: Menu Bar + Toolbar migration

**Deliverable:** Working main menu and basic editor with native controls

### **Month 2: Core Editor UI (Weeks 5-8)**
- Week 5: Advanced controls (TreeView, ListView, PropertyGrid)
- Week 6: Docking system foundation
- Week 7: Scene Hierarchy Panel migration
- Week 8: Inspector Panel migration

**Deliverable:** Core editor panels functional with native controls

### **Month 3: Feature Completion (Weeks 9-12)**
- Week 9: Content Browser Panel migration
- Week 10: Console Panel migration
- Week 11: Settings Panels migration
- Week 12: Tool Palettes migration

**Deliverable:** All UI panels migrated to native controls

### **Month 4: Polish & Integration (Weeks 13-16)**
- Week 13-14: Editor Manager docking system
- Week 15: Testing, bug fixes, performance optimization
- Week 16: Documentation, final polish

**Deliverable:** Production-ready native UI editor

---

## 🔧 Implementation Guidelines

### 1. Window Procedure Pattern

```cpp
LRESULT CALLBACK CustomButtonProc(HWND hwnd, UINT msg, 
                                  WPARAM wParam, LPARAM lParam) {
    NativeButton* button = reinterpret_cast<NativeButton*>(
        GetWindowLongPtr(hwnd, GWLP_USERDATA));
    
    switch (msg) {
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        if (button) button->onPaint(hdc);
        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_LBUTTONDOWN:
        if (button && button->m_onClick) button->m_onClick();
        return 0;
    // ... other messages
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}
```

### 2. Control Registration

```cpp
void NativeButton::registerClass() {
    static bool registered = false;
    if (registered) return;
    
    WNDCLASSEX wc = {0};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = CustomButtonProc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpszClassName = L"FreshNativeButton";
    wc.hCursor = LoadCursor(nullptr, IDC_HAND);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    
    RegisterClassEx(&wc);
    registered = true;
}
```

### 3. DPI Awareness

```cpp
int scaleDPI(int value) {
    HDC screen = GetDC(nullptr);
    int dpi = GetDeviceCaps(screen, LOGPIXELSX);
    ReleaseDC(nullptr, screen);
    return MulDiv(value, dpi, 96); // 96 is default DPI
}
```

---

## 🚀 Quick Start: Proof of Concept

### Minimal Example: Native Main Menu

```cpp
// NativeMainMenu.h
class NativeMainMenu {
public:
    bool initialize(HWND parent);
    void show();
    bool shouldCreateNewWorld() const { return m_createNewWorld; }
    
private:
    static LRESULT CALLBACK dialogProc(HWND hwnd, UINT msg, 
                                       WPARAM wParam, LPARAM lParam);
    void createControls(HWND hwnd);
    
    HWND m_hwnd;
    HWND m_btnCreateWorld;
    HWND m_btnLoadWorld;
    HWND m_btnExit;
    bool m_createNewWorld = false;
};
```

---

## ⚠️ Risks & Mitigation

### Risk 1: Development Time
**Impact:** HIGH  
**Probability:** HIGH  
**Mitigation:** 
- Phase implementation allows partial releases
- Keep ImGui available as fallback during transition
- Use preprocessor flags to enable/disable systems

### Risk 2: Feature Parity
**Impact:** MEDIUM  
**Probability:** MEDIUM  
**Mitigation:**
- Detailed feature list for each component
- Test cases to ensure all functionality preserved
- User feedback during beta testing

### Risk 3: Performance
**Impact:** LOW  
**Probability:** LOW  
**Mitigation:**
- Native controls generally faster than ImGui
- Profile and optimize custom rendering
- Use Direct2D for complex graphics

### Risk 4: Theme Consistency
**Impact:** MEDIUM  
**Probability:** MEDIUM  
**Mitigation:**
- Centralized theme management
- Consistent color palette
- Visual design mockups before implementation

---

## 📊 Success Metrics

1. **Functionality:** All ImGui features replaced 1:1
2. **Performance:** UI frame time < 2ms (better than ImGui)
3. **Visual:** Matches or exceeds Unreal Engine aesthetic
4. **Accessibility:** Full keyboard navigation and screen reader support
5. **Integration:** Native Windows features (taskbar, jump lists, etc.)

---

## 🔄 Migration Path Options

### Option A: Big Bang (Not Recommended)
- Replace all ImGui at once
- **Risk:** Very high
- **Timeline:** 4+ months of non-functional editor

### Option B: Gradual Component Replacement (Recommended)
- Replace one component at a time
- **Risk:** Moderate
- **Timeline:** 4 months with continuous functionality
- **This document follows this approach**

### Option C: Parallel Implementation
- Build native UI alongside ImGui
- Switch with feature flag
- **Risk:** Low but resource intensive
- **Timeline:** 5-6 months (more work)

---

## 📚 Resources & References

### Windows API Documentation
- [Win32 API Reference](https://docs.microsoft.com/en-us/windows/win32/api/)
- [Common Controls](https://docs.microsoft.com/en-us/windows/win32/controls/window-controls)
- [Direct2D](https://docs.microsoft.com/en-us/windows/win32/direct2d/direct2d-portal)

### Dark Theme Implementation
- [Windows 11 Dark Mode](https://docs.microsoft.com/en-us/windows/apps/desktop/modernize/apply-windows-themes)
- [DWM API](https://docs.microsoft.com/en-us/windows/win32/dwm/dwm-overview)

### Existing Codebase References
- `engine/core/Win32Window.cpp` - Native window implementation
- `engine/ui/native/Win32MenuBar.cpp` - Native menu bar
- `engine/ui/WindowsThemeManager.cpp` - Theme management

---

## 🎯 Next Steps

1. **Review and Approve** this roadmap
2. **Set up development branch** for native UI work
3. **Week 1: Start Foundation** - NativeUIContext and base classes
4. **Week 2-3: Proof of Concept** - Native Main Menu implementation
5. **Week 4: Demo Review** - Assess progress and adjust timeline

---

## ✅ Checklist for Component Migration

For each component being migrated:

- [ ] Document current ImGui features and interactions
- [ ] Design native control layout and hierarchy
- [ ] Implement native controls with event handlers
- [ ] Apply dark theme styling
- [ ] Implement all original features
- [ ] Add keyboard navigation
- [ ] Test on different DPI settings
- [ ] Update unit tests
- [ ] Update documentation
- [ ] Code review
- [ ] QA testing

---

**Document Status:** DRAFT - Ready for Review  
**Contact:** Development Team  
**Last Updated:** 2025-11-18
