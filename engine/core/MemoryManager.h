#pragma once
#include <cstddef>
#include <map>
#include <memory>
#include <mutex>
#include <vector>

namespace fresh
{

/**
 * @brief Memory pool for efficient allocation of same-sized objects
 */
template <typename T, size_t PoolSize = 1024>
class MemoryPool
{
public:
    MemoryPool()
    {
        allocateBlock();
    }

    ~MemoryPool()
    {
        for (auto block : blocks) {
            ::operator delete(block);
        }
    }

    T* allocate()
    {
        std::lock_guard<std::mutex> lock(mutex);

        if (freeList.empty()) {
            allocateBlock();
        }

        T* obj = freeList.back();
        freeList.pop_back();
        return obj;
    }

    void deallocate(T* obj)
    {
        std::lock_guard<std::mutex> lock(mutex);
        freeList.push_back(obj);
    }

private:
    void allocateBlock()
    {
        // Allocate raw memory for a block
        void* block = ::operator new(sizeof(T) * PoolSize);
        blocks.push_back(block);

        // Add all objects in block to free list
        for (size_t i = 0; i < PoolSize; ++i) {
            T* obj = static_cast<T*>(block) + i;
            freeList.push_back(obj);
        }
    }

    std::vector<void*> blocks;
    std::vector<T*> freeList;
    std::mutex mutex;
};

/**
 * @brief Central memory management system
 *
 * Provides efficient memory allocation strategies, tracking,
 * and debugging features to prevent fragmentation and leaks.
 */
class MemoryManager
{
public:
    static MemoryManager& getInstance();

    /**
     * @brief Allocate memory with tracking
     */
    void* allocate(size_t size, const char* tag = "Unknown");

    /**
     * @brief Free tracked memory
     */
    void deallocate(void* ptr);

    /**
     * @brief Get total allocated memory in bytes
     */
    size_t getTotalAllocated() const
    {
        return totalAllocated;
    }

    /**
     * @brief Get peak memory usage
     */
    size_t getPeakUsage() const
    {
        return peakUsage;
    }

    /**
     * @brief Get number of active allocations
     */
    size_t getAllocationCount() const
    {
        return allocationCount;
    }

    /**
     * @brief Print memory statistics
     */
    void printStats() const;

    /**
     * @brief Detect memory leaks (call on shutdown)
     */
    void checkLeaks() const;

private:
    MemoryManager() = default;
    ~MemoryManager() = default;
    MemoryManager(const MemoryManager&) = delete;
    MemoryManager& operator=(const MemoryManager&) = delete;

    struct AllocationInfo {
        size_t size;
        const char* tag;
    };

    std::map<void*, AllocationInfo> allocations;
    size_t totalAllocated = 0;
    size_t peakUsage = 0;
    size_t allocationCount = 0;
    mutable std::mutex mutex;
};

/**
 * @brief Stack allocator for temporary allocations
 */
class StackAllocator
{
public:
    StackAllocator(size_t size);
    ~StackAllocator();

    void* allocate(size_t size, size_t alignment = 16);
    void clear(); // Reset stack to beginning

    size_t getUsedMemory() const
    {
        return offset;
    }
    size_t getTotalMemory() const
    {
        return totalSize;
    }

private:
    void* memory;
    size_t totalSize;
    size_t offset;
};

} // namespace fresh
