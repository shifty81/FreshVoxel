/**
 * @file InputManagerCAPI.cpp
 * @brief C API implementation for InputManager
 *
 * Provides C-compatible wrapper functions for .NET P/Invoke interop
 */

#include "input/InputManagerCAPI.h"

#include <cstddef>

#include "input/InputManager.h"

using namespace fresh;

// Global InputManager instance pointer
static InputManager* g_inputManager = nullptr;

extern "C" {

void InputManager_SetGlobalInstance(InputManager* instance)
{
    g_inputManager = instance;
}

InputManagerHandle InputManager_GetInstance()
{
    return static_cast<InputManagerHandle>(g_inputManager);
}

int InputManager_IsKeyPressed(InputManagerHandle handle, int keyCode)
{
    if (!handle)
        return 0;
    auto* manager = static_cast<InputManager*>(handle);
    return manager->isKeyPressed(keyCode) ? 1 : 0;
}

int InputManager_IsKeyJustPressed(InputManagerHandle handle, int keyCode)
{
    if (!handle)
        return 0;
    auto* manager = static_cast<InputManager*>(handle);
    return manager->isKeyJustPressed(keyCode) ? 1 : 0;
}

int InputManager_IsMouseButtonPressed(InputManagerHandle handle, int button)
{
    if (!handle)
        return 0;
    auto* manager = static_cast<InputManager*>(handle);
    return manager->isMouseButtonPressed(button) ? 1 : 0;
}

int InputManager_IsMouseButtonJustPressed(InputManagerHandle handle, int button)
{
    if (!handle)
        return 0;
    auto* manager = static_cast<InputManager*>(handle);
    return manager->isMouseButtonJustPressed(button) ? 1 : 0;
}

float InputManager_GetMouseX(InputManagerHandle handle)
{
    if (!handle)
        return 0.0f;
    auto* manager = static_cast<InputManager*>(handle);
    return manager->getMouseX();
}

float InputManager_GetMouseY(InputManagerHandle handle)
{
    if (!handle)
        return 0.0f;
    auto* manager = static_cast<InputManager*>(handle);
    return manager->getMouseY();
}

float InputManager_GetMouseDeltaX(InputManagerHandle handle)
{
    if (!handle)
        return 0.0f;
    auto* manager = static_cast<InputManager*>(handle);
    glm::vec2 delta = manager->getMouseDelta();
    return delta.x;
}

float InputManager_GetMouseDeltaY(InputManagerHandle handle)
{
    if (!handle)
        return 0.0f;
    auto* manager = static_cast<InputManager*>(handle);
    glm::vec2 delta = manager->getMouseDelta();
    return delta.y;
}

int InputManager_IsActionActive(InputManagerHandle handle, int action)
{
    if (!handle)
        return 0;
    auto* manager = static_cast<InputManager*>(handle);
    return manager->isActionActive(static_cast<InputAction>(action)) ? 1 : 0;
}

int InputManager_IsActionJustPressed(InputManagerHandle handle, int action)
{
    if (!handle)
        return 0;
    auto* manager = static_cast<InputManager*>(handle);
    return manager->isActionJustPressed(static_cast<InputAction>(action)) ? 1 : 0;
}

void InputManager_SetKeyBinding(InputManagerHandle handle, int action, int keyCode)
{
    if (!handle)
        return;
    auto* manager = static_cast<InputManager*>(handle);
    manager->setKeyBinding(static_cast<InputAction>(action), keyCode);
}

float InputManager_GetMouseSensitivity(InputManagerHandle handle)
{
    if (!handle)
        return 1.0f;
    auto* manager = static_cast<InputManager*>(handle);
    return manager->getMouseSensitivity();
}

void InputManager_SetMouseSensitivity(InputManagerHandle handle, float sensitivity)
{
    if (!handle)
        return;
    auto* manager = static_cast<InputManager*>(handle);
    manager->setMouseSensitivity(sensitivity);
}

int InputManager_GetInputMode(InputManagerHandle handle)
{
    if (!handle)
        return 0;
    auto* manager = static_cast<InputManager*>(handle);
    return static_cast<int>(manager->getInputMode());
}

void InputManager_SetInputMode(InputManagerHandle handle, int mode, int temporary)
{
    if (!handle)
        return;
    auto* manager = static_cast<InputManager*>(handle);
    manager->setInputMode(static_cast<InputMode>(mode), temporary != 0);
}

} // extern "C"
