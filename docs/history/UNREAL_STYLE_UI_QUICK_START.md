# Quick Start: Unreal-Style Native UI

## For Developers

### Using the New Components

#### 1. Win32HUD (In-Game Heads-Up Display)

```cpp
#include "ui/native/Win32HUD.h"

// Initialize
Win32HUD hud;
hud.initialize(hwnd);
hud.setVisible(true);

// Configure what to show
hud.setShowCrosshair(true);
hud.setShowHealthBar(true);
hud.setShowStaminaBar(true);
hud.setShowHotbar(true);
hud.setShowMinimap(true);
hud.setShowDebugInfo(true);

// Update game statistics
Win32HUD::HUDStats stats;
stats.health = 75.0f;
stats.maxHealth = 100.0f;
stats.stamina = 80.0f;
stats.maxStamina = 100.0f;
stats.fps = 60;
stats.posX = 123.4f;
stats.posY = 56.7f;
stats.posZ = 89.0f;
hud.updateStats(stats);

// Set hotbar slots
Win32HUD::HotbarSlot slot;
slot.isEmpty = false;
slot.itemName = "Stone";
slot.count = 64;
hud.setSlot(0, slot);  // Slot 0 (key '1')

// Render HUD (call in your render loop)
HDC hdc = GetDC(hwnd);
RECT clientRect;
GetClientRect(hwnd, &clientRect);
hud.render(hdc, clientRect);
ReleaseDC(hwnd, hdc);

// Handle keyboard input for hotbar
hud.handleKeyPress('1');  // Select slot 0
hud.handleKeyPress('2');  // Select slot 1
// etc.

// Set callback for slot activation
hud.setSlotActivatedCallback([](int slotIndex) {
    // Handle slot activation (e.g., change selected block type)
    LOG_INFO("Slot " + std::to_string(slotIndex) + " activated");
});
```

#### 2. UnrealStyleTheme (Color Constants)

```cpp
#include "ui/native/UnrealStyleTheme.h"

// Use Unreal colors in your UI code
COLORREF bgColor = UnrealStyleTheme::WindowBackground;     // #202020
COLORREF accentColor = UnrealStyleTheme::AccentBlue;       // #007ACC
COLORREF textColor = UnrealStyleTheme::TextPrimary;        // #F1F1F1

// Create brushes with Unreal colors
HBRUSH brush = CreateSolidBrush(UnrealStyleTheme::ButtonNormal);

// Use spacing constants
int padding = UnrealStyleTheme::PaddingMedium;  // 8px
int iconSize = UnrealStyleTheme::IconMedium;    // 24px
```

#### 3. Native Menu Bar

The menu bar is automatically set up in `Engine::setupNativeMenuBar()`. To add new menu items:

```cpp
// In Engine::setupNativeMenuBar()
int myMenu = menuBar->addMenu("MyMenu");
menuBar->addMenuItem(myMenu, "My Action\tCtrl+M", [this]() {
    LOG_INFO_C("My Action triggered", "Engine");
    // Your action code here
});
menuBar->addSeparator(myMenu);
```

#### 4. Native Toolbar

The toolbar is automatically set up in `Engine::setupNativeToolbar()`. To add new buttons:

```cpp
// In Engine::setupNativeToolbar()
toolbar->addButton(6000, "MyTool", nullptr, [this]() {
    LOG_INFO_C("MyTool button clicked", "Engine");
    // Your tool activation code here
});
toolbar->addSeparator();  // Add separator between button groups
```

### Keyboard Shortcuts Reference

#### File Operations
- `Ctrl+N` - New World
- `Ctrl+O` - Open World
- `Ctrl+S` - Save
- `Ctrl+Shift+S` - Save As
- `Ctrl+Shift+A` - Save All
- `Ctrl+I` - Import
- `Ctrl+E` - Export
- `Alt+F4` - Exit

