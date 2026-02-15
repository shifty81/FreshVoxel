#include "core/ResourceManager.h"

#include <algorithm>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <unordered_map>

// Forward declare stb_image functions (implementation is in stb_image_impl.cpp)
extern "C" {
    unsigned char* stbi_load(char const* filename, int* x, int* y, int* channels_in_file, int desired_channels);
    void stbi_image_free(void* retval_from_stbi_load);
    const char* stbi_failure_reason(void);
}

// Include tinyobjloader for mesh loading
#ifdef FRESH_TINYOBJLOADER_AVAILABLE
#include <tiny_obj_loader.h>
#endif

// Include vorbis for OGG audio loading
#ifdef FRESH_VORBIS_AVAILABLE
#include <vorbis/vorbisfile.h>
#endif

namespace fresh
{

ResourceManager::ResourceManager()
{
    startAsyncLoader();
}

ResourceManager::~ResourceManager()
{
    stopAsyncLoader();
}

ResourceManager& ResourceManager::getInstance()
{
    static ResourceManager instance;
    return instance;
}

void ResourceManager::initialize(const std::string& assetDir)
{
    std::lock_guard<std::mutex> lock(mutex);
    assetDirectory = assetDir;

    std::cout << "ResourceManager initialized with asset directory: " << assetDirectory
              << std::endl;

    // Register default loaders
    registerLoader(ResourceType::Texture,
                   [](const std::string& path) { return std::make_shared<TextureResource>(path); });

    registerLoader(ResourceType::Mesh,
                   [](const std::string& path) { return std::make_shared<MeshResource>(path); });

    registerLoader(ResourceType::Audio, [](const std::string& path) {
        return std::make_shared<AudioClipResource>(path);
    });

    // Create default placeholder resources
    auto placeholderTexture = std::make_shared<TextureResource>("__placeholder_texture__");
    placeholderTexture->load();
    placeholders["texture"] = placeholderTexture;

    auto placeholderMesh = std::make_shared<MeshResource>("__placeholder_mesh__");
    placeholderMesh->load();
    placeholders["mesh"] = placeholderMesh;

    auto placeholderAudio = std::make_shared<AudioClipResource>("__placeholder_audio__");
    placeholderAudio->load();
    placeholders["audio"] = placeholderAudio;
}

void ResourceManager::shutdown()
{
    std::lock_guard<std::mutex> lock(mutex);

    std::cout << "ResourceManager shutting down. Unloading " << resources.size() << " resources..."
              << std::endl;

    // Stop async loader first
    stopAsyncLoader();

    for (auto& [path, resource] : resources) {
        if (resource && resource->isLoaded()) {
            resource->unload();
        }
    }

    resources.clear();
    placeholders.clear();
}

void ResourceManager::unload(const std::string& path)
{
    std::lock_guard<std::mutex> lock(mutex);

    auto it = resources.find(path);
    if (it != resources.end()) {
        if (it->second && it->second->isLoaded()) {
            it->second->unload();
        }
        resources.erase(it);
    }
}

void ResourceManager::unloadType(ResourceType type)
{
    std::lock_guard<std::mutex> lock(mutex);

    auto it = resources.begin();
    while (it != resources.end()) {
        if (it->second && it->second->getType() == type) {
            if (it->second->isLoaded()) {
                it->second->unload();
            }
            it = resources.erase(it);
        } else {
            ++it;
        }
    }
}

void ResourceManager::unloadUnused()
{
    std::lock_guard<std::mutex> lock(mutex);

    auto it = resources.begin();
    while (it != resources.end()) {
        if (it->second.use_count() <= 1) { // Only ResourceManager holds reference
            if (it->second->isLoaded()) {
                it->second->unload();
            }
            it = resources.erase(it);
        } else {
            ++it;
        }
    }
}

void ResourceManager::hotReload(const std::string& path)
{
    std::lock_guard<std::mutex> lock(mutex);

    auto it = resources.find(path);
    if (it != resources.end() && it->second) {
        std::cout << "Hot reloading resource: " << path << std::endl;
        it->second->unload();
        it->second->load();
    }
}

size_t ResourceManager::getTotalMemoryUsage() const
{
    std::lock_guard<std::mutex> lock(mutex);

    size_t total = 0;
    for (const auto& [path, resource] : resources) {
        if (resource && resource->isLoaded()) {
            total += resource->getMemoryUsage();
        }
    }
    return total;
}

size_t ResourceManager::getLoadedResourceCount() const
{
    std::lock_guard<std::mutex> lock(mutex);

    size_t count = 0;
    for (const auto& [path, resource] : resources) {
        if (resource && resource->isLoaded()) {
            count++;
        }
    }
    return count;
}

void ResourceManager::registerLoader(
    ResourceType type, std::function<std::shared_ptr<Resource>(const std::string&)> loader)
{
    std::lock_guard<std::mutex> lock(mutex);
    loaders[type] = loader;
}

void ResourceManager::scanDirectory(const std::string& directory, bool recursive)
{
    std::lock_guard<std::mutex> lock(mutex);

    std::cout << "Scanning directory: " << directory << std::endl;

    try {
        if (recursive) {
            for (const auto& entry : std::filesystem::recursive_directory_iterator(directory)) {
                if (entry.is_regular_file()) {
                    std::string path = entry.path().string();
                    ResourceType type = detectResourceType(path);
                    if (type != ResourceType::Unknown) {
                        // Register but don't load yet
                        std::cout << "  Found: " << path << std::endl;
                    }
                }
            }
        } else {
            for (const auto& entry : std::filesystem::directory_iterator(directory)) {
                if (entry.is_regular_file()) {
                    std::string path = entry.path().string();
                    ResourceType type = detectResourceType(path);
                    if (type != ResourceType::Unknown) {
                        std::cout << "  Found: " << path << std::endl;
                    }
                }
            }
        }
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Error scanning directory: " << e.what() << std::endl;
    }
}

std::vector<std::string> ResourceManager::getResourcesOfType(ResourceType type) const
{
    std::lock_guard<std::mutex> lock(mutex);

    std::vector<std::string> result;
    for (const auto& [path, resource] : resources) {
        if (resource && resource->getType() == type) {
            result.push_back(path);
        }
    }
    return result;
}

bool ResourceManager::exists(const std::string& path) const
{
    std::lock_guard<std::mutex> lock(mutex);
    return resources.find(path) != resources.end();
}

void ResourceManager::printStats() const
{
    std::lock_guard<std::mutex> lock(mutex);

    std::cout << "=== Resource Manager Statistics ===" << std::endl;
    std::cout << "Total resources: " << resources.size() << std::endl;
    std::cout << "Loaded resources: " << getLoadedResourceCount() << std::endl;
    std::cout << "Memory usage: " << (getTotalMemoryUsage() / 1024 / 1024) << " MB" << std::endl;

    // Count by type
    std::map<ResourceType, int> typeCounts;
    for (const auto& [path, resource] : resources) {
        if (resource) {
            typeCounts[resource->getType()]++;
        }
    }

    std::cout << "\nResources by type:" << std::endl;
    for (const auto& [type, count] : typeCounts) {
        std::cout << "  Type " << static_cast<int>(type) << ": " << count << std::endl;
    }
}

std::shared_ptr<Resource> ResourceManager::loadResource(const std::string& path, ResourceType type)
{
    auto it = loaders.find(type);
    if (it != loaders.end()) {
        auto resource = it->second(path);
        if (resource) {
            resource->load();
            return resource;
        }
    }
    return nullptr;
}

ResourceType ResourceManager::detectResourceType(const std::string& path) const
{
    std::string ext = std::filesystem::path(path).extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp" || ext == ".tga") {
        return ResourceType::Texture;
    } else if (ext == ".obj" || ext == ".fbx" || ext == ".gltf" || ext == ".glb") {
        return ResourceType::Mesh;
    } else if (ext == ".wav" || ext == ".ogg" || ext == ".mp3") {
        return ResourceType::Audio;
    } else if (ext == ".glsl" || ext == ".vert" || ext == ".frag") {
        return ResourceType::Shader;
    } else if (ext == ".asset" || ext == ".pack") {
        return ResourceType::AssetPack;
    }

