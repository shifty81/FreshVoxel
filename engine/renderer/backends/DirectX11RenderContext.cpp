#include "renderer/backends/DirectX11RenderContext.h"

#ifdef _WIN32

    #include <iostream>

    #include <glm/glm.hpp>
    #include <glm/gtc/matrix_transform.hpp>
    #include <glm/gtc/type_ptr.hpp>

    #include "core/Logger.h"
    #ifdef _WIN32
        #include "core/Win32Window.h"
    #endif
    #include "core/Window.h"
    #include "gameplay/Camera.h"
    #include "gameplay/Player.h"
    #include "voxel/Chunk.h"
    #include "voxel/VoxelTypes.h"
    #include "voxel/VoxelWorld.h"

    #include <d3dcompiler.h>

    #pragma comment(lib, "d3d11.lib")
    #pragma comment(lib, "dxgi.lib")
    #pragma comment(lib, "d3dcompiler.lib")

namespace fresh
{

// Helper struct to abstract window operations
struct WindowAdapter {
    static uint32_t getWidth(void* win) {
#ifdef _WIN32
        auto* w = static_cast<Win32Window*>(win);
#else
        auto* w = static_cast<Window*>(win);
#endif
        return w->getWidth();
    }
    
    static uint32_t getHeight(void* win) {
#ifdef _WIN32
        auto* w = static_cast<Win32Window*>(win);
#else
        auto* w = static_cast<Window*>(win);
#endif
        return w->getHeight();
    }
    
    static void* getNativeHandle(void* win) {
#ifdef _WIN32
        auto* w = static_cast<Win32Window*>(win);
#else
        auto* w = static_cast<Window*>(win);
#endif
        return w->getNativeWindowHandle();
    }
};

// Helper classes for DirectX 11 resources
class D3D11Buffer : public RenderBuffer
{
public:
    D3D11Buffer(ComPtr<ID3D11Buffer> buf, ComPtr<ID3D11DeviceContext> ctx, size_t sz)
        : buffer(buf), context(ctx), size(sz)
    {
    }

    void bind() override
    {
        // Binding is done through device context based on buffer type
    }

    void unbind() override
    {
        // Unbinding not typically needed in D3D11
    }

    void updateData(const void* data, size_t dataSize, size_t offset) override
    {
        if (!context || !buffer || !data)
            return;

        // For simple case with no offset, use UpdateSubresource
        if (offset == 0 && dataSize <= size) {
            context->UpdateSubresource(buffer.Get(), 0, nullptr, data, 0, 0);
        } else {
            // For partial updates, need to map/unmap the buffer
            D3D11_MAPPED_SUBRESOURCE mappedResource;
            HRESULT hr = context->Map(buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
            if (SUCCEEDED(hr)) {
                if (offset + dataSize <= size) {
                    memcpy(static_cast<char*>(mappedResource.pData) + offset, data, dataSize);
                }
                context->Unmap(buffer.Get(), 0);
            }
        }
    }

    size_t getSize() const override
    {
        return size;
    }
    void* getNativeHandle() override
    {
        return buffer.Get();
    }

private:
    ComPtr<ID3D11Buffer> buffer;
    ComPtr<ID3D11DeviceContext> context;
    size_t size;
};

class D3D11Texture : public RenderTexture
{
public:
    D3D11Texture(ComPtr<ID3D11Texture2D> tex, ComPtr<ID3D11ShaderResourceView> srv, int w, int h)
        : texture(tex), shaderResourceView(srv), width(w), height(h)
    {
    }

    void bind(int unit) override
    {
        (void)unit; // Unused - binding done through device context
        // Binding is done through device context
    }

    void unbind() override
    {
        // Unbinding not typically needed in D3D11
    }

    int getWidth() const override
    {
        return width;
    }
    int getHeight() const override
    {
        return height;
    }
    void* getNativeHandle() override
    {
        return texture.Get();
    }

private:
    ComPtr<ID3D11Texture2D> texture;
    ComPtr<ID3D11ShaderResourceView> shaderResourceView;
    int width;
    int height;
};

class D3D11Shader : public RenderShader
{
public:
    D3D11Shader(ComPtr<ID3D11VertexShader> vs, ComPtr<ID3D11PixelShader> ps)
        : vertexShader(vs), pixelShader(ps)
    {
    }

    void bind() override
    {
        // Binding is done through device context
    }

    void unbind() override
    {
        // Unbinding not typically needed in D3D11
    }

    void setUniformInt(const std::string& name, int value) override
    {
        (void)name;
        (void)value; // Parameters reserved for D3D11 constant buffer implementation
        // Set via constant buffers in D3D11
    }

    void setUniformFloat(const std::string& name, float value) override
    {
        (void)name;
        (void)value; // Parameters reserved for D3D11 constant buffer implementation
        // Set via constant buffers in D3D11
    }

