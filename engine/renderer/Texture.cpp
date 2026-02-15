#include "renderer/Texture.h"
#include "core/Logger.h"

#include <cstring>

// Forward declare stb_image functions (implementation is in stb_image_impl.cpp)
extern "C" {
    unsigned char* stbi_load(char const* filename, int* x, int* y, int* channels_in_file, int desired_channels);
    void stbi_image_free(void* retval_from_stbi_load);
    const char* stbi_failure_reason(void);
}

namespace fresh
{

Texture::Texture() {}

Texture::~Texture()
{
    cleanup();
}

Texture::Texture(Texture&& other) noexcept
    : textureID(other.textureID),
      width(other.width),
      height(other.height),
      format(other.format),
      filter(other.filter),
      wrap(other.wrap),
      hasMipmaps(other.hasMipmaps),
      path(std::move(other.path))
{
    other.textureID = 0;
}

Texture& Texture::operator=(Texture&& other) noexcept
{
    if (this != &other) {
        cleanup();

        textureID = other.textureID;
        width = other.width;
        height = other.height;
        format = other.format;
        filter = other.filter;
        wrap = other.wrap;
        hasMipmaps = other.hasMipmaps;
        path = std::move(other.path);

        other.textureID = 0;
    }
    return *this;
}

bool Texture::loadFromFile(const std::string& filePath, bool generateMipmaps)
{
    // Clean up any existing texture
    cleanup();
    
    // Load image data using stb_image
    int imgWidth = 0;
    int imgHeight = 0;
    int channels = 0;
    
    // Request 4 channels (RGBA) for consistency
    unsigned char* imageData = stbi_load(filePath.c_str(), &imgWidth, &imgHeight, &channels, 4);
    
    if (!imageData) {
        Logger::getInstance().error("Failed to load texture from file: " + filePath + 
                                   " - Reason: " + std::string(stbi_failure_reason()), "Texture");
        return false;
    }
    
    // Set texture properties
    this->path = filePath;
    this->width = imgWidth;
    this->height = imgHeight;
    this->format = TextureFormat::RGBA8; // We requested 4 channels
    this->hasMipmaps = generateMipmaps;
    
    // Create texture from loaded data
    bool success = createFromData(imageData, imgWidth, imgHeight, TextureFormat::RGBA8, generateMipmaps);
    
    // Free the loaded image data
    stbi_image_free(imageData);
    
    if (success) {
        Logger::getInstance().info("Successfully loaded texture: " + filePath + 
                                  " (" + std::to_string(imgWidth) + "x" + std::to_string(imgHeight) + ")", "Texture");
    } else {
        Logger::getInstance().error("Failed to create GPU texture from loaded data: " + filePath, "Texture");
    }
    
    return success;
}

bool Texture::createFromData(const void* data, int w, int h, TextureFormat fmt,
                             bool generateMipmaps)
{
    if (!data || w <= 0 || h <= 0) {
        return false;
    }

    cleanup();

    this->width = w;
    this->height = h;
    this->format = fmt;
    this->hasMipmaps = generateMipmaps;

    // TODO: Create actual GPU texture from data
    // For now, we mark the texture as valid by assigning a non-zero ID
    // Real GPU texture upload needs to be implemented in the rendering backend
    // (OpenGL, DirectX 11, DirectX 12)
    //
    // Example for OpenGL:
    //   glGenTextures(1, &textureID);
    //   glBindTexture(GL_TEXTURE_2D, textureID);
    //   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    //   if (generateMipmaps) glGenerateMipmap(GL_TEXTURE_2D);
    //
    // Example for DirectX 11:
    //   D3D11_TEXTURE2D_DESC desc;
    //   D3D11_SUBRESOURCE_DATA initData;
    //   device->CreateTexture2D(&desc, &initData, &texture);
    //
    // This will be implemented when integrating with the RenderContext backends
    
    this->textureID = 1; // Placeholder - indicates "valid" texture loaded in memory
    
    return true;
}

bool Texture::createEmpty(int w, int h, TextureFormat fmt)
{
    if (w <= 0 || h <= 0) {
        return false;
    }

    cleanup();

    this->width = w;
    this->height = h;
    this->format = fmt;
    this->hasMipmaps = false;

    // TODO: Create actual GPU texture
    this->textureID = 1; // Placeholder

    return true;
}

void Texture::setFilter(TextureFilter newFilter)
{
    filter = newFilter;
    // TODO: Update GPU texture filtering mode
}

void Texture::setWrap(TextureWrap newWrap)
{
    wrap = newWrap;
    // TODO: Update GPU texture wrap mode
}

void Texture::bind(int unit) const
{
    // TODO: Bind texture to GPU unit
    (void)unit;
}

void Texture::unbind() const
{
    // TODO: Unbind texture from GPU
}

void Texture::cleanup()
{
    if (textureID != 0) {
        // TODO: Delete GPU texture
        textureID = 0;
    }
}

} // namespace fresh
