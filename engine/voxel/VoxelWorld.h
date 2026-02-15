#pragma once

#include <memory>
#include <unordered_map>
#include <vector>

#include "Chunk.h"

namespace fresh
{

class TerrainGenerator;

/**
 * @brief Manages the entire voxel world (scene data layer)
 *
 * In the editor-first architecture, a VoxelWorld represents the data layer
 * of a scene. The editor manages scenes (via File > New Scene / Open Scene),
 * and each scene contains a VoxelWorld for its voxel-based environment.
 * 
 * Handles chunk loading/unloading, world updates, and provides
 * interface for voxel manipulation.
 */
class VoxelWorld
{
public:
    VoxelWorld();
    ~VoxelWorld();

    /**
     * @brief Initialize the world
     * @return true if initialization was successful
     */
    bool initialize();

    /**
     * @brief Update the world (chunk streaming, etc.)
     * @param playerPos Player position for chunk loading
     */
    void update(const WorldPos& playerPos);

    /**
     * @brief Get chunk at position
     * @param pos Chunk position
     * @return Pointer to chunk or nullptr if not loaded
     */
    [[nodiscard]] Chunk* getChunk(const ChunkPos& pos) noexcept;
    [[nodiscard]] const Chunk* getChunk(const ChunkPos& pos) const noexcept;

    /**
     * @brief Load chunk at position
     * @param pos Chunk position
     * @return Pointer to loaded chunk
     */
    Chunk* loadChunk(const ChunkPos& pos);

    /**
     * @brief Unload chunk at position
     * @param pos Chunk position
     */
    void unloadChunk(const ChunkPos& pos);

    /**
     * @brief Get voxel at world position
     * @param pos World position
     * @return Pointer to voxel or nullptr
     */
    Voxel* getVoxel(const WorldPos& pos);

    /**
     * @brief Set voxel at world position
     * @param pos World position
     * @param voxel Voxel to set
     */
    void setVoxel(const WorldPos& pos, const Voxel& voxel);

    /**
     * @brief Get all loaded chunks
     * @return Map of chunk positions to chunks
     */
    [[nodiscard]] const std::unordered_map<ChunkPos, std::unique_ptr<Chunk>>&
    getChunks() const noexcept
    {
        return m_chunks;
    }

    /**
     * @brief Set render distance
     * @param distance Render distance in chunks
     */
    void setRenderDistance(int distance)
    {
        m_renderDistance = distance;
    }

    /**
     * @brief Set the seed for terrain generation
     * @param seed Random seed for terrain generation
     */
    void setSeed(int seed);

    /**
     * @brief Set the 2D game style (0=Platformer, 1=Top-down)
     * @param style 2D game style index
     */
    void set2DStyle(int style);

    /**
     * @brief Clear all loaded chunks from the world
     */
    void clearAllChunks();

    /**
     * @brief Regenerate terrain for all currently loaded chunks
     */
    void regenerateLoadedChunks();

private:
    std::unordered_map<ChunkPos, std::unique_ptr<Chunk>> m_chunks;
    std::unique_ptr<TerrainGenerator> m_terrainGenerator;
    int m_renderDistance;
};

} // namespace fresh
