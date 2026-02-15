#pragma once

#include <vector>

#include "VoxelTypes.h"

namespace fresh
{

class Chunk;

/**
 * @brief Neighbor chunks for boundary face culling
 */
struct ChunkNeighbors {
    const Chunk* left = nullptr;   // -X direction
    const Chunk* right = nullptr;  // +X direction
    const Chunk* front = nullptr;  // +Z direction
    const Chunk* back = nullptr;   // -Z direction
};

/**
 * @brief Generates optimized meshes from voxel data
 *
 * Implements greedy meshing algorithm to reduce polygon count
 * by merging adjacent faces of the same type.
 */
class MeshGenerator
{
public:
    MeshGenerator();
    ~MeshGenerator();

    /**
     * @brief Generate mesh for a chunk using greedy meshing
     * @param chunk Chunk to generate mesh for
     * @param vertices Output vertex data
     * @param indices Output index data
     */
    void generateChunkMesh(const Chunk* chunk, std::vector<float>& vertices,
                           std::vector<uint32_t>& indices);

    /**
     * @brief Generate simple cubic mesh (faster, more polygons)
     * @param chunk Chunk to generate mesh for
     * @param vertices Output vertex data
     * @param indices Output index data
     */
    void generateSimpleMesh(const Chunk* chunk, std::vector<float>& vertices,
                            std::vector<uint32_t>& indices);

    /**
     * @brief Generate mesh with neighbor awareness for proper boundary culling
     * @param chunk Chunk to generate mesh for
     * @param neighbors Neighboring chunks for boundary checks
     * @param vertices Output vertex data
     * @param indices Output index data
     */
    void generateMeshWithNeighbors(const Chunk* chunk, const ChunkNeighbors& neighbors,
                                    std::vector<float>& vertices, std::vector<uint32_t>& indices);

private:
    struct Face {
        int x, y, z;
        int width, height;
        int direction; // 0-5 for +X,-X,+Y,-Y,+Z,-Z
        VoxelType type;
    };

    void addFace(const Face& face, std::vector<float>& vertices, std::vector<uint32_t>& indices);
    
    /**
     * @brief Check if a voxel is opaque, considering neighbors for boundary checks
     */
    bool isVoxelOpaque(const Chunk* chunk, int x, int y, int z, 
                       const ChunkNeighbors& neighbors) const;
};

} // namespace fresh
