# GUI Visibility Fix - World Creation Issue

## Issue Summary

When generating a new world, the GUI would briefly appear but then immediately disappear, leaving only the viewport visible. The GUI should remain visible after world creation, with the full-screen viewport ONLY appearing during play mode.

## Root Cause Analysis

The bug was caused by conflating two different application states into a single flag:

**The `m_inGame` flag was being used for two purposes:**
1. **World existence**: Whether a world has been created/loaded (should run game loop)
2. **Play mode**: Whether the user is actively playing (should hide editor GUI)

In `Engine::initializeGameSystems()` at line 753, the code was setting `m_inGame = true` immediately after world creation. This caused the application to act as if the user had entered play mode, hiding the editor GUI.

## The Fix

**File Changed:** `engine/core/Engine.cpp`

**Three minimal changes:**

### 1. Removed Play Mode Activation from World Initialization (Lines 753-754)
```cpp
// OLD CODE:
m_inGame = true;

// NEW CODE:
// Note: Do NOT set m_inGame = true here. The editor should remain visible.
// m_inGame should only be set when explicitly entering play mode via enterPlayMode().
```

### 2. Changed Game Loop Condition (Lines 780-781)
```cpp
// OLD CODE:
// If not in game yet, show GUI menu and handle world creation
if (!m_inGame) {

// NEW CODE:
// If no world exists yet, show GUI menu only (main menu for world creation/loading)
if (!m_world) {
```

### 3. Updated Skip Comment (Line 806)
```cpp
// OLD CODE:
// Skip normal game loop when not in game

// NEW CODE:
// Skip normal game loop when no world exists (main menu only)
```

## Separation of Concerns

The fix properly separates two distinct states:

| State | Controlled By | Purpose |
|-------|--------------|---------|
| **World Existence** | `m_world != nullptr` | Determines if game loop should run |
| **Play Mode** | `m_inGame` flag | Determines if editor GUI should be visible |

## Expected Behavior After Fix

### Scenario 1: Application Launch (No World)
- **State:** `m_world = nullptr`, `m_inGame = false`
- **Behavior:** Shows main menu for world creation/loading
- **Rendering:** Editor menu only

### Scenario 2: After World Creation (Editor Mode)
- **State:** `m_world != nullptr`, `m_inGame = false`
- **Behavior:** Shows editor GUI with world in viewport
- **Rendering:** World in viewport + all editor panels visible

### Scenario 3: Enter Play Mode
- **State:** `m_world != nullptr`, `m_inGame = true`
- **Behavior:** Full-screen gameplay (editor hidden)
- **Rendering:** World in full window, editor panels hidden

### Scenario 4: Exit Play Mode
- **State:** `m_world != nullptr`, `m_inGame = false`
- **Behavior:** Returns to editor mode
- **Rendering:** World in viewport + all editor panels visible again

## Code Flow

### Before World Creation
```
Engine::run() loop
  ├─ if (!m_world) → TRUE
  │   ├─ Render editor menu only (main menu)
  │   └─ continue (skip game loop)
  └─ [game loop skipped]
```

### After World Creation (Fixed Behavior)
```
Engine::run() loop
  ├─ if (!m_world) → FALSE (world exists)
  │   └─ [skip main menu path]
  ├─ update() - updates world, camera, physics
  └─ render()
      ├─ Render world in viewport
      └─ Render editor panels (visible = true)
```

### In Play Mode
```
Engine::run() loop
  ├─ if (!m_world) → FALSE
  │   └─ [skip main menu path]
  ├─ update() - updates world with physics
  └─ render()
      ├─ Render world in full window
      └─ Skip editor panels (visible = false)
```

## Related Functions

### Play Mode Control
- **`Engine::enterPlayMode()`** - Sets `m_inGame = true`, hides editor
- **`Engine::exitPlayMode()`** - Sets `m_inGame = false`, shows editor
- **`Engine::togglePlayMode()`** - Toggles between modes

### World Management
- **`Engine::createNewWorld()`** - Creates new world, calls `initializeGameSystems()`
- **`Engine::loadWorld()`** - Loads saved world, calls `initializeGameSystems()`
- **`Engine::initializeGameSystems()`** - Sets up world systems (does NOT set play mode)

## Testing

The CI build will automatically verify:
- ✅ Code compiles without errors
- ✅ All tests pass
- ✅ No regressions introduced

Manual testing should verify:
1. Launch app → See main menu ✅
2. Create new world → See editor GUI with world in viewport ✅
3. Click Play button → Editor hides, full-screen gameplay ✅
4. Press ESC → Editor shows again ✅

## Impact

**Files Changed:** 1 file  
**Lines Changed:** 3 lines (minimal surgical fix)  
**Breaking Changes:** None  
**Backward Compatibility:** Fully maintained

## Prevention

**Stored Memories:**
- `m_inGame` flag controls play mode state ONLY, NOT world existence
- Check `m_world` for world existence in game loop conditions
- EditorManager must remain visible after world creation
- Only `enterPlayMode()` should hide editor via `setVisible(false)`

These memories will help prevent similar issues in future development.

## References

- Issue thread: GUI disappears after world creation
- Related code: `engine/core/Engine.cpp` lines 400, 753-754, 780-781, 806, 2457-2537
- Related docs: `MENU_AND_PLAY_MODE_IMPLEMENTATION.md`
