#include "editor/ClientLauncher.h"

#include "core/Logger.h"

#include <cstdlib>
#include <filesystem>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/wait.h>
#include <cerrno>
#include <cstring>
#endif

namespace fs = std::filesystem;

namespace fresh
{

ClientLauncher::ClientLauncher() {}

ClientLauncher::~ClientLauncher() {}

LaunchResult ClientLauncher::launch(const LaunchConfig& config)
{
    LaunchResult result;

    std::string execName = getExecutableName(config.target);
    std::string execDir = config.executableDir.empty() ? m_executableDir : config.executableDir;

    // Build full executable path
    std::string execPath;
    if (!execDir.empty()) {
        execPath = execDir + "/" + execName;
    } else {
        // Try to find executable relative to current working directory
        execPath = execName;
    }

#ifdef _WIN32
    execPath += ".exe";
#endif

    // Check if executable exists
    if (!execDir.empty() && !fs::exists(execPath)) {
        result.errorMessage = "Executable not found: " + execPath;
        LOG_ERROR_C("ClientLauncher: " + result.errorMessage, "ClientLauncher");
        return result;
    }

    // Build command line
    result.commandLine = buildCommandLine(config);

    LOG_INFO_C("ClientLauncher: Launching " + execName + " with: " + result.commandLine,
               "ClientLauncher");

#ifdef _WIN32
    // Windows: Use CreateProcess for proper process separation
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    // Build full command line with executable path
    std::string fullCmdLine = "\"" + execPath + "\" " + result.commandLine;

    // Set working directory to project path if available
    const char* workingDir = nullptr;
    if (!config.projectPath.empty() && fs::exists(config.projectPath)) {
        workingDir = config.projectPath.c_str();
    }

    if (CreateProcessA(
            nullptr,                                       // Application name (nullptr = use command line)
            const_cast<char*>(fullCmdLine.c_str()),        // Command line
            nullptr,                                       // Process attributes
            nullptr,                                       // Thread attributes
            FALSE,                                         // Inherit handles
            CREATE_NEW_CONSOLE | DETACHED_PROCESS,         // Creation flags
            nullptr,                                       // Environment
            workingDir,                                    // Working directory
            &si,                                           // Startup info
            &pi                                            // Process info
        )) {
        result.success = true;
        result.processId = pi.dwProcessId;

        // Close handles - we don't need to track the process
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);

        LOG_INFO_C("ClientLauncher: Successfully launched " + execName + " (PID: " +
                       std::to_string(result.processId) + ")",
                   "ClientLauncher");
    } else {
        DWORD error = GetLastError();
        result.errorMessage =
            "CreateProcess failed with error " + std::to_string(error) + " for: " + fullCmdLine;
        LOG_ERROR_C("ClientLauncher: " + result.errorMessage, "ClientLauncher");
    }
#else
    // Linux/macOS: Use fork + exec for process creation
    pid_t pid = fork();

    if (pid == 0) {
        // Child process — exec the client
        std::vector<std::string> args;
        args.push_back(execPath);

        if (!config.projectPath.empty()) {
            args.push_back("--project");
            args.push_back(config.projectPath);
        }
        if (!config.worldPath.empty()) {
            args.push_back("--world");
            args.push_back(config.worldPath);
        }
        if (config.windowed) {
            args.push_back("--windowed");
        }
        args.push_back("--width");
        args.push_back(std::to_string(config.windowWidth));
        args.push_back("--height");
        args.push_back(std::to_string(config.windowHeight));

        // Build argv array
        std::vector<char*> argv;
        for (auto& arg : args) {
            argv.push_back(const_cast<char*>(arg.c_str()));
        }
        argv.push_back(nullptr);

        // Change working directory if project path is specified
        if (!config.projectPath.empty() && fs::exists(config.projectPath)) {
            if (chdir(config.projectPath.c_str()) != 0) {
                // Ignore chdir failure, just run from current dir
            }
        }

        execvp(argv[0], argv.data());

        // If execvp returns, it failed
        _exit(127);
    } else if (pid > 0) {
        // Parent process
        result.success = true;
        result.processId = pid;

        LOG_INFO_C("ClientLauncher: Successfully launched " + execName + " (PID: " +
                       std::to_string(result.processId) + ")",
                   "ClientLauncher");
    } else {
        result.errorMessage = std::string("fork() failed: ") + std::strerror(errno);
        LOG_ERROR_C("ClientLauncher: " + result.errorMessage, "ClientLauncher");
    }
#endif

    return result;
}

LaunchResult ClientLauncher::launchClient(const std::string& projectPath,
                                          const std::string& worldPath)
{
    LaunchConfig config;
    config.target = LaunchTarget::Client;
    config.projectPath = projectPath;
    config.worldPath = worldPath;
    config.executableDir = m_executableDir;
    return launch(config);
}

LaunchResult ClientLauncher::launchServer(const std::string& projectPath,
                                          const std::string& worldPath)
{
    LaunchConfig config;
    config.target = LaunchTarget::Server;
    config.projectPath = projectPath;
    config.worldPath = worldPath;
    config.executableDir = m_executableDir;
    return launch(config);
}

std::string ClientLauncher::getExecutableName(LaunchTarget target)
{
    switch (target) {
    case LaunchTarget::Client:
        return "FreshClient";
    case LaunchTarget::Server:
        return "FreshServer";
    case LaunchTarget::Runtime:
        return "FreshRuntime";
    default:
        return "FreshClient";
    }
}

std::string ClientLauncher::buildCommandLine(const LaunchConfig& config)
{
    std::ostringstream cmd;

    if (!config.projectPath.empty()) {
        cmd << "--project \"" << config.projectPath << "\" ";
    }
    if (!config.worldPath.empty()) {
        cmd << "--world \"" << config.worldPath << "\" ";
    }
    if (config.windowed) {
        cmd << "--windowed ";
    }
    cmd << "--width " << config.windowWidth << " ";
    cmd << "--height " << config.windowHeight;

    return cmd.str();
}

} // namespace fresh
