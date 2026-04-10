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

// ---------------------------------------------------------------------------
// computeAOValue — Minecraft-style corner AO
// ---------------------------------------------------------------------------

/*static*/ float MeshGenerator::computeAOValue(bool side1, bool side2, bool corner) noexcept
{
    // Both side neighbours opaque → maximum occlusion regardless of corner
    if (side1 && side2) return 0.0f;
    // Number of occluders (0-3)
    const int occ = (side1 ? 1 : 0) + (side2 ? 1 : 0) + (corner ? 1 : 0);
    // Map 0 occluders → 1.0, 1 → 0.75, 2 → 0.50, 3 → 0.25
    return 1.0f - 0.25f * static_cast<float>(occ);
}

// ---------------------------------------------------------------------------
// addFaceWithAO — like addFace but emits 8 floats per vertex (pos+normal+ao+type)
// ---------------------------------------------------------------------------

void MeshGenerator::addFaceWithAO(const Face&           face,
                                   const Chunk*          chunk,
                                   const ChunkNeighbors& neighbors,
                                   std::vector<float>&   vertices,
                                   std::vector<uint32_t>& indices)
{
    const float x = static_cast<float>(face.x);
    const float y = static_cast<float>(face.y);
    const float z = static_cast<float>(face.z);
    const float typeF = static_cast<float>(static_cast<int>(face.type));

    const uint32_t startIndex = static_cast<uint32_t>(vertices.size() / 8);

    // Helper lambda: sample a voxel's opaqueness
    auto opaque = [&](int bx, int by, int bz) -> bool {
        return isVoxelOpaque(chunk, bx, by, bz, neighbors);
    };

    // AO corner values for the 4 vertices of each face direction.
    // Per-vertex layout: pos(3) + normal(3) + ao(1) + typeId(1) = 8 floats
    float ao0 = 1.0f, ao1 = 1.0f, ao2 = 1.0f, ao3 = 1.0f;
    int cx = face.x, cy = face.y, cz = face.z;

    switch (face.direction) {
    case 0: // +X face; normal = (1,0,0)
        ao0 = computeAOValue(opaque(cx+1,cy-1,cz),   opaque(cx+1,cy,cz-1),   opaque(cx+1,cy-1,cz-1));
        ao1 = computeAOValue(opaque(cx+1,cy+1,cz),   opaque(cx+1,cy,cz-1),   opaque(cx+1,cy+1,cz-1));
        ao2 = computeAOValue(opaque(cx+1,cy+1,cz),   opaque(cx+1,cy,cz+1),   opaque(cx+1,cy+1,cz+1));
        ao3 = computeAOValue(opaque(cx+1,cy-1,cz),   opaque(cx+1,cy,cz+1),   opaque(cx+1,cy-1,cz+1));
        vertices.insert(vertices.end(), {x+1,y,  z,   1,0,0, ao0, typeF});
        vertices.insert(vertices.end(), {x+1,y+1,z,   1,0,0, ao1, typeF});
        vertices.insert(vertices.end(), {x+1,y+1,z+1, 1,0,0, ao2, typeF});
        vertices.insert(vertices.end(), {x+1,y,  z+1, 1,0,0, ao3, typeF});
        break;
    case 1: // -X face
        ao0 = computeAOValue(opaque(cx-1,cy-1,cz),   opaque(cx-1,cy,cz+1),   opaque(cx-1,cy-1,cz+1));
        ao1 = computeAOValue(opaque(cx-1,cy-1,cz),   opaque(cx-1,cy,cz-1),   opaque(cx-1,cy-1,cz-1));
        ao2 = computeAOValue(opaque(cx-1,cy+1,cz),   opaque(cx-1,cy,cz-1),   opaque(cx-1,cy+1,cz-1));
        ao3 = computeAOValue(opaque(cx-1,cy+1,cz),   opaque(cx-1,cy,cz+1),   opaque(cx-1,cy+1,cz+1));
        vertices.insert(vertices.end(), {x,y,  z,    -1,0,0, ao0, typeF});
        vertices.insert(vertices.end(), {x,y,  z+1,  -1,0,0, ao1, typeF});
        vertices.insert(vertices.end(), {x,y+1,z+1,  -1,0,0, ao2, typeF});
        vertices.insert(vertices.end(), {x,y+1,z,    -1,0,0, ao3, typeF});
        break;
    case 2: // +Y face
        ao0 = computeAOValue(opaque(cx-1,cy+1,cz),   opaque(cx,cy+1,cz-1),   opaque(cx-1,cy+1,cz-1));
        ao1 = computeAOValue(opaque(cx-1,cy+1,cz),   opaque(cx,cy+1,cz+1),   opaque(cx-1,cy+1,cz+1));
        ao2 = computeAOValue(opaque(cx+1,cy+1,cz),   opaque(cx,cy+1,cz+1),   opaque(cx+1,cy+1,cz+1));
        ao3 = computeAOValue(opaque(cx+1,cy+1,cz),   opaque(cx,cy+1,cz-1),   opaque(cx+1,cy+1,cz-1));
        vertices.insert(vertices.end(), {x,  y+1,z,   0,1,0, ao0, typeF});
        vertices.insert(vertices.end(), {x,  y+1,z+1, 0,1,0, ao1, typeF});
        vertices.insert(vertices.end(), {x+1,y+1,z+1, 0,1,0, ao2, typeF});
        vertices.insert(vertices.end(), {x+1,y+1,z,   0,1,0, ao3, typeF});
        break;
    case 3: // -Y face
        ao0 = computeAOValue(opaque(cx-1,cy-1,cz),   opaque(cx,cy-1,cz-1),   opaque(cx-1,cy-1,cz-1));
        ao1 = computeAOValue(opaque(cx+1,cy-1,cz),   opaque(cx,cy-1,cz-1),   opaque(cx+1,cy-1,cz-1));
        ao2 = computeAOValue(opaque(cx+1,cy-1,cz),   opaque(cx,cy-1,cz+1),   opaque(cx+1,cy-1,cz+1));
        ao3 = computeAOValue(opaque(cx-1,cy-1,cz),   opaque(cx,cy-1,cz+1),   opaque(cx-1,cy-1,cz+1));
        vertices.insert(vertices.end(), {x,  y,z,    0,-1,0, ao0, typeF});
        vertices.insert(vertices.end(), {x+1,y,z,    0,-1,0, ao1, typeF});
        vertices.insert(vertices.end(), {x+1,y,z+1,  0,-1,0, ao2, typeF});
        vertices.insert(vertices.end(), {x,  y,z+1,  0,-1,0, ao3, typeF});
        break;
    case 4: // +Z face
        ao0 = computeAOValue(opaque(cx-1,cy,cz+1),   opaque(cx,cy-1,cz+1),   opaque(cx-1,cy-1,cz+1));
        ao1 = computeAOValue(opaque(cx+1,cy,cz+1),   opaque(cx,cy-1,cz+1),   opaque(cx+1,cy-1,cz+1));
        ao2 = computeAOValue(opaque(cx+1,cy,cz+1),   opaque(cx,cy+1,cz+1),   opaque(cx+1,cy+1,cz+1));
        ao3 = computeAOValue(opaque(cx-1,cy,cz+1),   opaque(cx,cy+1,cz+1),   opaque(cx-1,cy+1,cz+1));
        vertices.insert(vertices.end(), {x,  y,  z+1, 0,0,1, ao0, typeF});
        vertices.insert(vertices.end(), {x+1,y,  z+1, 0,0,1, ao1, typeF});
        vertices.insert(vertices.end(), {x+1,y+1,z+1, 0,0,1, ao2, typeF});
        vertices.insert(vertices.end(), {x,  y+1,z+1, 0,0,1, ao3, typeF});
        break;
    default: // case 5: -Z face
        ao0 = computeAOValue(opaque(cx-1,cy,cz-1),   opaque(cx,cy-1,cz-1),   opaque(cx-1,cy-1,cz-1));
        ao1 = computeAOValue(opaque(cx-1,cy,cz-1),   opaque(cx,cy+1,cz-1),   opaque(cx-1,cy+1,cz-1));
        ao2 = computeAOValue(opaque(cx+1,cy,cz-1),   opaque(cx,cy+1,cz-1),   opaque(cx+1,cy+1,cz-1));
        ao3 = computeAOValue(opaque(cx+1,cy,cz-1),   opaque(cx,cy-1,cz-1),   opaque(cx+1,cy-1,cz-1));
        vertices.insert(vertices.end(), {x,  y,  z,  0,0,-1, ao0, typeF});
        vertices.insert(vertices.end(), {x,  y+1,z,  0,0,-1, ao1, typeF});
        vertices.insert(vertices.end(), {x+1,y+1,z,  0,0,-1, ao2, typeF});
        vertices.insert(vertices.end(), {x+1,y,  z,  0,0,-1, ao3, typeF});
        break;
    }

    // Flip winding on faces with AO anisotropy (reduces 'corner banding' artefact)
    const bool flip = (ao0 + ao2) < (ao1 + ao3);
    if (!flip) {
        indices.insert(indices.end(), {startIndex, startIndex+1, startIndex+2,
                                       startIndex, startIndex+2, startIndex+3});
    } else {
        indices.insert(indices.end(), {startIndex+1, startIndex+2, startIndex+3,
                                       startIndex+1, startIndex+3, startIndex});
    }
}

