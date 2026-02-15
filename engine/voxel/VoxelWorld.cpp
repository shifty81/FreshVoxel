#include "voxel/VoxelWorld.h"

#include <iostream>

#include "generation/TerrainGenerator.h"

namespace fresh
{

VoxelWorld::VoxelWorld() : m_renderDistance(16) {}  // Increased from 8 to 16 for larger worlds

VoxelWorld::~VoxelWorld() {}

bool VoxelWorld::initialize()
{
    m_terrainGenerator = std::make_unique<TerrainGenerator>(12345);
    return true;
}

void VoxelWorld::update(const WorldPos& playerPos)
{
    (void)playerPos; // Unused - placeholder for future implementation
    // Simple chunk loading/unloading based on player position
    // In a real implementation, this would be more sophisticated
}

Chunk* VoxelWorld::getChunk(const ChunkPos& pos) noexcept
{
    auto it = m_chunks.find(pos);
    if (it != m_chunks.end()) {
        return it->second.get();
    }
    return nullptr;
}

const Chunk* VoxelWorld::getChunk(const ChunkPos& pos) const noexcept
{
    auto it = m_chunks.find(pos);
    if (it != m_chunks.end()) {
        return it->second.get();
    }
    return nullptr;
}

Chunk* VoxelWorld::loadChunk(const ChunkPos& pos)
{
    auto existing = getChunk(pos);
    if (existing) {
        return existing;
    }

    auto chunk = std::make_unique<Chunk>(pos);
    m_terrainGenerator->generateChunk(chunk.get());
    chunk->generateMesh();

    Chunk* chunkPtr = chunk.get();
    m_chunks[pos] = std::move(chunk);

    return chunkPtr;
}

void VoxelWorld::unloadChunk(const ChunkPos& pos)
{
    m_chunks.erase(pos);
}

Voxel* VoxelWorld::getVoxel(const WorldPos& pos)
{
    const ChunkPos chunkPos = ChunkPos::fromWorldPos(pos);
    Chunk* chunk = getChunk(chunkPos);

    if (!chunk) {
        return nullptr;
    }

    // Compute local coordinates more efficiently
    // Use modulo for proper wrapping of negative coordinates
    const int localX = ((pos.x % CHUNK_SIZE) + CHUNK_SIZE) % CHUNK_SIZE;
    const int localZ = ((pos.z % CHUNK_SIZE) + CHUNK_SIZE) % CHUNK_SIZE;

    return &chunk->getVoxel(localX, pos.y, localZ);
}

void VoxelWorld::setVoxel(const WorldPos& pos, const Voxel& voxel)
{
    const ChunkPos chunkPos = ChunkPos::fromWorldPos(pos);
    Chunk* chunk = getChunk(chunkPos);

    if (!chunk) {
        return;
    }

    // Compute local coordinates more efficiently
    const int localX = ((pos.x % CHUNK_SIZE) + CHUNK_SIZE) % CHUNK_SIZE;
    const int localZ = ((pos.z % CHUNK_SIZE) + CHUNK_SIZE) % CHUNK_SIZE;

    chunk->setVoxel(localX, pos.y, localZ, voxel);
}

void VoxelWorld::setSeed(int seed)
{
    if (m_terrainGenerator) {
        m_terrainGenerator->setSeed(seed);
    }
}

void VoxelWorld::set2DStyle(int style)
{
    if (m_terrainGenerator) {
        m_terrainGenerator->set2DStyle(style);
    }
}

void VoxelWorld::clearAllChunks()
{
    m_chunks.clear();
}

void VoxelWorld::regenerateLoadedChunks()
{
    if (!m_terrainGenerator) {
        return;
    }

    // Collect positions of currently loaded chunks
    std::vector<ChunkPos> positions;
    positions.reserve(m_chunks.size());
    for (const auto& pair : m_chunks) {
        positions.push_back(pair.first);
    }

    // Regenerate each chunk in place
    for (const auto& pos : positions) {
        auto it = m_chunks.find(pos);
        if (it != m_chunks.end()) {
            auto chunk = std::make_unique<Chunk>(pos);
            m_terrainGenerator->generateChunk(chunk.get());
            chunk->generateMesh();
            it->second = std::move(chunk);
        }
    }
}

} // namespace fresh