    return ResourceType::Unknown;
}

// TextureResource implementation
TextureResource::TextureResource(const std::string& filePath)
{
    this->path = filePath;
    this->name = std::filesystem::path(filePath).stem().string();
    this->type = ResourceType::Texture;
}

void TextureResource::load()
{
    std::cout << "Loading texture: " << path << std::endl;
    
    // Check if this is a placeholder resource
    if (path.find("__placeholder") != std::string::npos) {
        // Create a 2x2 magenta placeholder texture (for visibility)
        width = 2;
        height = 2;
        channels = 4;
        data = new unsigned char[16]; // 2x2 RGBA
        usedStbiAlloc = false; // Allocated with new[]
        // Magenta color for visibility
        for (int i = 0; i < 4; ++i) {
            data[i * 4 + 0] = 255; // R
            data[i * 4 + 1] = 0;   // G
            data[i * 4 + 2] = 255; // B
            data[i * 4 + 3] = 255; // A
        }
        loaded = true;
        std::cout << "Placeholder texture created" << std::endl;
        return;
    }
    
    // Check if file exists
    if (!std::filesystem::exists(path)) {
        std::cerr << "ERROR: Texture file not found: " << path << std::endl;
        std::cerr << "Using placeholder texture instead" << std::endl;
        return;
    }
    
    // Load image using stb_image
    int imgWidth = 0;
    int imgHeight = 0;
    int imgChannels = 0;
    
    // Request 4 channels (RGBA) for consistency
    unsigned char* imageData = stbi_load(path.c_str(), &imgWidth, &imgHeight, &imgChannels, 4);
    
    if (!imageData) {
        std::cerr << "ERROR: Failed to load texture: " << path << std::endl;
        std::cerr << "  Reason: " << stbi_failure_reason() << std::endl;
        return;
    }
    
    // Store the loaded data
    width = imgWidth;
    height = imgHeight;
    channels = 4; // We requested RGBA
    data = imageData; // stbi_load allocates with malloc
    usedStbiAlloc = true; // Allocated with stb_image
    loaded = true;
    
    std::cout << "Texture loaded successfully: " << path 
              << " (" << width << "x" << height << ", " << imgChannels << " original channels)" << std::endl;
}

