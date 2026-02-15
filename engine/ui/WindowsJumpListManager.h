#pragma once

#ifdef _WIN32

#include <string>
#include <vector>

namespace fresh
{

/**
 * @brief Manages Windows Jump Lists for recent files and quick actions
 * 
 * Jump Lists provide quick access to recent worlds and common actions
 * from the Windows taskbar. Requires Windows 7 or later.
 */
class WindowsJumpListManager
{
public:
    /**
     * @brief Structure representing a recent file entry
     */
    struct RecentFile
    {
        std::string title;        // Display title
        std::string path;         // Full file path
        std::string description;  // Optional description
        std::string iconPath;     // Optional custom icon path
    };

    /**
     * @brief Structure representing a task/action
     */
    struct Task
    {
        std::string title;        // Display title
        std::string arguments;    // Command-line arguments
        std::string description;  // Optional description
        std::string iconPath;     // Optional custom icon path
    };

    WindowsJumpListManager();
    ~WindowsJumpListManager();

    /**
     * @brief Initialize the Jump List manager
     * @param appUserModelId Application User Model ID (optional, auto-generated if empty)
     * @return true if successful
     */
    bool initialize(const std::string& appUserModelId = "");

    /**
     * @brief Add a recent world to the jump list
     * @param title Display title (e.g., "My World")
     * @param path Full path to the world file
     * @param description Optional description
     * @return true if successful
     */
    bool addRecentWorld(const std::string& title, 
                       const std::string& path,
                       const std::string& description = "");

    /**
     * @brief Clear all recent worlds from jump list
     * @return true if successful
     */
    bool clearRecentWorlds();

    /**
     * @brief Add a custom task to the jump list
     * @param title Task title (e.g., "Create New Project")
     * @param arguments Command-line arguments
     * @param description Optional description
     * @return true if successful
     */
    bool addTask(const std::string& title,
                const std::string& arguments,
                const std::string& description = "");

    /**
     * @brief Clear all custom tasks from jump list
     * @return true if successful
     */
    bool clearTasks();

    /**
     * @brief Update the jump list with current items
     * @return true if successful
     */
    bool updateJumpList();

    /**
     * @brief Get the number of recent worlds in the jump list
     */
    size_t getRecentWorldCount() const { return m_recentWorlds.size(); }

    /**
     * @brief Check if jump lists are supported on this system
     */
    static bool isSupported();

private:
    bool initializeCOM();
    void cleanupCOM();
    
    std::string m_appUserModelId;
    std::vector<RecentFile> m_recentWorlds;
    std::vector<Task> m_tasks;
    bool m_initialized;
    void* m_customDestinationList;  // ICustomDestinationList*
    bool m_comInitialized;
};

} // namespace fresh

#endif // _WIN32
