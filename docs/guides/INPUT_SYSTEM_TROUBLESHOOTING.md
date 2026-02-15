# Input System Troubleshooting Guide

## Reported Issues

User reported the following issues:
1. Jump doesn't work
2. Mouse look in game doesn't work  
3. T key does not enter editor

## Code Analysis Results

### ✅ Input System Implementation is Correct

After thorough code review, the input system implementation appears to be correct:

#### 1. Key Bindings (InputManager.cpp lines 264-288)

```cpp
void InputManager::initializeDefaultBindings()
{
    keyBindings[InputAction::Jump] = GLFW_KEY_SPACE;          // Space → Jump
    keyBindings[InputAction::ToggleEditor] = GLFW_KEY_T;       // T → Toggle Editor
    keyBindings[InputAction::MoveForward] = GLFW_KEY_W;       // WASD movement
    // ... other bindings
}
```

✅ Keys are correctly mapped

#### 2. Input Processing (Engine.cpp lines 443-519)

```cpp
void Engine::processInput()
{
    m_inputManager->update();
    
    // Toggle editor with 'T' key
    if (!guiCapturesKeyboard && 
        m_inputManager->isActionJustPressed(InputAction::ToggleEditor)) {
        if (m_editorManager) {
            m_editorManager->toggle();
            // Switch input modes appropriately
        }
    }
}

void Engine::update(float deltaTime)
{
    bool inGameMode = (m_inputManager->getInputMode() == InputMode::GameMode);
    
    if (m_player && !guiCapturesMouse && !guiCapturesKeyboard && inGameMode) {
        m_player->handleInput(*m_inputManager, deltaTime);  // Processes jump
        
        // Handle mouse movement
        glm::vec2 mouseDelta = m_inputManager->getMouseDelta();
        if (glm::length(mouseDelta) > 0.0f) {
            m_player->handleMouseMovement(mouseDelta.x, mouseDelta.y);
        }
    }
}
```

✅ Input is correctly forwarded to player and editor

#### 3. Jump Implementation (Player.cpp lines 92-95)

```cpp
// Jump
if (input.isActionJustPressed(InputAction::Jump) && isGrounded && !isCrouching) {
    velocity.y = jumpVelocity;
    isGrounded = false;
}
```

✅ Jump logic is correct

#### 4. Mouse Look (Player.cpp lines 98-101)

```cpp
void Player::handleMouseMovement(float xOffset, float yOffset)
{
    camera.processMouseMovement(xOffset * mouseSensitivity, -yOffset * mouseSensitivity);
}
```

✅ Mouse movement is correctly processed

#### 5. Input Callbacks (Engine.cpp lines 694-743)

```cpp
void Engine::setupInputCallbacks()
{
    // Keyboard callback
    glfwSetKeyCallback(window, [](GLFWwindow* win, int key, int scancode, int action, int mods) {
        CallbackUserData* data = static_cast<CallbackUserData*>(glfwGetWindowUserPointer(win));
        if (data && data->inputManager) {
            data->inputManager->processKeyEvent(key, action);
        }
    });
    
    // Mouse movement callback  
    glfwSetCursorPosCallback(window, [](GLFWwindow* win, double xpos, double ypos) {
        CallbackUserData* data = static_cast<CallbackUserData*>(glfwGetWindowUserPointer(win));
        if (data && data->inputManager) {
            data->inputManager->processMouseMovement(xpos, ypos);
        }
    });
    
    // Mouse button callback
    glfwSetMouseButtonCallback(window, [...]);
}
```

✅ GLFW callbacks are correctly set up

## Likely Root Causes

Since the code is correct, the issue is likely runtime/environmental:

### 1. Input Mode Stuck in Wrong State

**Possible Issue:** Input mode might be stuck in `UIMode` instead of `GameMode`

**Check:**
```cpp
// In Engine or main loop
LOG_INFO("Current input mode: " + 
    (m_inputManager->getInputMode() == InputMode::GameMode ? "Game" : "UI"));
```

**Fix:** Ensure input mode is set to GameMode after world loads:
```cpp
m_inputManager->setInputMode(InputMode::GameMode);
```

