#pragma once

#include <functional>
#include <string>

namespace fresh
{

/**
 * @brief Launch target for testing from the editor
 */
enum class LaunchTarget
{
    Client,  ///< Launch FreshClient for game testing
    Server,  ///< Launch FreshServer for multiplayer testing
    Runtime  ///< Launch FreshRuntime for standalone testing
};

/**
 * @brief Configuration for launching a client/server from the editor
 */
struct LaunchConfig
{
    LaunchTarget target = LaunchTarget::Client; ///< What to launch
    std::string projectPath;                    ///< Path to the project directory
    std::string worldPath;                      ///< Path to the world save file to load
    std::string executableDir;                  ///< Directory containing the executables
    int windowWidth = 1280;                     ///< Client window width
    int windowHeight = 720;                     ///< Client window height
    bool windowed = true;                       ///< Launch in windowed mode
};

/**
 * @brief Result of a launch operation
 */
struct LaunchResult
{
    bool success = false;       ///< Whether the launch succeeded
    std::string errorMessage;   ///< Error message if failed
    std::string commandLine;    ///< The command line that was executed
#ifdef _WIN32
    unsigned long processId = 0; ///< Process ID of the launched process (Windows)
#else
    int processId = 0;           ///< Process ID of the launched process (Linux/macOS)
#endif
};

/**
 * @brief Launches game client/server from the editor for live testing
 *
 * Atlas-style workflow: from the editor, launch FreshClient as a separate
 * process that directly interacts with the game being built. The client
 * loads the same world/project the editor is working on, allowing real-time
 * play-testing without packaging.
 *
 * Usage from editor:
 *   1. Save the current world (auto-saved before launch)
 *   2. Launch FreshClient with --project pointing to current project
 *   3. Client loads the saved world and starts gameplay
 *   4. Developer can iterate in editor, re-save, re-launch
 *
 * The launched process runs independently — closing the editor does not
 * close the client, and vice versa.
 */
class ClientLauncher
{
public:
    ClientLauncher();
    ~ClientLauncher();

    /**
     * @brief Launch a client/server/runtime process
     * @param config Launch configuration
     * @return Result of the launch operation
     */
    LaunchResult launch(const LaunchConfig& config);

    /**
     * @brief Launch FreshClient with the current project
     *
     * Convenience method that builds a LaunchConfig for client mode.
     * @param projectPath Path to the project directory
     * @param worldPath Path to the world save file
     * @return Result of the launch operation
     */
    LaunchResult launchClient(const std::string& projectPath, const std::string& worldPath);

    /**
     * @brief Launch FreshServer for multiplayer testing
     *
     * Convenience method that builds a LaunchConfig for server mode.
     * @param projectPath Path to the project directory
     * @param worldPath Path to the world save file
     * @return Result of the launch operation
     */
    LaunchResult launchServer(const std::string& projectPath, const std::string& worldPath);

    /**
     * @brief Get the executable name for a launch target
     * @param target The launch target
     * @return Executable name (e.g. "FreshClient", "FreshServer")
     */
    static std::string getExecutableName(LaunchTarget target);

    /**
     * @brief Build the command line for a launch configuration
     * @param config Launch configuration
     * @return Command line string
     */
    static std::string buildCommandLine(const LaunchConfig& config);

    /**
     * @brief Set the directory containing the built executables
     * @param dir Directory path
     */
    void setExecutableDir(const std::string& dir) { m_executableDir = dir; }

    /**
     * @brief Get the directory containing the built executables
     * @return Directory path
     */
    const std::string& getExecutableDir() const { return m_executableDir; }

private:
    std::string m_executableDir;
};

} // namespace fresh
