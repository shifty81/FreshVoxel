#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file InputManagerCAPI.h
 * @brief C API for InputManager to enable .NET P/Invoke interop
 *
 * This file provides a C-compatible API wrapper around the C++ InputManager class
 * to enable .NET applications to call input functions via P/Invoke.
 */

// Opaque pointer to InputManager instance
typedef void* InputManagerHandle;

/**
 * @brief Get the global InputManager instance
 * @return Handle to InputManager (returns NULL if not initialized)
 */
InputManagerHandle InputManager_GetInstance();

/**
 * @brief Check if a key is currently pressed
 * @param handle InputManager handle
 * @param keyCode GLFW key code
 * @return 1 if pressed, 0 otherwise
 */
int InputManager_IsKeyPressed(InputManagerHandle handle, int keyCode);

/**
 * @brief Check if a key was just pressed this frame
 * @param handle InputManager handle
 * @param keyCode GLFW key code
 * @return 1 if just pressed, 0 otherwise
 */
int InputManager_IsKeyJustPressed(InputManagerHandle handle, int keyCode);

/**
 * @brief Check if a mouse button is currently pressed
 * @param handle InputManager handle
 * @param button GLFW mouse button code (0=left, 1=right, 2=middle)
 * @return 1 if pressed, 0 otherwise
 */
int InputManager_IsMouseButtonPressed(InputManagerHandle handle, int button);

/**
 * @brief Check if a mouse button was just pressed this frame
 * @param handle InputManager handle
 * @param button GLFW mouse button code
 * @return 1 if just pressed, 0 otherwise
 */
int InputManager_IsMouseButtonJustPressed(InputManagerHandle handle, int button);

/**
 * @brief Get mouse X position
 * @param handle InputManager handle
 * @return Mouse X coordinate
 */
float InputManager_GetMouseX(InputManagerHandle handle);

/**
 * @brief Get mouse Y position
 * @param handle InputManager handle
 * @return Mouse Y coordinate
 */
float InputManager_GetMouseY(InputManagerHandle handle);

/**
 * @brief Get mouse delta X (movement since last frame)
 * @param handle InputManager handle
 * @return Mouse delta X
 */
float InputManager_GetMouseDeltaX(InputManagerHandle handle);

/**
 * @brief Get mouse delta Y (movement since last frame)
 * @param handle InputManager handle
 * @return Mouse delta Y
 */
float InputManager_GetMouseDeltaY(InputManagerHandle handle);

/**
 * @brief Check if an action is currently active
 * @param handle InputManager handle
 * @param action Action ID (0=MoveForward, 1=MoveBackward, etc.)
 * @return 1 if active, 0 otherwise
 */
int InputManager_IsActionActive(InputManagerHandle handle, int action);

/**
 * @brief Check if an action was just pressed this frame
 * @param handle InputManager handle
 * @param action Action ID
 * @return 1 if just pressed, 0 otherwise
 */
int InputManager_IsActionJustPressed(InputManagerHandle handle, int action);

/**
 * @brief Set key binding for an action
 * @param handle InputManager handle
 * @param action Action ID
 * @param keyCode GLFW key code to bind
 */
void InputManager_SetKeyBinding(InputManagerHandle handle, int action, int keyCode);

/**
 * @brief Get mouse sensitivity
 * @param handle InputManager handle
 * @return Current sensitivity value
 */
float InputManager_GetMouseSensitivity(InputManagerHandle handle);

/**
 * @brief Set mouse sensitivity
 * @param handle InputManager handle
 * @param sensitivity New sensitivity value
 */
void InputManager_SetMouseSensitivity(InputManagerHandle handle, float sensitivity);

/**
 * @brief Get current input mode
 * @param handle InputManager handle
 * @return Input mode (0=GameMode, 1=UIMode, 2=BuildMode)
 */
int InputManager_GetInputMode(InputManagerHandle handle);

/**
 * @brief Set input mode
 * @param handle InputManager handle
 * @param mode Input mode to set
 * @param temporary If non-zero, mode is temporary
 */
void InputManager_SetInputMode(InputManagerHandle handle, int mode, int temporary);

#ifdef __cplusplus
}
#endif
