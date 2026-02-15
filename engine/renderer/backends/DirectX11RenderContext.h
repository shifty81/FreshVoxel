#pragma once

#include <unordered_map>

#include "renderer/RenderContext.h"
#include "voxel/VoxelTypes.h"

#ifdef _WIN32
    #ifndef NOMINMAX
    #define NOMINMAX
    #endif
    #include <wrl/client.h>

    #include <d3d11.h>
using Microsoft::WRL::ComPtr;

namespace fresh
{

/**
 * @brief DirectX 11 implementation of render context
 */
class DirectX11RenderContext : public IRenderContext
{
public:
    DirectX11RenderContext();
    ~DirectX11RenderContext() override;

    bool initialize(void* window) override;
    void shutdown() override;
    bool beginFrame() override;
    void endFrame() override;
    void waitIdle() override;

    void setViewport(int x, int y, int width, int height) override;
    void setScissor(int x, int y, int width, int height) override;
    void clearColor(float r, float g, float b, float a) override;
    void clearDepth(float depth) override;

    GraphicsAPI getAPI() const override
    {
        return GraphicsAPI::DirectX11;
    }
    void* getNativeDevice() override
    {
        return device.Get();
    }

    int getSwapchainWidth() const override
    {
        return width;
    }
    int getSwapchainHeight() const override
    {
        return height;
    }

    /**
     * @brief Set the viewport child window for rendering
     * @param viewportHwnd HWND of the viewport child window
     * @return true if successful
     */
    bool setViewportWindow(void* viewportHwnd) override;

    /**
     * @brief Recreate the swap chain with new dimensions
     * @param newWidth New width in pixels
     * @param newHeight New height in pixels
     * @return true if successful
     */
    bool recreateSwapChain(int newWidth, int newHeight) override;

    // DirectX 11 specific getters for ImGui integration
    ID3D11Device* getD3D11Device() const
    {
        return device.Get();
    }
    ID3D11DeviceContext* getD3D11DeviceContext() const
    {
        return deviceContext.Get();
    }

    std::shared_ptr<RenderBuffer> createVertexBuffer(const void* data, size_t size) override;
    std::shared_ptr<RenderBuffer> createIndexBuffer(const void* data, size_t size) override;
    std::shared_ptr<RenderBuffer> createUniformBuffer(size_t size) override;
    std::shared_ptr<RenderTexture> createTexture(int width, int height, const void* data) override;
    std::shared_ptr<RenderShader> createShader(const std::string& vertexCode,
                                               const std::string& fragmentCode) override;

    // DirectX 11 specific rendering methods for voxel world
    bool initializeVoxelRendering();
    void shutdownVoxelRendering();
    void renderVoxelWorld(class VoxelWorld* world, class Player* player);

private:
    bool createDevice();
    bool createSwapchain();
    bool createRenderTargetView();
    bool createDepthStencilView();

    ComPtr<ID3D11Device> device;
    ComPtr<ID3D11DeviceContext> deviceContext;
    ComPtr<IDXGISwapChain> swapchain;
    ComPtr<ID3D11RenderTargetView> renderTargetView;
    ComPtr<ID3D11DepthStencilView> depthStencilView;
    ComPtr<ID3D11Texture2D> depthStencilBuffer;

    void* window = nullptr;  // Points to Window or Win32Window (main window)
    void* viewportHwnd = nullptr;  // Viewport child window handle (if set)
    int width = 0;
    int height = 0;
    float clearColorValue[4] = {0.53f, 0.81f, 0.92f, 1.0f}; // Sky blue default
    bool useViewportSwapChain = false;  // True when viewport swap chain is active

    // Voxel rendering resources
    ComPtr<ID3D11VertexShader> voxelVertexShader;
    ComPtr<ID3D11PixelShader> voxelPixelShader;
    ComPtr<ID3D11InputLayout> voxelInputLayout;
    ComPtr<ID3D11Buffer> matrixConstantBuffer;
    ComPtr<ID3D11RasterizerState> rasterizerState;
    ComPtr<ID3D11DepthStencilState> depthStencilState;

    // Per-chunk rendering data
    struct ChunkRenderData {
        ComPtr<ID3D11Buffer> vertexBuffer;
        ComPtr<ID3D11Buffer> indexBuffer;
        size_t indexCount = 0;
    };
    std::unordered_map<ChunkPos, ChunkRenderData> chunkRenderData;
};

} // namespace fresh

#endif // _WIN32
