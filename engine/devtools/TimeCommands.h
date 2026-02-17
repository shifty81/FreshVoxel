#pragma once

namespace fresh
{

class TimeManager;

namespace devtools
{

class DebugConsole;

/**
 * @brief Registers time-related commands with the debug console
 * 
 * Provides console commands for controlling the day/night cycle:
 * - time: Show current time
 * - time set <dawn/noon/dusk/midnight/ticks>: Set time
 * - time rate <speed>: Set time progression speed
 * - time pause: Pause time
 * - time resume: Resume time
 * 
 * @note The console and timeManager pointers are captured by the registered
 *       command callbacks. Ensure unregisterCommands() is called before
 *       either object is destroyed to prevent use-after-free.
 */
class TimeCommands
{
public:
    /**
     * @brief Register all time commands with the debug console
     * @param console Pointer to the debug console (must remain valid until unregisterCommands)
     * @param timeManager Pointer to the TimeManager instance (must remain valid until unregisterCommands)
     */
    static void registerCommands(DebugConsole* console, TimeManager* timeManager);

    /**
     * @brief Unregister all time commands from the debug console
     * @param console Pointer to the debug console
     * @note Must be called before console or timeManager objects are destroyed
     */
    static void unregisterCommands(DebugConsole* console);
};

} // namespace devtools
} // namespace fresh
