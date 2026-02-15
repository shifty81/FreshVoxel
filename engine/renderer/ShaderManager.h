#pragma once

#include <memory>
#include <string>
#include <unordered_map>

namespace fresh
{

/**
 * @brief Manages shader loading, compilation, and hot-reloading
 *
 * Provides shader management functionality including loading from files,
 * compilation, and optional hot-reloading during development.
 * API-agnostic - works with all rendering backends.
 */
class ShaderManager
{
public:
    ShaderManager();
    ~ShaderManager();

    // Prevent copying
    ShaderManager(const ShaderManager&) = delete;
    ShaderManager& operator=(const ShaderManager&) = delete;

    /**
     * @brief Load a shader from file
     * @param path Path to shader file (GLSL or HLSL)
     * @return Shader code as string, empty on failure
     */
    std::string loadShader(const std::string& path);

    /**
     * @brief Compile shader code
     * @param shaderCode Shader source code
     * @param shaderType Shader type (vertex, fragment, etc.)
     * @return Compiled shader code
     */
    std::string compileShader(const std::string& shaderCode, const std::string& shaderType);

    /**
     * @brief Enable hot-reloading for development
     * @param enabled true to enable hot-reloading, false to disable
     */
    void setHotReloadEnabled(bool enabled)
    {
        m_hotReloadEnabled = enabled;
    }

    /**
     * @brief Check if any shaders have been modified
     * @return true if shaders need reloading, false otherwise
     */
    bool checkForChanges();

    /**
     * @brief Reload all modified shaders
     */
    void reloadModified();

private:
    bool m_hotReloadEnabled;
    std::unordered_map<std::string, time_t> m_shaderTimestamps;
};

} // namespace fresh
