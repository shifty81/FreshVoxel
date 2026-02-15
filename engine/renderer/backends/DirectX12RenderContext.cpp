#include "renderer/backends/DirectX12RenderContext.h"

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

    #pragma comment(lib, "d3d12.lib")
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

// Helper classes for DirectX 12 resources
class D3D12Buffer : public RenderBuffer
{
public:
    D3D12Buffer(ComPtr<ID3D12Resource> res, size_t sz) : resource(res), size(sz) {}

    void bind() override
    {
        // Binding in D3D12 is done via descriptor tables and root signatures
    }

    void unbind() override
    {
        // Not applicable in D3D12
    }

    void updateData(const void* data, size_t dataSize, size_t offset) override
    {
        if (!resource || !data || dataSize == 0)
            return;

        // Map the resource (only works for upload heap resources)
        void* mappedData = nullptr;
        D3D12_RANGE readRange = {0, 0}; // We won't read from this resource on the CPU

        HRESULT hr = resource->Map(0, &readRange, &mappedData);
        if (SUCCEEDED(hr)) {
            if (offset + dataSize <= size) {
                memcpy(static_cast<char*>(mappedData) + offset, data, dataSize);
            }
            resource->Unmap(0, nullptr);
        }
    }

    size_t getSize() const override
    {
        return size;
    }
    void* getNativeHandle() override
    {
        return resource.Get();
    }

private:
    ComPtr<ID3D12Resource> resource;
    size_t size;
};

class D3D12Texture : public RenderTexture
{
public:
    D3D12Texture(ComPtr<ID3D12Resource> res, int w, int h) : resource(res), width(w), height(h) {}

    void bind(int unit) override
    {
        (void)unit; // Unused - binding done via descriptor tables
        // Binding in D3D12 is done via descriptor tables
    }

    void unbind() override
    {
        // Not applicable in D3D12
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
        return resource.Get();
    }

private:
    ComPtr<ID3D12Resource> resource;
    int width;
    int height;
};

class D3D12Shader : public RenderShader
{
public:
    D3D12Shader() {}

    void bind() override
    {
        // Shaders in D3D12 are bound via pipeline state objects
    }

    void unbind() override
    {
        // Not applicable in D3D12
    }

    void setUniformInt(const std::string& name, int value) override
    {
        (void)name;
        (void)value; // Parameters reserved for D3D12 root constants/constant buffer implementation
        // Constants in D3D12 are set via root constants or constant buffers
    }

    void setUniformFloat(const std::string& name, float value) override
    {
        (void)name;
        (void)value; // Parameters reserved for D3D12 root constants/constant buffer implementation
        // Constants in D3D12 are set via root constants or constant buffers
    }

    void setUniformVec2(const std::string& name, const glm::vec2& value) override
    {
        (void)name;
        (void)value; // Parameters reserved for D3D12 root constants/constant buffer implementation
        // Constants in D3D12 are set via root constants or constant buffers
    }

    void setUniformVec3(const std::string& name, const glm::vec3& value) override
    {
        (void)name;
        (void)value; // Parameters reserved for D3D12 root constants/constant buffer implementation
        // Constants in D3D12 are set via root constants or constant buffers
    }

    void setUniformVec4(const std::string& name, const glm::vec4& value) override
    {
        (void)name;
        (void)value; // Parameters reserved for D3D12 root constants/constant buffer implementation
        // Constants in D3D12 are set via root constants or constant buffers
    }

    void setUniformMat4(const std::string& name, const glm::mat4& value) override
    {
        (void)name;
        (void)value; // Parameters reserved for D3D12 root constants/constant buffer implementation
        // Constants in D3D12 are set via root constants or constant buffers
    }

    void* getNativeHandle() override
    {
        return nullptr;
    }

private:
    // Would store shader bytecode or pipeline state
};

DirectX12RenderContext::DirectX12RenderContext()
    : window(nullptr), width(0), height(0), currentFrame(0)
{
    for (int i = 0; i < FRAME_COUNT; ++i) {
        fenceValues[i] = 0;
    }
}

DirectX12RenderContext::~DirectX12RenderContext()
{
    shutdown();
}

