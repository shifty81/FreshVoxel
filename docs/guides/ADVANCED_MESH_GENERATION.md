# Advanced Mesh Generation: Marching Cubes Implementation Guide

## Overview

This document describes potential enhancements to the Fresh Voxel Engine's mesh generation system, inspired by Enshrouded's smooth, detailed graphics achieved through isosurface algorithms.

## Current System

The engine currently uses:
- **Simple cubic meshing** with face culling
- **Greedy meshing** optimization (partially implemented)
- Face-by-face generation with neighbor checks

**Advantages**:
- Fast and simple
- Good for Minecraft-style blocky aesthetics
- Easy to understand and modify
- Efficient for cubic voxels

**Limitations**:
- Strictly cubic appearance
- No smooth terrain transitions
- Limited organic shapes
- Hard edges only

## Marching Cubes Algorithm

Marching cubes is an isosurface algorithm that generates smooth meshes from voxel data, as used by games like Enshrouded.

### How It Works

1. **Density Field**: Each voxel has a density value (0 = air, 1 = solid)
2. **Cube Processing**: For each cube of 8 voxels, determine which vertices are inside/outside
3. **Lookup Table**: Use a pre-computed table to determine triangle configuration
4. **Interpolation**: Calculate exact vertex positions based on density values
5. **Normal Generation**: Compute smooth normals for lighting

### Key Concepts

```
Cube Configuration (256 possible states):
- 8 corners = 8 bits = 256 combinations
- Each configuration maps to a set of triangles
- Lookup table provides edge intersections
```

### Algorithm Pseudocode

```cpp
for each cube in grid:
    // 1. Sample density at 8 corners
    uint8_t cubeIndex = 0;
    for i in 0..7:
        if density[corner[i]] > isoLevel:
            cubeIndex |= (1 << i);
    
    // 2. Check if cube is on surface
    if cubeIndex == 0 or cubeIndex == 255:
        continue;
    
    // 3. Lookup triangles for this configuration
    triangles = marchingCubesTable[cubeIndex];
    
    // 4. Interpolate vertex positions
    for each triangle in triangles:
        for each edge in triangle:
            vertex = interpolate(corner1, corner2, density1, density2);
            normal = computeGradient(vertex);
            vertices.push(vertex, normal);
```

## Implementation Plan

### Phase 1: Density Field

Add density values to voxel data:

```cpp
struct Voxel {
    VoxelType type;
    uint8_t light;
    float density;  // NEW: 0.0 = air, 1.0 = solid
    
    float getDensity() const {
        return isSolid() ? 1.0f : 0.0f;
    }
};
```

### Phase 2: Marching Cubes Tables

Pre-compute lookup tables:

```cpp
// 256 configurations, each with up to 15 vertices
constexpr int MARCHING_CUBES_EDGE_TABLE[256] = { /* ... */ };
constexpr int MARCHING_CUBES_TRI_TABLE[256][16] = { /* ... */ };

// Edge intersection points
constexpr glm::vec3 EDGE_VERTICES[12] = {
    {0.5f, 0.0f, 0.0f}, // Edge 0
    {1.0f, 0.5f, 0.0f}, // Edge 1
    // ... etc
};
```

### Phase 3: Mesh Generator Extension

Add new method to MeshGenerator:

```cpp
class MeshGenerator {
public:
    /**
     * @brief Generate smooth mesh using marching cubes
     * @param chunk Chunk to generate mesh for
     * @param vertices Output vertex data
     * @param indices Output index data
     * @param isoLevel Density threshold (default 0.5)
     */
    void generateMarchingCubesMesh(const Chunk* chunk,
                                   std::vector<float>& vertices,
                                   std::vector<uint32_t>& indices,
                                   float isoLevel = 0.5f);

private:
    glm::vec3 interpolateVertex(const glm::vec3& p1, const glm::vec3& p2,
                                float v1, float v2, float isoLevel);
    
    glm::vec3 computeNormal(const Chunk* chunk, const glm::vec3& pos);
};
```

### Phase 4: Hybrid Approach

Combine cubic and smooth meshes:

```cpp
enum class MeshStyle {
    Cubic,          // Current system (blocks)
    Smooth,         // Marching cubes (terrain)
    Hybrid          // Both (blocks for man-made, smooth for terrain)
};

void MeshGenerator::generateChunkMesh(const Chunk* chunk,
                                     std::vector<float>& vertices,
                                     std::vector<uint32_t>& indices,
                                     MeshStyle style) {
    switch (style) {
    case MeshStyle::Cubic:
        generateSimpleMesh(chunk, vertices, indices);
        break;
    case MeshStyle::Smooth:
        generateMarchingCubesMesh(chunk, vertices, indices);
        break;
    case MeshStyle::Hybrid:
        generateHybridMesh(chunk, vertices, indices);
        break;
    }
}
```

