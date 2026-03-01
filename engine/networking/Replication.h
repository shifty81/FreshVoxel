#pragma once
// ============================================================
// FreshVoxel Replication — Component Replication Rules
// ============================================================
//
// Adapted from AtlasForge engine. Provides rule-based
// component replication for multiplayer: server-to-client,
// client-to-server, bidirectional, on-change, every-tick,
// and manual replication modes with reliable/unreliable
// channel separation.

#include <cstdint>
#include <functional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace fresh
{
namespace ecs
{
class EntityManager;
}
} // namespace fresh

namespace fresh
{
namespace networking
{

enum class ReplicateFrequency {
    EveryTick,
    OnChange,
    Manual
};

enum class ReplicateDirection {
    ServerToClient,
    ClientToServer,
    Bidirectional
};

struct ReplicationRule {
    uint32_t typeTag = 0;
    std::string componentName;
    ReplicateFrequency frequency = ReplicateFrequency::OnChange;
    ReplicateDirection direction = ReplicateDirection::ServerToClient;
    bool reliable = true;
    uint8_t priority = 128;
};

/**
 * @brief Rule-based component replication for multiplayer
 *
 * Manages replication rules that define which components
 * are replicated, how often, and in which direction. Produces
 * binary delta payloads for network transmission.
 */
class ReplicationManager
{
public:
    void setEntityManager(ecs::EntityManager* entityManager);

    void addRule(const ReplicationRule& rule);
    void removeRule(uint32_t typeTag);
    bool hasRule(uint32_t typeTag) const;
    const ReplicationRule* getRule(uint32_t typeTag) const;

    std::vector<ReplicationRule> rules() const;
    size_t ruleCount() const;

    // Collect dirty components based on rules and produce a delta payload (reliable only)
    std::vector<uint8_t> collectDelta(uint32_t tick);

    // Collect delta for unreliable rules only
    std::vector<uint8_t> collectUnreliableDelta(uint32_t tick);

    // Trigger replication for a Manual-frequency component type
    void triggerManualReplication(uint32_t typeTag);

    // Apply a received delta payload to the local world
    bool applyDelta(const std::vector<uint8_t>& data);

    // Mark a component type as dirty (for OnChange mode)
    void markDirty(uint32_t typeTag, uint32_t entityID);

    // Query dirty state
    bool isDirty(uint32_t typeTag, uint32_t entityID) const;

    // Clear all dirty flags (called after collectDelta)
    void clearDirty();

    // Set callback for reliable delta payloads
    void setReliableCallback(std::function<void(const std::vector<uint8_t>&)> cb);

    // Set callback for unreliable delta payloads
    void setUnreliableCallback(std::function<void(const std::vector<uint8_t>&)> cb);

private:
    // Shared implementation for collectDelta and collectUnreliableDelta
    std::vector<uint8_t> collectDeltaFiltered(uint32_t tick, bool collectReliable);

    ecs::EntityManager* m_entityManager = nullptr;
    std::unordered_map<uint32_t, ReplicationRule> m_ruleMap;
    // typeTag -> set of dirty entity IDs
    std::unordered_map<uint32_t, std::vector<uint32_t>> m_dirty;

    // Manual replication triggers
    std::unordered_set<uint32_t> m_manuallyTriggered;

    // Callbacks for reliable/unreliable deltas
    std::function<void(const std::vector<uint8_t>&)> m_reliableCallback;
    std::function<void(const std::vector<uint8_t>&)> m_unreliableCallback;
};

} // namespace networking
} // namespace fresh