bool DirectX12RenderContext::initialize(void* win)
{
    std::cout << "[DirectX 12] Initializing DirectX 12 render context..." << std::endl;

    if (!win) {
        std::cerr << "[DirectX 12] Invalid window pointer" << std::endl;
        return false;
    }

    window = win;
    width = static_cast<int>(WindowAdapter::getWidth(window));
    height = static_cast<int>(WindowAdapter::getHeight(window));

    #ifdef _DEBUG
    if (!enableDebugLayer()) {
        std::cout << "[DirectX 12] Warning: Failed to enable debug layer" << std::endl;
    }
    #endif

    if (!createDevice()) {
        std::cerr << "[DirectX 12] Failed to create device" << std::endl;
        return false;
    }

    if (!createCommandQueue()) {
        std::cerr << "[DirectX 12] Failed to create command queue" << std::endl;
        return false;
    }

    // CRITICAL FIX FOR VIEWPORT-ONLY RENDERING:
    // DO NOT create a swap chain for the main window during initialization.
    // The swap chain will ONLY be created for the viewport child window when
    // setViewportWindow() + recreateSwapChain() are called.
    // This prevents any rendering to the main window that could show through panel gaps.
    std::cout << "[DirectX 12] Skipping main window swap chain creation - will only use viewport swap chain" << std::endl;
    std::cout << "[DirectX 12] Swap chain will be created when viewport is ready via setViewportWindow() + recreateSwapChain()" << std::endl;

    // NOTE: We skip creating swap chain during initialization.
    // However, we MUST create descriptor heaps as they are needed for rendering and don't depend on swap chain.
    // Descriptor heaps will be used when the viewport swap chain is created later.

    // Create descriptor heaps (needed for rendering, independent of swap chain)
    if (!createDescriptorHeaps()) {
        std::cerr << "[DirectX 12] Failed to create descriptor heaps" << std::endl;
        return false;
    }

    // Skip these calls that depend on swap chain existing:
    // - createSwapchain() - will be called via recreateSwapChain() when viewport is ready
    // - createRenderTargets() - depends on swap chain
    // - createDepthStencil() - depends on swap chain dimensions

    if (!createCommandAllocators()) {
        std::cerr << "[DirectX 12] Failed to create command allocators" << std::endl;
        return false;
    }

    if (!createCommandList()) {
        std::cerr << "[DirectX 12] Failed to create command list" << std::endl;
        return false;
    }

    if (!createFences()) {
        std::cerr << "[DirectX 12] Failed to create fences" << std::endl;
        return false;
    }

    // Initialize voxel rendering system
    if (!initializeVoxelRendering()) {
        std::cerr << "[DirectX 12] Failed to initialize voxel rendering" << std::endl;
        return false;
    }

    std::cout << "[DirectX 12] DirectX 12 context initialized successfully" << std::endl;
    return true;
}

void DirectX12RenderContext::shutdown()
{
    std::cout << "[DirectX 12] Shutting down DirectX 12 context..." << std::endl;

    // Wait for GPU to finish all work
    waitForGPU();

    // Shutdown voxel rendering first
    shutdownVoxelRendering();

    // Close fence event
    if (fenceEvent) {
        CloseHandle(fenceEvent);
        fenceEvent = nullptr;
    }

    // Release resources
    fence.Reset();
    commandList.Reset();

    for (int i = 0; i < FRAME_COUNT; ++i) {
        commandAllocators[i].Reset();
        renderTargets[i].Reset();
    }

    depthStencil.Reset();
    dsvHeap.Reset();
    srvHeap.Reset();
    rtvHeap.Reset();
    swapchain.Reset();
    commandQueue.Reset();
    device.Reset();
}

bool DirectX12RenderContext::beginFrame()
{
    // CRITICAL: Only render if we have a valid swap chain (i.e., viewport is set up)
    // This prevents any rendering to the main window
    if (!swapchain) {
        // No swap chain yet - viewport not set up, skip rendering
        return false;
    }

    // Reset command allocator for current frame
    if (commandAllocators[currentFrame]) {
        HRESULT hr = commandAllocators[currentFrame]->Reset();
        if (FAILED(hr)) {
            std::cerr << "[DirectX 12] Failed to reset command allocator" << std::endl;
            return false;
        }
    }

    // Reset command list
    if (commandList && commandAllocators[currentFrame]) {
        HRESULT hr = commandList->Reset(commandAllocators[currentFrame].Get(), nullptr);
        if (FAILED(hr)) {
            std::cerr << "[DirectX 12] Failed to reset command list" << std::endl;
            return false;
        }
    }

    // Transition render target from PRESENT to RENDER_TARGET state
    if (commandList && renderTargets[currentFrame]) {
        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource = renderTargets[currentFrame].Get();
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

        commandList->ResourceBarrier(1, &barrier);
    }

    // Set render target and depth stencil
    if (commandList && rtvHeap && dsvHeap) {
        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();
        rtvHandle.ptr += currentFrame * rtvDescriptorSize;

        D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dsvHeap->GetCPUDescriptorHandleForHeapStart();

        commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);
    }

    // Clear render target using stored clear color
    if (commandList && rtvHeap) {
        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();
        rtvHandle.ptr += currentFrame * rtvDescriptorSize;
        commandList->ClearRenderTargetView(rtvHandle, clearColorValue, 0, nullptr);
    }

    // Clear depth stencil
    clearDepth(1.0f);

    return true;
}

