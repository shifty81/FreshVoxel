#pragma once

#include <string>
#include <vector>

namespace fresh {
namespace python {

class PythonBindings {
public:
    static bool initialize();
    static void shutdown();
    static bool isInitialized();
    static bool executeScript(const std::string& filepath);
    static bool executeCode(const std::string& code);
    static std::string getLastError();
};

class AssetProcessing {
public:
    static bool loadTexture(const std::string& filepath);
    static std::vector<std::string> getSupportedTextureFormats();
    static int processDirectory(const std::string& directory, const std::string& filePattern);
};

class WorldDataAccess {
public:
    static bool loadWorld(const std::string& worldPath);
    static std::string getWorldInfo(const std::string& worldPath);
    static int getChunkCount();
};

class BuildAutomation {
public:
    static bool validateAssets(const std::string& directory);
    static std::string generateManifest(const std::string& directory);
};

} // namespace python
} // namespace fresh
