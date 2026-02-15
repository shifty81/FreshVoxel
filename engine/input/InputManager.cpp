#include "input/InputManager.h"

#include <cmath>

namespace fresh
{

void InputManager::initialize(GLFWwindow* win)
{
    window = win;
    initializeDefaultBindings();
    setCursorMode(true); // Start with captured cursor
}

void InputManager::update()
{
    // Clear just-pressed states from previous frame
    keyPressedThisFrame.clear();
    mouseButtonPressedThisFrame.clear();

    // Note: mouseDelta is NOT reset here - it will be reset after being consumed
    // The mouseDelta should be reset AFTER the game logic uses it

    // Update gamepads
    updateGamepads();

    // Handle "Hold Alt" feature for temporary UI mode switching
    bool altHeld = isAltHeld();
    if (altHeld && !temporaryModeSet && currentMode == InputMode::GameMode) {
        // Temporarily switch to UI mode while Alt is held
        // Only save baseMode if not already in temporary mode
        baseMode = currentMode;
        currentMode = InputMode::UIMode;
        temporaryModeSet = true;
        setCursorMode(false); // Show cursor
    } else if (!altHeld && temporaryModeSet) {
        // Release Alt, return to base mode
        currentMode = baseMode;
        temporaryModeSet = false;
        if (currentMode == InputMode::GameMode) {
            setCursorMode(true); // Hide and capture cursor
        }
    }
}

void InputManager::updateGamepads()
{
    // Poll all gamepads (GLFW supports up to 16)
    for (int i = 0; i < MAX_GAMEPADS; ++i) {
        int present = glfwJoystickPresent(GLFW_JOYSTICK_1 + i);
        gamepads[i].connected = (present == GLFW_TRUE);

        if (!gamepads[i].connected) {
            continue;
        }

        // Get gamepad name
        const char* name = glfwGetJoystickName(GLFW_JOYSTICK_1 + i);
        if (name) {
            gamepads[i].name = name;
        }

        // Get axes (analog sticks and triggers)
        int axisCount;
        const float* axes = glfwGetJoystickAxes(GLFW_JOYSTICK_1 + i, &axisCount);
        if (axes && axisCount >= 6) {
            for (int j = 0; j < 6 && j < axisCount; ++j) {
                gamepads[i].axes[j] = applyDeadzone(axes[j]);
            }
        }

        // Get buttons
        int buttonCount;
        const unsigned char* buttons = glfwGetJoystickButtons(GLFW_JOYSTICK_1 + i, &buttonCount);
        if (buttons) {
            for (int j = 0; j < 15 && j < buttonCount; ++j) {
                gamepads[i].buttons[j] = (buttons[j] == GLFW_PRESS);
            }
        }
    }
}

bool InputManager::isGamepadConnected(int gamepadID) const
{
    if (gamepadID < 0 || gamepadID >= MAX_GAMEPADS)
        return false;
    return gamepads[gamepadID].connected;
}

float InputManager::getGamepadAxis(int gamepadID, int axis) const
{
    if (gamepadID < 0 || gamepadID >= MAX_GAMEPADS)
        return 0.0f;
    if (!gamepads[gamepadID].connected)
        return 0.0f;
    if (axis < 0 || axis >= 6)
        return 0.0f;
    return gamepads[gamepadID].axes[axis];
}

bool InputManager::isGamepadButtonPressed(int gamepadID, int button) const
{
    if (gamepadID < 0 || gamepadID >= MAX_GAMEPADS)
        return false;
    if (!gamepads[gamepadID].connected)
        return false;
    if (button < 0 || button >= 15)
        return false;
    return gamepads[gamepadID].buttons[button];
}

std::string InputManager::getGamepadName(int gamepadID) const
{
    if (gamepadID < 0 || gamepadID >= MAX_GAMEPADS)
        return "";
    if (!gamepads[gamepadID].connected)
        return "";
    return gamepads[gamepadID].name;
}

void InputManager::setGamepadVibration(int gamepadID, float leftMotor, float rightMotor)
{
    // Note: GLFW doesn't support force feedback/vibration directly
    // This would require platform-specific implementations (XInput on Windows, etc.)
    // For now, this is a placeholder
    (void)gamepadID;
    (void)leftMotor;
    (void)rightMotor;
}

float InputManager::applyDeadzone(float value) const
{
    if (std::abs(value) < gamepadDeadzone) {
        return 0.0f;
    }
    // Rescale from deadzone to 1.0
    float sign = (value > 0.0f) ? 1.0f : -1.0f;
    float scaled = (std::abs(value) - gamepadDeadzone) / (1.0f - gamepadDeadzone);
    return sign * scaled;
}

void InputManager::processKeyEvent(int key, int action)
{
    if (action == GLFW_PRESS) {
        keyStates[key] = true;
        keyPressedThisFrame[key] = true;
    } else if (action == GLFW_RELEASE) {
        keyStates[key] = false;
    }
}

void InputManager::processMouseMovement(double xpos, double ypos)
{
    if (firstMouse) {
        lastMouseX = xpos;
        lastMouseY = ypos;
        firstMouse = false;
        // Don't return early - we still want to process this movement
        // but the delta will be zero since we just set lastMouse positions
    }

    float xOffset = static_cast<float>(xpos - lastMouseX);
    float yOffset = static_cast<float>(ypos - lastMouseY);

    lastMouseX = xpos;
    lastMouseY = ypos;

    // Always track mouse delta for consistency, even when cursor is not captured
    // This allows proper testing and can be useful for UI interactions
    mouseDelta = glm::vec2(xOffset, yOffset);
}

void InputManager::processMouseButton(int button, int action)
{
    if (action == GLFW_PRESS) {
        mouseButtonStates[button] = true;
        mouseButtonPressedThisFrame[button] = true;
    } else if (action == GLFW_RELEASE) {
        mouseButtonStates[button] = false;
    }
}

bool InputManager::isActionActive(InputAction action) const
{
    auto it = keyBindings.find(action);
    if (it == keyBindings.end())
        return false;

    auto keyIt = keyStates.find(it->second);
    if (keyIt == keyStates.end())
        return false;

    return keyIt->second;
}

bool InputManager::isActionJustPressed(InputAction action) const
{
    auto it = keyBindings.find(action);
    if (it == keyBindings.end())
        return false;

    auto keyIt = keyPressedThisFrame.find(it->second);
    if (keyIt == keyPressedThisFrame.end())
        return false;

    return keyIt->second;
}

void InputManager::setCursorMode(bool captured)
{
    if (!window)
        return;

    cursorCaptured = captured;
    if (captured) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        firstMouse = true; // Reset to avoid jump
    } else {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
}

void InputManager::toggleCursorCapture()
{
    // Toggle between GameMode (cursor captured) and UIMode (cursor free)
    if (currentMode == InputMode::GameMode) {
        setInputMode(InputMode::UIMode, false);
    } else {
        setInputMode(InputMode::GameMode, false);
    }
}

bool InputManager::isMouseButtonPressed(int button) const
{
    auto it = mouseButtonStates.find(button);
    if (it == mouseButtonStates.end())
        return false;
    return it->second;
}

bool InputManager::isMouseButtonJustPressed(int button) const
{
    auto it = mouseButtonPressedThisFrame.find(button);
    if (it == mouseButtonPressedThisFrame.end())
        return false;
    return it->second;
}

bool InputManager::isKeyPressed(int key) const
{
    auto it = keyStates.find(key);
    if (it == keyStates.end())
        return false;
    return it->second;
}

bool InputManager::isKeyJustPressed(int key) const
{
    auto it = keyPressedThisFrame.find(key);
    if (it == keyPressedThisFrame.end())
        return false;
    return it->second;
}

glm::vec2 InputManager::getMousePosition() const
{
    return glm::vec2(static_cast<float>(lastMouseX), static_cast<float>(lastMouseY));
}

void InputManager::setKeyBinding(InputAction action, int key)
{
    keyBindings[action] = key;
}

void InputManager::initializeDefaultBindings()
{
    // WASD movement
    keyBindings[InputAction::MoveForward] = GLFW_KEY_W;
    keyBindings[InputAction::MoveBackward] = GLFW_KEY_S;
    keyBindings[InputAction::MoveLeft] = GLFW_KEY_A;
    keyBindings[InputAction::MoveRight] = GLFW_KEY_D;

    // Actions
    keyBindings[InputAction::Jump] = GLFW_KEY_SPACE;
    keyBindings[InputAction::Crouch] = GLFW_KEY_LEFT_CONTROL;
    keyBindings[InputAction::Sprint] = GLFW_KEY_LEFT_SHIFT;
    keyBindings[InputAction::Use] = GLFW_KEY_E;

    // Mouse actions (mapped to keys for consistency, actual mouse handled separately)
    keyBindings[InputAction::Attack] = GLFW_MOUSE_BUTTON_LEFT;
    keyBindings[InputAction::PlaceBlock] = GLFW_MOUSE_BUTTON_RIGHT;
    keyBindings[InputAction::BreakBlock] = GLFW_MOUSE_BUTTON_LEFT;

    // UI
    keyBindings[InputAction::OpenInventory] = GLFW_KEY_TAB;
    keyBindings[InputAction::OpenMenu] = GLFW_KEY_ESCAPE;
    keyBindings[InputAction::ToggleEditor] = GLFW_KEY_T;
    keyBindings[InputAction::OpenChat] = GLFW_KEY_ENTER;
}

void InputManager::setInputMode(InputMode mode, bool temporary)
{
    // Don't update baseMode if we're currently in a temporary Alt-hold mode
    // This prevents overwriting the return-to mode when Alt is released
    if (!temporaryModeSet || !isAltHeld()) {
        baseMode = mode;
    }

    currentMode = mode;
    temporaryModeSet = temporary;

    // Update cursor mode based on input mode
    if (mode == InputMode::GameMode) {
        setCursorMode(true); // Capture and hide cursor
    } else {
        setCursorMode(false); // Free and show cursor
    }
}

bool InputManager::isAltHeld() const
{
    // Check both left and right Alt keys
    auto leftAlt = keyStates.find(GLFW_KEY_LEFT_ALT);
    auto rightAlt = keyStates.find(GLFW_KEY_RIGHT_ALT);

    bool leftAltPressed = (leftAlt != keyStates.end() && leftAlt->second);
    bool rightAltPressed = (rightAlt != keyStates.end() && rightAlt->second);

    return leftAltPressed || rightAltPressed;
}

bool InputManager::isInUIMode() const
{
    // UI mode is active if explicitly set or if Alt is held (temporary mode)
    // Check both the current mode and if we're temporarily in UI mode via Alt hold
    return (currentMode == InputMode::UIMode || currentMode == InputMode::BuildMode ||
            (temporaryModeSet && isAltHeld()));
}

} // namespace fresh