void DirectX12RenderContext::endFrame()
{
    // Transition render target from RENDER_TARGET to PRESENT state
    if (commandList && renderTargets[currentFrame]) {
        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource = renderTargets[currentFrame].Get();
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

        commandList->ResourceBarrier(1, &barrier);
    }

    // Close command list
    if (commandList) {
        HRESULT hr = commandList->Close();
        if (FAILED(hr)) {
            std::cerr << "[DirectX 12] Failed to close command list" << std::endl;
            return;
        }
    }

    // Execute command list
    if (commandQueue && commandList) {
        ID3D12CommandList* commandLists[] = {commandList.Get()};
        commandQueue->ExecuteCommandLists(1, commandLists);
    }

    // Present
    if (swapchain) {
        HRESULT hr = swapchain->Present(1, 0); // VSync enabled
        if (FAILED(hr)) {
            std::cerr << "[DirectX 12] Failed to present" << std::endl;
        }
    }

    // Move to next frame
    moveToNextFrame();
}

void DirectX12RenderContext::waitIdle()
{
    waitForGPU();
}

void DirectX12RenderContext::setViewport(int x, int y, int w, int h)
{
    if (!commandList)
        return;

    D3D12_VIEWPORT viewport = {};
    viewport.TopLeftX = static_cast<float>(x);
    viewport.TopLeftY = static_cast<float>(y);
    viewport.Width = static_cast<float>(w);
    viewport.Height = static_cast<float>(h);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    commandList->RSSetViewports(1, &viewport);
    width = w;
    height = h;
}

void DirectX12RenderContext::setScissor(int x, int y, int w, int h)
{
    if (!commandList)
        return;

    D3D12_RECT scissorRect = {};
    scissorRect.left = x;
    scissorRect.top = y;
    scissorRect.right = x + w;
    scissorRect.bottom = y + h;

    commandList->RSSetScissorRects(1, &scissorRect);
}

void DirectX12RenderContext::clearColor(float r, float g, float b, float a)
{
    // Store the clear color for use in beginFrame()
    clearColorValue[0] = r;
    clearColorValue[1] = g;
    clearColorValue[2] = b;
    clearColorValue[3] = a;
}

void DirectX12RenderContext::clearDepth(float depth)
{
    if (!commandList || !dsvHeap)
        return;

    commandList->ClearDepthStencilView(dsvHeap->GetCPUDescriptorHandleForHeapStart(),
                                       D3D12_CLEAR_FLAG_DEPTH, depth, 0, 0, nullptr);
}

std::shared_ptr<RenderBuffer> DirectX12RenderContext::createVertexBuffer(const void* data,
                                                                         size_t size)
{
    if (!device)
        return nullptr;

    // Create committed resource for vertex buffer
    D3D12_HEAP_PROPERTIES heapProps = {};
    heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;

    D3D12_RESOURCE_DESC resourceDesc = {};
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resourceDesc.Width = size;
    resourceDesc.Height = 1;
    resourceDesc.DepthOrArraySize = 1;
    resourceDesc.MipLevels = 1;
    resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    ComPtr<ID3D12Resource> buffer;
    HRESULT hr = device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &resourceDesc,
                                                 D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
                                                 IID_PPV_ARGS(&buffer));

    if (FAILED(hr)) {
        std::cerr << "[DirectX 12] Failed to create vertex buffer" << std::endl;
        return nullptr;
    }

    // Copy data if provided
    if (data) {
        void* mappedData = nullptr;
        hr = buffer->Map(0, nullptr, &mappedData);
        if (SUCCEEDED(hr)) {
            memcpy(mappedData, data, size);
            buffer->Unmap(0, nullptr);
        }
    }

    return std::make_shared<D3D12Buffer>(buffer, size);
}

std::shared_ptr<RenderBuffer> DirectX12RenderContext::createIndexBuffer(const void* data,
                                                                        size_t size)
{
    // Similar to vertex buffer
    return createVertexBuffer(data, size);
}

std::shared_ptr<RenderBuffer> DirectX12RenderContext::createUniformBuffer(size_t size)
{
    // Constant buffers must be 256-byte aligned
    size_t alignedSize = (size + 255) & ~255;
    return createVertexBuffer(nullptr, alignedSize);
}

std::shared_ptr<RenderTexture> DirectX12RenderContext::createTexture(int w, int h, const void* data)
{
    if (!device)
        return nullptr;

    // Create texture resource
    D3D12_HEAP_PROPERTIES heapProps = {};
    heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
    heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heapProps.CreationNodeMask = 1;
    heapProps.VisibleNodeMask = 1;

    D3D12_RESOURCE_DESC textureDesc = {};
    textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    textureDesc.Alignment = 0;
    textureDesc.Width = w;
    textureDesc.Height = h;
    textureDesc.DepthOrArraySize = 1;
    textureDesc.MipLevels = 1;
    textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.SampleDesc.Quality = 0;
    textureDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    ComPtr<ID3D12Resource> texture;
    HRESULT hr = device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &textureDesc,
                                                 D3D12_RESOURCE_STATE_COPY_DEST, nullptr,
                                                 IID_PPV_ARGS(&texture));

    if (FAILED(hr)) {
        std::cerr << "[DirectX 12] Failed to create texture resource" << std::endl;
        return nullptr;
    }

    // If data is provided, would need to upload it via upload heap
    // This is a complex operation in D3D12, skipping for now
    if (data) {
        std::cout << "[DirectX 12] Warning: Texture data upload not yet implemented" << std::endl;
    }

    std::cout << "[DirectX 12] Texture created successfully" << std::endl;
    return std::make_shared<D3D12Texture>(texture, w, h);
}

