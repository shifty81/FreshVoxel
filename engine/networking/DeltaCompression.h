#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
#include "networking/NetworkInterpolationBuffer.h"

namespace fresh::networking {

enum class FrameType : uint8_t {
    Keyframe = 0,
    Delta    = 1
};

struct FieldDelta {
    int32_t dPosX = 0;
    int32_t dPosY = 0;
    int32_t dPosZ = 0;
    int32_t dVelX = 0;
    int32_t dVelY = 0;
    int32_t dVelZ = 0;
    int32_t dRotYaw = 0;
};

struct CompressedSnapshot {
    uint32_t  entityId  = 0;
    uint32_t  tick      = 0;
    FrameType frameType = FrameType::Keyframe;
    FieldDelta delta;
};

struct CompressedFrame {
    uint32_t tick = 0;
    std::vector<CompressedSnapshot> entries;
};

class DeltaCompression {
public:
    explicit DeltaCompression(uint32_t keyframeInterval = 30);

    CompressedFrame encode(const std::vector<EntitySnapshot>& snapshots);
    std::vector<EntitySnapshot> decode(const CompressedFrame& frame);
    void forceKeyframe(uint32_t entityId);
    void forceAllKeyframes();
    void removeEntity(uint32_t entityId);
    void clear();
    size_t baselineCount() const { return m_baselines.size(); }
    uint32_t keyframeInterval() const { return m_keyframeInterval; }

    static int32_t quantizePosition(float v);
    static float   dequantizePosition(int32_t v);
    static int32_t quantizeRotation(float deg);
    static float   dequantizeRotation(int32_t v);

    static constexpr float kPositionScale = 100.0f;
    static constexpr float kRotationScale = 10.0f;

private:
    struct Baseline {
        EntitySnapshot snapshot;
        uint32_t lastKeyframeTick = 0;
        bool     forceKeyframe    = false;
    };

    bool needsKeyframe(uint32_t entityId, uint32_t currentTick) const;

    std::unordered_map<uint32_t, Baseline> m_baselines;
    uint32_t m_keyframeInterval;
    bool m_forceAll = false;
};

} // namespace fresh::networking
