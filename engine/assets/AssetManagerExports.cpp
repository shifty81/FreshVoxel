#include "assets/AssetManagerExports.h"

#include <cstring>

namespace fresh
{

// Thread-local storage for string returns
thread_local char g_assetTypeFolderBuffer[256];

EXPORT_API AssetManager* AssetManager_GetInstance()
{
    return AssetManager::GetInstance();
}

EXPORT_API bool AssetManager_Initialize(AssetManager* instance, const char* assetRootPath)
{
    if (!instance || !assetRootPath) {
        return false;
    }
    return instance->Initialize(std::string(assetRootPath));
}

EXPORT_API void AssetManager_Shutdown(AssetManager* instance)
{
    if (instance) {
        instance->Shutdown();
    }
}

EXPORT_API bool AssetManager_ProcessNewAsset(AssetManager* instance, const char* sourcePath,
                                             const char* assetType)
{
    if (!instance || !sourcePath || !assetType) {
        return false;
    }
    return instance->ProcessNewAsset(std::string(sourcePath), std::string(assetType));
}

EXPORT_API const char* AssetManager_GetAssetTypeFolder(AssetManager* instance,
                                                       const char* assetType)
{
    if (!instance || !assetType) {
        return "";
    }

    std::string folder = instance->GetAssetTypeFolder(std::string(assetType));

    // Ensure we don't overflow the buffer
    size_t copyLen = std::min(folder.length(), sizeof(g_assetTypeFolderBuffer) - 1);
    memcpy(g_assetTypeFolderBuffer, folder.c_str(), copyLen);
    g_assetTypeFolderBuffer[copyLen] = '\0';

    return g_assetTypeFolderBuffer;
}

EXPORT_API bool AssetManager_LoadAssetsFromLua(AssetManager* instance, const char* configPath)
{
    if (!instance || !configPath) {
        return false;
    }
    return instance->LoadAssetsFromLua(std::string(configPath));
}

} // namespace fresh