std::shared_ptr<RenderShader> DirectX12RenderContext::createShader(const std::string& vertexCode,
                                                                   const std::string& fragmentCode)
{
    (void)vertexCode;
    (void)fragmentCode; // Unused - stub implementation
    // Would compile HLSL shaders to bytecode and create pipeline state object
    std::cout << "[DirectX 12] Creating shader (stub)" << std::endl;
    return std::make_shared<D3D12Shader>();
}

bool DirectX12RenderContext::enableDebugLayer()
{
    #ifdef _DEBUG
    ComPtr<ID3D12Debug> debugController;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
        debugController->EnableDebugLayer();
        std::cout << "[DirectX 12] Debug layer enabled" << std::endl;
        return true;
    }
    #endif
    return false;
}

bool DirectX12RenderContext::createDevice()
{
    HRESULT hr = D3D12CreateDevice(nullptr,                // Default adapter
                                   D3D_FEATURE_LEVEL_11_0, // Minimum feature level
                                   IID_PPV_ARGS(&device));

    if (FAILED(hr)) {
        std::cerr << "[DirectX 12] Failed to create D3D12 device" << std::endl;
        return false;
    }

    std::cout << "[DirectX 12] D3D12 device created successfully" << std::endl;
    return true;
}

bool DirectX12RenderContext::createCommandQueue()
{
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

    HRESULT hr = device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue));

    if (FAILED(hr)) {
        std::cerr << "[DirectX 12] Failed to create command queue" << std::endl;
        return false;
    }

    std::cout << "[DirectX 12] Command queue created" << std::endl;
    return true;
}

bool DirectX12RenderContext::createSwapchain()
{
    // Get native window handle
    HWND hwnd = static_cast<HWND>(WindowAdapter::getNativeHandle(window));
    if (!hwnd) {
        std::cerr << "[DirectX 12] Failed to get native window handle" << std::endl;
        return false;
    }

    // Create DXGI factory
    ComPtr<IDXGIFactory4> factory;
    HRESULT hr = CreateDXGIFactory2(0, IID_PPV_ARGS(&factory));
    if (FAILED(hr)) {
        std::cerr << "[DirectX 12] Failed to create DXGI factory" << std::endl;
        return false;
    }

    // Describe and create the swap chain
    DXGI_SWAP_CHAIN_DESC1 swapchainDesc = {};
    swapchainDesc.Width = width;
    swapchainDesc.Height = height;
    swapchainDesc.Format = rtvFormat;
    swapchainDesc.Stereo = FALSE;
    swapchainDesc.SampleDesc.Count = 1;
    swapchainDesc.SampleDesc.Quality = 0;
    swapchainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapchainDesc.BufferCount = FRAME_COUNT;
    swapchainDesc.Scaling = DXGI_SCALING_STRETCH;
    swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapchainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    swapchainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    ComPtr<IDXGISwapChain1> swapchain1;
    hr = factory->CreateSwapChainForHwnd(commandQueue.Get(), hwnd, &swapchainDesc, nullptr, nullptr,
                                         &swapchain1);

    if (FAILED(hr)) {
        std::cerr << "[DirectX 12] Failed to create swapchain" << std::endl;
        return false;
    }

    // Disable Alt+Enter fullscreen toggle
    factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER);

    // Get IDXGISwapChain3 interface
    hr = swapchain1.As(&swapchain);
    if (FAILED(hr)) {
        std::cerr << "[DirectX 12] Failed to get IDXGISwapChain3 interface" << std::endl;
        return false;
    }

    currentFrame = swapchain->GetCurrentBackBufferIndex();

    std::cout << "[DirectX 12] Swapchain created successfully" << std::endl;
    return true;
}

bool DirectX12RenderContext::createRenderTargets()
{
    if (!swapchain || !rtvHeap) {
        std::cerr << "[DirectX 12] Swapchain or RTV heap not initialized" << std::endl;
        return false;
    }

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();

    // Create a RTV for each frame
    for (UINT i = 0; i < FRAME_COUNT; i++) {
        HRESULT hr = swapchain->GetBuffer(i, IID_PPV_ARGS(&renderTargets[i]));
        if (FAILED(hr)) {
            std::cerr << "[DirectX 12] Failed to get buffer " << i << " from swapchain"
                      << std::endl;
            return false;
        }

        device->CreateRenderTargetView(renderTargets[i].Get(), nullptr, rtvHandle);
        rtvHandle.ptr += rtvDescriptorSize;
    }

    std::cout << "[DirectX 12] Render targets created successfully" << std::endl;
    return true;
}