void TextureResource::unload()
{
    if (data) {
        // Use correct deallocation based on allocation type
        if (usedStbiAlloc) {
            stbi_image_free(data);
        } else {
            delete[] data;
        }
        data = nullptr;
    }
    width = 0;
    height = 0;
    channels = 0;
    usedStbiAlloc = false;
    loaded = false;
}

size_t TextureResource::getMemoryUsage() const
{
    return width * height * channels;
}

// MeshResource implementation
MeshResource::MeshResource(const std::string& filePath)
{
    this->path = filePath;
    this->name = std::filesystem::path(filePath).stem().string();
    this->type = ResourceType::Mesh;
}

void MeshResource::load()
{
    std::cout << "Loading mesh: " << path << std::endl;
    
    // Check if this is a placeholder resource
    if (path.find("__placeholder") != std::string::npos) {
        // Create a simple cube placeholder (8 vertices, 36 indices for 12 triangles)
        // Vertices: position (x, y, z) + normal (nx, ny, nz) + texcoord (u, v) = 8 floats per vertex
        vertices = {
            // Front face
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f,
             0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  1.0f, 0.0f,
             0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f,
            -0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  0.0f, 1.0f,
            // Back face
             0.5f, -0.5f, -0.5f,  0.0f, 0.0f, -1.0f,  0.0f, 0.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, -1.0f,  1.0f, 0.0f,
            -0.5f,  0.5f, -0.5f,  0.0f, 0.0f, -1.0f,  1.0f, 1.0f,
             0.5f,  0.5f, -0.5f,  0.0f, 0.0f, -1.0f,  0.0f, 1.0f,
        };
        indices = {
            0, 1, 2, 2, 3, 0,   // Front
            4, 5, 6, 6, 7, 4,   // Back
        };
        loaded = true;
        std::cout << "Placeholder mesh created" << std::endl;
        return;
    }
    
    // Check if file exists
    if (!std::filesystem::exists(path)) {
        std::cerr << "ERROR: Mesh file not found: " << path << std::endl;
        std::cerr << "Using placeholder mesh instead" << std::endl;
        return;
    }

#ifdef FRESH_TINYOBJLOADER_AVAILABLE
    // Load OBJ file using tinyobjloader
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str());

    if (!warn.empty()) {
        std::cout << "Warning loading mesh " << path << ": " << warn << std::endl;
    }

    if (!err.empty()) {
        std::cerr << "Error loading mesh " << path << ": " << err << std::endl;
    }

    if (!ret) {
        std::cerr << "ERROR: Failed to load mesh: " << path << std::endl;
        return;
    }

    // Process all shapes into a single vertex/index list
    // Each vertex has 8 floats: position (3) + normal (3) + texcoord (2)
    std::unordered_map<std::string, uint32_t> uniqueVertices;
    
    for (const auto& shape : shapes) {
        size_t indexOffset = 0;
        for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++) {
            int fv = shape.mesh.num_face_vertices[f];
            
            for (int v = 0; v < fv; v++) {
                tinyobj::index_t idx = shape.mesh.indices[indexOffset + v];
                
                // Position
                float px = attrib.vertices[3 * idx.vertex_index + 0];
                float py = attrib.vertices[3 * idx.vertex_index + 1];
                float pz = attrib.vertices[3 * idx.vertex_index + 2];
                
                // Normal
                float nx = 0.0f, ny = 1.0f, nz = 0.0f;
                if (idx.normal_index >= 0 && !attrib.normals.empty()) {
                    nx = attrib.normals[3 * idx.normal_index + 0];
                    ny = attrib.normals[3 * idx.normal_index + 1];
                    nz = attrib.normals[3 * idx.normal_index + 2];
                }
                
                // Texture coordinates
                float u = 0.0f, vt = 0.0f;
                if (idx.texcoord_index >= 0 && !attrib.texcoords.empty()) {
                    u = attrib.texcoords[2 * idx.texcoord_index + 0];
                    vt = attrib.texcoords[2 * idx.texcoord_index + 1];
                }
                
                // Create unique vertex key using original indices (more reliable than float comparison)
                // This uses a packed format: vertex_index|normal_index|texcoord_index
                size_t vertKey = static_cast<size_t>(idx.vertex_index);
                size_t normKey = idx.normal_index >= 0 ? static_cast<size_t>(idx.normal_index) : 0xFFFFFFFF;
                size_t texKey = idx.texcoord_index >= 0 ? static_cast<size_t>(idx.texcoord_index) : 0xFFFFFFFF;
                std::string key = std::to_string(vertKey) + "|" + 
                                  std::to_string(normKey) + "|" + 
                                  std::to_string(texKey);
                
                if (uniqueVertices.count(key) > 0) {
                    indices.push_back(uniqueVertices[key]);
                } else {
                    uint32_t index = static_cast<uint32_t>(vertices.size() / 8);
                    vertices.push_back(px);
                    vertices.push_back(py);
                    vertices.push_back(pz);
                    vertices.push_back(nx);
                    vertices.push_back(ny);
                    vertices.push_back(nz);
                    vertices.push_back(u);
                    vertices.push_back(vt);
                    indices.push_back(index);
                    uniqueVertices[key] = index;
                }
            }
            
            indexOffset += fv;
        }
    }
    
    loaded = true;
    std::cout << "Mesh loaded successfully: " << path 
              << " (" << (vertices.size() / 8) << " vertices, " 
              << indices.size() << " indices)" << std::endl;
