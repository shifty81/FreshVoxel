#include "viewport/InputContext.h"

#include <GLFW/glfw3.h>

namespace fresh
{

InputContext::InputContext() = default;

InputContext::~InputContext() = default;

void InputContext::initialize()
{
    m_keyStates.clear();
    m_keyJustPressed.clear();
    m_mouseButtonStates.clear();
    m_mouseButtonJustPressed.clear();
    m_mousePos = glm::vec2(0.0f);
    m_lastMousePos = glm::vec2(0.0f);
    m_mouseDelta = glm::vec2(0.0f);
    m_firstMouseEvent = true;
    m_focused = false;

    initializeDefaultBindings();
}

void InputContext::update()
{
    // Clear just-pressed flags at the start of each frame
    for (auto& [key, pressed] : m_keyJustPressed) {
        pressed = false;
    }
    for (auto& [button, pressed] : m_mouseButtonJustPressed) {
        pressed = false;
    }
    m_mouseDelta = glm::vec2(0.0f);
}

void InputContext::processKeyEvent(int key, bool pressed)
{
    if (!m_focused) {
        return;
    }

    bool wasPressed = m_keyStates[key];
    m_keyStates[key] = pressed;

    if (pressed && !wasPressed) {
        m_keyJustPressed[key] = true;
    }
}

bool InputContext::isKeyPressed(int key) const
{
    auto it = m_keyStates.find(key);
    return it != m_keyStates.end() && it->second;
}

bool InputContext::isKeyJustPressed(int key) const
{
    auto it = m_keyJustPressed.find(key);
    return it != m_keyJustPressed.end() && it->second;
}

void InputContext::processMouseMove(float x, float y)
{
    if (!m_focused) {
        return;
    }

    if (m_firstMouseEvent) {
        m_lastMousePos = glm::vec2(x, y);
        m_firstMouseEvent = false;
    }

    m_mousePos = glm::vec2(x, y);
    m_mouseDelta = m_mousePos - m_lastMousePos;
    m_lastMousePos = m_mousePos;
}

void InputContext::processMouseButton(int button, bool pressed)
{
    if (!m_focused) {
        return;
    }

    bool wasPressed = m_mouseButtonStates[button];
    m_mouseButtonStates[button] = pressed;

    if (pressed && !wasPressed) {
        m_mouseButtonJustPressed[button] = true;
    }
}

bool InputContext::isMouseButtonPressed(int button) const
{
    auto it = m_mouseButtonStates.find(button);
    return it != m_mouseButtonStates.end() && it->second;
}

bool InputContext::isMouseButtonJustPressed(int button) const
{
    auto it = m_mouseButtonJustPressed.find(button);
    return it != m_mouseButtonJustPressed.end() && it->second;
}

bool InputContext::isActionActive(InputAction action) const
{
    auto it = m_keyBindings.find(action);
    if (it == m_keyBindings.end()) {
        return false;
    }
    return isKeyPressed(it->second);
}

bool InputContext::isActionJustPressed(InputAction action) const
{
    auto it = m_keyBindings.find(action);
    if (it == m_keyBindings.end()) {
        return false;
    }
    return isKeyJustPressed(it->second);
}

void InputContext::setKeyBinding(InputAction action, int key)
{
    m_keyBindings[action] = key;
}

void InputContext::initializeDefaultBindings()
{
    m_keyBindings[InputAction::MoveForward]   = GLFW_KEY_W;
    m_keyBindings[InputAction::MoveBackward]  = GLFW_KEY_S;
    m_keyBindings[InputAction::MoveLeft]      = GLFW_KEY_A;
    m_keyBindings[InputAction::MoveRight]     = GLFW_KEY_D;
    m_keyBindings[InputAction::Jump]          = GLFW_KEY_SPACE;
    m_keyBindings[InputAction::Crouch]        = GLFW_KEY_LEFT_SHIFT;
    m_keyBindings[InputAction::Sprint]        = GLFW_KEY_LEFT_CONTROL;
    m_keyBindings[InputAction::Use]           = GLFW_KEY_E;
    m_keyBindings[InputAction::OpenInventory] = GLFW_KEY_I;
    m_keyBindings[InputAction::OpenMenu]      = GLFW_KEY_ESCAPE;
    m_keyBindings[InputAction::ToggleEditor]  = GLFW_KEY_F1;
}

} // namespace fresh
