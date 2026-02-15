#include "generation/WorldGenerationTask.h"

#include <iostream>
#include "core/Logger.h"
#include "voxel/VoxelWorld.h"
#include "voxel/VoxelTypes.h"

namespace fresh
{

WorldGenerationTask::WorldGenerationTask(const WorldCreationParams& params)
    : m_status(Status::NotStarted)
    , m_cancelRequested(false)
    , m_chunksGenerated(0)
    , m_totalChunks(0)
    , m_params(params)
{
}

WorldGenerationTask::~WorldGenerationTask()
{
    // Ensure thread is properly stopped before destruction
    if (m_thread && m_thread->joinable()) {
        cancel();
        waitForCompletion();
    }
}

bool WorldGenerationTask::start()
{
    // Check if already running
    Status expected = Status::NotStarted;
    if (!m_status.compare_exchange_strong(expected, Status::Running)) {
        LOG_WARNING_C("Cannot start world generation task - already running or completed", "WorldGenerationTask");
        return false;
    }

    // Start background thread
    try {
        m_thread = std::make_unique<std::thread>(&WorldGenerationTask::generateWorld, this);
        LOG_INFO_C("World generation task started in background thread", "WorldGenerationTask");
        return true;
    } catch (const std::exception& e) {
        m_status.store(Status::Failed);
        m_errorMessage = std::string("Failed to start generation thread: ") + e.what();
        LOG_ERROR_C(m_errorMessage, "WorldGenerationTask");
        
        // Notify failure
        if (m_completionCallback) {
            m_completionCallback(false, nullptr, m_errorMessage);
        }
        return false;
    }
}

void WorldGenerationTask::cancel()
{
    if (isRunning()) {
        LOG_INFO_C("Cancellation requested for world generation task", "WorldGenerationTask");
        m_cancelRequested.store(true);
    }
}

void WorldGenerationTask::waitForCompletion()
{
    if (m_thread && m_thread->joinable()) {
        m_thread->join();
    }
}

float WorldGenerationTask::getProgress() const
{
    int total = m_totalChunks.load();
    if (total <= 0) {
        return 0.0f;
    }
    
    int current = m_chunksGenerated.load();
    return static_cast<float>(current) / static_cast<float>(total);
}

std::unique_ptr<VoxelWorld> WorldGenerationTask::releaseWorld()
{
    if (m_status.load() == Status::Completed) {
        return std::move(m_world);
    }
    return nullptr;
}

void WorldGenerationTask::reportProgress(int current, int total, const std::string& message)
{
    m_chunksGenerated.store(current);
    m_totalChunks.store(total);
    
    if (m_progressCallback) {
        m_progressCallback(current, total, message);
    }
}

void WorldGenerationTask::generateWorld()
{
    LOG_INFO_C("Starting world generation: " + m_params.name + 
              " (seed: " + std::to_string(m_params.seed) + ")", "WorldGenerationTask");

    try {
        // Create voxel world
        m_world = std::make_unique<VoxelWorld>();
        if (!m_world->initialize()) {
            throw std::runtime_error("Failed to initialize voxel world");
        }

        reportProgress(0, 100, "Initializing world...");

        // Check for cancellation
        if (m_cancelRequested.load()) {
            LOG_INFO_C("World generation cancelled during initialization", "WorldGenerationTask");
            m_status.store(Status::Cancelled);
            if (m_completionCallback) {
                m_completionCallback(false, nullptr, "Cancelled by user");
            }
            return;
        }

        // Set custom seed for terrain generator
        m_world->setSeed(m_params.seed);
        
        // Set 2D world style if applicable
        if (!m_params.is3D) {
            m_world->set2DStyle(m_params.gameStyle2D);
        }

        reportProgress(0, 100, "Setting up world parameters...");

        // Generate initial chunks around spawn
        int chunkRadius = 3;
        int totalChunks = (2 * chunkRadius + 1) * (2 * chunkRadius + 1);
        m_totalChunks.store(totalChunks);
        int chunksGenerated = 0;

        LOG_INFO_C("Generating " + std::to_string(totalChunks) + " initial chunks", "WorldGenerationTask");

        for (int x = -chunkRadius; x <= chunkRadius; ++x) {
            for (int z = -chunkRadius; z <= chunkRadius; ++z) {
                // Check for cancellation before each chunk
                if (m_cancelRequested.load()) {
                    LOG_INFO_C("World generation cancelled at chunk " + 
                              std::to_string(chunksGenerated) + "/" + std::to_string(totalChunks), 
                              "WorldGenerationTask");
                    m_status.store(Status::Cancelled);
                    if (m_completionCallback) {
                        m_completionCallback(false, nullptr, "Cancelled by user");
                    }
                    return;
                }

                // Generate chunk
                m_world->loadChunk(ChunkPos(x, z));
                chunksGenerated++;
                m_chunksGenerated.store(chunksGenerated);

                // Report progress periodically
                if (chunksGenerated % 5 == 0 || chunksGenerated == totalChunks) {
                    std::string message = "Generating terrain: " + 
                                         std::to_string(chunksGenerated) + "/" + 
                                         std::to_string(totalChunks) + " chunks";
                    reportProgress(chunksGenerated, totalChunks, message);
                    
                    LOG_INFO_C(message, "WorldGenerationTask");
                }
            }
        }

        // Final progress update
        reportProgress(totalChunks, totalChunks, "World generation complete!");

        // Mark as completed
        m_status.store(Status::Completed);
        
        LOG_INFO_C("World generation completed successfully: " + m_params.name + 
                  " (" + std::to_string(totalChunks) + " chunks generated)", "WorldGenerationTask");

        // Notify completion
        if (m_completionCallback) {
            m_completionCallback(true, m_world.get(), "");
        }

    } catch (const std::exception& e) {
        m_errorMessage = std::string("World generation failed: ") + e.what();
        m_status.store(Status::Failed);
        
        LOG_ERROR_C(m_errorMessage, "WorldGenerationTask");
        
        // Notify failure
        if (m_completionCallback) {
            m_completionCallback(false, nullptr, m_errorMessage);
        }
    } catch (...) {
        m_errorMessage = "World generation failed: Unknown error";
        m_status.store(Status::Failed);
        
        LOG_ERROR_C(m_errorMessage, "WorldGenerationTask");
        
        // Notify failure
        if (m_completionCallback) {
            m_completionCallback(false, nullptr, m_errorMessage);
        }
    }
}

} // namespace fresh