#else
    std::cerr << "ERROR: tinyobjloader not available. Cannot load mesh: " << path << std::endl;
    std::cerr << "Mesh loading requires FRESH_TINYOBJLOADER_AVAILABLE to be defined." << std::endl;
#endif
}

void MeshResource::unload()
{
    vertices.clear();
    indices.clear();
    loaded = false;
}

size_t MeshResource::getMemoryUsage() const
{
    return vertices.size() * sizeof(float) + indices.size() * sizeof(uint32_t);
}

// AudioClipResource implementation
AudioClipResource::AudioClipResource(const std::string& filePath)
{
    this->path = filePath;
    this->name = std::filesystem::path(filePath).stem().string();
    this->type = ResourceType::Audio;
}

// Helper function to read WAV file header
static bool loadWAVFile(const std::string& filepath, std::vector<int16_t>& outSamples, 
                        int& outSampleRate, int& outChannels)
{
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    
    // Read RIFF header
    char riffHeader[4];
    file.read(riffHeader, 4);
    if (std::strncmp(riffHeader, "RIFF", 4) != 0) {
        std::cerr << "Invalid WAV file: missing RIFF header" << std::endl;
        return false;
    }
    
    uint32_t chunkSize;
    file.read(reinterpret_cast<char*>(&chunkSize), 4);
    
    char waveHeader[4];
    file.read(waveHeader, 4);
    if (std::strncmp(waveHeader, "WAVE", 4) != 0) {
        std::cerr << "Invalid WAV file: missing WAVE header" << std::endl;
        return false;
    }
    
    // Read fmt chunk
    char fmtHeader[4];
    file.read(fmtHeader, 4);
    if (std::strncmp(fmtHeader, "fmt ", 4) != 0) {
        std::cerr << "Invalid WAV file: missing fmt chunk" << std::endl;
        return false;
    }
    
    uint32_t fmtChunkSize;
    file.read(reinterpret_cast<char*>(&fmtChunkSize), 4);
    
    uint16_t audioFormat;
    file.read(reinterpret_cast<char*>(&audioFormat), 2);
    if (audioFormat != 1) { // PCM = 1
        std::cerr << "Unsupported WAV format: only PCM is supported" << std::endl;
        return false;
    }
    
    uint16_t numChannels;
    file.read(reinterpret_cast<char*>(&numChannels), 2);
    outChannels = numChannels;
    
    uint32_t wavSampleRate;
    file.read(reinterpret_cast<char*>(&wavSampleRate), 4);
    outSampleRate = static_cast<int>(wavSampleRate);
    
    uint32_t byteRate;
    file.read(reinterpret_cast<char*>(&byteRate), 4);
    
    uint16_t blockAlign;
    file.read(reinterpret_cast<char*>(&blockAlign), 2);
    
    uint16_t bitsPerSample;
    file.read(reinterpret_cast<char*>(&bitsPerSample), 2);
    
    if (bitsPerSample != 16) {
        std::cerr << "Unsupported WAV bits per sample: " << bitsPerSample << " (only 16-bit supported)" << std::endl;
        return false;
    }
    
    // Skip any extra fmt bytes
    if (fmtChunkSize > 16) {
        file.seekg(fmtChunkSize - 16, std::ios::cur);
    }
    
    // Find data chunk
    char dataHeader[4];
    uint32_t dataSize = 0;
    while (file.read(dataHeader, 4)) {
        file.read(reinterpret_cast<char*>(&dataSize), 4);
        if (std::strncmp(dataHeader, "data", 4) == 0) {
            break;
        }
        // Skip unknown chunk
        file.seekg(dataSize, std::ios::cur);
    }
    
    if (std::strncmp(dataHeader, "data", 4) != 0) {
        std::cerr << "Invalid WAV file: missing data chunk" << std::endl;
        return false;
    }
    
    // Read sample data
    size_t numSamples = dataSize / 2; // 16-bit samples = 2 bytes each
    outSamples.resize(numSamples);
    file.read(reinterpret_cast<char*>(outSamples.data()), dataSize);
    
    return true;
}

