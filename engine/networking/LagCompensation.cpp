#include "networking/LagCompensation.h"
#include <algorithm>

namespace fresh::networking {

LagCompensation::LagCompensation(size_t historyDuration,
                                  uint32_t maxRewindTicks)
    : m_historyDuration(historyDuration > 0 ? historyDuration : 1)
    , m_maxRewindTicks(maxRewindTicks)
{}

void LagCompensation::storeSnapshot(const EntitySnapshot& snap) {
    if (snap.tick > m_currentTick) {
        m_currentTick = snap.tick;
    }

    auto& eh = m_history[snap.entityId];
    eh.entityId = snap.entityId;

    if (eh.snapshots.empty() || snap.tick >= eh.snapshots.back().tick) {
        eh.snapshots.push_back(snap);
    } else {
        auto it = std::lower_bound(
            eh.snapshots.begin(), eh.snapshots.end(), snap.tick,
            [](const EntitySnapshot& s, uint32_t t) { return s.tick < t; });
        eh.snapshots.insert(it, snap);
    }

    prune(eh);
}

InterpolatedState LagCompensation::getStateAtTick(uint32_t entityId,
                                                   float tick) const
{
    InterpolatedState result;
    result.entityId = entityId;

    const EntityHistory* eh = findEntity(entityId);
    if (!eh || eh->snapshots.empty()) return result;

    const auto& snaps = eh->snapshots;
    float firstTick = static_cast<float>(snaps.front().tick);
    float lastTick  = static_cast<float>(snaps.back().tick);

    if (tick <= firstTick) {
        const auto& s = snaps.front();
        result.posX   = s.posX;
        result.posY   = s.posY;
        result.posZ   = s.posZ;
        result.rotYaw = s.rotYaw;
        result.valid  = (tick == firstTick);
        return result;
    }

    if (tick >= lastTick) {
        const auto& s = snaps.back();
        result.posX   = s.posX;
        result.posY   = s.posY;
        result.posZ   = s.posZ;
        result.rotYaw = s.rotYaw;
        result.valid  = true;
        return result;
    }

    for (size_t i = 0; i + 1 < snaps.size(); ++i) {
        float tA = static_cast<float>(snaps[i].tick);
        float tB = static_cast<float>(snaps[i + 1].tick);
        if (tick >= tA && tick <= tB) {
            float span = tB - tA;
            float t    = (span > 0.0f) ? (tick - tA) / span : 0.0f;

            const auto& a = snaps[i];
            const auto& b = snaps[i + 1];
            result.posX   = a.posX   + (b.posX   - a.posX)   * t;
            result.posY   = a.posY   + (b.posY   - a.posY)   * t;
            result.posZ   = a.posZ   + (b.posZ   - a.posZ)   * t;
            result.rotYaw = a.rotYaw + (b.rotYaw - a.rotYaw) * t;
            result.valid  = true;
            return result;
        }
    }

    return result;
}

HitTestResult LagCompensation::hitTest(uint32_t targetId,
                                        float originX, float originY,
                                        float originZ,
                                        uint32_t rewindTick,
                                        float hitRadius) const
{
    HitTestResult result;
    result.entityId = targetId;

    uint32_t clampedTick = clampRewindTick(rewindTick);
    result.rewindTick    = clampedTick;

    InterpolatedState state = getStateAtTick(targetId,
                                             static_cast<float>(clampedTick));
    if (!state.valid) return result;

    float dx = state.posX - originX;
    float dy = state.posY - originY;
    float dz = state.posZ - originZ;
    float distSq = dx * dx + dy * dy + dz * dz;
    float dist   = std::sqrt(distSq);

    result.distance = dist;
    result.hit      = (dist <= hitRadius);
    return result;
}

void LagCompensation::removeEntity(uint32_t entityId) {
    m_history.erase(entityId);
}

void LagCompensation::clear() {
    m_history.clear();
    m_currentTick = 0;
}

size_t LagCompensation::snapshotCount(uint32_t entityId) const {
    const EntityHistory* eh = findEntity(entityId);
    return eh ? eh->snapshots.size() : 0;
}

LagCompensation::EntityHistory* LagCompensation::findEntity(uint32_t id) {
    auto it = m_history.find(id);
    return (it != m_history.end()) ? &it->second : nullptr;
}

const LagCompensation::EntityHistory* LagCompensation::findEntity(
    uint32_t id) const
{
    auto it = m_history.find(id);
    return (it != m_history.end()) ? &it->second : nullptr;
}

void LagCompensation::prune(EntityHistory& eh) const {
    while (eh.snapshots.size() > m_historyDuration) {
        eh.snapshots.pop_front();
    }
}

uint32_t LagCompensation::clampRewindTick(uint32_t requested) const {
    if (m_currentTick == 0) return requested;

    uint32_t minTick = (m_currentTick > m_maxRewindTicks)
                       ? m_currentTick - m_maxRewindTicks
                       : 0;
    if (requested < minTick) return minTick;
    if (requested > m_currentTick) return m_currentTick;
    return requested;
}

} // namespace fresh::networking
