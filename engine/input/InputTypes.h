#pragma once

namespace fresh
{

/**
 * @brief Input mode - determines how input is processed
 */
enum class InputMode {
    GameMode, // Standard in-game mode: cursor captured, movement controls active
    UIMode,   // UI/Menu mode: cursor free, UI elements interactive
    BuildMode // Build menu mode: specialized controls for building/ship editor
};

/**
 * @brief Input actions that can be mapped to keys
 */
enum class InputAction {
    MoveForward,
    MoveBackward,
    MoveLeft,
    MoveRight,
    Jump,
    Crouch,
    Sprint,
    Use,
    Attack,
    PlaceBlock,
    BreakBlock,
    OpenInventory,
    OpenMenu,
    ToggleEditor,
    OpenChat
};

} // namespace fresh
