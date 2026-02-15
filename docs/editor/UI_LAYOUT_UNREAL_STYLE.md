# UI Layout Changes - Unreal Engine Style

## Overview
The editor UI has been reorganized to match Unreal Engine's layout for a more professional and intuitive editing experience.

## New Layout Structure

```
┌────────────────────────────────────────────────────────────────────────┐
│  Menu Bar + Toolbar (80px height)                                      │
├───────┬────────────────────────────────────────────────┬───────────────┤
│       │                                                │               │
│ Left  │                                                │  Right Top    │
│ Panel │            Center Viewport                     │  (Outliner)   │
│       │          (3D Scene View)                       │   40% height  │
│ Tools │                                                │               │
│ 220px │                                                ├───────────────┤
│       │                                                │               │
│       │                                                │  Right Bottom │
│       │                                                │  (Inspector)  │
│       │                                                │   60% height  │
│       │                                                │               │
│       │                                                │  350px width  │
├───────┴────────────────────┬───────────────────────────┴───────────────┤
│                            │                                           │
│  Content Browser (700px)   │         Console                          │
│                            │                                           │
│  Bottom Panel (220px height)                                          │
└────────────────────────────────────────────────────────────────────────┘
```

## Panel Details

### Top Area
- **Menu Bar + Toolbar**: 80px height (unchanged)
  - Contains menu items and tool icons
  - Always visible at top of window

### Left Side
- **Terraforming/Tools Panel**: 220px width, full height
  - Previously was split into multiple panels on left
  - Now unified narrow panel like Unreal's "Place Actors" panel
  - Contains all terraforming tools and voxel placement controls
  - Extends from toolbar to bottom panel

### Center
- **Viewport Panel**: Largest area, dynamically sized
  - 3D scene view for editing and visualization
  - Positioned between left and right panels
  - Extends from toolbar to bottom panel
  - Automatically resizes with window

### Right Side (Split Vertically)
- **Scene Hierarchy Panel (Top)**: 350px width, 40% of available height
  - World Outliner equivalent
  - Shows entity/object hierarchy
  - Tree view of world structure
  
- **Inspector Panel (Bottom)**: 350px width, 60% of available height
  - Details panel equivalent
  - Shows properties of selected objects
  - Property grid for editing

### Bottom
- **Content Browser Panel (Left)**: 700px width, 220px height
  - Asset browser and file management
  - Shows available assets and resources
  
- **Console Panel (Right)**: Remaining width, 220px height
  - Command console and output log
  - Dynamically sized to fill remaining space

## Layout Constants

### Panel Dimensions
```cpp
constexpr int PANEL_MARGIN = 5;              // Tight spacing like Unreal
constexpr int TOOLBAR_HEIGHT = 80;           // Menu/toolbar at top
constexpr int LEFT_PANEL_WIDTH = 220;        // Tools panel
constexpr int RIGHT_PANEL_WIDTH = 350;       // Outliner + Inspector
constexpr int BOTTOM_PANEL_HEIGHT = 220;     // Content + Console
constexpr int CONTENT_BROWSER_WIDTH = 700;   // Content browser width
```

### Split Ratios
```cpp
constexpr float OUTLINER_HEIGHT_RATIO = 0.40f;   // 40% for Outliner
constexpr float INSPECTOR_HEIGHT_RATIO = 0.60f;  // 60% for Inspector
```

## Key Improvements

1. **More Intuitive Layout**: Matches industry-standard Unreal Engine layout that users are familiar with
2. **Better Space Utilization**: Viewport takes center stage with supporting panels around it
3. **Logical Grouping**: Related panels are positioned together (Outliner above Inspector)
4. **Consistent Proportions**: Layout maintains proportions when window is resized
5. **Professional Appearance**: Tight margins and proper spacing like professional tools

## Dynamic Resizing

The `onWindowResize()` method now:
- Recalculates all panel positions based on new window size
- Maintains proportions (40/60 split for right panels)
- Ensures minimum viewport size
- Adjusts console width to fill available space
- Keeps left and right panels at fixed widths

## Comparison with Previous Layout

### Before (Old Layout)
- Left side: Inspector (350px) + Scene Hierarchy (300px) side by side
- Bottom: Content Browser (660px) + Console (variable) side by side
- Viewport: Right side only
- Total left width: 680px (too wide for tools)

### After (Unreal Style)
- Left side: Tools panel (220px) - much narrower
- Right side: Outliner (top) + Inspector (bottom) - logical vertical split
- Bottom: Content Browser + Console - better proportioned
- Viewport: True center position - more space for 3D work

## Technical Implementation

### Files Modified
- `engine/editor/EditorManager.cpp`:
  - Updated panel layout constants (lines 95-110)
  - Reorganized panel creation code (lines 488-574)
  - Updated `onWindowResize()` method (lines 1730-1801)

### Panel Creation Order
1. Calculate window dimensions
2. Calculate all panel positions upfront
3. Create Terraforming Panel (left)
4. Create Scene Hierarchy Panel (right top)
5. Create Inspector Panel (right bottom)
6. Create Content Browser Panel (bottom left)
7. Create Console Panel (bottom right)
8. Create Viewport Panel (center)
9. Create Status Bar (very bottom)

### Resize Behavior
- Window resize triggers recalculation of all panel positions
- Each panel is repositioned and resized via `setPosition()` and `setSize()`
- Proportions are maintained regardless of window size
- Minimum sizes prevent panels from becoming unusably small

## Testing

To test the new layout:
1. Build the project with Visual Studio 2022 on Windows
2. Run FreshVoxelEngine.exe
3. Verify panel positions match Unreal Engine style
4. Resize window and verify panels maintain proportions
5. Check that all panels are visible and functional
6. Verify viewport is centered and takes most space

## Future Enhancements

Possible improvements:
- Add splitter controls for user-adjustable panel sizes
- Save/load custom panel layouts
- Add panel docking and undocking
- Support for floating panels
- Tab groups for multiple panels in same area
