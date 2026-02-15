#pragma once

#include <string>
#include <vector>

namespace fresh {

/**
 * @brief Manager for native file dialogs
 * 
 * Provides a cross-platform interface for opening file dialogs.
 * Uses Native File Dialog Extended (NFD) when available.
 */
class FileDialogManager {
public:
    /**
     * @brief File dialog filter
     */
    struct Filter {
        std::string name;        ///< Display name (e.g., "World Files")
        std::string extensions;  ///< Extensions (e.g., "world,dat")
    };

    /**
     * @brief Open a file selection dialog
     * @param filters File type filters
     * @param defaultPath Default directory to open (empty for current)
     * @return Selected file path (empty if canceled)
     */
    static std::string openFile(const std::vector<Filter>& filters = {}, 
                                const std::string& defaultPath = "");

    /**
     * @brief Open a file save dialog
     * @param filters File type filters
     * @param defaultPath Default directory to open (empty for current)
     * @param defaultName Default file name
     * @return Selected file path (empty if canceled)
     */
    static std::string saveFile(const std::vector<Filter>& filters = {},
                                const std::string& defaultPath = "",
                                const std::string& defaultName = "");

    /**
     * @brief Open a multiple file selection dialog
     * @param filters File type filters
     * @param defaultPath Default directory to open (empty for current)
     * @return Selected file paths (empty if canceled)
     */
    static std::vector<std::string> openMultipleFiles(const std::vector<Filter>& filters = {},
                                                      const std::string& defaultPath = "");

    /**
     * @brief Open a folder selection dialog
     * @param defaultPath Default directory to open (empty for current)
     * @return Selected folder path (empty if canceled)
     */
    static std::string pickFolder(const std::string& defaultPath = "");

    /**
     * @brief Check if file dialogs are available
     * @return true if NFD is available
     */
    static bool isAvailable();
};

} // namespace fresh
