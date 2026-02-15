# Heightmap World Generation Guide

## Overview

The Heightmap World Generator creates voxel terrain from heightmap images. This allows you to design terrain in external tools (Photoshop, GIMP, WorldMachine, Gaea) and import it into the Fresh Voxel Engine.

## Supported Formats

The generator uses **stb_image** for image loading, supporting:
- **PNG** (recommended)
- **JPG/JPEG**
- **BMP**
- **TGA**

Images can be:
- **Grayscale**: Each pixel's brightness represents height
- **RGB**: Red channel is used for height (grayscale preferred)

## Quick Start

### Basic Usage

```cpp
#include "generation/HeightmapWorldGenerator.h"

// Create generator
HeightmapWorldGenerator generator;

// Configure parameters
HeightmapGeneratorParams params;
params.heightmapPath = "terrain/mountains.png";
params.minHeight = 0.0f;
params.maxHeight = 128.0f;
params.setupDefaultLayers();

// Initialize
if (generator.initialize(params)) {
    // Generator is ready to use
    VoxelWorld* world = getWorld();
    
    // Generate chunks
    for (ChunkPos pos : chunksToGenerate) {
        Chunk* chunk = world->getChunk(pos);
        generator.generateChunk(chunk, pos);
    }
}
```

## Parameters

### Height Range

Control the vertical range of your terrain:

```cpp
params.minHeight = 0.0f;      // Sea level / minimum terrain height
params.maxHeight = 256.0f;    // Maximum mountain height
```

- Heightmap values (0-255) are mapped linearly to this range
- Black pixels (0) → `minHeight`
- White pixels (255) → `maxHeight`

### Vertical Scaling

Exaggerate or flatten terrain:

```cpp
params.verticalScale = 1.5f;  // Exaggerate height by 50%
params.verticalScale = 0.5f;  // Flatten terrain by 50%
```

### Horizontal Scaling

Control how heightmap pixels map to voxels:

```cpp
params.horizontalScale = 1.0f;  // 1 pixel = 1 voxel (default)
params.horizontalScale = 2.0f;  // 1 pixel = 2 voxels (terrain is 2x larger)
params.horizontalScale = 0.5f;  // 2 pixels = 1 voxel (terrain is 2x smaller)
```

### Tiling/Wrapping

Create infinite terrain from a finite heightmap:

```cpp
params.repeatX = true;   // Wrap horizontally
params.repeatZ = true;   // Wrap vertically
```

Useful for:
- Seamless tileable terrains
- Planets (wrap on all axes)
- Endless worlds from small heightmaps

## Block Type Layers

Define which blocks appear at different heights.

### Default Layers

```cpp
params.setupDefaultLayers();
```

Creates layers:
- **0.0 - 0.3**: Sand (underwater/beaches)
- **0.3 - 0.4**: Dirt (transitional)
- **0.4 - 0.7**: Grass (plains/hills)
- **0.7 - 0.9**: Stone (mountains)
- **0.9 - 1.0**: Stone (peaks)

### Custom Layers

```cpp
params.layers.clear();

// Water layer
params.layers.push_back({0.0f, 0.3f, VoxelType::Water});

// Beach
params.layers.push_back({0.3f, 0.35f, VoxelType::Sand});

// Grassland
params.layers.push_back({0.35f, 0.6f, VoxelType::Grass});

// Forest
params.layers.push_back({0.6f, 0.75f, VoxelType::Grass}); // With trees

// Mountains
params.layers.push_back({0.75f, 0.9f, VoxelType::Stone});

// Snow peaks
params.layers.push_back({0.9f, 1.0f, VoxelType::Stone}); // Add snow block type
```

## Advanced Features

### Bilinear Interpolation

The generator uses bilinear interpolation for smooth terrain between pixels:

```cpp
// No configuration needed - automatic
// Eliminates blocky artifacts
// Smooth height transitions
```

### Query Terrain Height

Get terrain height at any world coordinate:

```cpp
float height = generator.getTerrainHeight(worldX, worldZ);

// Useful for:
// - Placing objects on terrain
// - Character spawn positions
// - Path finding
```

### Runtime Parameter Changes

Update parameters after initialization:

```cpp
HeightmapGeneratorParams newParams = generator.getParams();
newParams.verticalScale = 2.0f;
generator.setParams(newParams);

// Note: Changing heightmapPath requires reloading the image
```

## Creating Heightmaps

### In Image Editors

**Photoshop/GIMP:**
1. Create new grayscale image (recommended: 512x512, 1024x1024, 2048x2048)
2. Use brushes, filters, noise for terrain shapes
3. **Darker** = lower terrain
4. **Lighter** = higher terrain
5. Use **Gaussian Blur** for smooth transitions
6. Export as **PNG** (grayscale)

**Tips:**
- Use **Perlin Noise** filters for natural terrain
- **Clouds** filter creates realistic heightmaps
- **Levels/Curves** adjust height distribution
- Keep edges seamless for tiling (use **Offset** filter)

### In Terrain Generators

