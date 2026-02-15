#include "devtools/MemoryTracker.h"

#include <iomanip>
#include <sstream>

namespace fresh
{
namespace devtools
{

MemoryTracker::MemoryTracker() : totalAllocated(0), peakUsage(0) {}

MemoryTracker::~MemoryTracker() {}

void MemoryTracker::recordAllocation(const std::string& category, size_t bytes)
{
    categoryUsage[category] += bytes;
    totalAllocated += bytes;

    if (totalAllocated > peakUsage) {
        peakUsage = totalAllocated;
    }
}

void MemoryTracker::recordDeallocation(const std::string& category, size_t bytes)
{
    auto it = categoryUsage.find(category);
    if (it != categoryUsage.end()) {
        if (it->second >= bytes) {
            it->second -= bytes;
        } else {
            it->second = 0;
        }
    }

    if (totalAllocated >= bytes) {
        totalAllocated -= bytes;
    } else {
        totalAllocated = 0;
    }
}

size_t MemoryTracker::getTotalAllocated() const
{
    return totalAllocated;
}

size_t MemoryTracker::getCategoryAllocated(const std::string& category) const
{
    auto it = categoryUsage.find(category);
    return (it != categoryUsage.end()) ? it->second : 0;
}

size_t MemoryTracker::getGPUMemoryUsed() const
{
    // Platform-specific implementation would query GPU memory
    // For now, return 0
    return 0;
}

size_t MemoryTracker::getGPUMemoryTotal() const
{
    // Platform-specific implementation would query GPU memory
    // For now, return 0
    return 0;
}

void MemoryTracker::reset()
{
    categoryUsage.clear();
    totalAllocated = 0;
    peakUsage = 0;
}

std::string MemoryTracker::getReport() const
{
    std::ostringstream oss;
    oss << "=== Memory Report ===\n";
    oss << "Total Allocated: " << (totalAllocated / 1024.0f / 1024.0f) << " MB\n";
    oss << "Peak Usage: " << (peakUsage / 1024.0f / 1024.0f) << " MB\n";

    if (!categoryUsage.empty()) {
        oss << "\n=== Category Breakdown ===\n";
        for (const auto& pair : categoryUsage) {
            float mb = pair.second / 1024.0f / 1024.0f;
            oss << pair.first << ": " << mb << " MB\n";
        }
    }

    size_t gpuUsed = getGPUMemoryUsed();
    size_t gpuTotal = getGPUMemoryTotal();
    if (gpuTotal > 0) {
        oss << "\n=== GPU Memory ===\n";
        oss << "Used: " << (gpuUsed / 1024.0f / 1024.0f) << " MB\n";
        oss << "Total: " << (gpuTotal / 1024.0f / 1024.0f) << " MB\n";
    }

    return oss.str();
}

} // namespace devtools
} // namespace fresh
