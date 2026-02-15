#pragma once

#ifdef _WIN32

#include <map>
#include <string>
#include <Windows.h>
#include <glm/glm.hpp>
#include "input/InputTypes.h"

namespace fresh
{

class Win32Window;

/**
 * @brief Manages keyboard, mouse, and controller input using Win32 APIs
 *
 * Provides action-based input system with rebindable keys.
 * Tracks key states, mouse movement, and button presses natively on Windows.
 */
class Win32InputManager
{
public:
    /**
     * @brief Initialize input manager with Win32 window
     * @param window Win32Window handle
     */
    void initialize(Win32Window* window);

    /**
     * @brief Update input state (call once per frame)
     */
    void update();

    /**
     * @brief Process keyboard event (called by window message handler)
     * @param vkCode Virtual key code
     * @param isDown true if key is pressed, false if released
     */
    void processKeyEvent(WPARAM vkCode, bool isDown);

    /**
     * @brief Process mouse movement (called by window message handler)
     * @param xpos Mouse X position
     * @param ypos Mouse Y position
     */
    void processMouseMovement(int xpos, int ypos);

    /**
     * @brief Process mouse button event (called by window message handler)
     * @param button Mouse button (VK_LBUTTON, VK_RBUTTON, VK_MBUTTON)
     * @param isDown true if pressed, false if released
     */
    void processMouseButton(int button, bool isDown);

    /**
     * @brief Check if action is currently active
     * @param action Action to check
     * @return true if active
     */
    bool isActionActive(InputAction action) const;

    /**
     * @brief Check if action was just pressed this frame
     * @param action Action to check
     * @return true if just pressed
     */
    bool isActionJustPressed(InputAction action) const;

    /**
     * @brief Get mouse movement delta for this frame
     * @return Mouse delta as vec2
     */
    glm::vec2 getMouseDelta() const
    {
        return mouseDelta;
    }

    /**
     * @brief Reset mouse delta (called at start of frame)
     */
    void resetMouseDelta()
    {
        mouseDelta = glm::vec2(0.0f);
    }

    /**
     * @brief Set mouse sensitivity multiplier
     */
    void setMouseSensitivity(float sens)
    {
        sensitivity = sens;
    }
    
    float getMouseSensitivity() const
    {
        return sensitivity;
    }

    /**
     * @brief Set cursor capture mode
     * @param captured If true, hides and locks cursor
     */
    void setCursorMode(bool captured);
    
    /**
     * @brief Get current cursor capture state
     * @return true if cursor is captured
     */
    bool isCursorCaptured() const
    {
        return cursorCaptured;
    }

    /**
     * @brief Toggle cursor capture mode
     */
    void toggleCursorCapture();

    /**
     * @brief Check if mouse button is pressed
     * @param button Win32 virtual key code (VK_LBUTTON, VK_RBUTTON, VK_MBUTTON)
     * @return true if pressed
     */
    bool isMouseButtonPressed(int button) const;

    /**
     * @brief Check if mouse button was just pressed this frame
     * @param button Win32 virtual key code
     * @return true if just pressed
     */
    bool isMouseButtonJustPressed(int button) const;

    /**
     * @brief Check if a specific key is pressed (by virtual key code)
     * @param vk Win32 virtual key code (VK_* constants)
     * @return true if pressed
     */
    bool isKeyPressed(int vk) const;

    /**
     * @brief Check if a specific key was just pressed this frame
     * @param vk Win32 virtual key code
     * @return true if just pressed
     */
    bool isKeyJustPressed(int vk) const;

    /**
     * @brief Get current mouse position
     * @return Mouse position as vec2 (x, y)
     */
    glm::vec2 getMousePosition() const;

    /**
     * @brief Get mouse X coordinate
     * @return Mouse X position
     */
    float getMouseX() const
    {
        return static_cast<float>(lastMouseX);
    }

    /**
     * @brief Get mouse Y coordinate
     * @return Mouse Y position
     */
    float getMouseY() const
    {
        return static_cast<float>(lastMouseY);
    }

    /**
     * @brief Bind key to action
     * @param action Action to bind
     * @param vk Win32 virtual key code
     */
    void setKeyBinding(InputAction action, int vk);

    /**
     * @brief Set input mode (GameMode, UIMode, BuildMode)
     * @param mode New input mode
     * @param temporary If true, mode can be overridden by Hold Alt feature
     */
    void setInputMode(InputMode mode, bool temporary = false);

    /**
     * @brief Get current input mode
     * @return Current input mode
     */
    InputMode getInputMode() const
    {
        return currentMode;
    }

    /**
     * @brief Check if Alt key is being held down
     * @return true if Alt is held
     */
    bool isAltHeld() const;

    /**
     * @brief Check if cursor is in UI mode (either explicit or via Alt hold)
     * @return true if cursor should be visible and free
     */
    bool isInUIMode() const;

private:
    void initializeDefaultBindings();

    Win32Window* window = nullptr;
    std::map<InputAction, int> keyBindings; // Maps actions to virtual key codes
    std::map<int, bool> keyStates;          // Current key states
    std::map<int, bool> keyPressedThisFrame;
    std::map<int, bool> mouseButtonStates;
    std::map<int, bool> mouseButtonPressedThisFrame;

    glm::vec2 mouseDelta{0.0f};
    int lastMouseX = 0;
    int lastMouseY = 0;
    bool firstMouse = true;
    float sensitivity = 1.0f;
    bool cursorCaptured = false;
    bool expectingRecenterEvent = false; // Flag to ignore SetCursorPos event

    // Input mode management
    InputMode currentMode = InputMode::GameMode;
    InputMode baseMode = InputMode::GameMode;
    bool temporaryModeSet = false;
};

} // namespace fresh

#endif // _WIN32