    void setUniformVec2(const std::string& name, const glm::vec2& value) override
    {
        (void)name;
        (void)value; // Parameters reserved for D3D11 constant buffer implementation
        // Set via constant buffers in D3D11
    }

    void setUniformVec3(const std::string& name, const glm::vec3& value) override
    {
        (void)name;
        (void)value; // Parameters reserved for D3D11 constant buffer implementation
        // Set via constant buffers in D3D11
    }

    void setUniformVec4(const std::string& name, const glm::vec4& value) override
    {
        (void)name;
        (void)value; // Parameters reserved for D3D11 constant buffer implementation
        // Set via constant buffers in D3D11
    }

    void setUniformMat4(const std::string& name, const glm::mat4& value) override
    {
        (void)name;
        (void)value; // Parameters reserved for D3D11 constant buffer implementation
        // Set via constant buffers in D3D11
    }

    void* getNativeHandle() override
    {
        return vertexShader.Get();
    }

private:
    ComPtr<ID3D11VertexShader> vertexShader;
    ComPtr<ID3D11PixelShader> pixelShader;
};

DirectX11RenderContext::DirectX11RenderContext() : window(nullptr), width(0), height(0) {}

DirectX11RenderContext::~DirectX11RenderContext()
{
    shutdown();
}

bool DirectX11RenderContext::initialize(void* win)
{
    LOG_INFO_C("Initializing DirectX 11 render context...", "DirectX11");

    if (!win) {
        LOG_ERROR_C("Invalid window pointer", "DirectX11");
        return false;
    }

    window = win;
    width = static_cast<int>(WindowAdapter::getWidth(window));
    height = static_cast<int>(WindowAdapter::getHeight(window));

    if (!createDevice()) {
        LOG_ERROR_C("Failed to create device", "DirectX11");
        return false;
    }

    // CRITICAL FIX FOR VIEWPORT-ONLY RENDERING:
    // DO NOT create a swap chain for the main window during initialization.
    // The swap chain will ONLY be created for the viewport child window when
    // setViewportWindow() + recreateSwapChain() are called.
    // This prevents any rendering to the main window that could show through panel gaps.
    LOG_INFO_C("Skipping main window swap chain creation - will only use viewport swap chain", "DirectX11");
    LOG_INFO_C("Swap chain will be created when viewport is ready via setViewportWindow() + recreateSwapChain()", "DirectX11");

    // NOTE: We skip creating swap chain, render target view, and depth stencil view here.
    // These will be created later when the viewport is set up.
    // This ensures the main window is NEVER a DirectX rendering target.

    // Initialize voxel rendering system (doesn't require swap chain)
    if (!initializeVoxelRendering()) {
        LOG_ERROR_C("Failed to initialize voxel rendering", "DirectX11");
        return false;
    }

    LOG_INFO_C("DirectX 11 context initialized successfully", "DirectX11");
    return true;
}

void DirectX11RenderContext::shutdown()
{
    LOG_INFO_C("Shutting down DirectX 11 context...", "DirectX11");

    // Shutdown voxel rendering first
    shutdownVoxelRendering();

    // Release resources in reverse order
    depthStencilView.Reset();
    depthStencilBuffer.Reset();
    renderTargetView.Reset();
    swapchain.Reset();
    deviceContext.Reset();
    device.Reset();
}

bool DirectX11RenderContext::beginFrame()
{
    // CRITICAL: Only render if we have a valid swap chain (i.e., viewport is set up)
    // This prevents any rendering to the main window
    if (!swapchain) {
        // No swap chain yet - viewport not set up, skip rendering
        return false;
    }

    // CRITICAL: Bind render targets to output merger stage at the start of each frame
    // DirectX 11 best practice - ensures rendering always targets the correct surface
    // This is essential for proper viewport rendering (viewport child window vs main window)
    if (renderTargetView && depthStencilView && deviceContext) {
        deviceContext->OMSetRenderTargets(1, renderTargetView.GetAddressOf(), depthStencilView.Get());
    }
    
    // Clear render target and depth stencil using the stored clear color
    if (renderTargetView && deviceContext) {
        deviceContext->ClearRenderTargetView(renderTargetView.Get(), clearColorValue);
    }

    if (depthStencilView && deviceContext) {
        deviceContext->ClearDepthStencilView(depthStencilView.Get(),
                                             D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    }

    return true;
}

void DirectX11RenderContext::endFrame()
{
    // Present the frame
    // Only present if we have a valid swap chain
    // When using viewport, only present after viewport swap chain is created
    if (swapchain) {
        swapchain->Present(1, 0); // VSync enabled
    }
}

void DirectX11RenderContext::waitIdle()
{
    // Flush the device context
    if (deviceContext) {
        deviceContext->Flush();
    }
}

void DirectX11RenderContext::setViewport(int x, int y, int w, int h)
{
    if (!deviceContext)
        return;

    D3D11_VIEWPORT viewport = {};
    viewport.TopLeftX = static_cast<float>(x);
    viewport.TopLeftY = static_cast<float>(y);
    viewport.Width = static_cast<float>(w);
    viewport.Height = static_cast<float>(h);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    deviceContext->RSSetViewports(1, &viewport);
    width = w;
    height = h;
}

void DirectX11RenderContext::setScissor(int x, int y, int w, int h)
{
    if (!deviceContext)
        return;

    D3D11_RECT scissorRect = {};
    scissorRect.left = x;
    scissorRect.top = y;
    scissorRect.right = x + w;
    scissorRect.bottom = y + h;

    deviceContext->RSSetScissorRects(1, &scissorRect);
}

void DirectX11RenderContext::clearColor(float r, float g, float b, float a)
{
    // Store the clear color for use in beginFrame()
    clearColorValue[0] = r;
    clearColorValue[1] = g;
    clearColorValue[2] = b;
    clearColorValue[3] = a;
}

void DirectX11RenderContext::clearDepth(float depth)
{
    if (!depthStencilView || !deviceContext)
        return;

    deviceContext->ClearDepthStencilView(depthStencilView.Get(), D3D11_CLEAR_DEPTH, depth, 0);
}

std::shared_ptr<RenderBuffer> DirectX11RenderContext::createVertexBuffer(const void* data,
                                                                         size_t size)
{
    if (!device)
        return nullptr;

    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.ByteWidth = static_cast<UINT>(size);
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = data;

    ComPtr<ID3D11Buffer> buffer;
    HRESULT hr = device->CreateBuffer(&bufferDesc, data ? &initData : nullptr, &buffer);

    if (FAILED(hr)) {
        LOG_ERROR_C("Failed to create vertex buffer", "DirectX11");
        return nullptr;
    }

    return std::make_shared<D3D11Buffer>(buffer, deviceContext, size);
}

std::shared_ptr<RenderBuffer> DirectX11RenderContext::createIndexBuffer(const void* data,
                                                                        size_t size)
{
    if (!device)
        return nullptr;

    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.ByteWidth = static_cast<UINT>(size);
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = data;

    ComPtr<ID3D11Buffer> buffer;
    HRESULT hr = device->CreateBuffer(&bufferDesc, data ? &initData : nullptr, &buffer);

    if (FAILED(hr)) {
        LOG_ERROR_C("Failed to create index buffer", "DirectX11");
        return nullptr;
    }

    return std::make_shared<D3D11Buffer>(buffer, deviceContext, size);
}

std::shared_ptr<RenderBuffer> DirectX11RenderContext::createUniformBuffer(size_t size)
{
    if (!device)
        return nullptr;

    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.ByteWidth = static_cast<UINT>(size);
    bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    ComPtr<ID3D11Buffer> buffer;
    HRESULT hr = device->CreateBuffer(&bufferDesc, nullptr, &buffer);

    if (FAILED(hr)) {
        LOG_ERROR_C("Failed to create uniform buffer", "DirectX11");
        return nullptr;
    }

    return std::make_shared<D3D11Buffer>(buffer, deviceContext, size);
}

std::shared_ptr<RenderTexture> DirectX11RenderContext::createTexture(int w, int h, const void* data)
{
    if (!device)
        return nullptr;

    // Create texture
    D3D11_TEXTURE2D_DESC texDesc = {};
    texDesc.Width = w;
    texDesc.Height = h;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    texDesc.SampleDesc.Count = 1;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = data;
    initData.SysMemPitch = w * 4; // 4 bytes per pixel (RGBA)

    ComPtr<ID3D11Texture2D> texture;
    HRESULT hr = device->CreateTexture2D(&texDesc, data ? &initData : nullptr, &texture);

    if (FAILED(hr)) {
        LOG_ERROR_C("Failed to create texture", "DirectX11");
        return nullptr;
    }

    // Create shader resource view
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = texDesc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;

    ComPtr<ID3D11ShaderResourceView> srv;
    hr = device->CreateShaderResourceView(texture.Get(), &srvDesc, &srv);

    if (FAILED(hr)) {
        LOG_ERROR_C("Failed to create shader resource view", "DirectX11");
        return nullptr;
    }

    return std::make_shared<D3D11Texture>(texture, srv, w, h);
}

std::shared_ptr<RenderShader> DirectX11RenderContext::createShader(const std::string& vertexCode,
                                                                   const std::string& fragmentCode)
{
    if (!device)
        return nullptr;

    ComPtr<ID3D11VertexShader> vertexShader;
    ComPtr<ID3D11PixelShader> pixelShader;

    // Compile vertex shader
    ComPtr<ID3DBlob> vsBlob;
    ComPtr<ID3DBlob> errorBlob;

    HRESULT hr = D3DCompile(vertexCode.c_str(), vertexCode.size(), nullptr, nullptr, nullptr,
                            "main", "vs_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &vsBlob, &errorBlob);

    if (FAILED(hr)) {
        if (errorBlob) {
            std::string error = "Vertex shader compilation error: ";
            error += static_cast<const char*>(errorBlob->GetBufferPointer());
            LOG_ERROR_C(error, "DirectX11");
        } else {
            LOG_ERROR_C("Vertex shader compilation failed", "DirectX11");
        }
        return nullptr;
    }

    hr = device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr,
                                    &vertexShader);

    if (FAILED(hr)) {
        LOG_ERROR_C("Failed to create vertex shader", "DirectX11");
        return nullptr;
    }

    // Compile pixel shader
    ComPtr<ID3DBlob> psBlob;

    hr = D3DCompile(fragmentCode.c_str(), fragmentCode.size(), nullptr, nullptr, nullptr, "main",
                    "ps_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &psBlob, &errorBlob);

    if (FAILED(hr)) {
        if (errorBlob) {
            std::string error = "Pixel shader compilation error: ";
            error += static_cast<const char*>(errorBlob->GetBufferPointer());
            LOG_ERROR_C(error, "DirectX11");
        } else {
            LOG_ERROR_C("Pixel shader compilation failed", "DirectX11");
        }
        return nullptr;
    }

    hr = device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr,
                                   &pixelShader);

    if (FAILED(hr)) {
        LOG_ERROR_C("Failed to create pixel shader", "DirectX11");
        return nullptr;
    }

    LOG_INFO_C("Shaders compiled and created successfully", "DirectX11");
    return std::make_shared<D3D11Shader>(vertexShader, pixelShader);
}

bool DirectX11RenderContext::createDevice()
{
    UINT createDeviceFlags = 0;
    #ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
    #endif

    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
    };

    D3D_FEATURE_LEVEL featureLevel;

    HRESULT hr = D3D11CreateDevice(nullptr,                  // Default adapter
                                   D3D_DRIVER_TYPE_HARDWARE, // Hardware device
                                   nullptr,                  // No software device
                                   createDeviceFlags, featureLevels, ARRAYSIZE(featureLevels),
                                   D3D11_SDK_VERSION, &device, &featureLevel, &deviceContext);

    if (FAILED(hr)) {
        std::cerr << "[DirectX 11] Failed to create D3D11 device" << std::endl;
        return false;
    }

    std::cout << "[DirectX 11] D3D11 device created successfully" << std::endl;
    return true;
}

bool DirectX11RenderContext::createSwapchain()
{
    // Get native window handle
    HWND hwnd = static_cast<HWND>(WindowAdapter::getNativeHandle(window));
    if (!hwnd) {
        std::cerr << "[DirectX 11] Failed to get native window handle" << std::endl;
        return false;
    }

    // Get DXGI factory from device
    ComPtr<IDXGIDevice> dxgiDevice;
    HRESULT hr = device.As(&dxgiDevice);
    if (FAILED(hr)) {
        std::cerr << "[DirectX 11] Failed to get DXGI device" << std::endl;
        return false;
    }

    ComPtr<IDXGIAdapter> adapter;
    hr = dxgiDevice->GetAdapter(&adapter);
    if (FAILED(hr)) {
        std::cerr << "[DirectX 11] Failed to get DXGI adapter" << std::endl;
        return false;
    }

    ComPtr<IDXGIFactory> factory;
    hr = adapter->GetParent(__uuidof(IDXGIFactory), &factory);
    if (FAILED(hr)) {
        std::cerr << "[DirectX 11] Failed to get DXGI factory" << std::endl;
        return false;
    }

    // Create swapchain description
    DXGI_SWAP_CHAIN_DESC swapchainDesc = {};
    swapchainDesc.BufferCount = 2;
    swapchainDesc.BufferDesc.Width = width;
    swapchainDesc.BufferDesc.Height = height;
    swapchainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapchainDesc.BufferDesc.RefreshRate.Numerator = 60;
    swapchainDesc.BufferDesc.RefreshRate.Denominator = 1;
    swapchainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapchainDesc.OutputWindow = hwnd;
    swapchainDesc.SampleDesc.Count = 1;
    swapchainDesc.SampleDesc.Quality = 0;
    swapchainDesc.Windowed = TRUE;
    swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapchainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    // Create the swapchain
    hr = factory->CreateSwapChain(device.Get(), &swapchainDesc, &swapchain);
    if (FAILED(hr)) {
        std::cerr << "[DirectX 11] Failed to create swapchain" << std::endl;
        return false;
    }

    // Disable Alt+Enter fullscreen toggle (can be re-enabled if needed)
    factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER);

