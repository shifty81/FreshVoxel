#pragma once

#include <cstdint>
#include <string>

namespace fresh
{

/**
 * @brief Texture format enumeration
 */
enum class TextureFormat {
    R8,      // Single channel, 8-bit
    RG8,     // Two channels, 8-bit each
    RGB8,    // Three channels, 8-bit each
    RGBA8,   // Four channels, 8-bit each
    RGB16F,  // Three channels, 16-bit float
    RGBA16F, // Four channels, 16-bit float
    RGB32F,  // Three channels, 32-bit float
    RGBA32F, // Four channels, 32-bit float
    Depth24, // Depth buffer, 24-bit
    Depth32F // Depth buffer, 32-bit float
};

/**
 * @brief Texture filtering mode
 */
enum class TextureFilter {
    Nearest,  // No filtering
    Linear,   // Linear filtering
    Bilinear, // Bilinear filtering
    Trilinear // Trilinear filtering with mipmaps
};

/**
 * @brief Texture wrap mode
 */
enum class TextureWrap {
    Repeat, // Repeat texture
    Clamp,  // Clamp to edge
    Mirror  // Mirror repeat
};

/**
 * @brief Texture class for 2D textures
 *
 * Represents a texture that can be loaded from file or created procedurally.
 * Supports various formats, filtering modes, and wrapping modes.
 */
class Texture
{
public:
    Texture();
    ~Texture();

    // Prevent copying
    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;

    // Allow moving
    Texture(Texture&& other) noexcept;
    Texture& operator=(Texture&& other) noexcept;

    /**
     * @brief Load texture from file
     * @param path Path to texture file (PNG, JPG, TGA)
     * @param generateMipmaps Generate mipmaps for better quality
     * @return true if successful
     */
    bool loadFromFile(const std::string& path, bool generateMipmaps = true);

    /**
     * @brief Create texture from raw data
     * @param data Raw pixel data
     * @param width Width in pixels
     * @param height Height in pixels
     * @param format Texture format
     * @param generateMipmaps Generate mipmaps
     * @return true if successful
     */
    bool createFromData(const void* data, int width, int height, TextureFormat format,
                        bool generateMipmaps = true);

    /**
     * @brief Create empty texture (for render targets)
     * @param width Width in pixels
     * @param height Height in pixels
     * @param format Texture format
     * @return true if successful
     */
    bool createEmpty(int width, int height, TextureFormat format);

    /**
     * @brief Set filtering mode
     */
    void setFilter(TextureFilter filter);

    /**
     * @brief Set wrap mode
     */
    void setWrap(TextureWrap wrap);

    /**
     * @brief Bind texture to specified unit
     */
    void bind(int unit = 0) const;

    /**
     * @brief Unbind texture
     */
    void unbind() const;

    /**
     * @brief Get texture ID (GPU handle)
     */
    uint32_t getID() const
    {
        return textureID;
    }

    /**
     * @brief Get texture dimensions
     */
    int getWidth() const
    {
        return width;
    }
    int getHeight() const
    {
        return height;
    }
    TextureFormat getFormat() const
    {
        return format;
    }

    /**
     * @brief Check if texture is valid
     */
    bool isValid() const
    {
        return textureID != 0;
    }

    /**
     * @brief Get texture path (if loaded from file)
     */
    const std::string& getPath() const
    {
        return path;
    }

private:
    void cleanup();

    uint32_t textureID = 0; // Vulkan/OpenGL texture handle
    int width = 0;
    int height = 0;
    TextureFormat format = TextureFormat::RGBA8;
    TextureFilter filter = TextureFilter::Trilinear;
    TextureWrap wrap = TextureWrap::Repeat;
    bool hasMipmaps = false;
    std::string path; // Path if loaded from file
};

} // namespace fresh