void AudioClipResource::load()
{
    std::cout << "Loading audio: " << path << std::endl;
    
    // Check if this is a placeholder resource
    if (path.find("__placeholder") != std::string::npos) {
        // Create a short silent audio sample (1 second at 44100 Hz, mono)
        sampleRate = 44100;
        channels = 1;
        samples.resize(44100, 0); // 1 second of silence
        loaded = true;
        std::cout << "Placeholder audio created" << std::endl;
        return;
    }
    
    // Check if file exists
    if (!std::filesystem::exists(path)) {
        std::cerr << "ERROR: Audio file not found: " << path << std::endl;
        std::cerr << "Using placeholder audio instead" << std::endl;
        return;
    }
    
    // Check file extension for supported formats
    std::string ext = std::filesystem::path(path).extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    
    if (ext == ".wav") {
        // Load WAV file
        if (loadWAVFile(path, samples, sampleRate, channels)) {
            loaded = true;
            std::cout << "Audio loaded successfully: " << path 
                      << " (" << sampleRate << " Hz, " << channels << " channels, "
                      << samples.size() << " samples)" << std::endl;
        } else {
            std::cerr << "ERROR: Failed to load WAV file: " << path << std::endl;
        }
    }
#ifdef FRESH_VORBIS_AVAILABLE
    else if (ext == ".ogg") {
        // Load OGG file using libvorbis
        OggVorbis_File vf;
        if (ov_fopen(path.c_str(), &vf) != 0) {
            std::cerr << "ERROR: Failed to open OGG file: " << path << std::endl;
            return;
        }
        
        vorbis_info* vi = ov_info(&vf, -1);
        sampleRate = static_cast<int>(vi->rate);
        channels = vi->channels;
        
        // Pre-allocate samples vector using total PCM length
        ogg_int64_t totalSamples = ov_pcm_total(&vf, -1);
        if (totalSamples > 0) {
            samples.reserve(static_cast<size_t>(totalSamples * channels));
        }
        
        // Read all samples
        const int bufferSize = 4096;
        char buffer[bufferSize];
        int bitstream;
        long bytesRead;
        
        while ((bytesRead = ov_read(&vf, buffer, bufferSize, 0, 2, 1, &bitstream)) > 0) {
            size_t samplesRead = static_cast<size_t>(bytesRead) / 2; // 16-bit samples
            size_t oldSize = samples.size();
            samples.resize(oldSize + samplesRead);
            std::memcpy(samples.data() + oldSize, buffer, static_cast<size_t>(bytesRead));
        }
        
        ov_clear(&vf);
        
        loaded = true;
        std::cout << "Audio loaded successfully: " << path 
                  << " (" << sampleRate << " Hz, " << channels << " channels, "
                  << samples.size() << " samples)" << std::endl;
    }
#endif
    else {
        std::cerr << "WARNING: Unsupported audio format: " << ext << std::endl;
        std::cerr << "Supported formats: .wav";
#ifdef FRESH_VORBIS_AVAILABLE
        std::cerr << ", .ogg";
#endif
        std::cerr << std::endl;
    }
}