    std::cout << "[DirectX 11] Swapchain created successfully" << std::endl;
    return true;
}

bool DirectX11RenderContext::createRenderTargetView()
{
    if (!swapchain) {
        std::cerr << "[DirectX 11] Swapchain not initialized" << std::endl;
        return false;
    }

    // Get back buffer from swapchain
    ComPtr<ID3D11Texture2D> backBuffer;
    HRESULT hr = swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D),
                                      reinterpret_cast<void**>(backBuffer.GetAddressOf()));

    if (FAILED(hr)) {
        std::cerr << "[DirectX 11] Failed to get back buffer from swapchain" << std::endl;
        return false;
    }

    // Create render target view
    hr = device->CreateRenderTargetView(backBuffer.Get(), nullptr, &renderTargetView);

    if (FAILED(hr)) {
        std::cerr << "[DirectX 11] Failed to create render target view" << std::endl;
        return false;
    }

    std::cout << "[DirectX 11] Render target view created successfully" << std::endl;
    return true;
}

bool DirectX11RenderContext::createDepthStencilView()
{
    // Create depth stencil texture
    D3D11_TEXTURE2D_DESC depthStencilDesc = {};
    depthStencilDesc.Width = width;
    depthStencilDesc.Height = height;
    depthStencilDesc.MipLevels = 1;
    depthStencilDesc.ArraySize = 1;
    depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilDesc.SampleDesc.Count = 1;
    depthStencilDesc.SampleDesc.Quality = 0;
    depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
    depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depthStencilDesc.CPUAccessFlags = 0;
    depthStencilDesc.MiscFlags = 0;

    HRESULT hr = device->CreateTexture2D(&depthStencilDesc, nullptr, &depthStencilBuffer);
    if (FAILED(hr)) {
        std::cerr << "[DirectX 11] Failed to create depth stencil buffer" << std::endl;
        return false;
    }

    // Create depth stencil view
    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.Format = depthStencilDesc.Format;
    dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Texture2D.MipSlice = 0;

    hr = device->CreateDepthStencilView(depthStencilBuffer.Get(), &dsvDesc, &depthStencilView);
    if (FAILED(hr)) {
        std::cerr << "[DirectX 11] Failed to create depth stencil view" << std::endl;
        return false;
    }

    // Bind render targets
    deviceContext->OMSetRenderTargets(1, renderTargetView.GetAddressOf(), depthStencilView.Get());

    std::cout << "[DirectX 11] Depth stencil view created successfully" << std::endl;
    return true;
}

