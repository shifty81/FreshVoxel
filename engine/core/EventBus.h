#pragma once
#include <cstdint>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <algorithm>

namespace fresh {

struct Event {
    std::string type;
    uint32_t    senderId  = 0;
    int64_t     intParam  = 0;
    double      floatParam = 0.0;
    std::string strParam;
};

using SubscriptionId = uint64_t;

class EventBus {
public:
    using Callback = std::function<void(const Event&)>;

    EventBus() = default;

    SubscriptionId subscribe(const std::string& eventType, Callback cb);
    void unsubscribe(SubscriptionId id);
    void publish(const Event& event);
    void enqueue(const Event& event);
    void flush();
    size_t subscriptionCount() const;
    size_t queueSize() const;
    uint64_t totalPublished() const { return m_totalPublished; }
    void reset();

private:
    struct Subscription {
        SubscriptionId id = 0;
        std::string    eventType;
        Callback       callback;
    };

    mutable std::mutex m_mutex;
    std::unordered_map<std::string, std::vector<Subscription>> m_subs;
    std::vector<Subscription> m_wildcardSubs;
    std::vector<Event> m_queue;
    SubscriptionId m_nextId = 1;
    uint64_t       m_totalPublished = 0;
};

} // namespace fresh
