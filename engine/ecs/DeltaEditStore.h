#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace fresh::ecs {

enum class DeltaEditType : uint8_t {
    AddObject,
    RemoveObject,
    MoveObject,
    SetProperty
};

const char* deltaEditTypeName(DeltaEditType type);

struct DeltaEdit {
    DeltaEditType type;
    uint32_t      entityID   = 0;
    std::string   objectType;
    float         position[3] = {};
    std::string   propertyName;
    std::string   propertyValue;
};

class DeltaEditStore {
public:
    explicit DeltaEditStore(uint64_t seed = 0) : m_seed(seed) {}

    uint64_t seed() const { return m_seed; }
    void     setSeed(uint64_t seed) { m_seed = seed; }

    void record(const DeltaEdit& edit);
    size_t count() const { return m_edits.size(); }
    const std::vector<DeltaEdit>& edits() const { return m_edits; }
    void clear();

    std::string serializeToJSON() const;
    bool deserializeFromJSON(const std::string& json);

private:
    uint64_t               m_seed = 0;
    std::vector<DeltaEdit> m_edits;
};

} // namespace fresh::ecs