## Dual Contouring Alternative

An alternative to marching cubes is **Manifold Dual Contouring** (MDC):

### Advantages Over Marching Cubes

- **Sharp features preserved**: Can maintain edges and corners
- **Better quality**: Fewer triangles for similar detail
- **Hermite data**: Uses position + normal for better reconstruction

### Disadvantages

- **More complex**: Harder to implement correctly
- **Slower**: Requires QEF (Quadratic Error Function) solving
- **More data**: Needs normal information

### When to Use

- **Marching Cubes**: For smooth, organic terrain (hills, caves)
- **Dual Contouring**: For mixed content (terrain + buildings)
- **Cubic Meshing**: For Minecraft-style blocks and structures

## Integration with Construction Tools

### Smooth Construction Hammer Mode

Add option for smooth construction:

```cpp
class ConstructionHammer : public VoxelTool {
public:
    enum class PlacementMode {
        Cubic,      // Normal blocky placement
        Smooth      // Smooth marching cubes placement
    };
    
    void setPlacementMode(PlacementMode mode) {
        m_placementMode = mode;
    }
    
private:
    void placeSmoothBlock(const WorldPos& pos, VoxelType type) {
        // Set density field instead of binary voxel
        for (int dx = -1; dx <= 1; ++dx) {
            for (int dy = -1; dy <= 1; ++dy) {
                for (int dz = -1; dz <= 1; ++dz) {
                    float dist = sqrt(dx*dx + dy*dy + dz*dz);
                    float density = 1.0f - (dist / 1.73f); // Smooth falloff
                    setVoxelDensity(WorldPos(pos.x+dx, pos.y+dy, pos.z+dz), 
                                   density);
                }
            }
        }
    }
};
```

### Material Blending

Enable smooth transitions between materials:

```cpp
struct Voxel {
    VoxelType primaryType;
    VoxelType secondaryType;
    float blendFactor;  // 0.0 = all primary, 1.0 = all secondary
    
    glm::vec3 getColor() const {
        glm::vec3 color1 = getColorForType(primaryType);
        glm::vec3 color2 = getColorForType(secondaryType);
        return glm::mix(color1, color2, blendFactor);
    }
};
```

## Performance Considerations

### Marching Cubes Performance

**Pros**:
- Can generate fewer triangles than cubic meshing
- Smooth meshes compress better
- Better for organic shapes

**Cons**:
- More complex algorithm (slower generation)
- Requires table lookups and interpolation
- Normal calculation is expensive

### Optimization Strategies

1. **Chunk-level LOD**: Use marching cubes only for nearby chunks
2. **Async generation**: Generate meshes on background threads
3. **Caching**: Cache generated meshes until voxels change
4. **Octree culling**: Skip empty regions quickly
5. **SIMD**: Vectorize density sampling and interpolation

### Memory Usage

```cpp
// Cubic mesh (typical)
- Vertices: ~6 faces × 4 vertices × 32 bytes = 768 bytes/block
- Simple and predictable

// Marching cubes mesh
- Vertices: Variable (1-15 triangles per cube)
- More memory per voxel
- Better compression potential
```

## Example Implementation

### Basic Marching Cubes