#### Edit Operations
- `Ctrl+Z` - Undo
- `Ctrl+Y` - Redo
- `Ctrl+X` - Cut
- `Ctrl+C` - Copy
- `Ctrl+V` - Paste
- `Del` - Delete
- `Ctrl+A` - Select All
- `Ctrl+D` - Deselect All
- `Ctrl+F` - Find

#### View Operations
- `1` - Perspective View
- `2` - Top View
- `3` - Front View
- `4` - Side View
- `Home` - Reset Camera
- `Ctrl+B` - Content Browser
- `Ctrl+H` - Scene Hierarchy
- `Ctrl+I` - Inspector
- `~` - Console
- `F11` - Toggle Fullscreen
- `U` - Toggle UI
- `Ctrl+Shift+S` - Show Stats

#### World Operations
- `Alt+P` - Play
- `Alt+Pause` - Pause
- `Esc` - Stop

#### Tool Activation
- `B` - Brush Tool
- `P` - Paint Tool
- `S` - Sculpt Tool
- `M` - Smooth Tool
- `V` - Select Tool
- `W` - Move Tool
- `E` - Rotate Tool
- `R` - Scale Tool

#### Help
- `F1` - Documentation
- `Ctrl+?` - Keyboard Shortcuts

### Color Palette Quick Reference

```cpp
// Backgrounds
WindowBackground     = #202020  // Main dark gray
PanelBackground      = #252526  // Slightly lighter
DarkBackground       = #1C1C1C  // Darkest
MenuBarBackground    = #2D2D30  // Menu/toolbar

// Interactive
AccentBlue           = #007ACC  // Primary interactive (Unreal signature)
AccentBlueHover      = #1C97EA  // Hover state
AccentBlueActive     = #0062A3  // Active/pressed
ButtonNormal         = #3C3C3C  // Standard button

// Text
TextPrimary          = #F1F1F1  // High contrast white
TextSecondary        = #B4B4B4  // Labels, secondary
TextDisabled         = #6C6C6C  // Inactive elements
TextHighlight        = #FFFFFF  // Pure white emphasis

// Status
StatusSuccess        = #49BE46  // Green
StatusWarning        = #FFC20A  // Yellow/gold
StatusError          = #E81123  // Red
StatusInfo           = #007ACC  // Blue

// HUD Specific
HUDHealthBar         = #DC3232  // Red
HUDStaminaBar        = #78C850  // Green
HUDManaBar           = #5078C8  // Blue
Crosshair            = #FFFFFF  // White
```

### Best Practices

#### 1. Always Use Theme Colors
```cpp
// ❌ DON'T hardcode colors
HBRUSH brush = CreateSolidBrush(RGB(0, 122, 204));

// ✅ DO use theme constants
HBRUSH brush = CreateSolidBrush(UnrealStyleTheme::AccentBlue);
```

#### 2. Respect Platform Guards
```cpp
// ❌ DON'T forget platform guards
void MyFunction() {
    Win32HUD hud;  // Will fail on non-Windows!
}

// ✅ DO use proper guards
void MyFunction() {
#ifdef _WIN32
    Win32HUD hud;
#endif
}
```

#### 3. Clean Up GDI Resources
```cpp
// ❌ DON'T leak GDI objects
HBRUSH brush = CreateSolidBrush(color);
// ... use brush ...
// (no DeleteObject!)

// ✅ DO clean up
HBRUSH brush = CreateSolidBrush(color);
// ... use brush ...
DeleteObject(brush);
```

#### 4. Use Smart Pointers
```cpp
// ❌ DON'T use raw pointers
Win32HUD* hud = new Win32HUD();
// ... use hud ...
delete hud;

// ✅ DO use smart pointers
std::unique_ptr<Win32HUD> hud = std::make_unique<Win32HUD>();
// ... use hud ...
// (automatic cleanup)
```

### Common Tasks

#### Adding a New Menu
1. Open `engine/core/Engine.cpp`
2. Find `setupNativeMenuBar()` function
3. Add your menu:
```cpp
int myMenu = menuBar->addMenu("MyMenu");
menuBar->addMenuItem(myMenu, "Action 1", []() { /* ... */ });
menuBar->addMenuItem(myMenu, "Action 2", []() { /* ... */ });
```

