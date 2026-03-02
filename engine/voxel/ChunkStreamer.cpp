#include "voxel/ChunkStreamer.h"

#include <cmath>
#include <iostream>

#include "voxel/Chunk.h"
#include "voxel/VoxelWorld.h"

namespace fresh
{

ChunkStreamer::ChunkStreamer(VoxelWorld* world) : world(world)
{
    // Start background generation thread
    generationThread = std::thread(&ChunkStreamer::generationThreadFunc, this);
}

ChunkStreamer::~ChunkStreamer()
{
    shutdown();
}

void ChunkStreamer::update(const glm::vec3& playerPosition)
{
    if (!world)
        return;

    // Convert world position to chunk coordinates (16 blocks per chunk)
    glm::ivec2 playerChunk = worldToChunk(playerPosition);

    // Optimization: Only recalculate chunks when player crosses chunk boundary
    // This prevents unnecessary updates when player moves within same chunk
    if (playerChunk != lastPlayerChunk) {
        lastPlayerChunk = playerChunk;
        determineChunksToLoad(playerPosition);
        determineChunksToUnload(playerPosition);
    }

    // Process some chunks from load queue each frame
    // This spreads generation over multiple frames to avoid stuttering
    processLoadQueue();
}

void ChunkStreamer::setViewDistance(int chunks)
{
    // Clamp view distance to prevent excessive memory usage or too small range
    // Increased max to 64 for larger worlds (5K x 5K maps)
    viewDistance = std::max(1, std::min(chunks, 64)); // Increased from 32 to 64
}

void ChunkStreamer::shutdown()
{
    // Signal background thread to stop and wait for it to finish
    shouldRun = false;
    queueCV.notify_all();
    if (generationThread.joinable()) {
        generationThread.join();
    }
}

int ChunkStreamer::getLoadedChunkCount() const
{
    if (!world)
        return 0;
    return static_cast<int>(world->getChunks().size());
}

bool ChunkStreamer::isChunkLoaded(const glm::ivec2& chunkPos) const
{
    if (!world)
        return false;
    ChunkPos pos(chunkPos.x, chunkPos.y);
    return world->getChunk(pos) != nullptr;
}

void ChunkStreamer::determineChunksToLoad(const glm::vec3& playerPos)
{
    // Determine which chunks should be loaded based on player position
    // Uses circular loading pattern expanding outward from player
    glm::ivec2 playerChunk = worldToChunk(playerPos);

    std::lock_guard<std::mutex> lock(queueMutex);

    // Load chunks in expanding circular rings around player
    // This ensures chunks closest to player load first (better user experience)
    for (int radius = 0; radius <= viewDistance; ++radius) {
        // Only process chunks at current radius (forms expanding circles)
        for (int x = -radius; x <= radius; ++x) {
            for (int z = -radius; z <= radius; ++z) {
                // Check if this is on the current radius ring
                // Uses Chebyshev distance (max of abs differences)
                int dist = std::max(std::abs(x), std::abs(z));
                if (dist != radius)
                    continue;

                glm::ivec2 chunkPos = playerChunk + glm::ivec2(x, z);

                // Skip if already loaded or pending
                if (isChunkLoaded(chunkPos))
                    continue;
                if (pendingLoads.find(chunkPos) != pendingLoads.end())
                    continue;

                // Add to both load queue and background queue
                ChunkLoadRequest request;
                request.chunkPos = chunkPos;
                request.priority = radius;

                loadQueue.push(request);
                backgroundQueue.push(request);
                pendingLoads.insert(chunkPos);
            }
        }
    }

    // Notify background thread that new work is available
    queueCV.notify_one();
}

void ChunkStreamer::determineChunksToUnload(const glm::vec3& playerPos)
{
    if (!world)
        return;

    glm::ivec2 playerChunk = worldToChunk(playerPos);

    unloadQueue.clear();

    // Query VoxelWorld for all loaded chunks
    // For each loaded chunk, if distance > viewDistance + buffer, mark for unload
    float unloadDistance = static_cast<float>(viewDistance) + 2.0f;

    // Check if we're over the max loaded chunks limit
    if (getLoadedChunkCount() > maxLoadedChunks) {
        unloadDistance = static_cast<float>(viewDistance) + 1.0f; // Be more aggressive
    }

    for (const auto& [chunkPos, chunk] : world->getChunks()) {
        glm::ivec2 chunkPos2D(chunkPos.x, chunkPos.z);
        float dist = chunkDistance(chunkPos2D, playerChunk);
        if (dist > unloadDistance) {
            unloadQueue.push_back(chunkPos2D);
        }
    }
}

void ChunkStreamer::processLoadQueue()
{
    if (!world)
        return;

    std::lock_guard<std::mutex> lock(queueMutex);

    // First, insert any pre-generated chunks from the background thread
    int processed = 0;
    auto it = readyChunks.begin();
    while (it != readyChunks.end() && processed < chunksPerFrame) {
        ChunkPos pos(it->first.x, it->first.y);
        if (!world->getChunk(pos)) {
            // Move the pre-generated chunk directly into the world
            world->getChunks()[pos] = std::move(it->second);
        }
        pendingLoads.erase(it->first);
        it = readyChunks.erase(it);
        ++processed;
    }

    // Then process any remaining requests synchronously
    while (!loadQueue.empty() && processed < chunksPerFrame) {
        ChunkLoadRequest request = loadQueue.top();
        loadQueue.pop();

        // Remove from pending
        pendingLoads.erase(request.chunkPos);

        // Generate and add chunk to world
        ChunkPos pos(request.chunkPos.x, request.chunkPos.y);
        if (!world->getChunk(pos)) {
            world->loadChunk(pos);
        }

        ++processed;
    }

    // Process unload queue
    for (const auto& chunkPos : unloadQueue) {
        ChunkPos pos(chunkPos.x, chunkPos.y);
        world->unloadChunk(pos);
    }
    unloadQueue.clear();
}

void ChunkStreamer::generationThreadFunc()
{
    while (shouldRun) {
        ChunkLoadRequest request;
        bool hasWork = false;

        {
            std::unique_lock<std::mutex> lock(queueMutex);
            // Wait until there's work to do or we're shutting down
            queueCV.wait_for(lock, std::chrono::milliseconds(50),
                             [this] { return !backgroundQueue.empty() || !shouldRun; });

            if (!shouldRun) {
                break;
            }

            if (!backgroundQueue.empty()) {
                request = backgroundQueue.top();
                backgroundQueue.pop();
                hasWork = true;
            }
        }

        if (hasWork && world) {
            // Pre-generate chunk data in background thread
            // This is the expensive operation (terrain generation + mesh building)
            ChunkPos pos(request.chunkPos.x, request.chunkPos.y);

            auto chunk = std::make_unique<Chunk>(pos);
            {
                std::lock_guard<std::mutex> lock(worldMutex);
                // Access terrain generator through the world (thread-safe read)
                world->generateChunkData(chunk.get());
            }
            chunk->generateMesh();

            // Store in the ready cache for the main thread to pick up
            {
                std::lock_guard<std::mutex> lock(queueMutex);
                readyChunks[request.chunkPos] = std::move(chunk);
            }
        }
    }
}

glm::ivec2 ChunkStreamer::worldToChunk(const glm::vec3& worldPos) const
{
    // Use the global CHUNK_SIZE constant
    return glm::ivec2(static_cast<int>(std::floor(worldPos.x / CHUNK_SIZE)),
                      static_cast<int>(std::floor(worldPos.z / CHUNK_SIZE)));
}

float ChunkStreamer::chunkDistance(const glm::ivec2& a, const glm::ivec2& b) const
{
    int dx = a.x - b.x;
    int dz = a.y - b.y;
    return std::sqrt(static_cast<float>(dx * dx + dz * dz));
}

} // namespace fresh