**WorldMachine, Gaea, Terragen:**
1. Design terrain with nodes/tools
2. Export **Height Map** (grayscale)
3. Resolution: 1024x1024 or higher for detail
4. Format: 16-bit PNG (engine converts to 8-bit automatically)
5. Normalize to full 0-255 range

### Procedural Heightmaps

Generate programmatically:

```cpp
// Example: Create a heightmap with Perlin noise
#include <cmath>

HeightmapData createPerlinHeightmap(int width, int height) {
    HeightmapData data;
    data.width = width;
    data.height = height;
    data.heights.resize(width * height);
    
    for (int z = 0; z < height; ++z) {
        for (int x = 0; x < width; ++x) {
            float fx = static_cast<float>(x) / width;
            float fz = static_cast<float>(z) / height;
            
            // Multi-octave Perlin noise
            float height = 0.0f;
            height += perlinNoise(fx * 4, fz * 4) * 0.5f;
            height += perlinNoise(fx * 8, fz * 8) * 0.25f;
            height += perlinNoise(fx * 16, fz * 16) * 0.125f;
            
            data.heights[z * width + x] = (height + 1.0f) * 0.5f; // Normalize to 0-1
        }
    }
    
    return data;
}
```

## Examples

### Island Terrain

```cpp
HeightmapGeneratorParams params;
params.heightmapPath = "terrain/island.png";
params.minHeight = -20.0f;  // Underwater terrain
params.maxHeight = 100.0f;
params.horizontalScale = 1.0f;

params.layers.clear();
params.layers.push_back({0.0f, 0.4f, VoxelType::Sand});    // Ocean floor
params.layers.push_back({0.4f, 0.5f, VoxelType::Sand});    // Beach
params.layers.push_back({0.5f, 0.8f, VoxelType::Grass});   // Island
params.layers.push_back({0.8f, 1.0f, VoxelType::Stone});   // Mountains
```

### Tiled Terrain

```cpp
HeightmapGeneratorParams params;
params.heightmapPath = "terrain/seamless_tile.png";
params.minHeight = 0.0f;
params.maxHeight = 64.0f;
params.repeatX = true;  // Infinite in X
params.repeatZ = true;  // Infinite in Z
params.horizontalScale = 2.0f;  // Larger scale
```

### Mars-like Terrain

```cpp
HeightmapGeneratorParams params;
params.heightmapPath = "terrain/mars.png";
params.minHeight = 0.0f;
params.maxHeight = 200.0f;
params.verticalScale = 1.8f;  // Exaggerated terrain

params.layers.clear();
params.layers.push_back({0.0f, 1.0f, VoxelType::Sand});  // Red sand everywhere
```

## Performance Tips

1. **Heightmap Resolution**: Use power-of-2 sizes (512, 1024, 2048)
2. **Horizontal Scale**: Larger scale = fewer chunks to generate
3. **Caching**: Generator caches heightmap in memory
4. **Async Loading**: Load heightmap on background thread
5. **Chunk Prioritization**: Generate visible chunks first

## Troubleshooting

### Heightmap not loading

```cpp
if (!generator.initialize(params)) {
    std::cerr << "Failed to load heightmap" << std::endl;
    // Check file path, format, file permissions
}
```

### Terrain too flat/steep

Adjust `verticalScale`:
```cpp
params.verticalScale = 2.0f;  // If too flat
params.verticalScale = 0.5f;  // If too steep
```

### Wrong block types

Check layer definitions:
```cpp
// Ensure layers cover full range 0.0 - 1.0
// No gaps between layers
// MinHeight < MaxHeight for each layer
```

### Blocky terrain

- Use **bilinear interpolation** (automatic)
- Increase heightmap resolution
- Use **Gaussian Blur** on source image

## Integration with Other Systems

### Water Generation

```cpp
// Set sea level
const float SEA_LEVEL = 32.0f;

// Generate water after terrain
for (int y = 0; y <= SEA_LEVEL; ++y) {
    if (chunk->getVoxel(x, y, z)->type == VoxelType::Air) {
        chunk->setVoxel(x, y, z, Voxel(VoxelType::Water));
    }
}
```

### Vegetation Placement

```cpp
float terrainHeight = generator.getTerrainHeight(worldX, worldZ);

// Place tree at surface
if (terrainHeight > SEA_LEVEL && terrainHeight < 80.0f) {
    placeTree(worldX, terrainHeight + 1, worldZ);
}
```

### Structure Placement

```cpp
// Find flat areas for villages
bool isFlat = true;
for (int dx = -2; dx <= 2; ++dx) {
    for (int dz = -2; dz <= 2; ++dz) {
        float h = generator.getTerrainHeight(worldX + dx, worldZ + dz);
        if (abs(h - terrainHeight) > 2.0f) {
            isFlat = false;
            break;
        }
    }
}

if (isFlat) {
    placeVillage(worldX, terrainHeight, worldZ);
}
```

## See Also

- `engine/generation/HeightmapWorldGenerator.h` - API reference
- `tests/generation/HeightmapWorldGeneratorTests.cpp` - Unit tests
- `docs/guides/TERRAIN_GENERATION.md` - General terrain guide
- **stb_image** documentation for supported formats
