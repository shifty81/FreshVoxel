#pragma once
#include <cstdint>
#include <vector>
#include <cmath>

namespace fresh::networking {

struct EntitySnapshot {
    uint32_t entityId = 0;
    uint32_t tick     = 0;
    float    posX     = 0.0f;
    float    posY     = 0.0f;
    float    posZ     = 0.0f;
    float    velX     = 0.0f;
    float    velY     = 0.0f;
    float    velZ     = 0.0f;
    float    rotYaw   = 0.0f;
};

struct InterpolatedState {
    uint32_t entityId = 0;
    float    posX     = 0.0f;
    float    posY     = 0.0f;
    float    posZ     = 0.0f;
    float    rotYaw   = 0.0f;
    bool     valid    = false;
};

class NetworkInterpolationBuffer {
public:
    explicit NetworkInterpolationBuffer(size_t bufferDepth = 8,
                                         float maxExtrapolationTicks = 3.0f);

    void pushSnapshot(const EntitySnapshot& snap);
    InterpolatedState interpolate(uint32_t entityId, float renderTick) const;
    void removeEntity(uint32_t entityId);
    void clear();
    size_t trackedEntityCount() const { return m_entities.size(); }
    size_t snapshotCount(uint32_t entityId) const;
    size_t bufferDepth() const { return m_bufferDepth; }
    float maxExtrapolationTicks() const { return m_maxExtrapolationTicks; }

private:
    struct EntityBuffer {
        uint32_t entityId = 0;
        std::vector<EntitySnapshot> snapshots;
    };

    EntityBuffer*       findEntity(uint32_t id);
    const EntityBuffer* findEntity(uint32_t id) const;

    static InterpolatedState lerp(const EntitySnapshot& a,
                                  const EntitySnapshot& b,
                                  float t);
    static InterpolatedState extrapolate(const EntitySnapshot& snap,
                                         float ticksBeyond);

    std::vector<EntityBuffer> m_entities;
    size_t m_bufferDepth;
    float  m_maxExtrapolationTicks;
};

} // namespace fresh::networking
