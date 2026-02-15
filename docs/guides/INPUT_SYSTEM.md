# Input System Documentation

## Overview

The Fresh Voxel Engine features a comprehensive input management system with support for multiple input modes, keyboard and mouse controls, and an innovative "Hold Alt" feature for seamless mode switching.

## Input Modes

The engine supports three distinct input modes:

### 1. GameMode (Default)
- **Purpose**: Standard in-game gameplay
- **Cursor**: Hidden and captured (locked to center of screen)
- **Controls**: 
  - WASD for movement
  - Mouse for camera control
  - Left/Right click for block interaction
  - All game controls active
- **Use Case**: Normal gameplay, exploration, building

### 2. UIMode
- **Purpose**: Menu interaction and UI navigation
- **Cursor**: Visible and free to move
- **Controls**:
  - Mouse fully interactive with UI elements
  - Keyboard navigation still available
  - Game movement controls disabled
- **Use Case**: Main menu, pause menu, settings, editor UI

### 3. BuildMode
- **Purpose**: Specialized mode for ship builder/construction
- **Cursor**: Visible and free to move
- **Controls**:
  - Mouse for selection, placement, rotation
  - Specialized build controls active
  - Movement controls modified for building context
- **Use Case**: Ship editor, advanced construction tools

## Hold Alt Feature

The **Hold Alt** feature enables seamless temporary switching between modes:

### How It Works
1. **In GameMode**: Press and hold the Alt key (Left Alt or Right Alt)
2. **Temporary Switch**: The input mode temporarily changes to UIMode
3. **Cursor Appears**: The mouse cursor becomes visible and free
4. **Interact with UI**: Click on in-game overlays, buttons, or UI elements
5. **Release Alt**: Input mode returns to GameMode, cursor disappears

### Use Cases
- Quickly access in-game menus without fully pausing
- Interact with HUD elements without leaving gameplay
- Adjust settings on-the-fly
- Check inventory or map while maintaining game context

### Technical Details
```cpp
// The InputManager automatically handles Alt-hold mode switching in update()
void InputManager::update() {
    // ...
    bool altHeld = isAltHeld();
    if (altHeld && currentMode == InputMode::GameMode) {
        // Temporarily switch to UI mode
        currentMode = InputMode::UIMode;
        setCursorMode(false);  // Show cursor
    } else if (!altHeld && temporaryModeSet) {
        // Return to base mode
        currentMode = baseMode;
        setCursorMode(true);   // Hide cursor
    }
}
```

## Input Manager API

### Setting Input Mode
```cpp
// Set input mode explicitly
inputManager->setInputMode(InputMode::GameMode);
inputManager->setInputMode(InputMode::UIMode);
inputManager->setInputMode(InputMode::BuildMode);

// With temporary flag (can be overridden by Alt hold)
inputManager->setInputMode(InputMode::UIMode, /*temporary=*/true);
```

### Querying Input Mode
```cpp
// Get current input mode
InputMode mode = inputManager->getInputMode();

// Check if currently in UI mode (including Alt-hold)
bool inUI = inputManager->isInUIMode();

// Check if Alt is being held
bool altHeld = inputManager->isAltHeld();
```

### Cursor Control
```cpp
// Manual cursor control (usually handled automatically by mode)
inputManager->setCursorMode(true);   // Capture and hide
inputManager->setCursorMode(false);  // Free and show
```

## Integration with Engine

### Automatic Mode Switching

The engine automatically manages input modes in these scenarios:

1. **Editor Toggle (T key)**:
   ```cpp
   if (m_editorManager->isVisible()) {
       m_inputManager->setInputMode(InputMode::UIMode);
   } else {
       m_inputManager->setInputMode(InputMode::GameMode);
   }
   ```

2. **Menu Opening (ESC key)**:
   - Switches to UIMode when pause menu opens
   - Returns to GameMode when menu closes

3. **Build Menu**:
   - Switches to BuildMode when ship editor opens
   - Returns to previous mode when closed

### Game Logic Integration

Player input is only processed in GameMode:

```cpp
bool inGameMode = (m_inputManager->getInputMode() == InputMode::GameMode);

if (m_player && inGameMode) {
    m_player->handleInput(*m_inputManager, deltaTime);
    
    // Handle mouse camera control
    glm::vec2 mouseDelta = m_inputManager->getMouseDelta();
    m_player->handleMouseMovement(mouseDelta.x, mouseDelta.y);
}
```

## Mouse Button Handling

### In GameMode
- **Left Click**: Break/attack
- **Right Click**: Place blocks
- Mouse movement controls camera

### In UIMode
- **Left Click**: UI selection, button clicks
- **Right Click**: Context menus (future)
- Mouse movement moves cursor freely

### In BuildMode
- **Left Click**: Select components
- **Right Click**: Rotate/context actions
- **Drag**: Move components
- Mouse wheel: Zoom or cycle options

## Keyboard Shortcuts

### Global (All Modes)
- **ESC**: Open menu / Return to previous
- **T**: Toggle editor

### GameMode Only
- **W/A/S/D**: Movement
- **Space**: Jump
- **Left Shift**: Sprint
- **Left Ctrl**: Crouch
- **E**: Use/Interact
- **Tab**: Open inventory
- **Enter**: Open chat

### UIMode / BuildMode
- **Arrow Keys**: Navigate menu items
- **Enter**: Confirm selection
- **ESC**: Close menu / Cancel

## Best Practices

### For Gameplay Code
```cpp
// Always check input mode before processing game input
if (inputManager->getInputMode() == InputMode::GameMode) {
    // Process movement, combat, etc.
}
```

### For UI Code
```cpp
// UI should respond in both UIMode and when Alt is held
if (inputManager->isInUIMode()) {
    // Process UI mouse clicks
    if (inputManager->isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT)) {
        handleButtonClick();
    }
}
```

### For Menu Systems
```cpp
// Set mode when opening menu
void openPauseMenu() {
    inputManager->setInputMode(InputMode::UIMode);
    showMenu();
}

// Restore mode when closing
void closePauseMenu() {
    inputManager->setInputMode(InputMode::GameMode);
    hideMenu();
}
```

## Future Enhancements

### Planned Features
1. **Custom Cursor Sprites**: Replace OS cursor with in-game sprite
2. **Multi-Monitor Support**: Keep cursor captured within game window
3. **Customizable Hold Key**: Allow rebinding Alt to another modifier
4. **Touch Input**: Support for touchscreens and tablets
5. **Gesture Recognition**: Advanced mouse/touch gestures
6. **Context-Sensitive Cursors**: Change cursor based on what's under it

### Configuration Options
Future versions will support:
- Hold duration threshold before mode switch
- Cursor sensitivity adjustment per mode
- Custom cursor graphics per mode
- Input mode transition animations

## Troubleshooting

### Cursor Not Appearing When Holding Alt
- Ensure Alt key is recognized (check keyboard layout)
- Verify input manager is being updated each frame
- Check that GameMode is active before holding Alt

### Cursor Stuck in One Mode
- Release all modifier keys
- Press ESC to reset to menu mode
- Toggle editor (T) to force mode switch
- Restart if issue persists

### Mouse Not Working in UI
- Verify UIMode is active: `inputManager->getInputMode()`
- Check that UI elements have proper event handlers
- Ensure GUI isn't blocking input capture

## Example Usage

### Complete Integration Example
```cpp
// In your game initialization
inputManager->initialize(window);
inputManager->setInputMode(InputMode::GameMode);

// In your update loop
inputManager->update();  // Handles Alt-hold automatically

// Check mode and handle accordingly
switch (inputManager->getInputMode()) {
    case InputMode::GameMode:
        updatePlayer();
        updateGameplay();
        break;
    
    case InputMode::UIMode:
        updateMenus();
        processUIInput();
        break;
    
    case InputMode::BuildMode:
        updateBuildSystem();
        processBuildInput();
        break;
}
```

## See Also

- [Controls Documentation](../CONTROLS.md) - Complete control reference
- [Editor Guide](EDITOR_INTEGRATION.md) - Editor-specific controls
- [Input Manager API](../engine/input/InputManager.h) - Full API reference
