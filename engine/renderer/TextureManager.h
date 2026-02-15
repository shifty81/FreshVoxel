#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

#include "Texture.h"

namespace fresh
{

/**
 * @brief Manages texture loading, caching, and lifetime
 *
 * Singleton class that handles texture resources, provides caching
 * to avoid duplicate loads, and supports hot-reloading.
 */
class TextureManager
{
public:
    /**
     * @brief Get singleton instance
     */
    static TextureManager& getInstance();

    // Prevent copying and moving
    TextureManager(const TextureManager&) = delete;
    TextureManager& operator=(const TextureManager&) = delete;

    /**
     * @brief Load texture from file (with caching)
     * @param path Path to texture file
     * @param generateMipmaps Generate mipmaps
     * @return Shared pointer to texture
     */
    std::shared_ptr<Texture> loadTexture(const std::string& path, bool generateMipmaps = true);

    /**
     * @brief Create procedural texture
     * @param name Unique name for the texture
     * @param data Pixel data
     * @param width Width in pixels
     * @param height Height in pixels
     * @param format Texture format
     * @return Shared pointer to texture
     */
    std::shared_ptr<Texture> createTexture(const std::string& name, const void* data, int width,
                                           int height, TextureFormat format);

    /**
     * @brief Get cached texture by path
     * @param path Texture path
     * @return Shared pointer to texture or nullptr if not cached
     */
    std::shared_ptr<Texture> getTexture(const std::string& path);

    /**
     * @brief Remove texture from cache
     * @param path Texture path
     */
    void unloadTexture(const std::string& path);

    /**
     * @brief Clear all cached textures
     */
    void clearAll();

    /**
     * @brief Get cache size
     */
    size_t getCacheSize() const
    {
        return textureCache.size();
    }

    /**
     * @brief Get memory usage estimate (in bytes)
     */
    size_t getMemoryUsage() const;

    /**
     * @brief Hot-reload texture from file
     * @param path Path to texture file
     * @return true if successful
     */
    bool reloadTexture(const std::string& path);

    /**
     * @brief Create default textures (white, black, normal, etc.)
     */
    void createDefaultTextures();

    /**
     * @brief Get default white texture
     */
    std::shared_ptr<Texture> getWhiteTexture();

    /**
     * @brief Get default black texture
     */
    std::shared_ptr<Texture> getBlackTexture();

    /**
     * @brief Get default normal map
     */
    std::shared_ptr<Texture> getDefaultNormalMap();

private:
    TextureManager() = default;
    ~TextureManager() = default;

    std::unordered_map<std::string, std::shared_ptr<Texture>> textureCache;
    mutable std::mutex cacheMutex;

    // Default textures
    std::shared_ptr<Texture> defaultWhite;
    std::shared_ptr<Texture> defaultBlack;
    std::shared_ptr<Texture> defaultNormal;
};

} // namespace fresh