void AudioClipResource::unload()
{
    samples.clear();
    loaded = false;
}

size_t AudioClipResource::getMemoryUsage() const
{
    return samples.size() * sizeof(int16_t);
}

// Async loading implementation
void ResourceManager::startAsyncLoader()
{
    asyncLoaderRunning = true;
    asyncLoaderThread = std::thread(&ResourceManager::asyncLoadingWorker, this);
}

void ResourceManager::stopAsyncLoader()
{
    if (asyncLoaderRunning) {
        asyncLoaderRunning = false;
        loaderCondition.notify_all();
        if (asyncLoaderThread.joinable()) {
            asyncLoaderThread.join();
        }
    }
}

void ResourceManager::asyncLoadingWorker()
{
    while (asyncLoaderRunning) {
        LoadingRequest request;
        
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            loaderCondition.wait(lock, [this] { 
                return !loadingQueue.empty() || !asyncLoaderRunning; 
            });

            if (!asyncLoaderRunning) {
                break;
            }

            if (!loadingQueue.empty()) {
                request = std::move(loadingQueue.front());
                loadingQueue.pop_front();
            } else {
                continue;
            }
        }

        try {
            // Load the resource
            std::shared_ptr<Resource> resource = loadResource(request.path, request.type);
            
            if (resource) {
                // Store in resource map
                {
                    std::lock_guard<std::mutex> lock(mutex);
                    resources[request.path] = resource;
                }
                request.promise.set_value(resource);
            } else {
                // Failed to load - use placeholder
                std::cerr << "Failed to load resource: " << request.path 
                         << " - using placeholder" << std::endl;
                
                // Get appropriate placeholder
                std::shared_ptr<Resource> placeholder;
                {
                    std::lock_guard<std::mutex> lock(mutex);
                    switch (request.type) {
                        case ResourceType::Texture:
                            placeholder = placeholders["texture"];
                            break;
                        case ResourceType::Mesh:
                            placeholder = placeholders["mesh"];
                            break;
                        case ResourceType::Audio:
                            placeholder = placeholders["audio"];
                            break;
                        default:
                            break;
                    }
                }
                request.promise.set_value(placeholder);
            }
        } catch (const std::exception& e) {
            std::cerr << "Exception loading resource: " << request.path 
                     << " - " << e.what() << std::endl;
            request.promise.set_exception(std::current_exception());
        }

        completedLoadRequests++;
    }
}

bool ResourceManager::isLoading(const std::string& path) const
{
    std::lock_guard<std::mutex> lock(queueMutex);
    
    // Check if path is in loading queue
    for (const auto& request : loadingQueue) {
        if (request.path == path) {
            return true;
        }
    }
    return false;
}

float ResourceManager::getLoadingProgress() const
{
    size_t total = totalLoadRequests.load();
    if (total == 0) {
        return 1.0f;
    }
    return static_cast<float>(completedLoadRequests.load()) / static_cast<float>(total);
}

} // namespace fresh
