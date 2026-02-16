#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace fresh {

struct ReplayFrame {
    uint32_t tick = 0;
    std::vector<uint8_t> inputData;
    uint64_t stateHash = 0;  ///< Hash ladder value at this tick (0 = not recorded)
    bool isSavePoint = false; ///< True if this frame is a save-point marker
};

struct ReplayHeader {
    uint32_t magic = 0x52504C59;  // "RPLY"
    uint32_t version = 3;         // v3: added isSavePoint per frame
    uint32_t tickRate = 30;
    uint32_t frameCount = 0;
    uint32_t seed = 0;
};

enum class ReplayState {
    Idle,
    Recording,
    Playing,
};

class ReplayRecorder {
public:
    // Recording
    void startRecording(uint32_t tickRate, uint32_t seed = 0);
    void startFromSave(uint32_t saveTick, uint32_t tickRate, uint32_t seed);
    void recordFrame(uint32_t tick, const std::vector<uint8_t>& inputData);
    void recordFrame(uint32_t tick, const std::vector<uint8_t>& inputData, uint64_t stateHash);
    void stopRecording();

    // Playback
    bool loadReplay(const std::string& path);
    bool saveReplay(const std::string& path) const;
    const ReplayFrame* frameAtTick(uint32_t tick) const;

    // Save-point markers
    void markSavePoint(uint32_t tick);
    std::vector<uint32_t> savePoints() const;

    // Query
    ReplayState state() const;
    const ReplayHeader& header() const;
    const std::vector<ReplayFrame>& frames() const;
    size_t frameCount() const;
    uint32_t durationTicks() const;

    // Reset
    void clear();

private:
    ReplayState m_state = ReplayState::Idle;
    ReplayHeader m_header;
    std::vector<ReplayFrame> m_frames;
};

} // namespace fresh
