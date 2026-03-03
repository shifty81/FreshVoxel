#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace fresh::sim {

enum class AlertPriority : uint8_t {
    Low    = 0,
    Medium = 1,
    High   = 2,
    Critical = 3
};

struct Alert {
    uint32_t      id       = 0;
    std::string   message;
    std::string   category;
    AlertPriority priority = AlertPriority::Medium;
    float         ttl      = 5.0f;
    float         age      = 0.0f;
    bool          dismissed = false;
};

class AlertStack {
public:
    explicit AlertStack(size_t maxAlerts = 8);

    uint32_t push(const std::string& message,
                  const std::string& category,
                  AlertPriority priority = AlertPriority::Medium,
                  float durationSeconds = 5.0f);
    void tick(float deltaSeconds);
    bool dismiss(uint32_t id);
    size_t dismissCategory(const std::string& category);
    void clear();
    std::vector<Alert> active() const;
    std::vector<Alert> activeInCategory(const std::string& category) const;
    size_t activeCount() const;
    size_t maxAlerts() const { return m_maxAlerts; }
    uint32_t totalPushed() const { return m_nextId - 1; }

private:
    void evictIfNeeded();

    std::vector<Alert> m_alerts;
    size_t   m_maxAlerts;
    uint32_t m_nextId = 1;
};

} // namespace fresh::sim