// ---------------------------------------------------------------------------
// generateLOD1Mesh — half-resolution greedy merge
// ---------------------------------------------------------------------------

void MeshGenerator::generateLOD1Mesh(const Chunk* chunk,
                                      std::vector<float>& vertices,
                                      std::vector<uint32_t>& indices)
{
    // Process 2×2×2 cells; take the majority voxel type as representative.
    // If any cell voxel is Air, the super-voxel is Air.
    vertices.clear();
    indices.clear();

    constexpr int step = 2;

    for (int y = 0; y < CHUNK_HEIGHT - step + 1; y += step) {
        for (int z = 0; z < CHUNK_SIZE - step + 1; z += step) {
            for (int x = 0; x < CHUNK_SIZE - step + 1; x += step) {

                // Check if all 8 voxels in the 2×2×2 cell are the same solid type
                VoxelType rep = chunk->getVoxel(x, y, z).type;
                if (rep == VoxelType::Air) continue;

                bool allMatch = true;
                for (int dy = 0; dy < step && allMatch; ++dy)
                    for (int dz = 0; dz < step && allMatch; ++dz)
                        for (int dx = 0; dx < step && allMatch; ++dx)
                            if (chunk->getVoxel(x+dx, y+dy, z+dz).type != rep)
                                allMatch = false;

                if (!allMatch) continue;

                // Emit a 2×2×2 block face for each visible direction
                const ChunkNeighbors noNeighbors{};
                for (int dir = 0; dir < 6; ++dir) {
                    // Compute the neighbour position for each direction
                    int nx = x, ny = y, nz = z;
                    switch (dir) {
                    case 0: nx = x + step; break;
                    case 1: nx = x - 1;    break;
                    case 2: ny = y + step; break;
                    case 3: ny = y - 1;    break;
                    case 4: nz = z + step; break;
                    case 5: nz = z - 1;    break;
                    }
                    bool neighbourOpaque = false;
                    if (nx >= 0 && nx < CHUNK_SIZE &&
                        ny >= 0 && ny < CHUNK_HEIGHT &&
                        nz >= 0 && nz < CHUNK_SIZE)
                    {
                        neighbourOpaque = chunk->getVoxel(nx, ny, nz).isOpaque();
                    }
                    if (!neighbourOpaque) {
                        Face f;
                        f.x = x; f.y = y; f.z = z;
                        f.width  = step;
                        f.height = step;
                        f.direction = dir;
                        f.type = rep;
                        addFace(f, vertices, indices);
                    }
                }
            }
        }
    }
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
