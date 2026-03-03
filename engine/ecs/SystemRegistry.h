#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include <cstring>
#include <algorithm>

namespace fresh::ecs {

class ISystem {
public:
    virtual ~ISystem() = default;

    virtual const char* name() const = 0;
    virtual int priority() const { return 100; }
    virtual void init() {}
    virtual void execute(float dt) = 0;
    virtual void shutdown() {}

    bool isEnabled() const { return m_enabled; }
    void setEnabled(bool enabled) { m_enabled = enabled; }

private:
    bool m_enabled = true;
};

class SystemRegistry {
public:
    void registerSystem(std::unique_ptr<ISystem> system);
    bool unregisterSystem(ISystem* system);
    void initAll();
    void updateAll(float dt);
    void shutdownAll();
    size_t count() const { return m_systems.size(); }
    ISystem* findByName(const char* name) const;

private:
    void sortByPriority();

    struct Entry {
        std::unique_ptr<ISystem> system;
        bool initialized = false;
    };
    std::vector<Entry> m_systems;
    bool m_sorted = false;
};

} // namespace fresh::ecs