### 2. ImGui Capturing All Input

**Possible Issue:** ImGui might be capturing keyboard/mouse even when editor is hidden

**Check:**
```cpp
bool guiWantsMouse = m_editorManager && m_editorManager->wantCaptureMouse();
bool guiWantsKeyboard = m_editorManager && m_editorManager->wantCaptureKeyboard();

LOG_INFO("GUI captures mouse: " + std::to_string(guiWantsMouse));
LOG_INFO("GUI captures keyboard: " + std::to_string(guiWantsKeyboard));
```

**Fix:** Ensure ImGui doesn't capture when editor is hidden:
```cpp
if (m_editorManager && !m_editorManager->isVisible()) {
    // Force ImGui to not capture
    ImGuiIO& io = ImGui::GetIO();
    io.WantCaptureMouse = false;
    io.WantCaptureKeyboard = false;
}
```

### 3. Window Not Focused

**Possible Issue:** GLFW window doesn't have input focus

**Check:** Ensure window is focused after creation
```cpp
glfwFocusWindow(window);
```

### 4. Cursor Not Captured

**Possible Issue:** Cursor mode not set correctly

**Check:**
```cpp
bool captured = m_inputManager->isCursorCaptured();
LOG_INFO("Cursor captured: " + std::to_string(captured));
```

**Fix:**
```cpp
m_inputManager->setCursorMode(true);  // Capture cursor for game mode
```

### 5. Player Not Initialized

**Possible Issue:** Player object not properly initialized with world reference

**Check:**
```cpp
if (!m_player) {
    LOG_ERROR("Player is null!");
}
if (!m_player->getWorld()) {
    LOG_ERROR("Player has no world reference!");
}
```

**Fix:**
```cpp
m_player->setWorld(m_world.get());
```

### 6. Physics/Grounded State Issue

**Possible Issue:** Player never becomes grounded, preventing jump

**Check:**
```cpp
LOG_INFO("Player grounded: " + std::to_string(m_player->isPlayerGrounded()));
LOG_INFO("Player position Y: " + std::to_string(m_player->getPosition().y));
```

**Debug:** Add temporary logging to jump attempt:
```cpp
if (input.isActionJustPressed(InputAction::Jump)) {
    LOG_INFO("Jump key pressed!");
    if (!isGrounded) LOG_WARNING("Not grounded, can't jump");
    if (isCrouching) LOG_WARNING("Crouching, can't jump");
}
```

## Debugging Steps

### Step 1: Add Diagnostic Logging

Add to Engine::processInput():
```cpp
void Engine::processInput()
{
    m_window->pollEvents();
    m_inputManager->update();
    
    // DIAGNOSTIC LOGGING
    static int frameCount = 0;
    if (++frameCount % 60 == 0) {  // Log every 60 frames
        LOG_INFO("=== Input Diagnostics ===");
        LOG_INFO("Input Mode: " + 
            std::to_string(static_cast<int>(m_inputManager->getInputMode())));
        LOG_INFO("Cursor Captured: " + 
            std::to_string(m_inputManager->isCursorCaptured()));
        
        if (m_player) {
            LOG_INFO("Player Grounded: " + 
                std::to_string(m_player->isPlayerGrounded()));
        }
        
        #ifdef FRESH_IMGUI_AVAILABLE
        if (m_editorManager) {
            LOG_INFO("Editor Visible: " + 
                std::to_string(m_editorManager->isVisible()));
            LOG_INFO("GUI Captures Mouse: " + 
                std::to_string(m_editorManager->wantCaptureMouse()));
            LOG_INFO("GUI Captures Keyboard: " + 
                std::to_string(m_editorManager->wantCaptureKeyboard()));
        }
        #endif
    }
    
    // ... rest of function
}
```

### Step 2: Test Individual Systems

Create a test harness:
```cpp
// Test key detection
if (m_inputManager->isKeyPressed(GLFW_KEY_SPACE)) {
    LOG_INFO("SPACE key is pressed (raw)");
}

// Test action mapping
if (m_inputManager->isActionActive(InputAction::Jump)) {
    LOG_INFO("Jump action is active");
}

// Test mouse delta
glm::vec2 delta = m_inputManager->getMouseDelta();
if (glm::length(delta) > 0.0f) {
    LOG_INFO("Mouse delta: " + std::to_string(delta.x) + ", " + std::to_string(delta.y));
}
```

