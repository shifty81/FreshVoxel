#pragma once
#include <cstdint>
#include <vector>
#include <deque>
#include <unordered_map>
#include <cmath>
#include "networking/NetworkInterpolationBuffer.h"

namespace fresh::networking {

struct HitTestResult {
    bool     hit       = false;
    uint32_t entityId  = 0;
    uint32_t rewindTick = 0;
    float    distance  = 0.0f;
};

class LagCompensation {
public:
    explicit LagCompensation(size_t historyDuration = 64,
                              uint32_t maxRewindTicks = 40);

    void storeSnapshot(const EntitySnapshot& snap);
    InterpolatedState getStateAtTick(uint32_t entityId, float tick) const;
    HitTestResult hitTest(uint32_t targetId,
                          float originX, float originY, float originZ,
                          uint32_t rewindTick,
                          float hitRadius) const;
    void removeEntity(uint32_t entityId);
    void clear();
    size_t trackedEntityCount() const { return m_history.size(); }
    size_t snapshotCount(uint32_t entityId) const;
    size_t historyDuration() const { return m_historyDuration; }
    uint32_t maxRewindTicks() const { return m_maxRewindTicks; }
    uint32_t currentTick() const { return m_currentTick; }

    static constexpr float kDefaultHitRadius = 5.0f;

private:
    struct EntityHistory {
        uint32_t entityId = 0;
        std::deque<EntitySnapshot> snapshots;
    };

    EntityHistory*       findEntity(uint32_t id);
    const EntityHistory* findEntity(uint32_t id) const;
    void prune(EntityHistory& eh) const;
    uint32_t clampRewindTick(uint32_t requested) const;

    std::unordered_map<uint32_t, EntityHistory> m_history;
    size_t   m_historyDuration;
    uint32_t m_maxRewindTicks;
    uint32_t m_currentTick = 0;
};

} // namespace fresh::networking
