#pragma once

#include <array>
#include <memory>
#include <vector>

#include "VoxelTypes.h"

namespace fresh
{

class MeshGenerator;

/**
 * @brief Represents a chunk of voxels
 *
 * A chunk is a cubic section of the world containing voxel data.
 * Uses efficient storage and mesh generation for rendering.
 */
class Chunk
{
public:
    explicit Chunk(const ChunkPos& position);
    ~Chunk();

    /**
     * @brief Get voxel at local chunk coordinates
     * @param x Local x coordinate (0-CHUNK_SIZE)
     * @param y Local y coordinate (0-CHUNK_HEIGHT)
     * @param z Local z coordinate (0-CHUNK_SIZE)
     * @return Reference to voxel
     */
    [[nodiscard]] Voxel& getVoxel(int x, int y, int z) noexcept;
    [[nodiscard]] const Voxel& getVoxel(int x, int y, int z) const noexcept;

    /**
     * @brief Set voxel at local chunk coordinates
     * @param x Local x coordinate
     * @param y Local y coordinate
     * @param z Local z coordinate
     * @param voxel Voxel to set
     */
    void setVoxel(int x, int y, int z, const Voxel& voxel);

    /**
     * @brief Get chunk position
     * @return Chunk position
     */
    [[nodiscard]] const ChunkPos& getPosition() const noexcept
    {
        return m_position;
    }

    /**
     * @brief Check if chunk needs mesh regeneration
     * @return true if mesh is dirty, false otherwise
     */
    [[nodiscard]] bool isDirty() const noexcept
    {
        return m_dirty;
    }

    /**
     * @brief Mark chunk as needing mesh regeneration
     */
    void markDirty() noexcept
    {
        m_dirty = true;
    }

    /**
     * @brief Clear dirty flag
     */
    void clearDirty() noexcept
    {
        m_dirty = false;
    }

    /**
     * @brief Generate mesh for this chunk
     */
    void generateMesh();

    /**
     * @brief Get the mesh data
     * @return Pointer to mesh vertices
     */
    [[nodiscard]] const std::vector<float>& getMeshVertices() const noexcept
    {
        return m_meshVertices;
    }

    /**
     * @brief Get the mesh indices
     * @return Pointer to mesh indices
     */
    [[nodiscard]] const std::vector<uint32_t>& getMeshIndices() const noexcept
    {
        return m_meshIndices;
    }

private:
    inline int getIndex(int x, int y, int z) const noexcept
    {
        return x + CHUNK_SIZE * (z + CHUNK_SIZE * y);
    }

private:
    ChunkPos m_position;
    std::vector<Voxel> m_voxels;
    std::vector<float> m_meshVertices;
    std::vector<uint32_t> m_meshIndices;
    bool m_dirty;
};

} // namespace fresh
