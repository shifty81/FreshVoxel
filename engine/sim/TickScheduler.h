#pragma once
#include <cstdint>
#include <chrono>
#include <functional>

namespace fresh {

class TickScheduler {
public:
    void setTickRate(uint32_t hz);
    uint32_t tickRate() const;

    float fixedDeltaTime() const;

    void tick(const std::function<void(float)>& callback);
    uint64_t currentTick() const;

    // Frame pacing control
    void setFramePacing(bool enabled);
    bool framePacingEnabled() const;

private:
    uint32_t m_tickRate = 30;
    uint64_t m_currentTick = 0;
    bool m_framePacing = true;
    std::chrono::steady_clock::time_point m_nextTick{};
    bool m_firstTick = true;
};

} // namespace fresh
