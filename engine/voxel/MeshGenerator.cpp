#include "voxel/MeshGenerator.h"

#include <iostream>

#include "voxel/Chunk.h"

namespace fresh
{

MeshGenerator::MeshGenerator() {}

MeshGenerator::~MeshGenerator() {}

void MeshGenerator::generateChunkMesh(const Chunk* chunk, std::vector<float>& vertices,
                                      std::vector<uint32_t>& indices)
{
    // Entry point for chunk mesh generation
    // Future: Implement greedy meshing optimization for reduced triangle count
    // Current: Uses simple face-by-face meshing with basic culling
    generateSimpleMesh(chunk, vertices, indices);
}

void MeshGenerator::generateSimpleMesh(const Chunk* chunk, std::vector<float>& vertices,
                                       std::vector<uint32_t>& indices)
{
    // Simple mesh generation with face culling
    // Iterates through all voxels and generates faces for solid blocks
    // Only adds faces that are adjacent to transparent/air blocks (culling optimization)
    vertices.clear();
    indices.clear();

    // Reserve capacity to reduce allocations (estimate ~6 faces per solid block on average)
    // Each face has 4 vertices * 8 floats per vertex = 32 floats
    constexpr size_t estimatedSolidBlocks = CHUNK_VOLUME / 4;
    constexpr size_t estimatedFaces = estimatedSolidBlocks * 3; // ~half blocks are visible
    vertices.reserve(estimatedFaces * 32);
    indices.reserve(estimatedFaces * 6);

    uint32_t vertexCount = 0;

    // Iterate in Y-Z-X order for better cache locality during rendering
    for (int y = 0; y < CHUNK_HEIGHT; ++y) {
        for (int z = 0; z < CHUNK_SIZE; ++z) {
            for (int x = 0; x < CHUNK_SIZE; ++x) {
                const Voxel& voxel = chunk->getVoxel(x, y, z);

                if (!voxel.isSolid()) {
                    continue;
                }

                // Check each face and add if neighbor is air
                // +X face
                if (x == CHUNK_SIZE - 1 || !chunk->getVoxel(x + 1, y, z).isOpaque()) {
                    Face face;
                    face.x = x;
                    face.y = y;
                    face.z = z;
                    face.width = 1;
                    face.height = 1;
                    face.direction = 0;
                    face.type = voxel.type;
                    addFace(face, vertices, indices);
                    vertexCount += 4;
                }

                // -X face
                if (x == 0 || !chunk->getVoxel(x - 1, y, z).isOpaque()) {
                    Face face;
                    face.x = x;
                    face.y = y;
                    face.z = z;
                    face.width = 1;
                    face.height = 1;
                    face.direction = 1;
                    face.type = voxel.type;
                    addFace(face, vertices, indices);
                    vertexCount += 4;
                }

                // +Y face
                if (y == CHUNK_HEIGHT - 1 || !chunk->getVoxel(x, y + 1, z).isOpaque()) {
                    Face face;
                    face.x = x;
                    face.y = y;
                    face.z = z;
                    face.width = 1;
                    face.height = 1;
                    face.direction = 2;
                    face.type = voxel.type;
                    addFace(face, vertices, indices);
                    vertexCount += 4;
                }

                // -Y face
                if (y == 0 || !chunk->getVoxel(x, y - 1, z).isOpaque()) {
                    Face face;
                    face.x = x;
                    face.y = y;
                    face.z = z;
                    face.width = 1;
                    face.height = 1;
                    face.direction = 3;
                    face.type = voxel.type;
                    addFace(face, vertices, indices);
                    vertexCount += 4;
                }

                // +Z face
                if (z == CHUNK_SIZE - 1 || !chunk->getVoxel(x, y, z + 1).isOpaque()) {
                    Face face;
                    face.x = x;
                    face.y = y;
                    face.z = z;
                    face.width = 1;
                    face.height = 1;
                    face.direction = 4;
                    face.type = voxel.type;
                    addFace(face, vertices, indices);
                    vertexCount += 4;
                }

                // -Z face
                if (z == 0 || !chunk->getVoxel(x, y, z - 1).isOpaque()) {
                    Face face;
                    face.x = x;
                    face.y = y;
                    face.z = z;
                    face.width = 1;
                    face.height = 1;
                    face.direction = 5;
                    face.type = voxel.type;
                    addFace(face, vertices, indices);
                    vertexCount += 4;
                }
            }
        }
    }
}

void MeshGenerator::addFace(const Face& face, std::vector<float>& vertices,
                            std::vector<uint32_t>& indices)
{
    const float x = static_cast<float>(face.x);
    const float y = static_cast<float>(face.y);
    const float z = static_cast<float>(face.z);

    const uint32_t startIndex =
        static_cast<uint32_t>(vertices.size() / 6); // 6 floats per vertex (pos + normal)

    // Color based on block type (not currently used in vertex data, but may be used for
    // texturing/lighting)
    float r = 0.5f, g = 0.5f, b = 0.5f;
    switch (face.type) {
    case VoxelType::Grass:
        r = 0.2f;
        g = 0.8f;
        b = 0.2f;
        break;
    case VoxelType::Dirt:
        r = 0.6f;
        g = 0.4f;
        b = 0.2f;
        break;
    case VoxelType::Stone:
        r = 0.5f;
        g = 0.5f;
        b = 0.5f;
        break;
    default:
        break;
    }
    // Suppress warnings - colors reserved for future use
    (void)r;
    (void)g;
    (void)b;

    // Pre-allocate space for 4 vertices (24 floats: 4 vertices * 6 floats each)
    vertices.reserve(vertices.size() + 24);

    // Add vertices based on face direction
    // Using individual emplace_back calls is more efficient than insert with initializer lists
    switch (face.direction) {
    case 0: // +X face (right)
        // Vertex 0: bottom-left
        vertices.emplace_back(x + 1);
        vertices.emplace_back(y);
        vertices.emplace_back(z);
        vertices.emplace_back(1.0f);
        vertices.emplace_back(0.0f);
        vertices.emplace_back(0.0f);
        // Vertex 1: top-left
        vertices.emplace_back(x + 1);
        vertices.emplace_back(y + 1);
        vertices.emplace_back(z);
        vertices.emplace_back(1.0f);
        vertices.emplace_back(0.0f);
        vertices.emplace_back(0.0f);
        // Vertex 2: top-right
        vertices.emplace_back(x + 1);
        vertices.emplace_back(y + 1);
        vertices.emplace_back(z + 1);
        vertices.emplace_back(1.0f);
        vertices.emplace_back(0.0f);
        vertices.emplace_back(0.0f);
        // Vertex 3: bottom-right
        vertices.emplace_back(x + 1);
        vertices.emplace_back(y);
        vertices.emplace_back(z + 1);
        vertices.emplace_back(1.0f);
        vertices.emplace_back(0.0f);
        vertices.emplace_back(0.0f);
        break;
    case 1: // -X face (left)
        vertices.emplace_back(x);
        vertices.emplace_back(y);
        vertices.emplace_back(z);
        vertices.emplace_back(-1.0f);
        vertices.emplace_back(0.0f);
        vertices.emplace_back(0.0f);
        vertices.emplace_back(x);
        vertices.emplace_back(y);
        vertices.emplace_back(z + 1);
        vertices.emplace_back(-1.0f);
        vertices.emplace_back(0.0f);
        vertices.emplace_back(0.0f);
        vertices.emplace_back(x);
        vertices.emplace_back(y + 1);
        vertices.emplace_back(z + 1);
        vertices.emplace_back(-1.0f);
        vertices.emplace_back(0.0f);
        vertices.emplace_back(0.0f);
        vertices.emplace_back(x);
        vertices.emplace_back(y + 1);
        vertices.emplace_back(z);
        vertices.emplace_back(-1.0f);
        vertices.emplace_back(0.0f);
        vertices.emplace_back(0.0f);
        break;
    case 2: // +Y face (top)
        vertices.emplace_back(x);
        vertices.emplace_back(y + 1);
        vertices.emplace_back(z);
        vertices.emplace_back(0.0f);
        vertices.emplace_back(1.0f);
        vertices.emplace_back(0.0f);
        vertices.emplace_back(x);
        vertices.emplace_back(y + 1);
        vertices.emplace_back(z + 1);
        vertices.emplace_back(0.0f);
        vertices.emplace_back(1.0f);
        vertices.emplace_back(0.0f);
        vertices.emplace_back(x + 1);
        vertices.emplace_back(y + 1);
        vertices.emplace_back(z + 1);
        vertices.emplace_back(0.0f);
        vertices.emplace_back(1.0f);
        vertices.emplace_back(0.0f);
        vertices.emplace_back(x + 1);
        vertices.emplace_back(y + 1);
        vertices.emplace_back(z);
        vertices.emplace_back(0.0f);
        vertices.emplace_back(1.0f);
        vertices.emplace_back(0.0f);
        break;
    case 3: // -Y face (bottom)
        vertices.emplace_back(x);
        vertices.emplace_back(y);
        vertices.emplace_back(z);
        vertices.emplace_back(0.0f);
        vertices.emplace_back(-1.0f);
        vertices.emplace_back(0.0f);
        vertices.emplace_back(x + 1);
        vertices.emplace_back(y);
        vertices.emplace_back(z);
        vertices.emplace_back(0.0f);
        vertices.emplace_back(-1.0f);
        vertices.emplace_back(0.0f);
        vertices.emplace_back(x + 1);
        vertices.emplace_back(y);
        vertices.emplace_back(z + 1);
        vertices.emplace_back(0.0f);
        vertices.emplace_back(-1.0f);
        vertices.emplace_back(0.0f);
        vertices.emplace_back(x);
        vertices.emplace_back(y);
        vertices.emplace_back(z + 1);
        vertices.emplace_back(0.0f);
        vertices.emplace_back(-1.0f);
        vertices.emplace_back(0.0f);
        break;
    case 4: // +Z face (front)
        vertices.emplace_back(x);
        vertices.emplace_back(y);
        vertices.emplace_back(z + 1);
        vertices.emplace_back(0.0f);
        vertices.emplace_back(0.0f);
        vertices.emplace_back(1.0f);
        vertices.emplace_back(x + 1);
        vertices.emplace_back(y);
        vertices.emplace_back(z + 1);
        vertices.emplace_back(0.0f);
        vertices.emplace_back(0.0f);
        vertices.emplace_back(1.0f);
        vertices.emplace_back(x + 1);
        vertices.emplace_back(y + 1);
        vertices.emplace_back(z + 1);
        vertices.emplace_back(0.0f);
        vertices.emplace_back(0.0f);
        vertices.emplace_back(1.0f);
        vertices.emplace_back(x);
        vertices.emplace_back(y + 1);
        vertices.emplace_back(z + 1);
        vertices.emplace_back(0.0f);
        vertices.emplace_back(0.0f);
        vertices.emplace_back(1.0f);
        break;
    case 5: // -Z face (back)
        vertices.emplace_back(x);
        vertices.emplace_back(y);
        vertices.emplace_back(z);
        vertices.emplace_back(0.0f);
        vertices.emplace_back(0.0f);
        vertices.emplace_back(-1.0f);
        vertices.emplace_back(x);
        vertices.emplace_back(y + 1);
        vertices.emplace_back(z);
        vertices.emplace_back(0.0f);
        vertices.emplace_back(0.0f);
        vertices.emplace_back(-1.0f);
        vertices.emplace_back(x + 1);
        vertices.emplace_back(y + 1);
        vertices.emplace_back(z);
        vertices.emplace_back(0.0f);
        vertices.emplace_back(0.0f);
        vertices.emplace_back(-1.0f);
        vertices.emplace_back(x + 1);
        vertices.emplace_back(y);
        vertices.emplace_back(z);
        vertices.emplace_back(0.0f);
        vertices.emplace_back(0.0f);
        vertices.emplace_back(-1.0f);
        break;
    }

    // Add indices for two triangles (more efficient to reserve + push_back than insert)
    indices.reserve(indices.size() + 6);
    indices.emplace_back(startIndex);
    indices.emplace_back(startIndex + 1);
    indices.emplace_back(startIndex + 2);
    indices.emplace_back(startIndex);
    indices.emplace_back(startIndex + 2);
    indices.emplace_back(startIndex + 3);
}

bool MeshGenerator::isVoxelOpaque(const Chunk* chunk, int x, int y, int z,
                                   const ChunkNeighbors& neighbors) const
{
    // Check bounds and handle neighbor chunks
    if (x < 0) {
        // Check left neighbor
        if (neighbors.left) {
            return neighbors.left->getVoxel(CHUNK_SIZE - 1, y, z).isOpaque();
        }
        return false; // No neighbor, treat as transparent (render face)
    } else if (x >= CHUNK_SIZE) {
        // Check right neighbor
        if (neighbors.right) {
            return neighbors.right->getVoxel(0, y, z).isOpaque();
        }
        return false;
    }

    if (z < 0) {
        // Check back neighbor
        if (neighbors.back) {
            return neighbors.back->getVoxel(x, y, CHUNK_SIZE - 1).isOpaque();
        }
        return false;
    } else if (z >= CHUNK_SIZE) {
        // Check front neighbor
        if (neighbors.front) {
            return neighbors.front->getVoxel(x, y, 0).isOpaque();
        }
        return false;
    }

    // Check Y bounds (no vertical neighbors in current implementation)
    if (y < 0 || y >= CHUNK_HEIGHT) {
        return false; // Out of bounds, treat as transparent
    }

    // Normal in-chunk check
    return chunk->getVoxel(x, y, z).isOpaque();
}

void MeshGenerator::generateMeshWithNeighbors(const Chunk* chunk, const ChunkNeighbors& neighbors,
                                               std::vector<float>& vertices,
                                               std::vector<uint32_t>& indices)
{
    // Enhanced mesh generation with proper boundary culling
    vertices.clear();
    indices.clear();

    // Reserve capacity to reduce allocations
    constexpr size_t estimatedSolidBlocks = CHUNK_VOLUME / 4;
    constexpr size_t estimatedFaces = estimatedSolidBlocks * 3;
    vertices.reserve(estimatedFaces * 32);
    indices.reserve(estimatedFaces * 6);

    uint32_t vertexCount = 0;

    // Iterate through all voxels
    for (int y = 0; y < CHUNK_HEIGHT; ++y) {
        for (int z = 0; z < CHUNK_SIZE; ++z) {
            for (int x = 0; x < CHUNK_SIZE; ++x) {
                const Voxel& voxel = chunk->getVoxel(x, y, z);

                if (!voxel.isSolid()) {
                    continue;
                }

                // Check each face with proper neighbor awareness
                // +X face
                if (!isVoxelOpaque(chunk, x + 1, y, z, neighbors)) {
                    Face face;
                    face.x = x;
                    face.y = y;
                    face.z = z;
                    face.width = 1;
                    face.height = 1;
                    face.direction = 0;
                    face.type = voxel.type;
                    addFace(face, vertices, indices);
                    vertexCount += 4;
                }

                // -X face
                if (!isVoxelOpaque(chunk, x - 1, y, z, neighbors)) {
                    Face face;
                    face.x = x;
                    face.y = y;
                    face.z = z;
                    face.width = 1;
                    face.height = 1;
                    face.direction = 1;
                    face.type = voxel.type;
                    addFace(face, vertices, indices);
                    vertexCount += 4;
                }

                // +Y face
                if (y == CHUNK_HEIGHT - 1 || !chunk->getVoxel(x, y + 1, z).isOpaque()) {
                    Face face;
                    face.x = x;
                    face.y = y;
                    face.z = z;
                    face.width = 1;
                    face.height = 1;
                    face.direction = 2;
                    face.type = voxel.type;
                    addFace(face, vertices, indices);
                    vertexCount += 4;
                }

                // -Y face
                if (y == 0 || !chunk->getVoxel(x, y - 1, z).isOpaque()) {
                    Face face;
                    face.x = x;
                    face.y = y;
                    face.z = z;
                    face.width = 1;
                    face.height = 1;
                    face.direction = 3;
                    face.type = voxel.type;
                    addFace(face, vertices, indices);
                    vertexCount += 4;
                }

                // +Z face
                if (!isVoxelOpaque(chunk, x, y, z + 1, neighbors)) {
                    Face face;
                    face.x = x;
                    face.y = y;
                    face.z = z;
                    face.width = 1;
                    face.height = 1;
                    face.direction = 4;
                    face.type = voxel.type;
                    addFace(face, vertices, indices);
                    vertexCount += 4;
                }

                // -Z face
                if (!isVoxelOpaque(chunk, x, y, z - 1, neighbors)) {
                    Face face;
                    face.x = x;
                    face.y = y;
                    face.z = z;
                    face.width = 1;
                    face.height = 1;
                    face.direction = 5;
                    face.type = voxel.type;
                    addFace(face, vertices, indices);
                    vertexCount += 4;
                }
            }
        }
    }
}

} // namespace fresh
