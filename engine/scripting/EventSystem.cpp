#include "scripting/EventSystem.h"

#include <algorithm>
#include <iostream>

namespace fresh
{

// EventSystem implementation
EventSystem& EventSystem::getInstance()
{
    static EventSystem instance;
    return instance;
}

int EventSystem::subscribe(const std::string& eventType, EventCallback callback)
{
    int id = nextSubscriptionID++;

    Subscription sub;
    sub.id = id;
    sub.eventType = eventType;
    sub.callback = callback;

    subscribers[eventType].push_back(sub);

    std::cout << "Subscribed to event: " << eventType << " (ID: " << id << ")" << std::endl;
    return id;
}

void EventSystem::unsubscribe(int subscriptionID)
{
    for (auto& [eventType, subs] : subscribers) {
        auto it =
            std::remove_if(subs.begin(), subs.end(), [subscriptionID](const Subscription& sub) {
                return sub.id == subscriptionID;
            });

        if (it != subs.end()) {
            subs.erase(it, subs.end());
            std::cout << "Unsubscribed from event (ID: " << subscriptionID << ")" << std::endl;
            return;
        }
    }
}

void EventSystem::unsubscribeAll(const std::string& eventType)
{
    auto it = subscribers.find(eventType);
    if (it != subscribers.end()) {
        std::cout << "Unsubscribed all listeners from: " << eventType << std::endl;
        subscribers.erase(it);
    }
}

void EventSystem::emit(const std::string& eventType, const EventData& data)
{
    auto it = subscribers.find(eventType);
    if (it != subscribers.end()) {
        for (const auto& sub : it->second) {
            sub.callback(data);
        }
    }
}

void EventSystem::queueEvent(const std::string& eventType, const EventData& data)
{
    QueuedEvent event;
    event.eventType = eventType;
    event.data = data;
    eventQueue.push_back(event);
}

void EventSystem::processEvents()
{
    std::vector<QueuedEvent> eventsToProcess = std::move(eventQueue);
    eventQueue.clear();

    for (const auto& event : eventsToProcess) {
        emit(event.eventType, event.data);
    }
}

void EventSystem::clear()
{
    subscribers.clear();
    eventQueue.clear();
    std::cout << "EventSystem cleared" << std::endl;
}

// ScriptEngine implementation
ScriptEngine& ScriptEngine::getInstance()
{
    static ScriptEngine instance;
    return instance;
}

bool ScriptEngine::initialize()
{
    if (initialized)
        return true;

    std::cout << "Initializing ScriptEngine..." << std::endl;

    // TODO: Initialize scripting backend (Lua, Python, etc.)
    // For now, this is a stub implementation

    initialized = true;
    std::cout << "ScriptEngine initialized (stub mode)" << std::endl;
    return true;
}

void ScriptEngine::shutdown()
{
    if (!initialized)
        return;

    std::cout << "Shutting down ScriptEngine..." << std::endl;

    globals.clear();
    functions.clear();

    // TODO: Cleanup scripting backend

    initialized = false;
}

bool ScriptEngine::loadScript(const std::string& path)
{
    if (!initialized) {
        lastError = "ScriptEngine not initialized";
        return false;
    }

    std::cout << "Loading script: " << path << std::endl;

    // TODO: Actually load and execute script file
    // For now, return success
    lastError.clear();
    return true;
}

bool ScriptEngine::executeString(const std::string& script)
{
    if (!initialized) {
        lastError = "ScriptEngine not initialized";
        return false;
    }

    std::cout << "Executing script string: " << script.substr(0, 50) << "..." << std::endl;

    // TODO: Actually execute script
    lastError.clear();
    return true;
}

// Template implementations would go in header with inline
// These are placeholder implementations
template <typename... Args>
std::any ScriptEngine::callFunction(const std::string& functionName, Args&&... args)
{
    // Suppress unused parameter warning for variadic args - TODO: Pass args to scripting backend
    ((void)args, ...);
    auto it = functions.find(functionName);
    if (it != functions.end()) {
        return it->second();
    }
    lastError = "Function not found: " + functionName;
    return std::any();
}

template <typename Func>
void ScriptEngine::registerFunction(const std::string& name, Func&& func)
{
    (void)func; // Suppress unused parameter warning - TODO: Register with scripting backend
    std::cout << "Registering script function: " << name << std::endl;
    // TODO: Register function with scripting backend
}

template <typename T>
void ScriptEngine::registerClass(const std::string& name)
{
    // Suppress unused template parameter warning - TODO: Use T to register class type info
    (void)sizeof(T);
    std::cout << "Registering script class: " << name << std::endl;
    // TODO: Register class with scripting backend
}

template <typename T>
void ScriptEngine::setGlobal(const std::string& name, const T& value)
{
    globals[name] = value;
}

template <typename T>
T ScriptEngine::getGlobal(const std::string& name, const T& defaultValue)
{
    auto it = globals.find(name);
    if (it != globals.end()) {
        try {
            return std::any_cast<T>(it->second);
        } catch (const std::bad_any_cast&) {
            return defaultValue;
        }
    }
    return defaultValue;
}

} // namespace fresh
