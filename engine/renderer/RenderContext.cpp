#include "renderer/RenderContext.h"

#ifdef FRESH_DIRECTX_SUPPORT
    #include "renderer/backends/DirectX11RenderContext.h"
    #include "renderer/backends/DirectX12RenderContext.h"
#endif

#if defined(FRESH_OPENGL_SUPPORT) && defined(FRESH_GLEW_AVAILABLE)
    #include "renderer/backends/OpenGLRenderContext.h"
#endif

#include <iostream>
#include <memory>
#include <stdexcept>

namespace fresh
{

std::unique_ptr<IRenderContext> RenderContextFactory::create(GraphicsAPI api)
{
    // Check if API is available
    if (!isGraphicsAPIAvailable(api)) {
        std::cerr << "Graphics API " << getGraphicsAPIName(api)
                  << " is not available on this platform" << std::endl;
        return nullptr;
    }

    switch (api) {
#if defined(FRESH_OPENGL_SUPPORT) && defined(FRESH_GLEW_AVAILABLE)
    case GraphicsAPI::OpenGL:
        std::cout << "Creating OpenGL render context" << std::endl;
        return std::make_unique<OpenGLRenderContext>();
#endif

#ifdef FRESH_DIRECTX_SUPPORT
    case GraphicsAPI::DirectX11:
        std::cout << "Creating DirectX 11 render context" << std::endl;
        return std::make_unique<DirectX11RenderContext>();

    case GraphicsAPI::DirectX12:
        std::cout << "Creating DirectX 12 render context" << std::endl;
        return std::make_unique<DirectX12RenderContext>();
#endif

    case GraphicsAPI::Auto:
        return createBest();

    default:
        std::cerr << "Unknown or unsupported graphics API" << std::endl;
        return nullptr;
    }
}

std::unique_ptr<IRenderContext> RenderContextFactory::createBest()
{
    GraphicsAPI bestAPI = selectBestGraphicsAPI();
    std::cout << "Auto-selecting best graphics API: " << getGraphicsAPIName(bestAPI) << std::endl;
    return create(bestAPI);
}

} // namespace fresh
