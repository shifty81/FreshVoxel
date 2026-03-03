#include "ecs/SystemRegistry.h"

namespace fresh::ecs {

void SystemRegistry::registerSystem(std::unique_ptr<ISystem> system) {
    if (!system) return;
    m_systems.push_back({std::move(system), false});
    m_sorted = false;
}

bool SystemRegistry::unregisterSystem(ISystem* system) {
    if (!system) return false;
    auto it = std::find_if(m_systems.begin(), m_systems.end(),
        [system](const Entry& e) { return e.system.get() == system; });
    if (it == m_systems.end()) return false;
    m_systems.erase(it);
    return true;
}

void SystemRegistry::initAll() {
    sortByPriority();
    for (auto& entry : m_systems) {
        if (!entry.initialized) {
            entry.system->init();
            entry.initialized = true;
        }
    }
}

void SystemRegistry::updateAll(float dt) {
    if (!m_sorted) sortByPriority();
    for (auto& entry : m_systems) {
        if (entry.system->isEnabled()) {
            entry.system->execute(dt);
        }
    }
}

void SystemRegistry::shutdownAll() {
    for (auto it = m_systems.rbegin(); it != m_systems.rend(); ++it) {
        if (it->initialized) {
            it->system->shutdown();
            it->initialized = false;
        }
    }
}

ISystem* SystemRegistry::findByName(const char* name) const {
    if (!name) return nullptr;
    for (const auto& entry : m_systems) {
        if (std::strcmp(entry.system->name(), name) == 0) {
            return entry.system.get();
        }
    }
    return nullptr;
}

void SystemRegistry::sortByPriority() {
    std::stable_sort(m_systems.begin(), m_systems.end(),
        [](const Entry& a, const Entry& b) {
            return a.system->priority() < b.system->priority();
        });
    m_sorted = true;
}

} // namespace fresh::ecs
