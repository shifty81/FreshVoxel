#include "TickScheduler.h"
#include <thread>

namespace fresh {

void TickScheduler::setTickRate(uint32_t hz) {
    m_tickRate = hz > 0 ? hz : 1;
}

uint32_t TickScheduler::tickRate() const {
    return m_tickRate;
}

float TickScheduler::fixedDeltaTime() const {
    return 1.0f / static_cast<float>(m_tickRate);
}

void TickScheduler::tick(const std::function<void(float)>& callback) {
    if (m_framePacing) {
        auto now = std::chrono::steady_clock::now();
        if (m_firstTick) {
            m_nextTick = now;
            m_firstTick = false;
        }

        if (now < m_nextTick) {
            std::this_thread::sleep_until(m_nextTick);
        }

        auto tickInterval = std::chrono::microseconds(1000000 / m_tickRate);
        m_nextTick += tickInterval;

        // Prevent spiral of death: if we fell behind more than one full tick,
        // reset the target to now + one interval
        now = std::chrono::steady_clock::now();
        if (m_nextTick < now) {
            m_nextTick = now + tickInterval;
        }
    }

    if (callback) {
        callback(fixedDeltaTime());
    }
    m_currentTick++;
}

uint64_t TickScheduler::currentTick() const {
    return m_currentTick;
}

void TickScheduler::setFramePacing(bool enabled) {
    m_framePacing = enabled;
    if (enabled) {
        m_firstTick = true;
    }
}

bool TickScheduler::framePacingEnabled() const {
    return m_framePacing;
}

} // namespace fresh
