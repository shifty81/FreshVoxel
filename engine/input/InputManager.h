#pragma once
#include <map>
#include <string>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "input/InputTypes.h"

namespace fresh
{

/**
 * @brief Manages keyboard, mouse, and controller input
 *
 * Provides action-based input system with rebindable keys.
 * Tracks key states, mouse movement, and button presses.
 */
class InputManager
{
public:
    /**
     * @brief Initialize input manager with GLFW window
     * @param window GLFW window handle
     */
    void initialize(GLFWwindow* window);

    /**
     * @brief Update input state (call once per frame)
     */
    void update();

    /**
     * @brief Process keyboard event (called by GLFW callback)
     * @param key GLFW key code
     * @param action GLFW action (press/release)
     */
    void processKeyEvent(int key, int action);

    /**
     * @brief Process mouse movement (called by GLFW callback)
     * @param xpos Mouse X position
     * @param ypos Mouse Y position
     */
    void processMouseMovement(double xpos, double ypos);

    /**
     * @brief Process mouse button event (called by GLFW callback)
     * @param button Mouse button code
     * @param action GLFW action (press/release)
     */
    void processMouseButton(int button, int action);

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
     * @brief Reset mouse delta (called at start of frame before polling events)
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
     * Switches between captured (freelook) and free (UI) cursor
     */
    void toggleCursorCapture();

    /**
     * @brief Check if mouse button is pressed
     * @param button GLFW mouse button code
     * @return true if pressed
     */
    bool isMouseButtonPressed(int button) const;

    /**
     * @brief Check if mouse button was just pressed this frame
     * @param button GLFW mouse button code
     * @return true if just pressed
     */
    bool isMouseButtonJustPressed(int button) const;

    /**
     * @brief Check if a specific key is pressed (by GLFW key code)
     * @param key GLFW key code
     * @return true if pressed
     */
    bool isKeyPressed(int key) const;

    /**
     * @brief Check if a specific key was just pressed this frame
     * @param key GLFW key code
     * @return true if just pressed
     */
    bool isKeyJustPressed(int key) const;

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
     * @param key GLFW key code
     */
    void setKeyBinding(InputAction action, int key);

    // Gamepad support (up to 16 controllers)
    /**
     * @brief Update gamepad states (call once per frame)
     */
    void updateGamepads();

    /**
     * @brief Check if gamepad is connected
     * @param gamepadID Gamepad ID (0-15)
     */
    bool isGamepadConnected(int gamepadID) const;

    /**
     * @brief Get gamepad axis value
     * @param gamepadID Gamepad ID (0-15)
     * @param axis Axis index (0=LeftX, 1=LeftY, 2=RightX, 3=RightY, 4=LeftTrigger, 5=RightTrigger)
     * @return Axis value (-1.0 to 1.0)
     */
    float getGamepadAxis(int gamepadID, int axis) const;

    /**
     * @brief Check if gamepad button is pressed
     * @param gamepadID Gamepad ID (0-15)
     * @param button Button index
     * @return true if pressed
     */
    bool isGamepadButtonPressed(int gamepadID, int button) const;

    /**
     * @brief Get gamepad name
     * @param gamepadID Gamepad ID (0-15)
     * @return Gamepad name or empty string
     */
    std::string getGamepadName(int gamepadID) const;

    /**
     * @brief Set gamepad deadzone
     */
    void setGamepadDeadzone(float deadzone)
    {
        gamepadDeadzone = deadzone;
    }
    float getGamepadDeadzone() const
    {
        return gamepadDeadzone;
    }

    /**
     * @brief Enable/disable gamepad vibration
     * @param gamepadID Gamepad ID
     * @param leftMotor Left motor strength (0.0-1.0)
     * @param rightMotor Right motor strength (0.0-1.0)
     */
    void setGamepadVibration(int gamepadID, float leftMotor, float rightMotor);

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
    float applyDeadzone(float value) const;

    GLFWwindow* window = nullptr;
    std::map<InputAction, int> keyBindings;
    std::map<int, bool> keyStates;
    std::map<int, bool> keyPressedThisFrame;
    std::map<int, bool> mouseButtonStates;
    std::map<int, bool> mouseButtonPressedThisFrame;

    glm::vec2 mouseDelta{0.0f};
    double lastMouseX = 0.0;
    double lastMouseY = 0.0;
    bool firstMouse = true;
    float sensitivity = 1.0f;
    bool cursorCaptured = false;

    // Gamepad state (up to 16 controllers)
    static constexpr int MAX_GAMEPADS = 16;
    struct GamepadState {
        bool connected = false;
        float axes[6] = {0.0f}; // LeftX, LeftY, RightX, RightY, LeftTrigger, RightTrigger
        bool buttons[15] = {false};
        std::string name;
    };
    GamepadState gamepads[MAX_GAMEPADS];
    float gamepadDeadzone = 0.15f;

    // Input mode management
    InputMode currentMode = InputMode::GameMode;
    InputMode baseMode = InputMode::GameMode; // Mode before Alt hold
    bool temporaryModeSet = false;
};

} // namespace fresh
