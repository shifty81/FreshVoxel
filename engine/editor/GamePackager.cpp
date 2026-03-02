#include "editor/GamePackager.h"

#include "core/Logger.h"

#include <algorithm>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace fs = std::filesystem;

namespace fresh
{

GamePackager::GamePackager() {}

GamePackager::~GamePackager() {}

PackageResult GamePackager::buildPackage(const PackageConfig& config)
{
    PackageResult result;

    // Validate inputs
    if (config.projectName.empty()) {
        result.errorMessage = "Project name is empty";
        LOG_ERROR_C("GamePackager: " + result.errorMessage, "GamePackager");
        return result;
    }

    if (config.outputPath.empty()) {
        result.errorMessage = "Output path is empty";
        LOG_ERROR_C("GamePackager: " + result.errorMessage, "GamePackager");
        return result;
    }

    LOG_INFO_C("GamePackager: Building package for '" + config.projectName + "'", "GamePackager");

    // Create output directory: <outputPath>/<projectName>/
    std::string packageDir = config.outputPath + "/" + config.projectName;
    if (!createDirectory(packageDir)) {
        result.errorMessage = "Failed to create package directory: " + packageDir;
        LOG_ERROR_C("GamePackager: " + result.errorMessage, "GamePackager");
        return result;
    }

    std::vector<std::string> allFiles;

    // 1. Copy world save file
    if (!config.worldSavePath.empty() && fs::exists(config.worldSavePath)) {
        std::string destDir = packageDir + "/saves";
        createDirectory(destDir);
        std::string destFile = destDir + "/" + fs::path(config.worldSavePath).filename().string();
        if (copyFile(config.worldSavePath, destFile)) {
            allFiles.push_back(destFile);
            LOG_INFO_C("GamePackager: Included world save: " + config.worldSavePath, "GamePackager");
        }
    }

    // 2. Copy asset packs
    if (config.includeAssets) {
        std::string assetDir = config.projectPath + "/asset_packs";
        if (fs::exists(assetDir)) {
            std::string destDir = packageDir + "/asset_packs";
            createDirectory(destDir);
            std::vector<std::string> assetFiles;
            collectFiles(assetDir, {".json", ".png", ".jpg", ".obj", ".mtl"}, assetFiles);
            for (const auto& file : assetFiles) {
                std::string relPath = getRelativePath(file, assetDir);
                std::string destFile = destDir + "/" + relPath;
                createDirectory(fs::path(destFile).parent_path().string());
                if (copyFile(file, destFile)) {
                    allFiles.push_back(destFile);
                }
            }
            LOG_INFO_C("GamePackager: Included " + std::to_string(assetFiles.size()) + " asset files",
                        "GamePackager");
        }
    }

    // 3. Copy shaders
    if (config.includeShaders) {
        std::string shaderDir = config.projectPath + "/shaders";
        if (fs::exists(shaderDir)) {
            std::string destDir = packageDir + "/shaders";
            createDirectory(destDir);
            std::vector<std::string> shaderFiles;
            collectFiles(shaderDir, {".hlsl", ".glsl", ".vert", ".frag", ".comp"}, shaderFiles);
            for (const auto& file : shaderFiles) {
                std::string relPath = getRelativePath(file, shaderDir);
                std::string destFile = destDir + "/" + relPath;
                createDirectory(fs::path(destFile).parent_path().string());
                if (copyFile(file, destFile)) {
                    allFiles.push_back(destFile);
                }
            }
            LOG_INFO_C(
                "GamePackager: Included " + std::to_string(shaderFiles.size()) + " shader files",
                "GamePackager");
        }
    }

    // 4. Copy sounds
    if (config.includeSounds) {
        std::string soundDir = config.projectPath + "/sounds";
        if (fs::exists(soundDir)) {
            std::string destDir = packageDir + "/sounds";
            createDirectory(destDir);
            std::vector<std::string> soundFiles;
            collectFiles(soundDir, {".wav", ".ogg", ".mp3"}, soundFiles);
            for (const auto& file : soundFiles) {
                std::string relPath = getRelativePath(file, soundDir);
                std::string destFile = destDir + "/" + relPath;
                createDirectory(fs::path(destFile).parent_path().string());
                if (copyFile(file, destFile)) {
                    allFiles.push_back(destFile);
                }
            }
            LOG_INFO_C(
                "GamePackager: Included " + std::to_string(soundFiles.size()) + " sound files",
                "GamePackager");
        }
    }

    // 5. Copy textures
    if (config.includeTextures) {
        std::string textureDir = config.projectPath + "/textures";
        if (fs::exists(textureDir)) {
            std::string destDir = packageDir + "/textures";
            createDirectory(destDir);
            std::vector<std::string> textureFiles;
            collectFiles(textureDir, {".png", ".jpg", ".tga", ".bmp"}, textureFiles);
            for (const auto& file : textureFiles) {
                std::string relPath = getRelativePath(file, textureDir);
                std::string destFile = destDir + "/" + relPath;
                createDirectory(fs::path(destFile).parent_path().string());
                if (copyFile(file, destFile)) {
                    allFiles.push_back(destFile);
                }
            }
            LOG_INFO_C(
                "GamePackager: Included " + std::to_string(textureFiles.size()) + " texture files",
                "GamePackager");
        }
    }

    // 6. Copy config files
    if (config.includeConfig) {
        std::string configDir = config.projectPath + "/config";
        if (fs::exists(configDir)) {
            std::string destDir = packageDir + "/config";
            createDirectory(destDir);
            std::vector<std::string> configFiles;
            collectFiles(configDir, {".json", ".ini", ".cfg", ".toml"}, configFiles);
            for (const auto& file : configFiles) {
                std::string relPath = getRelativePath(file, configDir);
                std::string destFile = destDir + "/" + relPath;
                createDirectory(fs::path(destFile).parent_path().string());
                if (copyFile(file, destFile)) {
                    allFiles.push_back(destFile);
                }
            }
            LOG_INFO_C(
                "GamePackager: Included " + std::to_string(configFiles.size()) + " config files",
                "GamePackager");
        }
    }

    // 7. Copy Lua scripts
    if (config.includeScripts) {
        std::string scriptDir = config.projectPath + "/scripts";
        if (fs::exists(scriptDir)) {
            std::string destDir = packageDir + "/scripts";
            createDirectory(destDir);
            std::vector<std::string> scriptFiles;
            collectFiles(scriptDir, {".lua"}, scriptFiles);
            for (const auto& file : scriptFiles) {
                std::string relPath = getRelativePath(file, scriptDir);
                std::string destFile = destDir + "/" + relPath;
                createDirectory(fs::path(destFile).parent_path().string());
                if (copyFile(file, destFile)) {
                    allFiles.push_back(destFile);
                }
            }
            LOG_INFO_C(
                "GamePackager: Included " + std::to_string(scriptFiles.size()) + " script files",
                "GamePackager");
        }
    }

    // 8. Write package manifest
    std::string manifest = createManifest(config, allFiles);
    std::string manifestPath = packageDir + "/package.json";
    {
        std::ofstream manifestFile(manifestPath);
        if (manifestFile.is_open()) {
            manifestFile << manifest;
            manifestFile.close();
            allFiles.push_back(manifestPath);
        }
    }

    // Calculate total size
    size_t totalSize = 0;
    for (const auto& file : allFiles) {
        if (fs::exists(file)) {
            totalSize += fs::file_size(file);
        }
    }

    result.success = true;
    result.outputFilePath = packageDir;
    result.filesIncluded = allFiles;
    result.totalSizeBytes = totalSize;

    LOG_INFO_C("GamePackager: Package built successfully at '" + packageDir + "' (" +
                   std::to_string(allFiles.size()) + " files, " +
                   std::to_string(totalSize / 1024) + " KB)",
               "GamePackager");

    return result;
}

void GamePackager::collectFiles(const std::string& rootDir,
                                const std::vector<std::string>& extensions,
                                std::vector<std::string>& outFiles)
{
    if (!fs::exists(rootDir) || !fs::is_directory(rootDir)) {
        return;
    }

    for (const auto& entry : fs::recursive_directory_iterator(rootDir)) {
        if (!entry.is_regular_file()) {
            continue;
        }

        std::string ext = entry.path().extension().string();
        // Convert to lowercase for comparison
        std::transform(ext.begin(), ext.end(), ext.begin(),
                       [](unsigned char c) { return std::tolower(c); });

        for (const auto& allowedExt : extensions) {
            if (ext == allowedExt) {
                outFiles.push_back(entry.path().string());
                break;
            }
        }
    }
}

std::string GamePackager::createManifest(const PackageConfig& config,
                                         const std::vector<std::string>& files)
{
    // Build a simple JSON manifest without requiring nlohmann_json
    std::ostringstream json;
    json << "{\n";
    json << "  \"name\": \"" << config.projectName << "\",\n";
    json << "  \"engine\": \"FreshVoxel\",\n";
    json << "  \"version\": \"0.2.7\",\n";

    // Timestamp
    std::time_t now = std::time(nullptr);
    char timeBuf[64];
#ifdef _MSC_VER
    std::tm tmBuf;
    gmtime_s(&tmBuf, &now);
    std::strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%dT%H:%M:%SZ", &tmBuf);
#else
    std::strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%dT%H:%M:%SZ", std::gmtime(&now));
#endif
    json << "  \"packaged_at\": \"" << timeBuf << "\",\n";

    json << "  \"file_count\": " << files.size() << ",\n";
    json << "  \"world_save\": \"" << fs::path(config.worldSavePath).filename().string() << "\",\n";
    json << "  \"launch\": {\n";
    json << "    \"executable\": \"FreshClient\",\n";
    json << "    \"args\": [\"--project\", \".\"]\n";
    json << "  }\n";
    json << "}\n";

    return json.str();
}

bool GamePackager::copyFile(const std::string& src, const std::string& dst)
{
    try {
        fs::copy_file(src, dst, fs::copy_options::overwrite_existing);
        return true;
    } catch (const fs::filesystem_error& e) {
        LOG_WARNING_C("GamePackager: Failed to copy '" + src + "' to '" + dst + "': " + e.what(),
                      "GamePackager");
        return false;
    }
}

bool GamePackager::createDirectory(const std::string& path)
{
    try {
        fs::create_directories(path);
        return true;
    } catch (const fs::filesystem_error& e) {
        LOG_WARNING_C("GamePackager: Failed to create directory '" + path + "': " + e.what(),
                      "GamePackager");
        return false;
    }
}

std::string GamePackager::getRelativePath(const std::string& filePath, const std::string& basePath)
{
    return fs::relative(filePath, basePath).string();
}

} // namespace fresh