bool DirectX12RenderContext::createDepthStencil()
{
    if (!device || !dsvHeap) {
        std::cerr << "[DirectX 12] Device or DSV heap not initialized" << std::endl;
        return false;
    }

    // Describe and create a depth stencil
    D3D12_HEAP_PROPERTIES heapProps = {};
    heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
    heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heapProps.CreationNodeMask = 1;
    heapProps.VisibleNodeMask = 1;

    D3D12_RESOURCE_DESC depthStencilDesc = {};
    depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    depthStencilDesc.Alignment = 0;
    depthStencilDesc.Width = width;
    depthStencilDesc.Height = height;
    depthStencilDesc.DepthOrArraySize = 1;
    depthStencilDesc.MipLevels = 1;
    depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
    depthStencilDesc.SampleDesc.Count = 1;
    depthStencilDesc.SampleDesc.Quality = 0;
    depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_CLEAR_VALUE clearValue = {};
    clearValue.Format = DXGI_FORMAT_D32_FLOAT;
    clearValue.DepthStencil.Depth = 1.0f;
    clearValue.DepthStencil.Stencil = 0;

    HRESULT hr = device->CreateCommittedResource(
        &heapProps, D3D12_HEAP_FLAG_NONE, &depthStencilDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE,
        &clearValue, IID_PPV_ARGS(&depthStencil));

    if (FAILED(hr)) {
        std::cerr << "[DirectX 12] Failed to create depth stencil buffer" << std::endl;
        return false;
    }

    // Create depth stencil view
    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

    device->CreateDepthStencilView(depthStencil.Get(), &dsvDesc,
                                   dsvHeap->GetCPUDescriptorHandleForHeapStart());

    std::cout << "[DirectX 12] Depth stencil created successfully" << std::endl;
    return true;
}

bool DirectX12RenderContext::setViewportWindow(void* viewportHandle)
{
    if (!viewportHandle) {
        LOG_ERROR_C("Invalid viewport window handle", "DirectX12");
        return false;
    }

    viewportHwnd = viewportHandle;
    LOG_INFO_C("Viewport window handle set", "DirectX12");
    
    return true;
}

