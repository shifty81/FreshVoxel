#include "renderer/ShaderManager.h"

#include <fstream>
#include <iostream>

namespace fresh
{

ShaderManager::ShaderManager() : m_hotReloadEnabled(false) {}

ShaderManager::~ShaderManager() {}

std::string ShaderManager::loadShader(const std::string& path)
{
    std::ifstream file(path, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        std::cerr << "Failed to open shader file: " << path << std::endl;
        return "";
    }

    size_t fileSize = static_cast<size_t>(file.tellg());
    std::string buffer(fileSize, '\0');

    file.seekg(0);
    file.read(&buffer[0], fileSize);
    file.close();

    return buffer;
}

std::string ShaderManager::compileShader(const std::string& shaderCode,
                                         const std::string& shaderType)
{
    // Stub - compilation would be handled by specific render context backend
    std::cout << "ShaderManager::compileShader stub called for " << shaderType << std::endl;
    return shaderCode; // Return source as-is for now
}

bool ShaderManager::checkForChanges()
{
    return false;
}

void ShaderManager::reloadModified()
{
    // Stub implementation
}

} // namespace fresh
