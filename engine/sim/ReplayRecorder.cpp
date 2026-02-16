#include "ReplayRecorder.h"
#include <fstream>

namespace fresh {

void ReplayRecorder::startRecording(uint32_t tickRate, uint32_t seed) {
    m_frames.clear();
    m_header = ReplayHeader{};
    m_header.tickRate = tickRate;
    m_header.seed = seed;
    m_state = ReplayState::Recording;
}

void ReplayRecorder::startFromSave(uint32_t saveTick, uint32_t tickRate, uint32_t seed) {
    m_frames.clear();
    m_header = ReplayHeader{};
    m_header.tickRate = tickRate;
    m_header.seed = seed;
    m_state = ReplayState::Recording;

    // Record an initial frame at saveTick
    ReplayFrame frame;
    frame.tick = saveTick;
    m_frames.push_back(std::move(frame));
    m_header.frameCount = 1;
}

void ReplayRecorder::recordFrame(uint32_t tick, const std::vector<uint8_t>& inputData) {
    recordFrame(tick, inputData, 0);
}

void ReplayRecorder::recordFrame(uint32_t tick, const std::vector<uint8_t>& inputData, uint64_t stateHash) {
    if (m_state != ReplayState::Recording) return;
    ReplayFrame frame;
    frame.tick = tick;
    frame.inputData = inputData;
    frame.stateHash = stateHash;
    m_frames.push_back(std::move(frame));
    m_header.frameCount = static_cast<uint32_t>(m_frames.size());
}

void ReplayRecorder::stopRecording() {
    m_state = ReplayState::Idle;
}

void ReplayRecorder::markSavePoint(uint32_t tick) {
    for (auto& frame : m_frames) {
        if (frame.tick == tick) {
            frame.isSavePoint = true;
            return;
        }
    }
}

std::vector<uint32_t> ReplayRecorder::savePoints() const {
    std::vector<uint32_t> result;
    for (const auto& frame : m_frames) {
        if (frame.isSavePoint) {
            result.push_back(frame.tick);
        }
    }
    return result;
}

bool ReplayRecorder::loadReplay(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) return false;

    ReplayHeader hdr;
    file.read(reinterpret_cast<char*>(&hdr), sizeof(hdr));
    if (hdr.magic != 0x52504C59) return false;

    m_header = hdr;
    m_frames.clear();
    m_frames.reserve(hdr.frameCount);

    for (uint32_t i = 0; i < hdr.frameCount; ++i) {
        ReplayFrame frame;
        file.read(reinterpret_cast<char*>(&frame.tick), sizeof(frame.tick));
        uint32_t dataSize = 0;
        file.read(reinterpret_cast<char*>(&dataSize), sizeof(dataSize));
        frame.inputData.resize(dataSize);
        if (dataSize > 0) {
            file.read(reinterpret_cast<char*>(frame.inputData.data()), dataSize);
        }
        if (hdr.version >= 2) {
            file.read(reinterpret_cast<char*>(&frame.stateHash), sizeof(frame.stateHash));
        }
        if (hdr.version >= 3) {
            uint8_t sp = 0;
            file.read(reinterpret_cast<char*>(&sp), sizeof(sp));
            frame.isSavePoint = (sp != 0);
        }
        m_frames.push_back(std::move(frame));
    }

    m_state = ReplayState::Playing;
    return true;
}

bool ReplayRecorder::saveReplay(const std::string& path) const {
    std::ofstream file(path, std::ios::binary);
    if (!file.is_open()) return false;

    file.write(reinterpret_cast<const char*>(&m_header), sizeof(m_header));

    for (const auto& frame : m_frames) {
        file.write(reinterpret_cast<const char*>(&frame.tick), sizeof(frame.tick));
        uint32_t dataSize = static_cast<uint32_t>(frame.inputData.size());
        file.write(reinterpret_cast<const char*>(&dataSize), sizeof(dataSize));
        if (dataSize > 0) {
            file.write(reinterpret_cast<const char*>(frame.inputData.data()), dataSize);
        }
        file.write(reinterpret_cast<const char*>(&frame.stateHash), sizeof(frame.stateHash));
        uint8_t sp = frame.isSavePoint ? 1 : 0;
        file.write(reinterpret_cast<const char*>(&sp), sizeof(sp));
    }

    return true;
}

const ReplayFrame* ReplayRecorder::frameAtTick(uint32_t tick) const {
    for (const auto& frame : m_frames) {
        if (frame.tick == tick) return &frame;
    }
    return nullptr;
}

ReplayState ReplayRecorder::state() const {
    return m_state;
}

const ReplayHeader& ReplayRecorder::header() const {
    return m_header;
}

const std::vector<ReplayFrame>& ReplayRecorder::frames() const {
    return m_frames;
}

size_t ReplayRecorder::frameCount() const {
    return m_frames.size();
}

uint32_t ReplayRecorder::durationTicks() const {
    if (m_frames.empty()) return 0;
    // Returns the tick value of the last recorded frame.
    // For standard recordings (starting at tick 0), this equals the duration.
    return m_frames.back().tick;
}

void ReplayRecorder::clear() {
    m_state = ReplayState::Idle;
    m_header = ReplayHeader{};
    m_frames.clear();
}

} // namespace fresh
