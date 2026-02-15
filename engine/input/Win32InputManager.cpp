#ifdef _WIN32

#include "input/Win32InputManager.h"
#include "core/Win32Window.h"
#include "core/Logger.h"

namespace fresh
{

void Win32InputManager::initialize(Win32Window* win)
{
    window = win;
    initializeDefaultBindings();
    LOG_INFO_C("Win32InputManager initialized", "Win32InputManager");
}

void Win32InputManager::initializeDefaultBindings()
{
    // Default WASD + common key bindings
    keyBindings[InputAction::MoveForward] = 'W';
    keyBindings[InputAction::MoveBackward] = 'S';
    keyBindings[InputAction::MoveLeft] = 'A';
    keyBindings[InputAction::MoveRight] = 'D';
    keyBindings[InputAction::Jump] = VK_SPACE;
    keyBindings[InputAction::Crouch] = VK_CONTROL;
    keyBindings[InputAction::Sprint] = VK_SHIFT;
    keyBindings[InputAction::Use] = 'E';
    keyBindings[InputAction::Attack] = VK_LBUTTON; // Left mouse button
    keyBindings[InputAction::PlaceBlock] = VK_RBUTTON; // Right mouse button
    keyBindings[InputAction::BreakBlock] = VK_LBUTTON;
    keyBindings[InputAction::OpenInventory] = 'I';
    keyBindings[InputAction::OpenMenu] = VK_ESCAPE;
    keyBindings[InputAction::ToggleEditor] = VK_F1;
    keyBindings[InputAction::OpenChat] = 'T';
}

void Win32InputManager::update()
{
    // Clear just-pressed states from previous frame
    keyPressedThisFrame.clear();
    mouseButtonPressedThisFrame.clear();

    // Handle Alt-hold for temporary UI mode
    if (isAltHeld() && !temporaryModeSet && currentMode == InputMode::GameMode) {
        baseMode = currentMode;
        currentMode = InputMode::UIMode;
        setCursorMode(false);
        temporaryModeSet = true;
    } else if (!isAltHeld() && temporaryModeSet) {
        currentMode = baseMode;
        setCursorMode(currentMode == InputMode::GameMode);
        temporaryModeSet = false;
    }
}

void Win32InputManager::processKeyEvent(WPARAM vkCode, bool isDown)
{
    int key = static_cast<int>(vkCode);

    if (isDown && !keyStates[key]) {
        keyPressedThisFrame[key] = true;
    }

    keyStates[key] = isDown;
}

void Win32InputManager::processMouseMovement(int xpos, int ypos)
{
    // Ignore mouse movement events caused by SetCursorPos when re-centering
    if (expectingRecenterEvent) {
        expectingRecenterEvent = false;
        lastMouseX = xpos;
        lastMouseY = ypos;
        return;
    }

    if (firstMouse) {
        lastMouseX = xpos;
        lastMouseY = ypos;
        firstMouse = false;
        return; // Skip this frame to avoid jump
    }

    float xoffset = static_cast<float>(xpos - lastMouseX);
    float yoffset = static_cast<float>(lastMouseY - ypos); // Reversed since y-coordinates go from bottom to top

    if (cursorCaptured) {
        mouseDelta.x += xoffset * sensitivity;
        mouseDelta.y += yoffset * sensitivity;
        
        // Re-center cursor to prevent hitting window edges
        if (window) {
            HWND hwnd = window->getHandle();
            RECT rect;
            GetClientRect(hwnd, &rect);
            POINT center = {rect.right / 2, rect.bottom / 2};
            ClientToScreen(hwnd, &center);
            
            // Set flag before SetCursorPos to ignore the resulting WM_MOUSEMOVE
            expectingRecenterEvent = true;
            SetCursorPos(center.x, center.y);
            
            // Update lastMouse to the center position
            lastMouseX = rect.right / 2;
            lastMouseY = rect.bottom / 2;
        }
    } else {
        // Not captured, just track position normally
        lastMouseX = xpos;
        lastMouseY = ypos;
    }
}

void Win32InputManager::processMouseButton(int button, bool isDown)
{
    if (isDown && !mouseButtonStates[button]) {
        mouseButtonPressedThisFrame[button] = true;
    }

    mouseButtonStates[button] = isDown;
}

bool Win32InputManager::isActionActive(InputAction action) const
{
    auto it = keyBindings.find(action);
    if (it == keyBindings.end()) {
        return false;
    }

    int key = it->second;
    
    // Check if it's a mouse button
    if (key == VK_LBUTTON || key == VK_RBUTTON || key == VK_MBUTTON) {
        auto state = mouseButtonStates.find(key);
        return state != mouseButtonStates.end() && state->second;
    }

    // Check keyboard
    auto state = keyStates.find(key);
    return state != keyStates.end() && state->second;
}

bool Win32InputManager::isActionJustPressed(InputAction action) const
{
    auto it = keyBindings.find(action);
    if (it == keyBindings.end()) {
        return false;
    }

    int key = it->second;
    
    // Check if it's a mouse button
    if (key == VK_LBUTTON || key == VK_RBUTTON || key == VK_MBUTTON) {
        auto state = mouseButtonPressedThisFrame.find(key);
        return state != mouseButtonPressedThisFrame.end() && state->second;
    }

    // Check keyboard
    auto state = keyPressedThisFrame.find(key);
    return state != keyPressedThisFrame.end() && state->second;
}

void Win32InputManager::setCursorMode(bool captured)
{
    if (cursorCaptured == captured || !window) {
        return;
    }

    cursorCaptured = captured;

    if (captured) {
        // Hide and lock cursor to center of window
        HWND hwnd = window->getHandle();
        RECT rect;
        GetClientRect(hwnd, &rect);  // Get client area dimensions (excludes title bar, borders)
        POINT center = {rect.right / 2, rect.bottom / 2};
        ClientToScreen(hwnd, &center);
        
        // Initialize lastMouse to center position in client coordinates to avoid jump
        lastMouseX = rect.right / 2;
        lastMouseY = rect.bottom / 2;
        
        // Set flag to ignore the SetCursorPos event
        expectingRecenterEvent = true;
        SetCursorPos(center.x, center.y);
        ShowCursor(FALSE);
        
        // Clip cursor to window
        RECT windowRect;
        GetWindowRect(hwnd, &windowRect);
        ClipCursor(&windowRect);
        
        // Don't set firstMouse = true here because we've properly initialized lastMouse.
        // The firstMouse flag is used in processMouseMovement() to skip the first movement
        // event after cursor state changes, preventing camera jumps from position deltas.
        // Setting firstMouse = true would cause the next real mouse movement to be
        // ignored (early return in processMouseMovement), defeating the purpose of
        // capturing the cursor for camera control. Since we've initialized lastMouse
        // to the current position, we don't need to skip any events.
    } else {
        // Show cursor and release clip
        ShowCursor(TRUE);
        ClipCursor(nullptr);
        firstMouse = true; // Reset for next capture
    }
}

void Win32InputManager::toggleCursorCapture()
{
    setCursorMode(!cursorCaptured);
}

bool Win32InputManager::isMouseButtonPressed(int button) const
{
    auto it = mouseButtonStates.find(button);
    return it != mouseButtonStates.end() && it->second;
}

bool Win32InputManager::isMouseButtonJustPressed(int button) const
{
    auto it = mouseButtonPressedThisFrame.find(button);
    return it != mouseButtonPressedThisFrame.end() && it->second;
}

bool Win32InputManager::isKeyPressed(int vk) const
{
    auto it = keyStates.find(vk);
    return it != keyStates.end() && it->second;
}

bool Win32InputManager::isKeyJustPressed(int vk) const
{
    auto it = keyPressedThisFrame.find(vk);
    return it != keyPressedThisFrame.end() && it->second;
}

glm::vec2 Win32InputManager::getMousePosition() const
{
    if (!window) {
        return glm::vec2(0.0f, 0.0f);
    }

    POINT pt;
    if (GetCursorPos(&pt)) {
        HWND hwnd = window->getHandle();
        ScreenToClient(hwnd, &pt);
        return glm::vec2(static_cast<float>(pt.x), static_cast<float>(pt.y));
    }

    return glm::vec2(0.0f, 0.0f);
}

void Win32InputManager::setKeyBinding(InputAction action, int vk)
{
    keyBindings[action] = vk;
}

void Win32InputManager::setInputMode(InputMode mode, bool temporary)
{
    if (temporary) {
        baseMode = currentMode;
        temporaryModeSet = true;
    } else {
        baseMode = mode;
        temporaryModeSet = false;
    }
    
    currentMode = mode;

    // Update cursor capture based on mode
    bool shouldCapture = (mode == InputMode::GameMode);
    setCursorMode(shouldCapture);
}

bool Win32InputManager::isAltHeld() const
{
    return (GetKeyState(VK_MENU) & 0x8000) != 0;
}

bool Win32InputManager::isInUIMode() const
{
    return currentMode == InputMode::UIMode || isAltHeld();
}

} // namespace fresh

#endif // _WIN32
