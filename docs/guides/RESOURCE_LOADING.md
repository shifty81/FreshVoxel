# Resource Loading System

This guide covers the enhanced resource loading system in the Fresh Voxel Engine, including texture loading with stb_image, mesh loading with tinyobjloader, and audio loading with WAV/OGG support.

## Overview

The ResourceManager provides a centralized system for loading, caching, and managing game resources. It supports:

- **Textures**: PNG, JPG, TGA, BMP images via stb_image
- **Meshes**: OBJ models via tinyobjloader  
- **Audio**: WAV (native), OGG (via libvorbis)
- **Async Loading**: Background thread pool for non-blocking resource loading
- **Memory Management**: Automatic unloading of unused resources
- **Hot Reloading**: Development feature for instant resource updates

## Quick Start

### Loading Resources

```cpp
#include "core/ResourceManager.h"

// Initialize the resource manager
auto& resources = fresh::ResourceManager::getInstance();
resources.initialize("assets/");

// Load a texture synchronously
auto textureHandle = resources.load<fresh::TextureResource>("textures/player.png");
if (textureHandle.isValid()) {
    int width = textureHandle->getWidth();
    int height = textureHandle->getHeight();
    const unsigned char* pixels = textureHandle->getData();
}

// Load a mesh
auto meshHandle = resources.load<fresh::MeshResource>("models/character.obj");
if (meshHandle.isValid()) {
    const auto& vertices = meshHandle->getVertices();
    const auto& indices = meshHandle->getIndices();
}

// Load audio
auto audioHandle = resources.load<fresh::AudioClipResource>("sounds/jump.wav");
if (audioHandle.isValid()) {
    int sampleRate = audioHandle->getSampleRate();
    int channels = audioHandle->getChannels();
    const auto& samples = audioHandle->getSamples();
}
```

### Async Loading

```cpp
// Load resources asynchronously (doesn't block main thread)
auto futureTexture = resources.loadAsync<fresh::TextureResource>("textures/world.png");

// Do other work...

// Wait for resource when needed
auto textureHandle = futureTexture.get();
```

## Supported Formats

### Textures

The engine uses **stb_image** for texture loading, supporting:

| Format | Extension | Description |
|--------|-----------|-------------|
| PNG | `.png` | Lossless compression, supports transparency |
| JPEG | `.jpg`, `.jpeg` | Lossy compression, no transparency |
| TGA | `.tga` | Truevision format, supports transparency |
| BMP | `.bmp` | Uncompressed bitmap format |

**Notes:**
- All textures are converted to RGBA8 format (4 channels) for consistency
- Mipmaps can be generated automatically
- Maximum texture size depends on GPU capabilities

### Meshes

The engine uses **tinyobjloader** for mesh loading:

| Format | Extension | Description |
|--------|-----------|-------------|
| OBJ | `.obj` | Wavefront OBJ format |

**Vertex Data Layout:**
Each vertex contains 8 floats:
- Position: x, y, z (3 floats)
- Normal: nx, ny, nz (3 floats)
- Texture Coordinates: u, v (2 floats)

**Notes:**
- Automatic vertex deduplication for optimization
- Normals are preserved from the file or set to default (0, 1, 0)
- Texture coordinates are preserved from the file

### Audio

| Format | Extension | Library | Description |
|--------|-----------|---------|-------------|
| WAV | `.wav` | Native | Uncompressed PCM audio (16-bit only) |
| OGG | `.ogg` | libvorbis | Compressed audio format |

**Notes:**
- WAV files must be 16-bit PCM format
- OGG support requires `FRESH_VORBIS_AVAILABLE` compile flag
- Audio data is stored as 16-bit signed integers

## Resource Types

### TextureResource

```cpp
class TextureResource : public Resource
{
public:
    int getWidth() const;      // Texture width in pixels
    int getHeight() const;     // Texture height in pixels
    int getChannels() const;   // Number of color channels (always 4 for RGBA)
    const unsigned char* getData() const;  // Raw pixel data
};
```

