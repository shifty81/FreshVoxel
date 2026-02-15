#pragma once
#include <chrono>
#include <map>
#include <string>
#include <vector>

namespace fresh
{
namespace devtools
{

/**
 * @brief Performance metrics tracking (FPS and frame timing)
 */
class PerformanceProfiler
{
public:
    PerformanceProfiler();
    ~PerformanceProfiler();

    // Frame tracking
    void beginFrame();
    void endFrame();

    // Get metrics
    float getFPS() const
    {
        return fps;
    }
    float getAverageFrameTime() const
    {
        return avgFrameTime;
    }
    float getMinFrameTime() const
    {
        return minFrameTime;
    }
    float getMaxFrameTime() const
    {
        return maxFrameTime;
    }

    // Named sections for profiling specific parts of code
    void beginSection(const std::string& name);
    void endSection(const std::string& name);
    float getSectionTime(const std::string& name) const;

    // Reset statistics
    void reset();

    // Get profiling report
    std::string getReport() const;

private:
    std::chrono::high_resolution_clock::time_point frameStartTime;
    std::chrono::high_resolution_clock::time_point lastFrameTime;

    std::vector<float> frameTimes;
    size_t maxSamples;

    float fps;
    float avgFrameTime;
    float minFrameTime;
    float maxFrameTime;

    // Section profiling
    struct SectionProfile {
        std::string name;
        std::chrono::high_resolution_clock::time_point startTime;
        float totalTime;
        int callCount;
    };
    std::map<std::string, SectionProfile> sections;

    void updateStatistics();
};

} // namespace devtools
} // namespace fresh
