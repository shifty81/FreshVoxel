#include "core/MemoryManager.h"

#include <cstring>
#include <iostream>

namespace fresh
{

MemoryManager& MemoryManager::getInstance()
{
    static MemoryManager instance;
    return instance;
}

void* MemoryManager::allocate(size_t size, const char* tag)
{
    std::lock_guard<std::mutex> lock(mutex);

    void* ptr = ::operator new(size);

    AllocationInfo info;
    info.size = size;
    info.tag = tag;
    allocations[ptr] = info;

    totalAllocated += size;
    allocationCount++;

    if (totalAllocated > peakUsage) {
        peakUsage = totalAllocated;
    }

    return ptr;
}

void MemoryManager::deallocate(void* ptr)
{
    if (!ptr)
        return;

    std::lock_guard<std::mutex> lock(mutex);

    auto it = allocations.find(ptr);
    if (it != allocations.end()) {
        totalAllocated -= it->second.size;
        allocationCount--;
        allocations.erase(it);
    }

    ::operator delete(ptr);
}

void MemoryManager::printStats() const
{
    std::lock_guard<std::mutex> lock(mutex);

    std::cout << "=== Memory Statistics ===" << std::endl;
    std::cout << "Total Allocated: " << (totalAllocated / 1024) << " KB" << std::endl;
    std::cout << "Peak Usage: " << (peakUsage / 1024) << " KB" << std::endl;
    std::cout << "Active Allocations: " << allocationCount << std::endl;

    // Group by tag
    std::map<std::string, size_t> taggedSizes;
    for (const auto& [ptr, info] : allocations) {
        taggedSizes[info.tag] += info.size;
    }

    std::cout << "\nAllocations by tag:" << std::endl;
    for (const auto& [tag, size] : taggedSizes) {
        std::cout << "  " << tag << ": " << (size / 1024) << " KB" << std::endl;
    }
}

void MemoryManager::checkLeaks() const
{
    std::lock_guard<std::mutex> lock(mutex);

    if (!allocations.empty()) {
        std::cout << "=== MEMORY LEAKS DETECTED ===" << std::endl;
        std::cout << "Leaked allocations: " << allocations.size() << std::endl;

        for (const auto& [ptr, info] : allocations) {
            std::cout << "  Leak at " << ptr << ": " << info.size << " bytes [" << info.tag << "]"
                      << std::endl;
        }
    }
}

// StackAllocator implementation
StackAllocator::StackAllocator(size_t size) : totalSize(size), offset(0)
{
    memory = ::operator new(size);
}

StackAllocator::~StackAllocator()
{
    ::operator delete(memory);
}

void* StackAllocator::allocate(size_t size, size_t alignment)
{
    // Align offset
    size_t padding = 0;
    size_t currentAddress = reinterpret_cast<size_t>(memory) + offset;
    size_t misalignment = currentAddress % alignment;

    if (misalignment != 0) {
        padding = alignment - misalignment;
    }

    if (offset + padding + size > totalSize) {
        return nullptr; // Out of memory
    }

    offset += padding;
    void* ptr = static_cast<char*>(memory) + offset;
    offset += size;

    return ptr;
}

void StackAllocator::clear()
{
    offset = 0;
}

} // namespace fresh
