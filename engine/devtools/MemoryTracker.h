#pragma once
#include <cstddef>
#include <map>
#include <string>

namespace fresh
{
namespace devtools
{

/**
 * @brief Memory usage monitoring (including GPU when available)
 */
class MemoryTracker
{
public:
    MemoryTracker();
    ~MemoryTracker();

    // Memory tracking
    void recordAllocation(const std::string& category, size_t bytes);
    void recordDeallocation(const std::string& category, size_t bytes);

    // Get memory usage
    size_t getTotalAllocated() const;
    size_t getCategoryAllocated(const std::string& category) const;
    size_t getPeakMemoryUsage() const
    {
        return peakUsage;
    }

    // GPU memory (requires platform-specific implementation)
    size_t getGPUMemoryUsed() const;
    size_t getGPUMemoryTotal() const;

    // Get all categories
    std::map<std::string, size_t> getAllCategories() const
    {
        return categoryUsage;
    }

    // Reset tracking
    void reset();

    // Get report
    std::string getReport() const;

private:
    std::map<std::string, size_t> categoryUsage;
    size_t totalAllocated;
    size_t peakUsage;
};

} // namespace devtools
} // namespace fresh
