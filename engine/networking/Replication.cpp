#include "networking/Replication.h"

#include <algorithm>
#include <cstring>

namespace fresh
{
namespace networking
{

void ReplicationManager::setEntityManager(ecs::EntityManager* entityManager)
{
    m_entityManager = entityManager;
}

void ReplicationManager::addRule(const ReplicationRule& rule)
{
    m_ruleMap[rule.typeTag] = rule;
}

void ReplicationManager::removeRule(uint32_t typeTag)
{
    m_ruleMap.erase(typeTag);
    m_dirty.erase(typeTag);
}

bool ReplicationManager::hasRule(uint32_t typeTag) const
{
    return m_ruleMap.count(typeTag) > 0;
}

const ReplicationRule* ReplicationManager::getRule(uint32_t typeTag) const
{
    auto it = m_ruleMap.find(typeTag);
    if (it != m_ruleMap.end())
        return &it->second;
    return nullptr;
}

std::vector<ReplicationRule> ReplicationManager::rules() const
{
    std::vector<ReplicationRule> result;
    result.reserve(m_ruleMap.size());
    for (const auto& pair : m_ruleMap) {
        result.push_back(pair.second);
    }
    return result;
}

size_t ReplicationManager::ruleCount() const
{
    return m_ruleMap.size();
}

void ReplicationManager::markDirty(uint32_t typeTag, uint32_t entityID)
{
    auto& dirtyList = m_dirty[typeTag];
    if (std::find(dirtyList.begin(), dirtyList.end(), entityID) == dirtyList.end()) {
        dirtyList.push_back(entityID);
    }
}

bool ReplicationManager::isDirty(uint32_t typeTag, uint32_t entityID) const
{
    auto it = m_dirty.find(typeTag);
    if (it == m_dirty.end())
        return false;
    return std::find(it->second.begin(), it->second.end(), entityID) != it->second.end();
}

void ReplicationManager::clearDirty()
{
    m_dirty.clear();
    m_manuallyTriggered.clear();
}

void ReplicationManager::triggerManualReplication(uint32_t typeTag)
{
    m_manuallyTriggered.insert(typeTag);
}

void ReplicationManager::setReliableCallback(std::function<void(const std::vector<uint8_t>&)> cb)
{
    m_reliableCallback = std::move(cb);
}

void ReplicationManager::setUnreliableCallback(std::function<void(const std::vector<uint8_t>&)> cb)
{
    m_unreliableCallback = std::move(cb);
}

std::vector<uint8_t> ReplicationManager::collectDelta(uint32_t tick)
{
    auto result = collectDeltaFiltered(tick, true);
    clearDirty();
    return result;
}

std::vector<uint8_t> ReplicationManager::collectUnreliableDelta(uint32_t tick)
{
    return collectDeltaFiltered(tick, false);
}

std::vector<uint8_t> ReplicationManager::collectDeltaFiltered(uint32_t tick, bool collectReliable)
{
    // Delta format:
    // [tick:4][ruleCount:4][{typeTag:4, entityCount:4, [{entityID:4, dataSize:4, data...}]...}...]
    std::vector<uint8_t> buffer;

    auto writeU32 = [&](uint32_t val) {
        size_t pos = buffer.size();
        buffer.resize(pos + 4);
        std::memcpy(buffer.data() + pos, &val, 4);
    };

    writeU32(tick);

    // Count rules that have dirty data
    uint32_t activeRuleCount = 0;
    for (const auto& pair : m_ruleMap) {
        const auto& rule = pair.second;
        if (rule.reliable != collectReliable)
            continue;
        if (rule.frequency == ReplicateFrequency::EveryTick) {
            activeRuleCount++;
        } else if (rule.frequency == ReplicateFrequency::OnChange) {
            auto it = m_dirty.find(rule.typeTag);
            if (it != m_dirty.end() && !it->second.empty()) {
                activeRuleCount++;
            }
        } else if (rule.frequency == ReplicateFrequency::Manual) {
            if (m_manuallyTriggered.count(rule.typeTag)) {
                activeRuleCount++;
            }
        }
    }
    writeU32(activeRuleCount);

    // Delta payload populated when EntityManager serialization is integrated
    return buffer;
}

bool ReplicationManager::applyDelta(const std::vector<uint8_t>& data)
{
    if (data.size() < 8)
        return false;

    auto readU32 = [&](size_t offset) -> uint32_t {
        uint32_t val = 0;
        std::memcpy(&val, data.data() + offset, 4);
        return val;
    };

    // Read header
    uint32_t tick = readU32(0);
    uint32_t ruleCount = readU32(4);
    (void)tick;
    (void)ruleCount;

    // Delta application will be implemented when EntityManager
    // serialization is integrated
    return true;
}

} // namespace networking
} // namespace fresh
