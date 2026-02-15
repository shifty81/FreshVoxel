#pragma once
#include <memory>

#include "DebugConsole.h"
#include "DebugRenderer.h"
#include "MemoryTracker.h"
#include "PerformanceProfiler.h"

namespace fresh
{
namespace devtools
{

/**
 * @brief Manages all development tools
 *
 * Central hub for debug rendering, performance profiling, memory tracking,
 * and debug console.
 */
class DevToolsManager
{
public:
    DevToolsManager();
    ~DevToolsManager();

    // Lifecycle
    void initialize();
    void shutdown();
    void update(float deltaTime);
    void render();

    // Get individual tools
    DebugRenderer& getDebugRenderer()
    {
        return debugRenderer;
    }
    PerformanceProfiler& getProfiler()
    {
        return profiler;
    }
    MemoryTracker& getMemoryTracker()
    {
        return memoryTracker;
    }
    DebugConsole& getConsole()
    {
        return console;
    }

    // Enable/disable all tools
    void setEnabled(bool isEnabled);
    bool isEnabled() const
    {
        return enabled;
    }

    // Toggle console
    void toggleConsole()
    {
        console.toggle();
    }

private:
    DebugRenderer debugRenderer;
    PerformanceProfiler profiler;
    MemoryTracker memoryTracker;
    DebugConsole console;
    bool enabled;

    void setupConsoleCommands();
};

} // namespace devtools
} // namespace fresh
