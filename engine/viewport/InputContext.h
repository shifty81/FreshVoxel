#pragma once

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include <glm/glm.hpp>

#include "input/InputTypes.h"

namespace fresh
{

/**
 * @brief Viewport-owned input context
 *
 * Per ENGINE.md: "Input is never global — Input routes through
 * viewport-owned InputContext."
 *
 * Each ViewportContext owns an InputContext that receives OS events
 * only when the viewport has focus. This prevents global input
 * routing and ensures PIE and standalone Client use the same
 * input code path.
 */
class InputContext
{
public:
    InputContext();
    ~InputContext();

    // Non-copyable, movable
    InputContext(const InputContext&) = delete;
    InputContext& operator=(const InputContext&) = delete;
    InputContext(InputContext&&) noexcept = default;
    InputContext& operator=(InputContext&&) noexcept = default;

    /**
     * @brief Initialize the input context
     */
    void initialize();

    /**
     * @brief Update per-frame state (clear just-pressed flags, etc.)
     */
    void update();

    // --- Key events ---

    /**
     * @brief Process a key event from the OS
     * @param key Platform key code
     * @param pressed true if pressed, false if released
     */
    void processKeyEvent(int key, bool pressed);

    /**
     * @brief Check if a key is currently held
     */
    [[nodiscard]] bool isKeyPressed(int key) const;

    /**
     * @brief Check if a key was just pressed this frame
     */
    [[nodiscard]] bool isKeyJustPressed(int key) const;

    // --- Mouse events ---

    /**
     * @brief Process mouse movement relative to the viewport
     * @param x Viewport-local X coordinate
     * @param y Viewport-local Y coordinate
     */
    void processMouseMove(float x, float y);

    /**
     * @brief Process mouse button event
     * @param button Button code
     * @param pressed true if pressed, false if released
     */
    void processMouseButton(int button, bool pressed);

    /**
     * @brief Get mouse position in viewport-local coordinates
     */
    [[nodiscard]] glm::vec2 getMousePosition() const noexcept { return m_mousePos; }

    /**
     * @brief Get mouse movement delta since last frame
     */
    [[nodiscard]] glm::vec2 getMouseDelta() const noexcept { return m_mouseDelta; }

    /**
     * @brief Check if a mouse button is pressed
     */
    [[nodiscard]] bool isMouseButtonPressed(int button) const;

    /**
     * @brief Check if a mouse button was just pressed this frame
     */
    [[nodiscard]] bool isMouseButtonJustPressed(int button) const;

    // --- Action mapping ---

    /**
     * @brief Check if a named input action is currently active
     */
    [[nodiscard]] bool isActionActive(InputAction action) const;

    /**
     * @brief Check if a named input action was just triggered this frame
     */
    [[nodiscard]] bool isActionJustPressed(InputAction action) const;

    /**
     * @brief Bind a key to an input action
     */
    void setKeyBinding(InputAction action, int key);

    // --- Focus ---

    /**
     * @brief Set whether this input context has focus
     *
     * Only the focused viewport's InputContext receives events.
     */
    void setFocused(bool focused) noexcept { m_focused = focused; }

    /**
     * @brief Check if this input context has focus
     */
    [[nodiscard]] bool isFocused() const noexcept { return m_focused; }

    // --- Input mode ---

    /**
     * @brief Set the current input mode
     */
    void setInputMode(InputMode mode) noexcept { m_inputMode = mode; }

    /**
     * @brief Get the current input mode
     */
    [[nodiscard]] InputMode getInputMode() const noexcept { return m_inputMode; }

private:
    void initializeDefaultBindings();

    std::unordered_map<int, bool> m_keyStates;
    std::unordered_map<int, bool> m_keyJustPressed;
    std::unordered_map<int, bool> m_mouseButtonStates;
    std::unordered_map<int, bool> m_mouseButtonJustPressed;
    std::unordered_map<InputAction, int> m_keyBindings;

    glm::vec2 m_mousePos{0.0f};
    glm::vec2 m_lastMousePos{0.0f};
    glm::vec2 m_mouseDelta{0.0f};
    bool m_firstMouseEvent = true;

    bool m_focused = false;
    InputMode m_inputMode = InputMode::GameMode;
};

} // namespace fresh
