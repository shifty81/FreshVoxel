#include "devtools/PerformanceProfiler.h"

#include <algorithm>
#include <iomanip>
#include <numeric>
#include <sstream>

namespace fresh
{
namespace devtools
{

PerformanceProfiler::PerformanceProfiler()
    : maxSamples(60), fps(0.0f), avgFrameTime(0.0f), minFrameTime(0.0f), maxFrameTime(0.0f)
{
    frameTimes.reserve(maxSamples);
}

PerformanceProfiler::~PerformanceProfiler() {}

void PerformanceProfiler::beginFrame()
{
    frameStartTime = std::chrono::high_resolution_clock::now();
}

void PerformanceProfiler::endFrame()
{
    auto frameEndTime = std::chrono::high_resolution_clock::now();
    auto duration =
        std::chrono::duration_cast<std::chrono::microseconds>(frameEndTime - frameStartTime);

    float frameTimeMs = duration.count() / 1000.0f;
    frameTimes.push_back(frameTimeMs);

    if (frameTimes.size() > maxSamples) {
        frameTimes.erase(frameTimes.begin());
    }

    updateStatistics();
    lastFrameTime = frameEndTime;
}

void PerformanceProfiler::beginSection(const std::string& name)
{
    auto& section = sections[name];
    section.name = name;
    section.startTime = std::chrono::high_resolution_clock::now();
}

void PerformanceProfiler::endSection(const std::string& name)
{
    auto endTime = std::chrono::high_resolution_clock::now();
    auto it = sections.find(name);

    if (it != sections.end()) {
        auto duration =
            std::chrono::duration_cast<std::chrono::microseconds>(endTime - it->second.startTime);
        it->second.totalTime += duration.count() / 1000.0f;
        it->second.callCount++;
    }
}

float PerformanceProfiler::getSectionTime(const std::string& name) const
{
    auto it = sections.find(name);
    return (it != sections.end()) ? it->second.totalTime : 0.0f;
}

void PerformanceProfiler::reset()
{
    frameTimes.clear();
    sections.clear();
    fps = 0.0f;
    avgFrameTime = 0.0f;
    minFrameTime = 0.0f;
    maxFrameTime = 0.0f;
}

std::string PerformanceProfiler::getReport() const
{
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2);
    oss << "=== Performance Report ===\n";
    oss << "FPS: " << fps << "\n";
    oss << "Avg Frame Time: " << avgFrameTime << " ms\n";
    oss << "Min Frame Time: " << minFrameTime << " ms\n";
    oss << "Max Frame Time: " << maxFrameTime << " ms\n";

    if (!sections.empty()) {
        oss << "\n=== Section Timings ===\n";
        for (const auto& pair : sections) {
            const auto& section = pair.second;
            float avgTime = section.callCount > 0 ? section.totalTime / section.callCount : 0.0f;
            oss << section.name << ": " << avgTime << " ms (calls: " << section.callCount << ")\n";
        }
    }

    return oss.str();
}

void PerformanceProfiler::updateStatistics()
{
    if (frameTimes.empty()) {
        return;
    }

    // Calculate average
    avgFrameTime = std::accumulate(frameTimes.begin(), frameTimes.end(), 0.0f) / frameTimes.size();

    // Calculate min and max
    minFrameTime = *std::min_element(frameTimes.begin(), frameTimes.end());
    maxFrameTime = *std::max_element(frameTimes.begin(), frameTimes.end());

    // Calculate FPS
    fps = (avgFrameTime > 0.0f) ? (1000.0f / avgFrameTime) : 0.0f;
}

} // namespace devtools
} // namespace fresh
