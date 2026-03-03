#pragma once
#include <cstdint>
#include <vector>
#include <deque>

namespace fresh::networking {

struct JitterEntry {
    uint32_t             tick        = 0;
    float                arrivalTime = 0.0f;
    std::vector<uint8_t> payload;
};

class JitterBuffer {
public:
    explicit JitterBuffer(float targetDelay   = 0.1f,
                          size_t maxBufferSize = 64,
                          bool   adaptive      = true);

    void push(uint32_t tick, float arrivalTime,
              const std::vector<uint8_t>& payload);
    std::vector<JitterEntry> flush(float currentTime);
    size_t bufferedCount() const { return m_buffer.size(); }
    float targetDelay() const { return m_targetDelay; }
    size_t maxBufferSize() const { return m_maxBufferSize; }
    bool isAdaptive() const { return m_adaptive; }
    uint64_t totalPushed() const { return m_totalPushed; }
    uint64_t totalDropped() const { return m_totalDropped; }
    void reset();

    static constexpr float kJitterAlpha     = 0.125f;
    static constexpr float kAdaptiveScale   = 2.0f;
    static constexpr float kMinTargetDelay  = 0.02f;
    static constexpr float kMaxTargetDelay  = 0.50f;

private:
    void adaptDelay(float interArrivalJitter);
    void trimOverflow();

    std::deque<JitterEntry> m_buffer;
    float    m_targetDelay;
    size_t   m_maxBufferSize;
    bool     m_adaptive;
    float    m_smoothedJitter   = 0.0f;
    float    m_lastArrivalTime  = -1.0f;
    float    m_lastInterArrival = 0.0f;
    bool     m_firstPush        = true;
    uint32_t m_lastReleasedTick = 0;
    uint64_t m_totalPushed      = 0;
    uint64_t m_totalDropped     = 0;
};

} // namespace fresh::networking
