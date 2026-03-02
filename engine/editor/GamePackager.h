#pragma once

#include <functional>
#include <string>
#include <vector>

namespace fresh
{

/**
 * @brief Configuration for game packaging
 *
 * Defines what to include in the distributable game package.
 */
struct PackageConfig
{
    std::string projectName;         ///< Name of the game project
    std::string projectPath;         ///< Root path of the project
    std::string outputPath;          ///< Directory where the package will be created
    std::string worldSavePath;       ///< Path to the world save file to include
    bool includeAssets = true;       ///< Include asset packs
    bool includeShaders = true;      ///< Include shader files
    bool includeSounds = true;       ///< Include sound files
    bool includeTextures = true;     ///< Include texture files
    bool includeConfig = true;       ///< Include configuration files
    bool includeScripts = true;      ///< Include Lua scripts
};

/**
 * @brief Result of a packaging operation
 */
struct PackageResult
{
    bool success = false;                ///< Whether packaging succeeded
    std::string outputFilePath;          ///< Path to the created package file
    std::string errorMessage;            ///< Error message if failed
    std::vector<std::string> filesIncluded; ///< List of files included in package
    size_t totalSizeBytes = 0;           ///< Total package size in bytes
};

/**
 * @brief Packages the current game project for distribution
 *
 * Atlas-style workflow: from the editor, build a distributable package
 * that users can play-test. The package includes:
 * - World save files
 * - Asset packs
 * - Shaders, textures, sounds
 * - Configuration files
 * - A manifest describing the package contents
 *
 * The packaged game can be run with FreshClient or FreshRuntime:
 *   FreshClient --project <package_dir>
 *   FreshRuntime --project <package_dir>
 */
class GamePackager
{
public:
    GamePackager();
    ~GamePackager();

    /**
     * @brief Build a distributable game package
     * @param config Package configuration
     * @return Result of the packaging operation
     */
    PackageResult buildPackage(const PackageConfig& config);

    /**
     * @brief Collect files from a directory matching a pattern
     * @param rootDir Directory to scan
     * @param extensions File extensions to include (e.g. {".lua", ".json"})
     * @param outFiles Output list of file paths
     */
    static void collectFiles(const std::string& rootDir,
                             const std::vector<std::string>& extensions,
                             std::vector<std::string>& outFiles);

    /**
     * @brief Create a package manifest JSON
     * @param config Package configuration
     * @param files Files included in the package
     * @return Manifest JSON string
     */
    static std::string createManifest(const PackageConfig& config,
                                      const std::vector<std::string>& files);

    /**
     * @brief Set progress callback
     * @param callback Function called with (currentFile, fileIndex, totalFiles)
     */
    void setProgressCallback(std::function<void(const std::string&, int, int)> callback)
    {
        m_progressCallback = callback;
    }

private:
    bool copyFile(const std::string& src, const std::string& dst);
    bool createDirectory(const std::string& path);
    std::string getRelativePath(const std::string& filePath, const std::string& basePath);

    std::function<void(const std::string&, int, int)> m_progressCallback;
};

} // namespace fresh
