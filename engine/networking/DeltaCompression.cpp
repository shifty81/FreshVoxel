#include "networking/DeltaCompression.h"
#include <cmath>

namespace fresh::networking {

DeltaCompression::DeltaCompression(uint32_t keyframeInterval)
    : m_keyframeInterval(keyframeInterval > 0 ? keyframeInterval : 1)
{}

int32_t DeltaCompression::quantizePosition(float v) {
    return static_cast<int32_t>(std::round(v * kPositionScale));
}

float DeltaCompression::dequantizePosition(int32_t v) {
    return static_cast<float>(v) / kPositionScale;
}

int32_t DeltaCompression::quantizeRotation(float deg) {
    return static_cast<int32_t>(std::round(deg * kRotationScale));
}

float DeltaCompression::dequantizeRotation(int32_t v) {
    return static_cast<float>(v) / kRotationScale;
}

CompressedFrame DeltaCompression::encode(
    const std::vector<EntitySnapshot>& snapshots)
{
    CompressedFrame frame;
    if (!snapshots.empty()) {
        frame.tick = snapshots.front().tick;
    }

    for (const auto& snap : snapshots) {
        CompressedSnapshot cs;
        cs.entityId = snap.entityId;
        cs.tick     = snap.tick;

        if (needsKeyframe(snap.entityId, snap.tick)) {
            cs.frameType        = FrameType::Keyframe;
            cs.delta.dPosX      = quantizePosition(snap.posX);
            cs.delta.dPosY      = quantizePosition(snap.posY);
            cs.delta.dPosZ      = quantizePosition(snap.posZ);
            cs.delta.dVelX      = quantizePosition(snap.velX);
            cs.delta.dVelY      = quantizePosition(snap.velY);
            cs.delta.dVelZ      = quantizePosition(snap.velZ);
            cs.delta.dRotYaw    = quantizeRotation(snap.rotYaw);

            Baseline& bl       = m_baselines[snap.entityId];
            bl.snapshot        = snap;
            bl.lastKeyframeTick = snap.tick;
            bl.forceKeyframe   = false;
        } else {
            cs.frameType = FrameType::Delta;
            const auto& base = m_baselines[snap.entityId].snapshot;
            cs.delta.dPosX   = quantizePosition(snap.posX)   - quantizePosition(base.posX);
            cs.delta.dPosY   = quantizePosition(snap.posY)   - quantizePosition(base.posY);
            cs.delta.dPosZ   = quantizePosition(snap.posZ)   - quantizePosition(base.posZ);
            cs.delta.dVelX   = quantizePosition(snap.velX)   - quantizePosition(base.velX);
            cs.delta.dVelY   = quantizePosition(snap.velY)   - quantizePosition(base.velY);
            cs.delta.dVelZ   = quantizePosition(snap.velZ)   - quantizePosition(base.velZ);
            cs.delta.dRotYaw = quantizeRotation(snap.rotYaw) - quantizeRotation(base.rotYaw);

            m_baselines[snap.entityId].snapshot = snap;
        }

        frame.entries.push_back(cs);
    }

    m_forceAll = false;
    return frame;
}

std::vector<EntitySnapshot> DeltaCompression::decode(
    const CompressedFrame& frame)
{
    std::vector<EntitySnapshot> result;
    result.reserve(frame.entries.size());

    for (const auto& cs : frame.entries) {
        EntitySnapshot snap;
        snap.entityId = cs.entityId;
        snap.tick     = cs.tick;

        if (cs.frameType == FrameType::Keyframe) {
            snap.posX   = dequantizePosition(cs.delta.dPosX);
            snap.posY   = dequantizePosition(cs.delta.dPosY);
            snap.posZ   = dequantizePosition(cs.delta.dPosZ);
            snap.velX   = dequantizePosition(cs.delta.dVelX);
            snap.velY   = dequantizePosition(cs.delta.dVelY);
            snap.velZ   = dequantizePosition(cs.delta.dVelZ);
            snap.rotYaw = dequantizeRotation(cs.delta.dRotYaw);

            Baseline& bl       = m_baselines[cs.entityId];
            bl.snapshot        = snap;
            bl.lastKeyframeTick = cs.tick;
        } else {
            auto it = m_baselines.find(cs.entityId);
            if (it == m_baselines.end()) {
                continue;
            }
            const auto& base = it->second.snapshot;
            snap.posX   = dequantizePosition(quantizePosition(base.posX)   + cs.delta.dPosX);
            snap.posY   = dequantizePosition(quantizePosition(base.posY)   + cs.delta.dPosY);
            snap.posZ   = dequantizePosition(quantizePosition(base.posZ)   + cs.delta.dPosZ);
            snap.velX   = dequantizePosition(quantizePosition(base.velX)   + cs.delta.dVelX);
            snap.velY   = dequantizePosition(quantizePosition(base.velY)   + cs.delta.dVelY);
            snap.velZ   = dequantizePosition(quantizePosition(base.velZ)   + cs.delta.dVelZ);
            snap.rotYaw = dequantizeRotation(quantizeRotation(base.rotYaw) + cs.delta.dRotYaw);

            it->second.snapshot = snap;
        }

        result.push_back(snap);
    }

    return result;
}

void DeltaCompression::forceKeyframe(uint32_t entityId) {
    auto it = m_baselines.find(entityId);
    if (it != m_baselines.end()) {
        it->second.forceKeyframe = true;
    }
}

void DeltaCompression::forceAllKeyframes() {
    m_forceAll = true;
    for (auto& [id, bl] : m_baselines) {
        bl.forceKeyframe = true;
    }
}

void DeltaCompression::removeEntity(uint32_t entityId) {
    m_baselines.erase(entityId);
}

void DeltaCompression::clear() {
    m_baselines.clear();
    m_forceAll = false;
}

bool DeltaCompression::needsKeyframe(uint32_t entityId,
                                      uint32_t currentTick) const
{
    if (m_forceAll) return true;

    auto it = m_baselines.find(entityId);
    if (it == m_baselines.end()) return true;

    const auto& bl = it->second;
    if (bl.forceKeyframe) return true;

    if (currentTick >= bl.lastKeyframeTick + m_keyframeInterval) return true;

    return false;
}

} // namespace fresh::networking
