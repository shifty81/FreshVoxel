#include "devtools/GitManager.h"

#include "core/Logger.h"

#include <algorithm>
#include <array>
#include <cstdio>
#include <filesystem>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#else
#include <cerrno>
#include <cstring>
#include <sys/wait.h>
#include <unistd.h>
#endif

namespace fs = std::filesystem;

namespace fresh
{

namespace
{
    // Maximum number of commits to retrieve in history queries
    constexpr int MAX_COMMIT_HISTORY = 100;

    // Minimum length of a git status --porcelain line ("XY " + 1 char filename)
    constexpr size_t MIN_STATUS_LINE_LENGTH = 4;

#ifdef _WIN32
    // Timeout for git operations in milliseconds
    constexpr unsigned long GIT_OPERATION_TIMEOUT_MS = 30000;
#endif
} // anonymous namespace

GitManager::GitManager() = default;

GitManager::~GitManager() = default;

bool GitManager::initialize(const std::string& repoPath)
{
    if (repoPath.empty()) {
        LOG_ERROR_C("GitManager::initialize - empty repository path", "GitManager");
        return false;
    }

    // Check that the path exists and contains a .git directory
    fs::path gitDir = fs::path(repoPath) / ".git";
    if (!fs::exists(gitDir)) {
        LOG_ERROR_C("GitManager::initialize - not a git repository: " + repoPath, "GitManager");
        return false;
    }

    m_repoPath = repoPath;

    // Verify git is accessible
    if (!isGitAvailable()) {
        LOG_ERROR_C("GitManager::initialize - git CLI not found", "GitManager");
        m_repoPath.clear();
        return false;
    }

    m_initialized = true;
    LOG_INFO_C("GitManager initialized for repository: " + repoPath, "GitManager");
    return true;
}

bool GitManager::isGitAvailable() const
{
    GitResult result = executeGit({"--version"});
    return result.success && result.output.find("git version") != std::string::npos;
}

// --- Status ---

std::vector<GitFileStatus> GitManager::getStatus()
{
    std::vector<GitFileStatus> statuses;
    if (!m_initialized) {
        return statuses;
    }

    GitResult result = executeGit({"status", "--porcelain=v1"});
    if (!result.success) {
        LOG_ERROR_C("GitManager::getStatus failed: " + result.errorMessage, "GitManager");
        return statuses;
    }

    // Parse porcelain v1 output: XY filename
    std::istringstream stream(result.output);
    std::string line;
    while (std::getline(stream, line)) {
        if (line.size() < MIN_STATUS_LINE_LENGTH) {
            continue; // Need at least "XY " + 1 char filename
        }

        GitFileStatus status;
        status.indexState = parseStatusChar(line[0]);
        status.workTreeState = parseStatusChar(line[1]);
        status.path = line.substr(3); // Skip "XY "

        // Remove quotes if present (git quotes paths with special chars)
        if (!status.path.empty() && status.path.front() == '"' && status.path.back() == '"') {
            status.path = status.path.substr(1, status.path.size() - 2);
        }

        statuses.push_back(std::move(status));
    }

    return statuses;
}

bool GitManager::hasUncommittedChanges()
{
    if (!m_initialized) {
        return false;
    }

    GitResult result = executeGit({"status", "--porcelain=v1"});
    return result.success && !result.output.empty();
}

// --- Staging ---

GitResult GitManager::stageFiles(const std::vector<std::string>& files)
{
    if (!m_initialized) {
        return {false, "", "GitManager not initialized", -1};
    }

    if (files.empty()) {
        return {true, "", "", 0};
    }

    // Validate all file paths before executing
    for (const auto& file : files) {
        if (!isValidGitArgument(file)) {
            return {false, "", "Invalid file path: " + file, -1};
        }
    }

    std::vector<std::string> args = {"add", "--"};
    args.insert(args.end(), files.begin(), files.end());
    return executeGit(args);
}

GitResult GitManager::stageAll()
{
    if (!m_initialized) {
        return {false, "", "GitManager not initialized", -1};
    }

    return executeGit({"add", "."});
}

// --- Commit ---

GitResult GitManager::commit(const std::string& message)
{
    if (!m_initialized) {
        return {false, "", "GitManager not initialized", -1};
    }

    if (message.empty()) {
        return {false, "", "Commit message cannot be empty", -1};
    }

    return executeGit({"commit", "-m", message});
}

// --- Remote operations ---

GitResult GitManager::pull(const std::string& remote)
{
    if (!m_initialized) {
        return {false, "", "GitManager not initialized", -1};
    }

    if (!isValidGitArgument(remote)) {
        return {false, "", "Invalid remote name: " + remote, -1};
    }

    return executeGit({"pull", remote});
}

GitResult GitManager::push(const std::string& remote)
{
    if (!m_initialized) {
        return {false, "", "GitManager not initialized", -1};
    }

    if (!isValidGitArgument(remote)) {
        return {false, "", "Invalid remote name: " + remote, -1};
    }

    return executeGit({"push", remote});
}

// --- History ---

std::vector<GitCommitInfo> GitManager::getRecentCommits(int count)
{
    std::vector<GitCommitInfo> commits;
    if (!m_initialized || count <= 0) {
        return commits;
    }

    // Clamp count to reasonable limit
    if (count > MAX_COMMIT_HISTORY) {
        count = MAX_COMMIT_HISTORY;
    }

    // Use a delimiter that won't appear in commit data
    GitResult result = executeGit(
        {"log", "--format=%H%n%an%n%aI%n%s", "-n", std::to_string(count)});
    if (!result.success) {
        return commits;
    }

    // Parse: every 4 lines is one commit (hash, author, date, message)
    std::istringstream stream(result.output);
    std::string hash, author, date, message;
    while (std::getline(stream, hash)) {
        if (hash.empty()) {
            continue;
        }
        if (!std::getline(stream, author)) break;
        if (!std::getline(stream, date)) break;
        if (!std::getline(stream, message)) break;

        GitCommitInfo info;
        info.hash = hash;
        info.author = author;
        info.date = date;
        info.message = message;
        commits.push_back(std::move(info));
    }

    return commits;
}

// --- Branch operations ---

std::string GitManager::getCurrentBranch()
{
    if (!m_initialized) {
        return "";
    }

    GitResult result = executeGit({"rev-parse", "--abbrev-ref", "HEAD"});
    if (!result.success) {
        return "";
    }

    // Trim trailing newline
    std::string branch = result.output;
    while (!branch.empty() && (branch.back() == '\n' || branch.back() == '\r')) {
        branch.pop_back();
    }
    return branch;
}

std::vector<std::string> GitManager::listBranches()
{
    std::vector<std::string> branches;
    if (!m_initialized) {
        return branches;
    }

    GitResult result = executeGit({"branch", "--format=%(refname:short)"});
    if (!result.success) {
        return branches;
    }

    std::istringstream stream(result.output);
    std::string line;
    while (std::getline(stream, line)) {
        // Trim whitespace
        while (!line.empty() && (line.front() == ' ' || line.front() == '*')) {
            line.erase(line.begin());
        }
        while (!line.empty() && (line.back() == '\n' || line.back() == '\r' || line.back() == ' ')) {
            line.pop_back();
        }
        if (!line.empty()) {
            branches.push_back(line);
        }
    }

    return branches;
}

GitResult GitManager::switchBranch(const std::string& branch)
{
    if (!m_initialized) {
        return {false, "", "GitManager not initialized", -1};
    }

    if (!isValidGitArgument(branch)) {
        return {false, "", "Invalid branch name: " + branch, -1};
    }

    return executeGit({"checkout", branch});
}

GitResult GitManager::createBranch(const std::string& name)
{
    if (!m_initialized) {
        return {false, "", "GitManager not initialized", -1};
    }

    if (!isValidGitArgument(name)) {
        return {false, "", "Invalid branch name: " + name, -1};
    }

    return executeGit({"branch", name});
}

// --- Private helpers ---

GitResult GitManager::executeGit(const std::vector<std::string>& args) const
{
    GitResult result;

#ifdef _WIN32
    // Windows: Use CreateProcess with pipe for output capture
    // Build command line: "git" arg1 arg2 ...
    std::string cmdLine = "git";
    for (const auto& arg : args) {
        cmdLine += " ";
        // Quote arguments that contain spaces
        if (arg.find(' ') != std::string::npos) {
            cmdLine += "\"" + arg + "\"";
        } else {
            cmdLine += arg;
        }
    }

    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = nullptr;

    HANDLE hStdOutRead = nullptr;
    HANDLE hStdOutWrite = nullptr;
    HANDLE hStdErrRead = nullptr;
    HANDLE hStdErrWrite = nullptr;

    if (!CreatePipe(&hStdOutRead, &hStdOutWrite, &sa, 0) ||
        !CreatePipe(&hStdErrRead, &hStdErrWrite, &sa, 0)) {
        result.errorMessage = "Failed to create pipes";
        return result;
    }

    SetHandleInformation(hStdOutRead, HANDLE_FLAG_INHERIT, 0);
    SetHandleInformation(hStdErrRead, HANDLE_FLAG_INHERIT, 0);

    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.hStdOutput = hStdOutWrite;
    si.hStdError = hStdErrWrite;
    si.dwFlags |= STARTF_USESTDHANDLES;
    ZeroMemory(&pi, sizeof(pi));

    const char* workDir = m_repoPath.empty() ? nullptr : m_repoPath.c_str();

    if (CreateProcessA(nullptr, const_cast<char*>(cmdLine.c_str()),
                        nullptr, nullptr, TRUE, CREATE_NO_WINDOW,
                        nullptr, workDir, &si, &pi)) {
        CloseHandle(hStdOutWrite);
        CloseHandle(hStdErrWrite);

        // Read stdout
        char buffer[4096];
        DWORD bytesRead;
        while (ReadFile(hStdOutRead, buffer, sizeof(buffer) - 1, &bytesRead, nullptr) && bytesRead > 0) {
            buffer[bytesRead] = '\0';
            result.output += buffer;
        }

        // Read stderr
        std::string errOutput;
        while (ReadFile(hStdErrRead, buffer, sizeof(buffer) - 1, &bytesRead, nullptr) && bytesRead > 0) {
            buffer[bytesRead] = '\0';
            errOutput += buffer;
        }

        WaitForSingleObject(pi.hProcess, GIT_OPERATION_TIMEOUT_MS);

        DWORD exitCode;
        GetExitCodeProcess(pi.hProcess, &exitCode);
        result.exitCode = static_cast<int>(exitCode);
        result.success = (exitCode == 0);
        if (!result.success) {
            result.errorMessage = errOutput;
        }

        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    } else {
        result.errorMessage = "CreateProcess failed for git";
        CloseHandle(hStdOutWrite);
        CloseHandle(hStdErrWrite);
    }

    CloseHandle(hStdOutRead);
    CloseHandle(hStdErrRead);

#else
    // Linux/macOS: Use fork + exec with pipe for output capture
    int stdoutPipe[2];
    int stderrPipe[2];

    if (pipe(stdoutPipe) != 0 || pipe(stderrPipe) != 0) {
        result.errorMessage = std::string("pipe() failed: ") + std::strerror(errno);
        return result;
    }

    pid_t pid = fork();

    if (pid == 0) {
        // Child process
        close(stdoutPipe[0]); // Close read end
        close(stderrPipe[0]);

        dup2(stdoutPipe[1], STDOUT_FILENO);
        dup2(stderrPipe[1], STDERR_FILENO);
        close(stdoutPipe[1]);
        close(stderrPipe[1]);

        // Change to repo directory
        if (!m_repoPath.empty()) {
            if (chdir(m_repoPath.c_str()) != 0) {
                _exit(127);
            }
        }

        // Build argv: ["git", arg1, arg2, ..., nullptr]
        std::vector<const char*> argv;
        argv.push_back("git");
        for (const auto& arg : args) {
            argv.push_back(arg.c_str());
        }
        argv.push_back(nullptr);

        execvp("git", const_cast<char* const*>(argv.data()));

        // If execvp returns, it failed
        _exit(127);
    } else if (pid > 0) {
        // Parent process
        close(stdoutPipe[1]); // Close write ends
        close(stderrPipe[1]);

        // Read stdout
        char buffer[4096];
        ssize_t bytesRead;
        while ((bytesRead = read(stdoutPipe[0], buffer, sizeof(buffer) - 1)) > 0) {
            buffer[bytesRead] = '\0';
            result.output += buffer;
        }
        close(stdoutPipe[0]);

        // Read stderr
        std::string errOutput;
        while ((bytesRead = read(stderrPipe[0], buffer, sizeof(buffer) - 1)) > 0) {
            buffer[bytesRead] = '\0';
            errOutput += buffer;
        }
        close(stderrPipe[0]);

        int status;
        waitpid(pid, &status, 0);

        if (WIFEXITED(status)) {
            result.exitCode = WEXITSTATUS(status);
            result.success = (result.exitCode == 0);
        }

        if (!result.success) {
            result.errorMessage = errOutput;
        }
    } else {
        // Fork failed
        close(stdoutPipe[0]);
        close(stdoutPipe[1]);
        close(stderrPipe[0]);
        close(stderrPipe[1]);
        result.errorMessage = std::string("fork() failed: ") + std::strerror(errno);
    }
#endif

    return result;
}

GitFileState GitManager::parseStatusChar(char c)
{
    switch (c) {
    case ' ':
        return GitFileState::Unmodified;
    case 'M':
        return GitFileState::Modified;
    case 'A':
        return GitFileState::Added;
    case 'D':
        return GitFileState::Deleted;
    case 'R':
        return GitFileState::Renamed;
    case '?':
        return GitFileState::Untracked;
    case '!':
        return GitFileState::Ignored;
    case 'U':
        return GitFileState::Conflicted;
    default:
        return GitFileState::Unknown;
    }
}

bool GitManager::isValidGitArgument(const std::string& arg)
{
    if (arg.empty()) {
        return false;
    }

    // Reject arguments starting with '-' to prevent option injection
    // (except for well-known flags passed internally)
    // This check is for user-provided values like branch names, file paths, etc.
    if (arg[0] == '-') {
        return false;
    }

    // Reject null bytes
    if (arg.find('\0') != std::string::npos) {
        return false;
    }

    return true;
}

} // namespace fresh
