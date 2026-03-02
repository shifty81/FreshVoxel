#pragma once

#include <functional>
#include <string>
#include <vector>

namespace fresh
{

/**
 * @brief Status of a file in the git repository
 */
enum class GitFileState
{
    Unmodified,
    Modified,
    Added,
    Deleted,
    Renamed,
    Untracked,
    Ignored,
    Conflicted,
    Unknown
};

/**
 * @brief Information about a tracked file's status
 */
struct GitFileStatus
{
    std::string path;           ///< File path relative to repository root
    GitFileState indexState;    ///< Status in the staging area (index)
    GitFileState workTreeState; ///< Status in the working tree
};

/**
 * @brief Information about a commit
 */
struct GitCommitInfo
{
    std::string hash;    ///< Full commit SHA
    std::string author;  ///< Author name
    std::string date;    ///< Commit date (ISO 8601)
    std::string message; ///< Commit subject line
};

/**
 * @brief Result of a git operation
 */
struct GitResult
{
    bool success = false;     ///< Whether the operation succeeded
    std::string output;       ///< Standard output from the command
    std::string errorMessage; ///< Error message if failed
    int exitCode = -1;        ///< Process exit code
};

/**
 * @brief Git integration for the FreshVoxel editor
 *
 * Provides version control operations directly from the editor using
 * the system git CLI. Supports status, commit, pull, push, and branch
 * management.
 *
 * Security: All arguments are passed via argv arrays (fork+exec on
 * Linux/macOS, CreateProcess on Windows) — never through a shell —
 * to prevent command injection.
 */
class GitManager
{
public:
    GitManager();
    ~GitManager();

    /**
     * @brief Initialize with a repository path
     * @param repoPath Path to the repository root (must contain .git)
     * @return true if the path is a valid git repository
     */
    bool initialize(const std::string& repoPath);

    /**
     * @brief Check if git is available on the system
     * @return true if the git CLI is accessible
     */
    bool isGitAvailable() const;

    /**
     * @brief Check if the manager is initialized with a valid repository
     */
    [[nodiscard]] bool isInitialized() const noexcept { return m_initialized; }

    /**
     * @brief Get the repository root path
     */
    [[nodiscard]] const std::string& getRepoPath() const noexcept { return m_repoPath; }

    // --- Status ---

    /**
     * @brief Get the status of files in the working tree
     * @return Vector of file statuses
     */
    std::vector<GitFileStatus> getStatus();

    /**
     * @brief Check if the working tree has uncommitted changes
     * @return true if there are modified/added/deleted files
     */
    bool hasUncommittedChanges();

    // --- Staging ---

    /**
     * @brief Stage files for commit
     * @param files Paths to stage (relative to repo root)
     * @return Result of the operation
     */
    GitResult stageFiles(const std::vector<std::string>& files);

    /**
     * @brief Stage all changes (git add .)
     * @return Result of the operation
     */
    GitResult stageAll();

    // --- Commit ---

    /**
     * @brief Create a commit with a message
     * @param message Commit message
     * @return Result of the operation
     */
    GitResult commit(const std::string& message);

    // --- Remote operations ---

    /**
     * @brief Pull from a remote
     * @param remote Remote name (default: "origin")
     * @return Result of the operation
     */
    GitResult pull(const std::string& remote = "origin");

    /**
     * @brief Push to a remote
     * @param remote Remote name (default: "origin")
     * @return Result of the operation
     */
    GitResult push(const std::string& remote = "origin");

    // --- History ---

    /**
     * @brief Get recent commits
     * @param count Number of commits to retrieve (default: 10)
     * @return Vector of commit info
     */
    std::vector<GitCommitInfo> getRecentCommits(int count = 10);

    // --- Branch operations ---

    /**
     * @brief Get the current branch name
     * @return Branch name, or empty string on error
     */
    std::string getCurrentBranch();

    /**
     * @brief List all local branches
     * @return Vector of branch names
     */
    std::vector<std::string> listBranches();

    /**
     * @brief Switch to a branch
     * @param branch Branch name
     * @return Result of the operation
     */
    GitResult switchBranch(const std::string& branch);

    /**
     * @brief Create a new branch
     * @param name Branch name
     * @return Result of the operation
     */
    GitResult createBranch(const std::string& name);

private:
    /**
     * @brief Execute a git command safely using fork+exec (no shell)
     * @param args Arguments to pass to git (e.g. {"status", "--porcelain"})
     * @return Result of the command
     */
    GitResult executeGit(const std::vector<std::string>& args) const;

    /**
     * @brief Parse a git status --porcelain=v1 character into GitFileState
     */
    static GitFileState parseStatusChar(char c);

    /**
     * @brief Validate that a string is safe for use as a git argument
     *
     * Rejects strings containing shell metacharacters as an extra safety
     * measure, even though we don't use a shell.
     * @return true if the string is safe
     */
    static bool isValidGitArgument(const std::string& arg);

    std::string m_repoPath;
    bool m_initialized = false;
};

} // namespace fresh