bool DirectX11RenderContext::setViewportWindow(void* viewportHandle)
{
    if (!viewportHandle) {
        LOG_ERROR_C("Invalid viewport window handle (null)", "DirectX11");
        return false;
    }

    viewportHwnd = viewportHandle;
    useViewportSwapChain = true;  // Mark that we're now using viewport swap chain
    
    HWND hwnd = static_cast<HWND>(viewportHandle);
    LOG_INFO_C("✓ Viewport window handle set successfully (HWND: " + 
              std::to_string(reinterpret_cast<uintptr_t>(hwnd)) + ")", "DirectX11");
    LOG_INFO_C("✓ Viewport swap chain mode enabled", "DirectX11");
    
    return true;
}

bool DirectX11RenderContext::recreateSwapChain(int newWidth, int newHeight)
{
    if (newWidth <= 0 || newHeight <= 0) {
        LOG_ERROR_C("✗ Invalid swap chain dimensions: " + std::to_string(newWidth) + "x" + std::to_string(newHeight), "DirectX11");
        return false;
    }

    LOG_INFO_C("═══ RECREATING SWAP CHAIN FOR VIEWPORT ═══", "DirectX11");
    LOG_INFO_C("Target dimensions: " + std::to_string(newWidth) + "x" + std::to_string(newHeight), "DirectX11");

    // Update dimensions
    width = newWidth;
    height = newHeight;

    // Release existing swap chain resources (order matters!)
    if (deviceContext) {
        deviceContext->OMSetRenderTargets(0, nullptr, nullptr);
        deviceContext->Flush();
        LOG_INFO_C("✓ Unbound render targets and flushed device context", "DirectX11");
    }

    depthStencilView.Reset();
    depthStencilBuffer.Reset();
    renderTargetView.Reset();
    swapchain.Reset();
    LOG_INFO_C("✓ Released existing swap chain resources", "DirectX11");

    // CRITICAL: ONLY use viewport window handle for swap chain creation
    // NEVER fall back to main window - viewport is the ONLY DirectX rendering target
    if (!viewportHwnd) {
        LOG_ERROR_C("✗ Cannot recreate swap chain - viewport window handle not set. Call setViewportWindow() first.", "DirectX11");
        return false;
    }
    
    HWND targetHwnd = static_cast<HWND>(viewportHwnd);
    if (!targetHwnd) {
        LOG_ERROR_C("✗ Invalid viewport window handle for swap chain", "DirectX11");
        return false;
    }
    
    LOG_INFO_C("✓ Using viewport HWND: " + std::to_string(reinterpret_cast<uintptr_t>(targetHwnd)), "DirectX11");

    // Get DXGI factory from device
    ComPtr<IDXGIDevice> dxgiDevice;
    HRESULT hr = device.As(&dxgiDevice);
    if (FAILED(hr)) {
        LOG_ERROR_C("✗ Failed to get DXGI device (HRESULT: 0x" + 
                   std::to_string(hr) + ")", "DirectX11");
        return false;
    }
    LOG_INFO_C("✓ Got DXGI device", "DirectX11");

    ComPtr<IDXGIAdapter> adapter;
    hr = dxgiDevice->GetAdapter(&adapter);
    if (FAILED(hr)) {
        LOG_ERROR_C("✗ Failed to get DXGI adapter (HRESULT: 0x" + 
                   std::to_string(hr) + ")", "DirectX11");
        return false;
    }
    LOG_INFO_C("✓ Got DXGI adapter", "DirectX11");

    ComPtr<IDXGIFactory> factory;
    hr = adapter->GetParent(__uuidof(IDXGIFactory), &factory);
    if (FAILED(hr)) {
        LOG_ERROR_C("✗ Failed to get DXGI factory (HRESULT: 0x" + 
                   std::to_string(hr) + ")", "DirectX11");
        return false;
    }
    LOG_INFO_C("✓ Got DXGI factory", "DirectX11");

    // Create new swap chain description for viewport
    // Using FLIP_SEQUENTIAL for smoother presentation (Unreal Engine approach)
    DXGI_SWAP_CHAIN_DESC swapchainDesc = {};
    swapchainDesc.BufferCount = 2;
    swapchainDesc.BufferDesc.Width = width;
    swapchainDesc.BufferDesc.Height = height;
    swapchainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapchainDesc.BufferDesc.RefreshRate.Numerator = 60;
    swapchainDesc.BufferDesc.RefreshRate.Denominator = 1;
    swapchainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapchainDesc.OutputWindow = targetHwnd;  // Use the viewport child window
    swapchainDesc.SampleDesc.Count = 1;
    swapchainDesc.SampleDesc.Quality = 0;
    swapchainDesc.Windowed = TRUE;
    // FLIP_DISCARD provides better performance and wider compatibility than FLIP_SEQUENTIAL
    swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapchainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    // Create the new swapchain
    hr = factory->CreateSwapChain(device.Get(), &swapchainDesc, &swapchain);
    if (FAILED(hr)) {
        LOG_ERROR_C("✗ Failed to create new swapchain for viewport (HRESULT: 0x" + 
                   std::to_string(hr) + ")", "DirectX11");
        return false;
    }
    LOG_INFO_C("✓ Swap chain created successfully", "DirectX11");

    // Recreate render target view
    if (!createRenderTargetView()) {
        LOG_ERROR_C("✗ Failed to recreate render target view", "DirectX11");
        return false;
    }
    LOG_INFO_C("✓ Render target view created", "DirectX11");

    // Recreate depth stencil view
    if (!createDepthStencilView()) {
        LOG_ERROR_C("✗ Failed to recreate depth stencil view", "DirectX11");
        return false;
    }
    LOG_INFO_C("✓ Depth stencil view created", "DirectX11");

    // CRITICAL FIX: Immediately clear the new render target to prevent artifacts
    // When resizing, the old swap chain content can show through if we don't clear immediately
    if (renderTargetView && depthStencilView && deviceContext) {
        // Bind the new render targets
        deviceContext->OMSetRenderTargets(1, renderTargetView.GetAddressOf(), depthStencilView.Get());
        
        // Clear to black to prevent any artifacts from old swap chain
        float clearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};  // Black
        deviceContext->ClearRenderTargetView(renderTargetView.Get(), clearColor);
        deviceContext->ClearDepthStencilView(depthStencilView.Get(), 
                                            D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
        
        LOG_INFO_C("Cleared new render target to prevent artifacts", "DirectX11");
    }

    LOG_INFO_C("Swap chain recreated successfully for viewport: " + std::to_string(width) + "x" + std::to_string(height), "DirectX11");
    return true;
}