bool DirectX12RenderContext::recreateSwapChain(int newWidth, int newHeight)
{
    if (newWidth <= 0 || newHeight <= 0) {
        LOG_ERROR_C("Invalid swap chain dimensions: " + std::to_string(newWidth) + "x" + std::to_string(newHeight), "DirectX12");
        return false;
    }

    LOG_INFO_C("Recreating DirectX 12 swap chain for viewport: " + std::to_string(newWidth) + "x" + std::to_string(newHeight), "DirectX12");

    // Wait for GPU to finish all work
    waitForGPU();

    // Update dimensions
    width = newWidth;
    height = newHeight;

    // Release existing swap chain resources
    for (UINT i = 0; i < FRAME_COUNT; i++) {
        renderTargets[i].Reset();
        fenceValues[i] = fenceValues[currentFrame];
    }
    depthStencil.Reset();
    swapchain.Reset();

    // CRITICAL: ONLY use viewport window handle for swap chain creation
    // NEVER fall back to main window - viewport is the ONLY DirectX rendering target
    if (!viewportHwnd) {
        LOG_ERROR_C("Cannot recreate swap chain - viewport window handle not set. Swap chain requires viewport.", "DirectX12");
        return false;
    }
    
    HWND targetHwnd = static_cast<HWND>(viewportHwnd);
    if (!targetHwnd) {
        LOG_ERROR_C("Invalid viewport window handle for swap chain", "DirectX12");
        return false;
    }

    // Get DXGI factory
    ComPtr<IDXGIFactory4> factory;
    HRESULT hr = CreateDXGIFactory2(0, IID_PPV_ARGS(&factory));
    if (FAILED(hr)) {
        LOG_ERROR_C("Failed to create DXGI factory for swap chain recreation", "DirectX12");
        return false;
    }

    // Create swap chain description for viewport
    DXGI_SWAP_CHAIN_DESC1 swapchainDesc = {};
    swapchainDesc.Width = width;
    swapchainDesc.Height = height;
    swapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapchainDesc.Stereo = FALSE;
    swapchainDesc.SampleDesc.Count = 1;
    swapchainDesc.SampleDesc.Quality = 0;
    swapchainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapchainDesc.BufferCount = FRAME_COUNT;
    swapchainDesc.Scaling = DXGI_SCALING_STRETCH;
    swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapchainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    swapchainDesc.Flags = 0;

    // Create new swapchain for viewport
    ComPtr<IDXGISwapChain1> swapchain1;
    hr = factory->CreateSwapChainForHwnd(commandQueue.Get(), targetHwnd, &swapchainDesc, nullptr, nullptr, &swapchain1);
    if (FAILED(hr)) {
        LOG_ERROR_C("Failed to create new swapchain for viewport", "DirectX12");
        return false;
    }

    // Query for IDXGISwapChain3
    hr = swapchain1.As(&swapchain);
    if (FAILED(hr)) {
        LOG_ERROR_C("Failed to query IDXGISwapChain3", "DirectX12");
        return false;
    }

    // Disable Alt+Enter
    factory->MakeWindowAssociation(targetHwnd, DXGI_MWA_NO_ALT_ENTER);

    // Get current frame index
    currentFrame = swapchain->GetCurrentBackBufferIndex();

    // Recreate render targets
    if (!createRenderTargets()) {
        LOG_ERROR_C("Failed to recreate render targets", "DirectX12");
        return false;
    }

    // Recreate depth stencil
    if (!createDepthStencil()) {
        LOG_ERROR_C("Failed to recreate depth stencil", "DirectX12");
        return false;
    }

    // CRITICAL FIX: Immediately clear the new render targets to prevent artifacts
    // When resizing, old swap chain content can show through if we don't clear immediately
    // Wait for command allocator to be ready
    UINT frameIndex = swapchain->GetCurrentBackBufferIndex();
    if (commandAllocators[frameIndex]) {
        commandAllocators[frameIndex]->Reset();
        commandList->Reset(commandAllocators[frameIndex].Get(), nullptr);
        
        // Transition render target to render target state
        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource = renderTargets[frameIndex].Get();
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        commandList->ResourceBarrier(1, &barrier);
        
        // Clear render target to black
        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();
        rtvHandle.ptr += frameIndex * rtvDescriptorSize;
        float clearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};  // Black
        commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
        
        // Clear depth stencil
        if (dsvHeap) {
            D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dsvHeap->GetCPUDescriptorHandleForHeapStart();
            commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
        }
        
        // Transition back to present state
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
        commandList->ResourceBarrier(1, &barrier);
        
        // Execute and wait
        commandList->Close();
        ID3D12CommandList* cmdLists[] = {commandList.Get()};
        commandQueue->ExecuteCommandLists(1, cmdLists);
        waitForGPU();
        
        LOG_INFO_C("Cleared new render targets to prevent artifacts", "DirectX12");
    }

    LOG_INFO_C("DirectX 12 swap chain recreated successfully for viewport: " + std::to_string(width) + "x" + std::to_string(height), "DirectX12");
    return true;
}

bool DirectX12RenderContext::createDescriptorHeaps()
{
    // Create RTV descriptor heap
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = FRAME_COUNT;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

    HRESULT hr = device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvHeap));
    if (FAILED(hr)) {
        std::cerr << "[DirectX 12] Failed to create RTV heap" << std::endl;
        return false;
    }

    rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    // Create DSV descriptor heap
    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
    dsvHeapDesc.NumDescriptors = 1;
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

    hr = device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&dsvHeap));
    if (FAILED(hr)) {
        std::cerr << "[DirectX 12] Failed to create DSV heap" << std::endl;
        return false;
    }

    // Create SRV descriptor heap for ImGui and other shader resources
    D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
    srvHeapDesc.NumDescriptors = SRV_HEAP_SIZE;
    srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

    hr = device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&srvHeap));
    if (FAILED(hr)) {
        std::cerr << "[DirectX 12] Failed to create SRV heap" << std::endl;
        return false;
    }

    std::cout << "[DirectX 12] Descriptor heaps created" << std::endl;
    return true;
}

bool DirectX12RenderContext::createCommandAllocators()
{
    for (int i = 0; i < FRAME_COUNT; ++i) {
        HRESULT hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
                                                    IID_PPV_ARGS(&commandAllocators[i]));

        if (FAILED(hr)) {
            std::cerr << "[DirectX 12] Failed to create command allocator " << i << std::endl;
            return false;
        }
    }

    std::cout << "[DirectX 12] Command allocators created" << std::endl;
    return true;
}

bool DirectX12RenderContext::createCommandList()
{
    HRESULT hr =
        device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocators[0].Get(),
                                  nullptr, IID_PPV_ARGS(&commandList));

    if (FAILED(hr)) {
        std::cerr << "[DirectX 12] Failed to create command list" << std::endl;
        return false;
    }

    // Command lists are created in recording state, close it for now
    commandList->Close();

    std::cout << "[DirectX 12] Command list created" << std::endl;
    return true;
}

