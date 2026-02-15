#include "devtools/DevToolsManager.h"

namespace fresh
{
namespace devtools
{

DevToolsManager::DevToolsManager() : enabled(true) {}

DevToolsManager::~DevToolsManager() {}

void DevToolsManager::initialize()
{
    console.initialize();
    setupConsoleCommands();
}

void DevToolsManager::shutdown()
{
    console.shutdown();
}

void DevToolsManager::update(float deltaTime)
{
    if (!enabled) {
        return;
    }

    // Update profiler
    profiler.beginFrame();

    // Update debug renderer
    debugRenderer.update(deltaTime);

    // Update console
    console.update(deltaTime);

    // End profiler frame
    profiler.endFrame();
}

void DevToolsManager::render()
{
    if (!enabled) {
        return;
    }

    // Render debug shapes
    debugRenderer.render();

    // Render console
    console.render();
}

void DevToolsManager::setEnabled(bool isEnabled)
{
    this->enabled = isEnabled;
    debugRenderer.setEnabled(isEnabled);
}

void DevToolsManager::setupConsoleCommands()
{
    // Performance profiler commands
    console.registerCommand(
        "fps",
        [this](const std::vector<std::string>& args) {
            (void)args; // Unused
            console.print("FPS: " + std::to_string(profiler.getFPS()));
        },
        "Show current FPS");

    console.registerCommand(
        "profile",
        [this](const std::vector<std::string>& args) {
            (void)args; // Unused
            console.print(profiler.getReport());
        },
        "Show performance profile");

    // Memory tracker commands
    console.registerCommand(
        "memory",
        [this](const std::vector<std::string>& args) {
            (void)args; // Unused
            console.print(memoryTracker.getReport());
        },
        "Show memory usage");

    // Debug renderer commands
    console.registerCommand(
        "debug_render",
        [this](const std::vector<std::string>& args) {
            if (args.empty()) {
                console.print("Debug rendering: " +
                              std::string(debugRenderer.isEnabled() ? "ON" : "OFF"));
            } else {
                bool enable = (args[0] == "on" || args[0] == "true" || args[0] == "1");
                debugRenderer.setEnabled(enable);
                console.print("Debug rendering: " + std::string(enable ? "ON" : "OFF"));
            }
        },
        "Toggle debug rendering (on/off)");

    // Dev tools enable/disable
    console.registerCommand(
        "devtools",
        [this](const std::vector<std::string>& args) {
            if (args.empty()) {
                console.print("Dev tools: " + std::string(isEnabled() ? "ON" : "OFF"));
            } else {
                bool enable = (args[0] == "on" || args[0] == "true" || args[0] == "1");
                setEnabled(enable);
                console.print("Dev tools: " + std::string(enable ? "ON" : "OFF"));
            }
        },
        "Toggle all dev tools (on/off)");
}

} // namespace devtools
} // namespace fresh
