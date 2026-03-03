#pragma once
#include <cstdint>
#include <deque>

namespace fresh::networking {

class NetworkQualityMonitor {
public:
    void recordRTT(float rttSeconds);
    void recordPacketArrival(uint32_t sequenceNumber);
    float getSmoothedRTT() const { return m_smoothedRTT; }
    float getJitter() const { return m_jitter; }
    float getPacketLossRate() const;
    float getAdaptiveInterpolationTime() const;
    void reset();

    static constexpr float kRTTAlpha          = 0.125f;
    static constexpr float kJitterAlpha        = 0.0625f;
    static constexpr float kMinInterpTime      = 0.05f;
    static constexpr float kMaxInterpTime      = 0.30f;
    static constexpr float kJitterMultiplier   = 2.0f;
    static constexpr uint32_t kLossWindowSize  = 100;

private:
    float m_smoothedRTT = 0.0f;
    float m_jitter      = 0.0f;
    bool  m_firstRTT    = true;
    uint32_t m_expectedSeq     = 0;
    bool     m_seqInitialized  = false;
    struct SeqRecord { uint32_t seq; bool received; };
    std::deque<SeqRecord> m_seqWindow;
};

} // namespace fresh::networking
