#include "devtools/DebugConsole.h"

#include <algorithm>
#include <sstream>

namespace fresh
{
namespace devtools
{

DebugConsole::DebugConsole() : visible(false), historyIndex(-1), maxLogLines(100) {}

DebugConsole::~DebugConsole() {}

void DebugConsole::initialize()
{
    registerDefaultCommands();
}

void DebugConsole::shutdown()
{
    commands.clear();
    history.clear();
    outputLog.clear();
}

void DebugConsole::update(float deltaTime)
{
    (void)deltaTime; // Unused - placeholder for future implementation
    // Console update logic (handle input, etc.)
}

void DebugConsole::render()
{
    if (!visible) {
        return;
    }

    // Render console UI
    // This would integrate with ImGui or custom UI system
}

void DebugConsole::toggle()
{
    visible = !visible;
}

void DebugConsole::registerCommand(const std::string& name, ConsoleCommand callback,
                                   const std::string& description)
{
    CommandInfo info;
    info.callback = callback;
    info.description = description;
    commands[name] = info;
}

void DebugConsole::unregisterCommand(const std::string& name)
{
    commands.erase(name);
}

void DebugConsole::executeCommand(const std::string& commandLine)
{
    if (commandLine.empty()) {
        return;
    }

    // Add to history
    history.push_back(commandLine);
    historyIndex = static_cast<int>(history.size());

    // Echo command
    print("> " + commandLine);

    // Parse and execute
    parseAndExecute(commandLine);
}

void DebugConsole::print(const std::string& message)
{
    outputLog.push_back(message);

    // Limit log size
    while (outputLog.size() > maxLogLines) {
        outputLog.erase(outputLog.begin());
    }
}

void DebugConsole::printError(const std::string& message)
{
    print("[ERROR] " + message);
}

void DebugConsole::printWarning(const std::string& message)
{
    print("[WARNING] " + message);
}

void DebugConsole::registerDefaultCommands()
{
    // Help command
    registerCommand(
        "help",
        [this](const std::vector<std::string>& args) {
            (void)args; // Unused
            print("Available commands:");
            for (const auto& pair : commands) {
                std::string desc =
                    pair.second.description.empty() ? "No description" : pair.second.description;
                print("  " + pair.first + " - " + desc);
            }
        },
        "Show available commands");

    // Clear command
    registerCommand(
        "clear",
        [this](const std::vector<std::string>& args) {
            (void)args; // Unused
            outputLog.clear();
        },
        "Clear console output");

    // Quit command
    registerCommand(
        "quit",
        [this](const std::vector<std::string>& args) {
            (void)args; // Unused
            print("Closing console...");
            hide();
        },
        "Close the console");
}

void DebugConsole::parseAndExecute(const std::string& commandLine)
{
    // Split command line into tokens
    std::istringstream iss(commandLine);
    std::vector<std::string> tokens;
    std::string token;

    while (iss >> token) {
        tokens.push_back(token);
    }

    if (tokens.empty()) {
        return;
    }

    // First token is the command name
    std::string cmdName = tokens[0];
    std::vector<std::string> args(tokens.begin() + 1, tokens.end());

    // Find and execute command
    auto it = commands.find(cmdName);
    if (it != commands.end()) {
        try {
            it->second.callback(args);
        } catch (const std::exception& e) {
            printError("Command failed: " + std::string(e.what()));
        }
    } else {
        printError("Unknown command: " + cmdName);
    }
}

} // namespace devtools
} // namespace fresh
