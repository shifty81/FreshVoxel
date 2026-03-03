#include "core/EventBus.h"

namespace fresh {

SubscriptionId EventBus::subscribe(const std::string& eventType, Callback cb) {
    if (!cb) return 0;
    std::lock_guard<std::mutex> lock(m_mutex);
    Subscription sub;
    sub.id        = m_nextId++;
    sub.eventType = eventType;
    sub.callback  = std::move(cb);
    const auto id = sub.id;
    if (eventType == "*") {
        m_wildcardSubs.push_back(std::move(sub));
    } else {
        m_subs[eventType].push_back(std::move(sub));
    }
    return id;
}

void EventBus::unsubscribe(SubscriptionId id) {
    if (id == 0) return;
    std::lock_guard<std::mutex> lock(m_mutex);
    {
        auto& v = m_wildcardSubs;
        v.erase(std::remove_if(v.begin(), v.end(),
            [id](const Subscription& s) { return s.id == id; }),
            v.end());
    }
    for (auto& [type, vec] : m_subs) {
        auto it = std::remove_if(vec.begin(), vec.end(),
            [id](const Subscription& s) { return s.id == id; });
        if (it != vec.end()) {
            vec.erase(it, vec.end());
            return;
        }
    }
}

void EventBus::publish(const Event& event) {
    std::vector<Callback> toInvoke;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        ++m_totalPublished;
        auto it = m_subs.find(event.type);
        if (it != m_subs.end()) {
            for (const auto& sub : it->second) {
                toInvoke.push_back(sub.callback);
            }
        }
        for (const auto& sub : m_wildcardSubs) {
            toInvoke.push_back(sub.callback);
        }
    }
    for (auto& cb : toInvoke) {
        cb(event);
    }
}

void EventBus::enqueue(const Event& event) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_queue.push_back(event);
}

void EventBus::flush() {
    std::vector<Event> pending;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        pending = std::move(m_queue);
        m_queue.clear();
    }
    for (const auto& event : pending) {
        publish(event);
    }
}

size_t EventBus::subscriptionCount() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    size_t count = m_wildcardSubs.size();
    for (const auto& [type, vec] : m_subs) {
        count += vec.size();
    }
    return count;
}

size_t EventBus::queueSize() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_queue.size();
}

void EventBus::reset() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_subs.clear();
    m_wildcardSubs.clear();
    m_queue.clear();
    m_nextId = 1;
    m_totalPublished = 0;
}

} // namespace fresh
