#pragma once

#include "assets/AssetManager.h"

// C API exports for P/Invoke interop
// These functions provide a C-compatible interface to the C++ AssetManager

#ifdef _WIN32
    #define EXPORT_API extern "C" __declspec(dllexport)
#else
    #define EXPORT_API extern "C" __attribute__((visibility("default")))
#endif

namespace fresh
{

// Asset Manager C API
EXPORT_API AssetManager* AssetManager_GetInstance();

EXPORT_API bool AssetManager_Initialize(AssetManager* instance, const char* assetRootPath);

EXPORT_API void AssetManager_Shutdown(AssetManager* instance);

EXPORT_API bool AssetManager_ProcessNewAsset(AssetManager* instance, const char* sourcePath,
                                             const char* assetType);

EXPORT_API const char* AssetManager_GetAssetTypeFolder(AssetManager* instance,
                                                       const char* assetType);

EXPORT_API bool AssetManager_LoadAssetsFromLua(AssetManager* instance, const char* configPath);

} // namespace fresh