bool DirectX12RenderContext::createFences()
{
    HRESULT hr = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
    if (FAILED(hr)) {
        std::cerr << "[DirectX 12] Failed to create fence" << std::endl;
        return false;
    }

    // Create event for fence signaling
    fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (!fenceEvent) {
        std::cerr << "[DirectX 12] Failed to create fence event" << std::endl;
        return false;
    }

    // Initialize fence values
    for (int i = 0; i < FRAME_COUNT; ++i) {
        fenceValues[i] = 0;
    }

    std::cout << "[DirectX 12] Fences created" << std::endl;
    return true;
}

void DirectX12RenderContext::waitForGPU()
{
    if (!commandQueue || !fence)
        return;

    // Signal and wait for fence
    const UINT64 fenceValue = fenceValues[currentFrame];
    HRESULT hr = commandQueue->Signal(fence.Get(), fenceValue);
    if (FAILED(hr))
        return;

    hr = fence->SetEventOnCompletion(fenceValue, fenceEvent);
    if (FAILED(hr))
        return;

    WaitForSingleObject(fenceEvent, INFINITE);

    fenceValues[currentFrame]++;
}

void DirectX12RenderContext::moveToNextFrame()
{
    // Schedule a Signal command in the queue
    const UINT64 currentFenceValue = fenceValues[currentFrame];
    if (commandQueue) {
        commandQueue->Signal(fence.Get(), currentFenceValue);
    }

    // Update frame index
    currentFrame = (currentFrame + 1) % FRAME_COUNT;

    // Wait if next frame is not ready yet
    if (fence && fence->GetCompletedValue() < fenceValues[currentFrame]) {
        fence->SetEventOnCompletion(fenceValues[currentFrame], fenceEvent);
        WaitForSingleObject(fenceEvent, INFINITE);
    }

    // Set fence value for next frame
    fenceValues[currentFrame] = currentFenceValue + 1;
}

bool DirectX12RenderContext::initializeVoxelRendering()
{
    // Create root signature for voxel rendering
    // Root parameter for constant buffer (MVP matrix)
    D3D12_ROOT_PARAMETER rootParameter = {};
    rootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
    rootParameter.Constants.ShaderRegister = 0;
    rootParameter.Constants.RegisterSpace = 0;
    rootParameter.Constants.Num32BitValues = 16; // 4x4 matrix
    rootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
    rootSignatureDesc.NumParameters = 1;
    rootSignatureDesc.pParameters = &rootParameter;
    rootSignatureDesc.NumStaticSamplers = 0;
    rootSignatureDesc.pStaticSamplers = nullptr;
    rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    ComPtr<ID3DBlob> signature;
    ComPtr<ID3DBlob> error;
    HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1,
                                             &signature, &error);

    if (FAILED(hr)) {
        if (error) {
            std::cerr << "[DirectX 12] Root signature serialization error: "
                      << static_cast<const char*>(error->GetBufferPointer()) << std::endl;
        }
        return false;
    }

    hr = device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(),
                                     IID_PPV_ARGS(&voxelRootSignature));

    if (FAILED(hr)) {
        std::cerr << "[DirectX 12] Failed to create root signature" << std::endl;
        return false;
    }

    // Compile shaders
    std::string shaderCode = R"(
        // Voxel Rendering Shader for DirectX 12
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

    ComPtr<ID3DBlob> vsBlob;
    ComPtr<ID3DBlob> psBlob;
    ComPtr<ID3DBlob> errorBlob;

    hr = D3DCompile(shaderCode.c_str(), shaderCode.size(), nullptr, nullptr, nullptr, "VSMain",
                   "vs_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &vsBlob, &errorBlob);

    if (FAILED(hr)) {
        if (errorBlob) {
            std::cerr << "[DirectX 12] Vertex shader compilation error: "
                      << static_cast<const char*>(errorBlob->GetBufferPointer()) << std::endl;
        }
        return false;
    }

    hr = D3DCompile(shaderCode.c_str(), shaderCode.size(), nullptr, nullptr, nullptr, "PSMain",
                   "ps_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &psBlob, &errorBlob);

    if (FAILED(hr)) {
        if (errorBlob) {
            std::cerr << "[DirectX 12] Pixel shader compilation error: "
                      << static_cast<const char*>(errorBlob->GetBufferPointer()) << std::endl;
        }
        return false;
    }

    // Create input layout
    D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
    };

    // Create pipeline state object
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = {inputLayout, _countof(inputLayout)};
    psoDesc.pRootSignature = voxelRootSignature.Get();
    psoDesc.VS = {vsBlob->GetBufferPointer(), vsBlob->GetBufferSize()};
    psoDesc.PS = {psBlob->GetBufferPointer(), psBlob->GetBufferSize()};
    psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
    psoDesc.RasterizerState.FrontCounterClockwise = FALSE;
    psoDesc.RasterizerState.DepthClipEnable = TRUE;
    psoDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    psoDesc.DepthStencilState.DepthEnable = TRUE;
    psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
    psoDesc.DepthStencilState.StencilEnable = FALSE;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = rtvFormat;
    psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    psoDesc.SampleDesc.Count = 1;

    hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&voxelPipelineState));

    if (FAILED(hr)) {
        std::cerr << "[DirectX 12] Failed to create voxel pipeline state" << std::endl;
        return false;
    }

    std::cout << "[DirectX 12] Voxel rendering initialized successfully" << std::endl;
    return true;
}

