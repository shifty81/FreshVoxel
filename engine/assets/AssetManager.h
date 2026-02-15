#pragma once

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

// Forward declare Lua state
struct lua_State;

namespace fresh
{

/**
 * @brief Core Asset Manager for handling dynamic asset loading
 *
 * Manages asset loading, file organization, and Lua configuration updates.
 * Integrates with C# FileSystemWatcher and drag-and-drop functionality.
 */
class AssetManager
{
public:
    /**
     * @brief Get singleton instance
     */
    static AssetManager* GetInstance();

    /**
     * @brief Initialize the asset manager
     * @param assetRootPath Root path for all assets
     */
    bool Initialize(const std::string& assetRootPath);

    /**
     * @brief Shutdown and cleanup
     */
    void Shutdown();

    /**
     * @brief Process a newly dropped asset
     * @param sourcePath Full path to the dropped file
     * @param assetType Type of asset (Texture, Model, Script, Audio, etc.)
     * @return true if successfully processed
     */
    bool ProcessNewAsset(const std::string& sourcePath, const std::string& assetType);

    /**
     * @brief Get the destination folder for an asset type
     * @param assetType Type of asset
     * @return Folder path relative to asset root
     */
    std::string GetAssetTypeFolder(const std::string& assetType) const;

    /**
     * @brief Update Lua configuration file with new asset
     * @param assetPath Path to the asset (relative to asset root)
     * @param assetType Type of asset
     * @param assetName Name/identifier for the asset
     * @return true if successfully updated
     */
    bool UpdateLuaConfiguration(const std::string& assetPath, const std::string& assetType,
                                const std::string& assetName);

    /**
     * @brief Load assets from Lua configuration file
     * @param configPath Path to asset_config.lua
     * @return true if successfully loaded
     */
    bool LoadAssetsFromLua(const std::string& configPath);

    /**
     * @brief Validate an asset file
     * @param filePath Path to the asset file
     * @param assetType Expected asset type
     * @return true if valid
     */
    bool ValidateAsset(const std::string& filePath, const std::string& assetType) const;

    /**
     * @brief Get list of supported asset types
     */
    std::vector<std::string> GetSupportedAssetTypes() const;

    /**
     * @brief Callback for asset import events
     */
    using AssetImportCallback =
        std::function<void(const std::string& path, const std::string& type)>;

    /**
     * @brief Register a callback for when assets are imported
     */
    void RegisterImportCallback(const std::string& callbackName, AssetImportCallback callback);

    /**
     * @brief Get the asset root path
     */
    const std::string& GetAssetRoot() const
    {
        return assetRootPath_;
    }

private:
    AssetManager();
    ~AssetManager();

    // Singleton - delete copy/move
    AssetManager(const AssetManager&) = delete;
    AssetManager& operator=(const AssetManager&) = delete;

    /**
     * @brief Initialize Lua state
     */
    bool InitializeLua();

    /**
     * @brief Close Lua state
     */
    void CloseLua();

    /**
     * @brief Fire asset imported callbacks
     */
    void OnAssetImported(const std::string& path, const std::string& type);

    /**
     * @brief Get file extension
     */
    std::string GetFileExtension(const std::string& path) const;

    /**
     * @brief Extract filename from path
     */
    std::string GetFileName(const std::string& path) const;

    /**
     * @brief Create directory if it doesn't exist
     */
    bool EnsureDirectoryExists(const std::string& path);

    // Member variables
    static AssetManager* instance_;

    std::string assetRootPath_;
    std::string luaConfigPath_;
    lua_State* luaState_;

    std::map<std::string, AssetImportCallback> importCallbacks_;

    // Supported file extensions for each asset type
    std::map<std::string, std::vector<std::string>> assetTypeExtensions_;

    bool initialized_;
};

} // namespace fresh