bool DirectX11RenderContext::initializeVoxelRendering()
{
    // Load and compile HLSL shader
    std::string shaderCode = R"(
        // Voxel Rendering Shader for DirectX 11
        cbuffer MatrixBuffer : register(b0)
        {
            matrix modelViewProj;
        };

        struct VertexInput
        {
            float3 position : POSITION;
            float3 normal : NORMAL;
        };

        struct PixelInput
        {
            float4 position : SV_POSITION;
            float3 normal : NORMAL;
            float3 worldPos : TEXCOORD0;
        };

        PixelInput VSMain(VertexInput input)
        {
            PixelInput output;
            output.position = mul(float4(input.position, 1.0f), modelViewProj);
            output.normal = input.normal;
            output.worldPos = input.position;
            return output;
        }

        float4 PSMain(PixelInput input) : SV_TARGET
        {
            // Simple directional lighting
            float3 lightDir = normalize(float3(0.5f, 1.0f, 0.3f));
            float3 normal = normalize(input.normal);
            float diff = max(dot(normal, lightDir), 0.0f);
            
            // Ambient + diffuse
            float3 ambient = float3(0.3f, 0.3f, 0.3f);
            float3 diffuse = float3(0.7f, 0.7f, 0.7f) * diff;
            
            // Base voxel color
            float3 color = float3(0.5f, 0.7f, 0.5f);
            float3 result = (ambient + diffuse) * color;
            
            return float4(result, 1.0f);
        }
    )";

    // Compile vertex shader
    ComPtr<ID3DBlob> vsBlob;
    ComPtr<ID3DBlob> errorBlob;

    HRESULT hr =
        D3DCompile(shaderCode.c_str(), shaderCode.size(), nullptr, nullptr, nullptr, "VSMain",
                   "vs_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &vsBlob, &errorBlob);

    if (FAILED(hr)) {
        if (errorBlob) {
            std::cerr << "[DirectX 11] Vertex shader compilation error: "
                      << static_cast<const char*>(errorBlob->GetBufferPointer()) << std::endl;
        }
        return false;
    }

    hr = device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr,
                                    &voxelVertexShader);

    if (FAILED(hr)) {
        std::cerr << "[DirectX 11] Failed to create voxel vertex shader" << std::endl;
        return false;
    }

    // Compile pixel shader
    ComPtr<ID3DBlob> psBlob;

    hr = D3DCompile(shaderCode.c_str(), shaderCode.size(), nullptr, nullptr, nullptr, "PSMain",
                    "ps_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &psBlob, &errorBlob);

    if (FAILED(hr)) {
        if (errorBlob) {
            std::cerr << "[DirectX 11] Pixel shader compilation error: "
                      << static_cast<const char*>(errorBlob->GetBufferPointer()) << std::endl;
        }
        return false;
    }

    hr = device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr,
                                   &voxelPixelShader);

    if (FAILED(hr)) {
        std::cerr << "[DirectX 11] Failed to create voxel pixel shader" << std::endl;
        return false;
    }

    // Create input layout
    D3D11_INPUT_ELEMENT_DESC inputLayout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}};

    hr = device->CreateInputLayout(inputLayout, ARRAYSIZE(inputLayout), vsBlob->GetBufferPointer(),
                                   vsBlob->GetBufferSize(), &voxelInputLayout);

    if (FAILED(hr)) {
        std::cerr << "[DirectX 11] Failed to create input layout" << std::endl;
        return false;
    }

    // Create constant buffer for matrices
    D3D11_BUFFER_DESC cbDesc = {};
    cbDesc.ByteWidth = sizeof(float) * 16; // 4x4 matrix
    cbDesc.Usage = D3D11_USAGE_DYNAMIC;
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    hr = device->CreateBuffer(&cbDesc, nullptr, &matrixConstantBuffer);
    if (FAILED(hr)) {
        std::cerr << "[DirectX 11] Failed to create matrix constant buffer" << std::endl;
        return false;
    }

    // Create rasterizer state (backface culling enabled)
    D3D11_RASTERIZER_DESC rasterizerDesc = {};
    rasterizerDesc.FillMode = D3D11_FILL_SOLID;
    rasterizerDesc.CullMode = D3D11_CULL_BACK;
    rasterizerDesc.FrontCounterClockwise = FALSE;
    rasterizerDesc.DepthClipEnable = TRUE;

    hr = device->CreateRasterizerState(&rasterizerDesc, &rasterizerState);
    if (FAILED(hr)) {
        std::cerr << "[DirectX 11] Failed to create rasterizer state" << std::endl;
        return false;
    }

    // Create depth stencil state
    D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};
    depthStencilDesc.DepthEnable = TRUE;
    depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
    depthStencilDesc.StencilEnable = FALSE;

    hr = device->CreateDepthStencilState(&depthStencilDesc, &depthStencilState);
    if (FAILED(hr)) {
        std::cerr << "[DirectX 11] Failed to create depth stencil state" << std::endl;
        return false;
    }

    std::cout << "[DirectX 11] Voxel rendering initialized successfully" << std::endl;
    return true;
}

