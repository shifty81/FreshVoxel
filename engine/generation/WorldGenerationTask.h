#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include <string>
#include <thread>

#include "ui/NativeMainMenu.h"  // For WorldCreationParams

namespace fresh
{

class VoxelWorld;

/**
 * @brief Asynchronous world generation task (Unreal Editor style)
 * 
 * This class manages world generation in a background thread,
 * similar to how Unreal Editor handles shader compilation and asset loading.
 * Provides progress callbacks, cancellation support, and completion notification.
 */
class WorldGenerationTask
{
public:
    /**
     * @brief Status of the world generation task
     */
    enum class Status
    {
        NotStarted,    ///< Task has not been started yet
        Running,       ///< Task is currently running
        Completed,     ///< Task completed successfully
        Cancelled,     ///< Task was cancelled by user
        Failed         ///< Task failed with an error
    };

    /**
     * @brief Progress callback function type
     * @param current Current number of chunks generated
     * @param total Total number of chunks to generate
     * @param message Progress message (e.g., "Generating terrain...")
     */
    using ProgressCallback = std::function<void(int current, int total, const std::string& message)>;

    /**
     * @brief Completion callback function type
     * @param success true if generation completed successfully, false otherwise
     * @param world Pointer to the generated world (nullptr on failure)
     * @param errorMessage Error message if failed, empty otherwise
     */
    using CompletionCallback = std::function<void(bool success, VoxelWorld* world, const std::string& errorMessage)>;

    /**
     * @brief Constructor
     * @param params World creation parameters
     */
    explicit WorldGenerationTask(const WorldCreationParams& params);

    /**
     * @brief Destructor - ensures thread is properly joined
     */
    ~WorldGenerationTask();

    // Prevent copying
    WorldGenerationTask(const WorldGenerationTask&) = delete;
    WorldGenerationTask& operator=(const WorldGenerationTask&) = delete;

    /**
     * @brief Set progress callback
     * @param callback Callback to be called with progress updates
     */
    void setProgressCallback(ProgressCallback callback)
    {
        m_progressCallback = callback;
    }

    /**
     * @brief Set completion callback
     * @param callback Callback to be called when generation completes
     */
    void setCompletionCallback(CompletionCallback callback)
    {
        m_completionCallback = callback;
    }

    /**
     * @brief Start the world generation task
     * @return true if task started successfully, false if already running
     */
    bool start();

    /**
     * @brief Request cancellation of the task
     * 
     * This is a cooperative cancellation - the task will check the flag
     * periodically and stop at the next safe point.
     */
    void cancel();

    /**
     * @brief Wait for the task to complete
     * 
     * Blocks the calling thread until generation finishes.
     * Safe to call multiple times.
     */
    void waitForCompletion();

    /**
     * @brief Check if task is currently running
     * @return true if running, false otherwise
     */
    bool isRunning() const
    {
        return m_status.load() == Status::Running;
    }

    /**
     * @brief Check if task has completed (successfully or not)
     * @return true if completed, cancelled, or failed
     */
    bool isComplete() const
    {
        Status status = m_status.load();
        return status == Status::Completed || status == Status::Cancelled || status == Status::Failed;
    }

    /**
     * @brief Get current task status
     * @return Current status
     */
    Status getStatus() const
    {
        return m_status.load();
    }

    /**
     * @brief Get current progress (0.0 to 1.0)
     * @return Progress as a float from 0.0 (not started) to 1.0 (complete)
     */
    float getProgress() const;

    /**
     * @brief Get the generated world
     * 
     * Only valid after successful completion.
     * Returns nullptr if generation hasn't completed or failed.
     * 
     * @return Pointer to generated world, or nullptr
     */
    VoxelWorld* getWorld() const
    {
        return m_world.get();
    }

    /**
     * @brief Transfer ownership of the generated world
     * 
     * Only valid after successful completion.
     * After calling this, getWorld() will return nullptr.
     * 
     * @return Unique pointer to generated world, or nullptr
     */
    std::unique_ptr<VoxelWorld> releaseWorld();

    /**
     * @brief Get error message if generation failed
     * @return Error message string, empty if no error
     */
    const std::string& getErrorMessage() const
    {
        return m_errorMessage;
    }

private:
    /**
     * @brief Main generation function that runs in background thread
     */
    void generateWorld();

    /**
     * @brief Report progress to callback (thread-safe)
     */
    void reportProgress(int current, int total, const std::string& message);

    // Task state
    std::atomic<Status> m_status;
    std::atomic<bool> m_cancelRequested;
    std::atomic<int> m_chunksGenerated;
    std::atomic<int> m_totalChunks;

    // World creation parameters
    WorldCreationParams m_params;

    // Generated world
    std::unique_ptr<VoxelWorld> m_world;

    // Error information
    std::string m_errorMessage;

    // Callbacks
    ProgressCallback m_progressCallback;
    CompletionCallback m_completionCallback;

    // Background thread
    std::unique_ptr<std::thread> m_thread;
};

} // namespace fresh
