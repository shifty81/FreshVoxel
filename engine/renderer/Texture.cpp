#include "renderer/Texture.h"
#include "core/Logger.h"

#include <cstring>

// OpenGL texture operations (when available)
#if defined(FRESH_OPENGL_SUPPORT) && defined(FRESH_GLEW_AVAILABLE)
#include <GL/glew.h>
#endif

// Forward declare stb_image functions (implementation is in stb_image_impl.cpp)
extern "C" {
    unsigned char* stbi_load(char const* filename, int* x, int* y, int* channels_in_file, int desired_channels);
    void stbi_image_free(void* retval_from_stbi_load);
    const char* stbi_failure_reason(void);
}

namespace fresh
{

// Helper: Convert TextureFormat to OpenGL internal format, pixel format, and type
#if defined(FRESH_OPENGL_SUPPORT) && defined(FRESH_GLEW_AVAILABLE)
static void getGLFormats(TextureFormat format, GLenum& internalFormat, GLenum& pixelFormat,
                         GLenum& pixelType)
{
    switch (format) {
    case TextureFormat::R8:
        internalFormat = GL_R8;
        pixelFormat = GL_RED;
        pixelType = GL_UNSIGNED_BYTE;
        break;
    case TextureFormat::RG8:
        internalFormat = GL_RG8;
        pixelFormat = GL_RG;
        pixelType = GL_UNSIGNED_BYTE;
        break;
    case TextureFormat::RGB8:
        internalFormat = GL_RGB8;
        pixelFormat = GL_RGB;
        pixelType = GL_UNSIGNED_BYTE;
        break;
    case TextureFormat::RGBA8:
        internalFormat = GL_RGBA8;
        pixelFormat = GL_RGBA;
        pixelType = GL_UNSIGNED_BYTE;
        break;
    case TextureFormat::RGB16F:
        internalFormat = GL_RGB16F;
        pixelFormat = GL_RGB;
        pixelType = GL_FLOAT;
        break;
    case TextureFormat::RGBA16F:
        internalFormat = GL_RGBA16F;
        pixelFormat = GL_RGBA;
        pixelType = GL_FLOAT;
        break;
    case TextureFormat::RGB32F:
        internalFormat = GL_RGB32F;
        pixelFormat = GL_RGB;
        pixelType = GL_FLOAT;
        break;
    case TextureFormat::RGBA32F:
        internalFormat = GL_RGBA32F;
        pixelFormat = GL_RGBA;
        pixelType = GL_FLOAT;
        break;
    case TextureFormat::Depth24:
        internalFormat = GL_DEPTH_COMPONENT24;
        pixelFormat = GL_DEPTH_COMPONENT;
        pixelType = GL_UNSIGNED_INT;
        break;
    case TextureFormat::Depth32F:
        internalFormat = GL_DEPTH_COMPONENT32F;
        pixelFormat = GL_DEPTH_COMPONENT;
        pixelType = GL_FLOAT;
        break;
    default:
        internalFormat = GL_RGBA8;
        pixelFormat = GL_RGBA;
        pixelType = GL_UNSIGNED_BYTE;
        break;
    }
}

static GLenum getGLMinFilter(TextureFilter filter, bool hasMipmaps)
{
    switch (filter) {
    case TextureFilter::Nearest:
        return hasMipmaps ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST;
    case TextureFilter::Linear:
    case TextureFilter::Bilinear:
        return hasMipmaps ? GL_LINEAR_MIPMAP_NEAREST : GL_LINEAR;
    case TextureFilter::Trilinear:
        return hasMipmaps ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR;
    default:
        return GL_LINEAR;
    }
}

static GLenum getGLMagFilter(TextureFilter filter)
{
    return (filter == TextureFilter::Nearest) ? GL_NEAREST : GL_LINEAR;
}

static GLenum getGLWrapMode(TextureWrap wrap)
{
    switch (wrap) {
    case TextureWrap::Repeat: return GL_REPEAT;
    case TextureWrap::Clamp:  return GL_CLAMP_TO_EDGE;
    case TextureWrap::Mirror: return GL_MIRRORED_REPEAT;
    default:                  return GL_REPEAT;
    }
}
#endif

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

#if defined(FRESH_OPENGL_SUPPORT) && defined(FRESH_GLEW_AVAILABLE)
    // Create GPU texture using OpenGL
    GLenum internalFormat, pixelFormat, pixelType;
    getGLFormats(fmt, internalFormat, pixelFormat, pixelType);

    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Upload pixel data
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, w, h, 0, pixelFormat, pixelType, data);

    // Generate mipmaps if requested
    if (generateMipmaps) {
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    // Set default filtering and wrap modes
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    getGLMinFilter(filter, generateMipmaps));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, getGLMagFilter(filter));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, getGLWrapMode(wrap));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, getGLWrapMode(wrap));

    glBindTexture(GL_TEXTURE_2D, 0);

    return textureID != 0;
#else
    // Non-OpenGL fallback: mark as valid placeholder
    this->textureID = 1;
    return true;
#endif
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

#if defined(FRESH_OPENGL_SUPPORT) && defined(FRESH_GLEW_AVAILABLE)
    GLenum internalFormat, pixelFormat, pixelType;
    getGLFormats(fmt, internalFormat, pixelFormat, pixelType);

    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Create empty texture (no data upload)
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, w, h, 0, pixelFormat, pixelType, nullptr);

    // Default filtering for render targets
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, 0);

    return textureID != 0;
#else
    this->textureID = 1;
    return true;
#endif
}

void Texture::setFilter(TextureFilter newFilter)
{
    filter = newFilter;
#if defined(FRESH_OPENGL_SUPPORT) && defined(FRESH_GLEW_AVAILABLE)
    if (textureID != 0) {
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                        getGLMinFilter(filter, hasMipmaps));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, getGLMagFilter(filter));
        glBindTexture(GL_TEXTURE_2D, 0);
    }
#endif
}

void Texture::setWrap(TextureWrap newWrap)
{
    wrap = newWrap;
#if defined(FRESH_OPENGL_SUPPORT) && defined(FRESH_GLEW_AVAILABLE)
    if (textureID != 0) {
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, getGLWrapMode(wrap));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, getGLWrapMode(wrap));
        glBindTexture(GL_TEXTURE_2D, 0);
    }
#endif
}

void Texture::bind(int unit) const
{
#if defined(FRESH_OPENGL_SUPPORT) && defined(FRESH_GLEW_AVAILABLE)
    if (textureID != 0) {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_2D, textureID);
    }
#else
    (void)unit;
#endif
}

void Texture::unbind() const
{
#if defined(FRESH_OPENGL_SUPPORT) && defined(FRESH_GLEW_AVAILABLE)
    glBindTexture(GL_TEXTURE_2D, 0);
#endif
}

void Texture::cleanup()
{
    if (textureID != 0) {
#if defined(FRESH_OPENGL_SUPPORT) && defined(FRESH_GLEW_AVAILABLE)
        glDeleteTextures(1, &textureID);
#endif
        textureID = 0;
    }
}

} // namespace fresh