#### Adding a New Toolbar Button
1. Open `engine/core/Engine.cpp`
2. Find `setupNativeToolbar()` function
3. Add your button:
```cpp
toolbar->addButton(9999, "MyButton", nullptr, []() {
    // Button action
});
```

#### Customizing HUD Appearance
1. Open `engine/ui/native/Win32HUD.cpp`
2. Find rendering functions (e.g., `renderHotbar()`)
3. Modify colors, sizes, positions as needed
4. Use `UnrealStyleTheme` constants for consistency

#### Changing ImGui Colors
1. Open `engine/ui/ImGuiContext.cpp`
2. Find the color setup section
3. Use Unreal theme colors:
```cpp
colors[ImGuiCol_Button] = ImVec4(0.000f, 0.478f, 0.800f, 1.00f);  // #007ACC
```

### Integration Checklist

To fully integrate the new UI system:

- [ ] Add Win32HUD to Engine class as member variable
- [ ] Initialize HUD in Engine::initialize()
- [ ] Update HUD stats in Engine::update()
- [ ] Render HUD in Engine::render() (after world, before ImGui)
- [ ] Connect hotbar to voxel placement system
- [ ] Implement menu action callbacks
- [ ] Implement toolbar button callbacks
- [ ] Add toolbar icons (16x16 PNG files)
- [ ] Test all keyboard shortcuts
- [ ] Verify high DPI scaling
- [ ] Test with dark/light Windows themes

### Troubleshooting

**Q: Menu items don't respond**
A: Check that callbacks are properly connected in `setupNativeMenuBar()`

**Q: Toolbar buttons are missing**
A: Ensure `setupNativeToolbar()` is called during initialization

**Q: HUD not rendering**
A: Verify HUD is visible (`setVisible(true)`) and render is called with valid HDC

**Q: Colors look wrong**
A: Check that you're using UnrealStyleTheme constants, not hardcoded RGB values

**Q: Build fails on Linux/macOS**
A: Ensure all Win32 code is wrapped in `#ifdef _WIN32` blocks

## For Designers

### Color Specifications

All colors follow Unreal Engine 5 standards:

- **Primary Background**: `#202020` (RGB 32, 32, 32) - Professional charcoal
- **Accent Color**: `#007ACC` (RGB 0, 122, 204) - Unreal signature blue
- **Primary Text**: `#F1F1F1` (RGB 241, 241, 241) - High contrast white
- **Contrast Ratio**: 15:1+ (WCAG AAA compliant)

### Typography

- **Main Font**: Segoe UI (Windows standard)
- **Code Font**: Consolas (for technical info)
- **Sizes**: 8pt (small), 9pt (normal), 10pt (large), 12pt (titles)
- **Weight**: Medium (500) for headings, Normal (400) for body

### Spacing

- **Small**: 4px - Between related items
- **Medium**: 8px - Standard padding
- **Large**: 12px - Between sections
- **Border**: 1px - Subtle separation

### Layout Principles

1. **Sharp Corners**: No border radius (Unreal style)
2. **Compact**: Efficient use of space
3. **Grouped**: Related tools together with separators
4. **Consistent**: Same spacing throughout
5. **Accessible**: High contrast, keyboard navigation

## Documentation

- **Full Details**: See `UNREAL_STYLE_NATIVE_UI.md`
- **Implementation**: See `IN_GAME_UI_IMPLEMENTATION_SUMMARY.md`
- **Color Theme**: See `engine/ui/native/UnrealStyleTheme.h`
- **HUD API**: See `engine/ui/native/Win32HUD.h`

## Support

For questions or issues:
1. Check the documentation files listed above
2. Review code examples in this guide
3. Examine existing implementations in Engine.cpp
4. Submit an issue on GitHub with "[UI]" tag

---

**Version**: 1.0.0  
**Last Updated**: November 16, 2025  
**Compatibility**: Windows 10/11 (x64)