### MeshResource

```cpp
class MeshResource : public Resource
{
public:
    const std::vector<float>& getVertices() const;    // Interleaved vertex data
    const std::vector<uint32_t>& getIndices() const;  // Triangle indices
};
```

### AudioClipResource

```cpp
class AudioClipResource : public Resource
{
public:
    const std::vector<int16_t>& getSamples() const;  // Audio sample data
    int getSampleRate() const;   // Samples per second (e.g., 44100)
    int getChannels() const;     // Number of audio channels (1=mono, 2=stereo)
};
```

## Memory Management

### Automatic Cleanup

```cpp
// Unload a specific resource
resources.unload("textures/old_texture.png");

// Unload all resources of a type
resources.unloadType(fresh::ResourceType::Texture);

// Unload unused resources (reference count = 0)
resources.unloadUnused();
```

### Memory Statistics

```cpp
// Get total memory used by loaded resources
size_t memoryBytes = resources.getTotalMemoryUsage();

// Get number of loaded resources
size_t count = resources.getLoadedResourceCount();

// Print detailed statistics
resources.printStats();
```

## Hot Reloading

For development, resources can be reloaded without restarting the engine:

```cpp
// Reload a specific resource from disk
resources.hotReload("textures/player.png");
```

## Placeholder Resources

When a resource fails to load, the system provides placeholder resources:

- **Texture Placeholder**: 2x2 magenta texture (for visibility)
- **Mesh Placeholder**: Simple cube geometry
- **Audio Placeholder**: 1 second of silence

## CMake Configuration

The resource system requires these dependencies in your `vcpkg.json`:

```json
{
  "dependencies": [
    "tinyobjloader",
    "libvorbis"
  ]
}
```

CMake will automatically detect these and define:
- `FRESH_TINYOBJLOADER_AVAILABLE` - Enables OBJ mesh loading
- `FRESH_VORBIS_AVAILABLE` - Enables OGG audio loading

## Best Practices

### 1. Use Async Loading for Large Resources

```cpp
// Load level resources in background
std::vector<std::future<ResourceHandle<TextureResource>>> futures;
for (const auto& path : texturePaths) {
    futures.push_back(resources.loadAsync<TextureResource>(path));
}

// Show loading progress
while (!allReady(futures)) {
    float progress = resources.getLoadingProgress();
    showLoadingScreen(progress);
}
```

### 2. Check Resource Validity

```cpp
auto handle = resources.load<TextureResource>("textures/missing.png");
if (!handle.isValid()) {
    // Handle missing resource
    Logger::getInstance().warning("Resource not found, using fallback");
}
```

### 3. Preload Critical Resources

```cpp
void preloadGameResources() {
    // Player textures - needed immediately
    resources.load<TextureResource>("textures/player.png");
    resources.load<MeshResource>("models/player.obj");
    
    // UI elements
    resources.load<TextureResource>("textures/ui/buttons.png");
    resources.load<AudioClipResource>("sounds/ui/click.wav");
}
```

### 4. Release Resources When Changing Scenes

```cpp
void onSceneChange() {
    // Unload scene-specific resources
    resources.unloadUnused();
}
```

## Troubleshooting

### Resource Not Loading

1. Check the file path is correct (relative to asset directory)
2. Verify the file exists on disk
3. Check console for error messages
4. Ensure the file format is supported

### Out of Memory

1. Call `unloadUnused()` periodically
2. Use lower resolution textures
3. Compress audio with OGG instead of WAV
4. Check `getTotalMemoryUsage()` to monitor memory

### OGG Files Not Loading

Ensure libvorbis is installed and `FRESH_VORBIS_AVAILABLE` is defined:
```bash
vcpkg install libvorbis
```

### OBJ Files Not Loading

Ensure tinyobjloader is installed and `FRESH_TINYOBJLOADER_AVAILABLE` is defined:
```bash
vcpkg install tinyobjloader
```

## API Reference

See the header file `engine/core/ResourceManager.h` for complete API documentation.
