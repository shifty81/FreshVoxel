#pragma once
#include <functional>
#include <map>
#include <string>
#include <vector>

namespace fresh
{
namespace devtools
{

/**
 * @brief Console command callback
 */
using ConsoleCommand = std::function<void(const std::vector<std::string>& args)>;

/**
 * @brief Interactive debug console (runtime command console)
 *
 * Press ` key to toggle console
 */
class DebugConsole
{
public:
    DebugConsole();
    ~DebugConsole();

    // Console lifecycle
    void initialize();
    void shutdown();
    void update(float deltaTime);
    void render();

    // Toggle console
    void toggle();
    void show()
    {
        visible = true;
    }
    void hide()
    {
        visible = false;
    }
    bool isVisible() const
    {
        return visible;
    }

    // Command registration
    void registerCommand(const std::string& name, ConsoleCommand callback,
                         const std::string& description = "");
    void unregisterCommand(const std::string& name);

    // Execute command
    void executeCommand(const std::string& commandLine);

    // Output
    void print(const std::string& message);
    void printError(const std::string& message);
    void printWarning(const std::string& message);

    // Get command history
    const std::vector<std::string>& getHistory() const
    {
        return history;
    }

    // Get output log
    const std::vector<std::string>& getLog() const
    {
        return outputLog;
    }

private:
    bool visible;
    std::string inputBuffer;
    std::vector<std::string> history;
    std::vector<std::string> outputLog;
    int historyIndex;
    size_t maxLogLines;

    struct CommandInfo {
        ConsoleCommand callback;
        std::string description;
    };
    std::map<std::string, CommandInfo> commands;

    void registerDefaultCommands();
    void parseAndExecute(const std::string& commandLine);
};

} // namespace devtools
} // namespace fresh