void DirectX11RenderContext::shutdownVoxelRendering()
{
    chunkRenderData.clear();
    matrixConstantBuffer.Reset();
    voxelInputLayout.Reset();
    voxelPixelShader.Reset();
    voxelVertexShader.Reset();
    rasterizerState.Reset();
    depthStencilState.Reset();
}

void DirectX11RenderContext::renderVoxelWorld(VoxelWorld* world, Player* player)
{
    if (!world || !player || !voxelVertexShader || !voxelPixelShader) {
        return;
    }

    // Avoid division by zero if swap chain dimensions are invalid
    if (width <= 0 || height <= 0) {
        return;
    }

    // Set up rendering pipeline
    deviceContext->IASetInputLayout(voxelInputLayout.Get());
    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    deviceContext->VSSetShader(voxelVertexShader.Get(), nullptr, 0);
    deviceContext->PSSetShader(voxelPixelShader.Get(), nullptr, 0);
    deviceContext->RSSetState(rasterizerState.Get());
    deviceContext->OMSetDepthStencilState(depthStencilState.Get(), 0);

    // Calculate view-projection matrix
    glm::mat4 view = player->getCamera().getViewMatrix();
    float aspectRatio = static_cast<float>(width) / static_cast<float>(height);
    glm::mat4 projection = player->getCamera().getProjectionMatrix(aspectRatio);

    // Render each chunk
    for (const auto& chunkPair : world->getChunks()) {
        const ChunkPos& chunkPos = chunkPair.first;
        Chunk* chunk = chunkPair.second.get();

        if (!chunk)
            continue;

        // Check if chunk needs mesh update
        bool needsUpload =
            chunk->isDirty() || (chunkRenderData.find(chunkPos) == chunkRenderData.end());

        if (needsUpload) {
            // Generate mesh if dirty
            if (chunk->isDirty()) {
                chunk->generateMesh();
                chunk->clearDirty();
            }

            // Get mesh data
            const auto& vertices = chunk->getMeshVertices();
            const auto& indices = chunk->getMeshIndices();

            if (vertices.empty() || indices.empty()) {
                continue;
            }

            // Create or update GPU buffers
            ChunkRenderData& renderData = chunkRenderData[chunkPos];

            // Create vertex buffer
            D3D11_BUFFER_DESC vbDesc = {};
            vbDesc.ByteWidth = static_cast<UINT>(vertices.size() * sizeof(float));
            vbDesc.Usage = D3D11_USAGE_DEFAULT;
            vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

            D3D11_SUBRESOURCE_DATA vbData = {};
            vbData.pSysMem = vertices.data();

            renderData.vertexBuffer.Reset();
            HRESULT hr = device->CreateBuffer(&vbDesc, &vbData, &renderData.vertexBuffer);
            if (FAILED(hr)) {
                continue;
            }

            // Create index buffer
            D3D11_BUFFER_DESC ibDesc = {};
            ibDesc.ByteWidth = static_cast<UINT>(indices.size() * sizeof(uint32_t));
            ibDesc.Usage = D3D11_USAGE_DEFAULT;
            ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

            D3D11_SUBRESOURCE_DATA ibData = {};
            ibData.pSysMem = indices.data();

            renderData.indexBuffer.Reset();
            hr = device->CreateBuffer(&ibDesc, &ibData, &renderData.indexBuffer);
            if (FAILED(hr)) {
                continue;
            }

            renderData.indexCount = indices.size();
        }

        // Render the chunk if it has render data
        auto it = chunkRenderData.find(chunkPos);
        if (it != chunkRenderData.end() && it->second.indexCount > 0) {
            const ChunkRenderData& renderData = it->second;

            // Calculate model matrix (chunk position)
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(chunkPos.x * 16, // CHUNK_SIZE
                                                    0,
                                                    chunkPos.z * 16 // CHUNK_SIZE
                                                    ));

            // Calculate MVP matrix
            glm::mat4 mvp = projection * view * model;

            // Update constant buffer
            D3D11_MAPPED_SUBRESOURCE mappedResource;
            HRESULT hr = deviceContext->Map(matrixConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD,
                                            0, &mappedResource);
            if (SUCCEEDED(hr)) {
                memcpy(mappedResource.pData, glm::value_ptr(mvp), sizeof(float) * 16);
                deviceContext->Unmap(matrixConstantBuffer.Get(), 0);
            }

            // Bind constant buffer
            deviceContext->VSSetConstantBuffers(0, 1, matrixConstantBuffer.GetAddressOf());

            // Bind vertex buffer
            UINT stride = sizeof(float) * 6; // 3 pos + 3 normal
            UINT offset = 0;
            deviceContext->IASetVertexBuffers(0, 1, renderData.vertexBuffer.GetAddressOf(), &stride,
                                              &offset);

            // Bind index buffer
            deviceContext->IASetIndexBuffer(renderData.indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

            // Draw
            deviceContext->DrawIndexed(static_cast<UINT>(renderData.indexCount), 0, 0);
        }
    }
}

} // namespace fresh

#endif // _WIN32
