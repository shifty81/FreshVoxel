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
 *
 * Vertex layout (floats per vertex):
 *   [0..2]  position  (x, y, z)
 *   [3..5]  normal    (nx, ny, nz)
 *   [6]     ao        ambient occlusion factor [0,1] — packed as a single float
 *   [7]     typeId    VoxelType cast to float, for palette lookup in shader
 *
 * Total: 8 floats per vertex.
 */
class MeshGenerator
{
public:
    MeshGenerator();
    ~MeshGenerator();

    /**
     * @brief Generate mesh for a chunk using simple per-face culling.
     * @param chunk Chunk to generate mesh for
     * @param vertices Output vertex data (8 floats per vertex)
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

    /**
     * @brief LOD 1 mesh — half-resolution greedy merge.
     *
     * Groups 2×2×2 voxel cells into a single face where all voxels match.
     * Switch to this mesh when the chunk is > 128 m from the camera.
     * @param chunk Chunk to generate LOD1 mesh for
     * @param vertices Output vertex data
     * @param indices Output index data
     */
    void generateLOD1Mesh(const Chunk* chunk, std::vector<float>& vertices,
                          std::vector<uint32_t>& indices);

private:
    struct Face {
        int x, y, z;
        int width, height;
        int direction; // 0-5 for +X,-X,+Y,-Y,+Z,-Z
        VoxelType type;
    };

    void addFace(const Face& face, std::vector<float>& vertices, std::vector<uint32_t>& indices);
    void addFaceWithAO(const Face& face,
                       const Chunk* chunk,
                       const ChunkNeighbors& neighbors,
                       std::vector<float>& vertices,
                       std::vector<uint32_t>& indices);

    /**
     * @brief Check if a voxel is opaque, considering neighbors for boundary checks
     */
    bool isVoxelOpaque(const Chunk* chunk, int x, int y, int z, 
                       const ChunkNeighbors& neighbors) const;

    /**
     * @brief Compute per-corner ambient occlusion factor [0,1] for one face vertex.
     *
     * Samples the 8 neighbour voxels around the corner (side1, side2, corner)
     * using the standard Minecraft-style AO formula.
     * @param side1Opaque First edge-neighbour is opaque
     * @param side2Opaque Second edge-neighbour is opaque
     * @param cornerOpaque Corner-diagonal voxel is opaque
     * @return AO factor in [0,1]; 0 = fully occluded, 1 = unoccluded
     */
    static float computeAOValue(bool side1Opaque, bool side2Opaque, bool cornerOpaque) noexcept;
};

} // namespace fresh