void DirectX12RenderContext::shutdownVoxelRendering()
{
    chunkRenderData.clear();
    voxelPipelineState.Reset();
    voxelRootSignature.Reset();
}

void DirectX12RenderContext::renderVoxelWorld(VoxelWorld* world, Player* player)
{
    if (!world || !player || !voxelPipelineState || !commandList) {
        return;
    }

    // Avoid division by zero if swap chain dimensions are invalid
    if (width <= 0 || height <= 0) {
        return;
    }

    // Set pipeline state
    commandList->SetPipelineState(voxelPipelineState.Get());
    commandList->SetGraphicsRootSignature(voxelRootSignature.Get());
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

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

            // Create vertex buffer in upload heap
            size_t vertexBufferSize = vertices.size() * sizeof(float);
            
            D3D12_HEAP_PROPERTIES uploadHeapProps = {};
            uploadHeapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
            uploadHeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
            uploadHeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

            D3D12_RESOURCE_DESC vertexBufferDesc = {};
            vertexBufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
            vertexBufferDesc.Width = vertexBufferSize;
            vertexBufferDesc.Height = 1;
            vertexBufferDesc.DepthOrArraySize = 1;
            vertexBufferDesc.MipLevels = 1;
            vertexBufferDesc.Format = DXGI_FORMAT_UNKNOWN;
            vertexBufferDesc.SampleDesc.Count = 1;
            vertexBufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

            renderData.vertexBuffer.Reset();
            HRESULT hr = device->CreateCommittedResource(
                &uploadHeapProps,
                D3D12_HEAP_FLAG_NONE,
                &vertexBufferDesc,
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(&renderData.vertexBuffer));

            if (FAILED(hr)) {
                continue;
            }

            // Upload vertex data
            void* vertexDataPtr = nullptr;
            D3D12_RANGE readRange = {0, 0};
            renderData.vertexBuffer->Map(0, &readRange, &vertexDataPtr);
            memcpy(vertexDataPtr, vertices.data(), vertexBufferSize);
            renderData.vertexBuffer->Unmap(0, nullptr);

            // Create vertex buffer view
            renderData.vertexBufferView.BufferLocation = renderData.vertexBuffer->GetGPUVirtualAddress();
            renderData.vertexBufferView.StrideInBytes = sizeof(float) * 6; // 3 pos + 3 normal
            renderData.vertexBufferView.SizeInBytes = static_cast<UINT>(vertexBufferSize);

            // Create index buffer in upload heap
            size_t indexBufferSize = indices.size() * sizeof(uint32_t);

            D3D12_RESOURCE_DESC indexBufferDesc = {};
            indexBufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
            indexBufferDesc.Width = indexBufferSize;
            indexBufferDesc.Height = 1;
            indexBufferDesc.DepthOrArraySize = 1;
            indexBufferDesc.MipLevels = 1;
            indexBufferDesc.Format = DXGI_FORMAT_UNKNOWN;
            indexBufferDesc.SampleDesc.Count = 1;
            indexBufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

            renderData.indexBuffer.Reset();
            hr = device->CreateCommittedResource(
                &uploadHeapProps,
                D3D12_HEAP_FLAG_NONE,
                &indexBufferDesc,
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(&renderData.indexBuffer));

            if (FAILED(hr)) {
                continue;
            }

            // Upload index data
            void* indexDataPtr = nullptr;
            renderData.indexBuffer->Map(0, &readRange, &indexDataPtr);
            memcpy(indexDataPtr, indices.data(), indexBufferSize);
            renderData.indexBuffer->Unmap(0, nullptr);

            // Create index buffer view
            renderData.indexBufferView.BufferLocation = renderData.indexBuffer->GetGPUVirtualAddress();
            renderData.indexBufferView.Format = DXGI_FORMAT_R32_UINT;
            renderData.indexBufferView.SizeInBytes = static_cast<UINT>(indexBufferSize);

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

            // Set root constants (MVP matrix)
            commandList->SetGraphicsRoot32BitConstants(0, 16, glm::value_ptr(mvp), 0);

            // Set vertex and index buffers
            commandList->IASetVertexBuffers(0, 1, &renderData.vertexBufferView);
            commandList->IASetIndexBuffer(&renderData.indexBufferView);

            // Draw
            commandList->DrawIndexedInstanced(static_cast<UINT>(renderData.indexCount), 1, 0, 0, 0);
        }
    }
}

} // namespace fresh

#endif // _WIN32