### Step 3: Force Known Good State

Add to initialization:
```cpp
// After world loads and player is created
m_inputManager->setInputMode(InputMode::GameMode);
m_inputManager->setCursorMode(true);
glfwFocusWindow(m_window->getHandle());

#ifdef FRESH_IMGUI_AVAILABLE
if (m_editorManager) {
    m_editorManager->hide();  // Ensure editor starts hidden
}
#endif

LOG_INFO("Forced input to known good state");
```

### Step 4: Verify Build Configuration

Ensure the engine was built with correct flags:
```bash
# Check if ImGui is enabled
grep "FRESH_IMGUI_AVAILABLE" build/*

# Verify input system is compiled
ls build/CMakeFiles/FreshVoxelEngine.dir/engine/input/
```

## Quick Fixes to Try

### Fix 1: Force Game Mode on Startup

In `Engine::initialize()` after player creation:
```cpp
m_inputManager->setInputMode(InputMode::GameMode);
m_inputManager->setCursorMode(true);
```

### Fix 2: Disable Editor Input Capture

In `Engine::processInput()`:
```cpp
#ifdef FRESH_IMGUI_AVAILABLE
// Override ImGui input capture when editor is hidden
if (m_editorManager && !m_editorManager->isVisible()) {
    ImGui::GetIO().WantCaptureMouse = false;
    ImGui::GetIO().WantCaptureKeyboard = false;
}
#endif
```

### Fix 3: Increase Mouse Sensitivity

Default might be too low:
```cpp
m_player->setMouseSensitivity(0.1f);  // Increase if mouse look seems absent
```

### Fix 4: Verify Player Ground Check

In `Player::checkGrounded()`:
```cpp
bool Player::checkGrounded()
{
    if (!world) {
        LOG_WARNING("checkGrounded: No world reference!");
        return false;  // This would prevent jumping!
    }
    // ... rest of function
}
```

## Testing Checklist

- [ ] Build completes without errors
- [ ] Engine starts and shows main menu
- [ ] World loads successfully
- [ ] Console shows "Input mode: Game" or similar
- [ ] Console shows "Cursor captured: true"
- [ ] Player spawns at valid position (not falling through world)
- [ ] WASD movement works (if jump doesn't, but movement does, different issue)
- [ ] Mouse movement changes view direction
- [ ] Pressing T shows console message about editor toggle
- [ ] Pressing Space shows console message about jump attempt
- [ ] Check logs for any ERROR or WARNING messages

## If All Else Fails

### Nuclear Option: Rebuild Input System State

Add this emergency reset function:
```cpp
void Engine::resetInputSystem()
{
    LOG_INFO("=== RESETTING INPUT SYSTEM ===");
    
    if (!m_inputManager || !m_window) return;
    
    // Re-setup callbacks
    setupInputCallbacks();
    
    // Force game mode
    m_inputManager->setInputMode(InputMode::GameMode);
    m_inputManager->setCursorMode(true);
    
    // Focus window
    glfwFocusWindow(m_window->getHandle());
    
    // Clear any stuck states
    m_inputManager->update();
    
    LOG_INFO("Input system reset complete");
}
```

Call during initialization or on a debug key (F12):
```cpp
if (m_inputManager->isKeyJustPressed(GLFW_KEY_F12)) {
    resetInputSystem();
}
```

## Summary

**The input system code is correct.** The issue is likely:
1. Runtime state management (input mode stuck in UI mode)
2. ImGui capturing input when it shouldn't
3. Window focus or cursor capture not set correctly
4. Player not properly initialized with world reference

**Recommended Action:** Add diagnostic logging and test the quick fixes above. The issue will likely reveal itself through the logs.

## Contact

If issues persist after trying these solutions, provide:
1. Console output with diagnostic logging enabled
2. Platform (Windows/Linux/macOS)
3. Build configuration (Debug/Release)
4. ImGui enabled/disabled status
5. Steps to reproduce from engine startup
