#pragma once

namespace fresh
{

/**
 * @brief Supported graphics APIs (cross-platform)
 */
enum class GraphicsAPI {
    Auto,      // Automatically select best available API
    OpenGL,    // OpenGL 4.5+ (cross-platform)
    DirectX11, // DirectX 11 (Windows)
    DirectX12  // DirectX 12 (Windows)
};

/**
 * @brief Check if a graphics API is available on the current platform
 */
inline bool isGraphicsAPIAvailable(GraphicsAPI api)
{
    switch (api) {
    case GraphicsAPI::Auto:
        return true;
    case GraphicsAPI::OpenGL:
#if defined(FRESH_OPENGL_SUPPORT) && defined(FRESH_GLEW_AVAILABLE)
        return true;
#else
        return false;
#endif
#ifdef _WIN32
    case GraphicsAPI::DirectX11:
    case GraphicsAPI::DirectX12:
        return true;
#endif
    default:
        return false;
    }
}

/**
 * @brief Get the name of a graphics API
 */
inline const char* getGraphicsAPIName(GraphicsAPI api)
{
    switch (api) {
    case GraphicsAPI::Auto:
        return "Auto";
    case GraphicsAPI::OpenGL:
        return "OpenGL";
    case GraphicsAPI::DirectX11:
        return "DirectX 11";
    case GraphicsAPI::DirectX12:
        return "DirectX 12";
    default:
        return "Unknown";
    }
}

/**
 * @brief Select the best graphics API for the platform
 * Windows: OpenGL (default, complete rendering implementation) -> DirectX 12 -> DirectX 11
 * Linux/Mac: OpenGL (if GLEW available)
 *
 * Note: OpenGL is the default on Windows as it is cross-platform and well-tested.
 * DirectX 11 and DirectX 12 backends are fully implemented and available as alternatives.
 * On Linux/Mac, if GLEW is not available, OpenGL will be returned as the best option,
 * but runtime initialization will fail with proper error message.
 */
inline GraphicsAPI selectBestGraphicsAPI()
{
#ifdef _WIN32
    // On Windows, prefer OpenGL if available (cross-platform, well-tested)
    // DirectX backends are fully functional alternatives
    #if defined(FRESH_OPENGL_SUPPORT) && defined(FRESH_GLEW_AVAILABLE)
    return GraphicsAPI::OpenGL;
    #else
    // Fall back to DirectX 12 if OpenGL not available
    return GraphicsAPI::DirectX12;
    #endif
#else
    // Linux, Mac, and other platforms use OpenGL
    // If GLEW is not available, this will still return OpenGL
    // but runtime initialization will fail with proper error message
    return GraphicsAPI::OpenGL;
#endif
}

} // namespace fresh
