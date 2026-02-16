#pragma once

#include <string>

namespace fresh
{

/**
 * @brief Operating mode for the engine
 *
 * Determines which subsystems are initialized and how the engine behaves:
 * - Editor: Full editor UI with viewport, panels, menus, world editing tools
 * - Client: Game-only mode with rendering but no editor UI, connects to server
 * - Server: Headless mode with no window or rendering, runs game simulation
 * - Runtime: Standalone game mode with rendering, no editor UI
 */
enum class EngineMode
{
    Editor,   ///< Full editor with viewport panels, menus, world editing tools
    Client,   ///< Game client with rendering, no editor UI
    Server,   ///< Headless server, no window or rendering
    Runtime   ///< Standalone game player with rendering, no editor UI
};

/**
 * @brief Configuration for engine initialization
 *
 * Passed to Engine::initialize() to control which subsystems are created
 * and how the engine behaves. Each executable (FreshEditor, FreshClient,
 * FreshServer, FreshRuntime) creates an appropriate config for its role.
 */
struct EngineConfig
{
    EngineMode mode = EngineMode::Editor;         ///< Operating mode
    std::string windowTitle = "Fresh Voxel Engine"; ///< Window title
    int windowWidth = 1280;                        ///< Initial window width
    int windowHeight = 720;                        ///< Initial window height
    bool enableRendering = true;                   ///< Create renderer and window
    bool enableEditor = true;                      ///< Create editor UI panels
    bool enableNetworking = false;                 ///< Initialize networking subsystem
    bool headless = false;                         ///< Run without a window (server mode)
    bool autoLoadLastWorld = false;                ///< Auto-load last saved world on startup (client/runtime)

    /**
     * @brief Create a default config for the given mode
     *
     * Sets sensible defaults based on the mode:
     * - Editor: rendering + editor UI enabled
     * - Client: rendering enabled, editor disabled, networking enabled
     * - Server: headless, no rendering, no editor, networking enabled
     * - Runtime: rendering enabled, editor disabled
     */
    static EngineConfig createDefault(EngineMode mode)
    {
        EngineConfig config;
        config.mode = mode;

        switch (mode) {
        case EngineMode::Editor:
            config.windowTitle = "Fresh Voxel Engine - Editor";
            config.enableRendering = true;
            config.enableEditor = true;
            config.enableNetworking = false;
            config.headless = false;
            config.autoLoadLastWorld = false;
            break;

        case EngineMode::Client:
            config.windowTitle = "Fresh Voxel Engine";
            config.enableRendering = true;
            config.enableEditor = false;
            config.enableNetworking = true;
            config.headless = false;
            config.autoLoadLastWorld = true;
            break;

        case EngineMode::Server:
            config.windowTitle = "Fresh Server";
            config.enableRendering = false;
            config.enableEditor = false;
            config.enableNetworking = true;
            config.headless = true;
            config.autoLoadLastWorld = true;
            break;

        case EngineMode::Runtime:
            config.windowTitle = "Fresh Voxel Engine";
            config.enableRendering = true;
            config.enableEditor = false;
            config.enableNetworking = false;
            config.headless = false;
            config.autoLoadLastWorld = true;
            break;
        }

        return config;
    }

    /// Helper queries for common mode checks
    bool isEditor() const { return mode == EngineMode::Editor; }
    bool isClient() const { return mode == EngineMode::Client; }
    bool isServer() const { return mode == EngineMode::Server; }
    bool isRuntime() const { return mode == EngineMode::Runtime; }
    bool hasWindow() const { return enableRendering && !headless; }
};

/**
 * @brief Get a human-readable name for the engine mode
 */
inline const char* getEngineModeName(EngineMode mode)
{
    switch (mode) {
    case EngineMode::Editor:  return "Editor";
    case EngineMode::Client:  return "Client";
    case EngineMode::Server:  return "Server";
    case EngineMode::Runtime: return "Runtime";
    default:                  return "Unknown";
    }
}

} // namespace fresh
