#include "renderer/TextureManager.h"

namespace fresh
{

TextureManager& TextureManager::getInstance()
{
    static TextureManager instance;
    return instance;
}

std::shared_ptr<Texture> TextureManager::loadTexture(const std::string& path, bool generateMipmaps)
{
    std::lock_guard<std::mutex> lock(cacheMutex);

    // Check cache first
    auto it = textureCache.find(path);
    if (it != textureCache.end()) {
        return it->second;
    }

    // Load new texture
    auto texture = std::make_shared<Texture>();
    if (texture->loadFromFile(path, generateMipmaps)) {
        textureCache[path] = texture;
        return texture;
    }

    // Return default white texture on failure
    return getWhiteTexture();
}

std::shared_ptr<Texture> TextureManager::createTexture(const std::string& name, const void* data,
                                                       int width, int height, TextureFormat format)
{
    std::lock_guard<std::mutex> lock(cacheMutex);

    auto texture = std::make_shared<Texture>();
    texture->createFromData(data, width, height, format, false);
    textureCache[name] = texture;
    return texture;
}

std::shared_ptr<Texture> TextureManager::getTexture(const std::string& path)
{
    std::lock_guard<std::mutex> lock(cacheMutex);

    auto it = textureCache.find(path);
    if (it != textureCache.end()) {
        return it->second;
    }
    return nullptr;
}

void TextureManager::unloadTexture(const std::string& path)
{
    std::lock_guard<std::mutex> lock(cacheMutex);
    textureCache.erase(path);
}

void TextureManager::clearAll()
{
    std::lock_guard<std::mutex> lock(cacheMutex);
    textureCache.clear();
}

size_t TextureManager::getMemoryUsage() const
{
    std::lock_guard<std::mutex> lock(cacheMutex);

    size_t total = 0;
    for (const auto& pair : textureCache) {
        if (pair.second && pair.second->isValid()) {
            // Estimate memory usage based on format and dimensions
            int width = pair.second->getWidth();
            int height = pair.second->getHeight();
            TextureFormat fmt = pair.second->getFormat();

            int bytesPerPixel = 4; // Default RGBA8
            switch (fmt) {
            case TextureFormat::R8:
                bytesPerPixel = 1;
                break;
            case TextureFormat::RG8:
                bytesPerPixel = 2;
                break;
            case TextureFormat::RGB8:
                bytesPerPixel = 3;
                break;
            case TextureFormat::RGBA8:
                bytesPerPixel = 4;
                break;
            case TextureFormat::RGB16F:
                bytesPerPixel = 6;
                break;
            case TextureFormat::RGBA16F:
                bytesPerPixel = 8;
                break;
            case TextureFormat::RGB32F:
                bytesPerPixel = 12;
                break;
            case TextureFormat::RGBA32F:
                bytesPerPixel = 16;
                break;
            case TextureFormat::Depth24:
                bytesPerPixel = 3;
                break;
            case TextureFormat::Depth32F:
                bytesPerPixel = 4;
                break;
            }

            total += width * height * bytesPerPixel;
        }
    }
    return total;
}

bool TextureManager::reloadTexture(const std::string& path)
{
    std::lock_guard<std::mutex> lock(cacheMutex);

    auto it = textureCache.find(path);
    if (it != textureCache.end()) {
        return it->second->loadFromFile(path, true);
    }
    return false;
}

void TextureManager::createDefaultTextures()
{
    // Create 1x1 white texture
    unsigned char whitePixel[] = {255, 255, 255, 255};
    defaultWhite = std::make_shared<Texture>();
    defaultWhite->createFromData(whitePixel, 1, 1, TextureFormat::RGBA8, false);

    // Create 1x1 black texture
    unsigned char blackPixel[] = {0, 0, 0, 255};
    defaultBlack = std::make_shared<Texture>();
    defaultBlack->createFromData(blackPixel, 1, 1, TextureFormat::RGBA8, false);

    // Create 1x1 default normal map (pointing up)
    unsigned char normalPixel[] = {128, 128, 255, 255};
    defaultNormal = std::make_shared<Texture>();
    defaultNormal->createFromData(normalPixel, 1, 1, TextureFormat::RGBA8, false);
}

std::shared_ptr<Texture> TextureManager::getWhiteTexture()
{
    if (!defaultWhite) {
        createDefaultTextures();
    }
    return defaultWhite;
}

std::shared_ptr<Texture> TextureManager::getBlackTexture()
{
    if (!defaultBlack) {
        createDefaultTextures();
    }
    return defaultBlack;
}

std::shared_ptr<Texture> TextureManager::getDefaultNormalMap()
{
    if (!defaultNormal) {
        createDefaultTextures();
    }
    return defaultNormal;
}

} // namespace fresh
