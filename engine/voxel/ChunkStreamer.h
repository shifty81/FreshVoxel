#pragma once
#include <atomic>
#include <map>
#include <mutex>
#include <queue>
#include <set>
#include <thread>

#include <glm/glm.hpp>

namespace fresh
{

class VoxelWorld;
class Chunk;

/**
 * @brief Comparator for glm::ivec2 in STL containers
 */
struct IVec2Comparator {
    bool operator()(const glm::ivec2& a, const glm::ivec2& b) const
    {
        if (a.x != b.x)
            return a.x < b.x;
        return a.y < b.y;
    }
};

/**
 * @brief Request to load a chunk
 */
struct ChunkLoadRequest {
    glm::ivec2 chunkPos;
    int priority; // Distance from player (lower = higher priority)

    bool operator<(const ChunkLoadRequest& other) const
    {
        return priority > other.priority; // Reverse for priority queue
    }
};

/**
 * @brief Manages dynamic chunk loading/unloading for infinite world
 *
 * Loads chunks in a circular pattern around the player and unloads
 * distant chunks to manage memory. Uses background thread for generation.
 */
class ChunkStreamer
{
public:
    /**
     * @brief Construct chunk streamer
     * @param world VoxelWorld to manage chunks for
     */
    ChunkStreamer(VoxelWorld* world);
    ~ChunkStreamer();

    /**
     * @brief Update streamer based on player position
     * @param playerPosition Current player world position
     */
    void update(const glm::vec3& playerPosition);

    /**
     * @brief Set view distance in chunks
     * @param chunks Number of chunks to load around player
     */
    void setViewDistance(int chunks);
    int getViewDistance() const
    {
        return viewDistance;
    }

    /**
     * @brief Set maximum number of loaded chunks
     */
    void setMaxLoadedChunks(int max)
    {
        maxLoadedChunks = max;
    }
    int getMaxLoadedChunks() const
    {
        return maxLoadedChunks;
    }

    /**
     * @brief Shutdown background generation thread
     */
    void shutdown();

    /**
     * @brief Get number of currently loaded chunks
     */
    int getLoadedChunkCount() const;

    /**
     * @brief Check if chunk is loaded
     */
    bool isChunkLoaded(const glm::ivec2& chunkPos) const;

private:
    void determineChunksToLoad(const glm::vec3& playerPos);
    void determineChunksToUnload(const glm::vec3& playerPos);
    void processLoadQueue();

    // Background generation thread
    void generationThreadFunc();

    // Convert world position to chunk position
    glm::ivec2 worldToChunk(const glm::vec3& worldPos) const;

    // Calculate distance between chunk positions
    float chunkDistance(const glm::ivec2& a, const glm::ivec2& b) const;

    VoxelWorld* world;
    int viewDistance = 8; // chunks
    int maxLoadedChunks = 1000;
    int chunksPerFrame = 2; // Max chunks to process per frame

    std::priority_queue<ChunkLoadRequest> loadQueue;
    std::set<glm::ivec2, IVec2Comparator> pendingLoads;
    std::vector<glm::ivec2> unloadQueue;

    // Threading
    std::thread generationThread;
    std::mutex queueMutex;
    std::mutex worldMutex;
    std::atomic<bool> shouldRun{true};

    glm::ivec2 lastPlayerChunk{0, 0};
};

} // namespace fresh