```cpp
void MeshGenerator::generateMarchingCubesMesh(
    const Chunk* chunk,
    std::vector<float>& vertices,
    std::vector<uint32_t>& indices,
    float isoLevel)
{
    vertices.clear();
    indices.clear();
    
    // Process each cube in chunk
    for (int y = 0; y < CHUNK_HEIGHT - 1; ++y) {
        for (int z = 0; z < CHUNK_SIZE - 1; ++z) {
            for (int x = 0; x < CHUNK_SIZE - 1; ++x) {
                processCube(chunk, x, y, z, isoLevel, vertices, indices);
            }
        }
    }
}

void MeshGenerator::processCube(
    const Chunk* chunk,
    int x, int y, int z,
    float isoLevel,
    std::vector<float>& vertices,
    std::vector<uint32_t>& indices)
{
    // Sample 8 corners
    float density[8];
    density[0] = chunk->getVoxel(x,   y,   z  ).getDensity();
    density[1] = chunk->getVoxel(x+1, y,   z  ).getDensity();
    density[2] = chunk->getVoxel(x+1, y,   z+1).getDensity();
    density[3] = chunk->getVoxel(x,   y,   z+1).getDensity();
    density[4] = chunk->getVoxel(x,   y+1, z  ).getDensity();
    density[5] = chunk->getVoxel(x+1, y+1, z  ).getDensity();
    density[6] = chunk->getVoxel(x+1, y+1, z+1).getDensity();
    density[7] = chunk->getVoxel(x,   y+1, z+1).getDensity();
    
    // Calculate cube index
    uint8_t cubeIndex = 0;
    for (int i = 0; i < 8; ++i) {
        if (density[i] > isoLevel) {
            cubeIndex |= (1 << i);
        }
    }
    
    // Skip if entirely inside or outside
    if (cubeIndex == 0 || cubeIndex == 255) {
        return;
    }
    
    // Get triangles for this configuration
    const int* triangles = MARCHING_CUBES_TRI_TABLE[cubeIndex];
    
    // Generate vertices and triangles
    for (int i = 0; triangles[i] != -1; i += 3) {
        for (int j = 0; j < 3; ++j) {
            int edgeIndex = triangles[i + j];
            glm::vec3 vertex = interpolateEdge(x, y, z, edgeIndex,
                                              density, isoLevel);
            glm::vec3 normal = computeNormal(chunk, vertex);
            
            // Add vertex data (position + normal + texcoords)
            vertices.push_back(vertex.x);
            vertices.push_back(vertex.y);
            vertices.push_back(vertex.z);
            vertices.push_back(normal.x);
            vertices.push_back(normal.y);
            vertices.push_back(normal.z);
            // ... texture coordinates
            
            indices.push_back(static_cast<uint32_t>(vertices.size() / 8 - 1));
        }
    }
}
```

## Configuration Options

Add mesh generation options:

```cpp
struct MeshGenerationOptions {
    MeshStyle style = MeshStyle::Cubic;
    float isoLevel = 0.5f;
    bool generateNormals = true;
    bool smoothNormals = true;
    int lodLevel = 0;  // 0 = full detail
    bool enableBlending = false;
};

class VoxelWorld {
public:
    void setMeshGenerationOptions(const MeshGenerationOptions& options) {
        m_meshOptions = options;
        regenerateAllChunks();
    }
};
```

## Testing Smooth Meshes

Add tests for marching cubes:

```cpp
TEST_F(MeshGeneratorTest, MarchingCubes_GeneratesSmoothSphere) {
    // Create a spherical density field
    Chunk chunk;
    for (int x = 0; x < CHUNK_SIZE; ++x) {
        for (int y = 0; y < CHUNK_HEIGHT; ++y) {
            for (int z = 0; z < CHUNK_SIZE; ++z) {
                float dx = x - CHUNK_SIZE/2;
                float dy = y - CHUNK_HEIGHT/2;
                float dz = z - CHUNK_SIZE/2;
                float dist = sqrt(dx*dx + dy*dy + dz*dz);
                float density = (dist < 5.0f) ? 1.0f : 0.0f;
                chunk.setVoxelDensity(x, y, z, density);
            }
        }
    }
    
    // Generate mesh
    MeshGenerator generator;
    std::vector<float> vertices;
    std::vector<uint32_t> indices;
    generator.generateMarchingCubesMesh(&chunk, vertices, indices);
    
    // Verify smooth mesh was generated
    EXPECT_GT(vertices.size(), 0);
    EXPECT_GT(indices.size(), 0);
    
    // Check for smooth normals (no sharp 90° angles)
    // ... additional validation
}
```

## Resources

### Reference Implementations

- Paul Bourke's Marching Cubes: http://paulbourke.net/geometry/polygonise/
- 0fps Blog (Dual Contouring): https://0fps.net/2012/07/12/smooth-voxel-terrain-part-2/
- Transvoxel Algorithm: http://transvoxel.org/

### Lookup Tables

Marching cubes tables are available in the public domain:
- Edge table (256 entries)
- Triangle table (256 × 16 entries)

### Papers

- Lorensen & Cline (1987): "Marching Cubes: A High Resolution 3D Surface Construction Algorithm"
- Ju et al. (2002): "Dual Contouring of Hermite Data"
- Schaefer & Warren (2004): "Dual Marching Cubes"

## Conclusion

Adding marching cubes or dual contouring would enable:
- Smooth, organic terrain like Enshrouded
- Better visual quality
- Natural cave systems
- Smooth construction options
- Material blending

The current cubic system should be maintained for:
- Minecraft-style building
- Performance-critical scenarios
- Simple, predictable results

A hybrid approach offers the best of both worlds.

## See Also

- [Voxel Tools Guide](VOXEL_TOOLS.md)
- [Mesh Generator API](../../engine/voxel/MeshGenerator.h)
- [Performance Optimization Guide](../architecture/PERFORMANCE.md)
