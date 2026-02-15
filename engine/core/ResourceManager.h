#pragma once
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include <future>
#include <queue>
#include <thread>
#include <condition_variable>

namespace fresh
{

// Forward declarations
class Texture;
class Mesh;
class Material;
class AudioClip;
class Shader;

/**
 * @brief Type of resource
 */
enum class ResourceType {
    Texture,
    Mesh,
    Material,
    Audio,
    Shader,
    AssetPack,
    Script,
    Prefab,
    Unknown
};

/**
 * @brief Base class for all resources
 */
class Resource
{
public:
    virtual ~Resource() = default;

    const std::string& getName() const
    {
        return name;
    }
    const std::string& getPath() const
    {
        return path;
    }
    ResourceType getType() const
    {
        return type;
    }
    bool isLoaded() const
    {
        return loaded;
    }

    virtual void load() = 0;
    virtual void unload() = 0;
    virtual size_t getMemoryUsage() const = 0;

protected:
    std::string name;
    std::string path;
    ResourceType type = ResourceType::Unknown;
    bool loaded = false;
};

/**
 * @brief Resource handle for reference counting
 */
template <typename T>
class ResourceHandle
{
public:
    ResourceHandle() : resource(nullptr) {}
    ResourceHandle(std::shared_ptr<T> res) : resource(res) {}

    T* get() const
    {
        return resource.get();
    }
    T* operator->() const
    {
        return resource.get();
    }
    bool isValid() const
    {
        return resource != nullptr && resource->isLoaded();
    }

private:
    std::shared_ptr<T> resource;
};

/**
 * @brief Loading request for async loading
 */
struct LoadingRequest {
    std::string path;
    ResourceType type;
    std::promise<std::shared_ptr<Resource>> promise;
    
    // Make it move-only since std::promise is non-copyable
    LoadingRequest() = default;
    LoadingRequest(const LoadingRequest&) = delete;
    LoadingRequest& operator=(const LoadingRequest&) = delete;
    LoadingRequest(LoadingRequest&&) = default;
    LoadingRequest& operator=(LoadingRequest&&) = default;
};

/**
 * @brief Central resource management system
 *
 * Handles loading, unloading, streaming, and optimization of all game assets.
 * Implements reference counting, hot-reloading, memory management, and async loading.
 */
class ResourceManager
{
public:
    static ResourceManager& getInstance();

    /**
     * @brief Initialize resource manager
     * @param assetDirectory Root directory for assets
     */
    void initialize(const std::string& assetDirectory);

    /**
     * @brief Shutdown and cleanup all resources
     */
    void shutdown();

    /**
     * @brief Load a resource by path (synchronous)
     * @param path Path to resource file
     * @param type Type of resource
     * @return Resource handle
     */
    template <typename T>
    ResourceHandle<T> load(const std::string& path);

    /**
     * @brief Load a resource asynchronously
     * @param path Path to resource file
     * @return Future to resource handle
     */
    template <typename T>
    std::future<ResourceHandle<T>> loadAsync(const std::string& path);

    /**
     * @brief Unload a specific resource
     */
    void unload(const std::string& path);

    /**
     * @brief Unload all resources of a type
     */
    void unloadType(ResourceType type);

    /**
     * @brief Unload all unused resources (ref count = 0)
     */
    void unloadUnused();

    /**
     * @brief Hot reload a resource (for development)
     */
    void hotReload(const std::string& path);

    /**
     * @brief Get total memory used by resources
     */
    size_t getTotalMemoryUsage() const;

    /**
     * @brief Get number of loaded resources
     */
    size_t getLoadedResourceCount() const;

    /**
     * @brief Register custom resource loader
     */
    void registerLoader(ResourceType type,
                        std::function<std::shared_ptr<Resource>(const std::string&)> loader);

    /**
     * @brief Scan directory for assets and register them
     */
    void scanDirectory(const std::string& directory, bool recursive = true);

    /**
     * @brief Get all resources of a specific type
     */
    std::vector<std::string> getResourcesOfType(ResourceType type) const;

    /**
     * @brief Check if resource exists
     */
    bool exists(const std::string& path) const;

    /**
     * @brief Check if resource is currently loading
     */
    bool isLoading(const std::string& path) const;

    /**
     * @brief Get loading progress (0.0 to 1.0)
     */
    float getLoadingProgress() const;

    /**
     * @brief Print resource statistics
     */
    void printStats() const;

    /**
     * @brief Create placeholder resource for missing files
     */
    template <typename T>
    ResourceHandle<T> createPlaceholder(const std::string& name);

private:
    ResourceManager();
    ~ResourceManager();
    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;

    std::shared_ptr<Resource> loadResource(const std::string& path, ResourceType type);
    ResourceType detectResourceType(const std::string& path) const;
    
    // Async loading worker thread
    void asyncLoadingWorker();
    void startAsyncLoader();
    void stopAsyncLoader();

    std::string assetDirectory;
    std::map<std::string, std::shared_ptr<Resource>> resources;
    std::map<std::string, std::shared_ptr<Resource>> placeholders;
    std::map<ResourceType, std::function<std::shared_ptr<Resource>(const std::string&)>> loaders;

    // Async loading
    std::deque<LoadingRequest> loadingQueue;
    std::thread asyncLoaderThread;
    std::condition_variable loaderCondition;
    std::atomic<bool> asyncLoaderRunning{false};
    std::atomic<size_t> totalLoadRequests{0};
    std::atomic<size_t> completedLoadRequests{0};

    mutable std::mutex mutex;
    mutable std::mutex queueMutex;
};

/**
 * @brief Texture resource
 */
class TextureResource : public Resource
{
public:
    TextureResource(const std::string& path);

    void load() override;
    void unload() override;
    size_t getMemoryUsage() const override;

    int getWidth() const
    {
        return width;
    }
    int getHeight() const
    {
        return height;
    }
    int getChannels() const
    {
        return channels;
    }
    const unsigned char* getData() const
    {
        return data;
    }

private:
    int width = 0;
    int height = 0;
    int channels = 0;
    unsigned char* data = nullptr;
    bool usedStbiAlloc = false; // Track allocation type for proper deallocation
};

/**
 * @brief Mesh resource
 */
class MeshResource : public Resource
{
public:
    MeshResource(const std::string& path);

    void load() override;
    void unload() override;
    size_t getMemoryUsage() const override;

    const std::vector<float>& getVertices() const
    {
        return vertices;
    }
    const std::vector<uint32_t>& getIndices() const
    {
        return indices;
    }

private:
    std::vector<float> vertices;
    std::vector<uint32_t> indices;
};

/**
 * @brief Audio clip resource
 */
class AudioClipResource : public Resource
{
public:
    AudioClipResource(const std::string& path);

    void load() override;
    void unload() override;
    size_t getMemoryUsage() const override;

    const std::vector<int16_t>& getSamples() const
    {
        return samples;
    }
    int getSampleRate() const
    {
        return sampleRate;
    }
    int getChannels() const
    {
        return channels;
    }

private:
    std::vector<int16_t> samples;
    int sampleRate = 0;
    int channels = 0;
};

} // namespace fresh
