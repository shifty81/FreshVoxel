#pragma once
#include <any>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace fresh
{

/**
 * @brief Event data container
 */
class EventData
{
public:
    template <typename T>
    void set(const std::string& key, const T& value)
    {
        data[key] = value;
    }

    template <typename T>
    T get(const std::string& key, const T& defaultValue = T()) const
    {
        auto it = data.find(key);
        if (it != data.end()) {
            try {
                return std::any_cast<T>(it->second);
            } catch (const std::bad_any_cast&) {
                return defaultValue;
            }
        }
        return defaultValue;
    }

    bool has(const std::string& key) const
    {
        return data.find(key) != data.end();
    }

private:
    std::map<std::string, std::any> data;
};

/**
 * @brief Event type
 */
using EventCallback = std::function<void(const EventData&)>;

/**
 * @brief Event system for game-wide communication
 *
 * Provides pub-sub pattern for decoupled communication between systems.
 * Systems can emit events and subscribe to events of interest.
 */
class EventSystem
{
public:
    static EventSystem& getInstance();

    /**
     * @brief Subscribe to an event type
     * @param eventType Name of event to listen for
     * @param callback Function to call when event fires
     * @return Subscription ID (for unsubscribing)
     */
    int subscribe(const std::string& eventType, EventCallback callback);

    /**
     * @brief Unsubscribe from event
     * @param subscriptionID ID returned from subscribe()
     */
    void unsubscribe(int subscriptionID);

    /**
     * @brief Unsubscribe all callbacks for an event type
     */
    void unsubscribeAll(const std::string& eventType);

    /**
     * @brief Emit an event immediately
     */
    void emit(const std::string& eventType, const EventData& data = EventData());

    /**
     * @brief Queue event for processing next frame
     */
    void queueEvent(const std::string& eventType, const EventData& data = EventData());

    /**
     * @brief Process all queued events
     */
    void processEvents();

    /**
     * @brief Clear all subscriptions and queued events
     */
    void clear();

private:
    EventSystem() = default;
    ~EventSystem() = default;
    EventSystem(const EventSystem&) = delete;
    EventSystem& operator=(const EventSystem&) = delete;

    struct Subscription {
        int id;
        std::string eventType;
        EventCallback callback;
    };

    struct QueuedEvent {
        std::string eventType;
        EventData data;
    };

    int nextSubscriptionID = 1;
    std::map<std::string, std::vector<Subscription>> subscribers;
    std::vector<QueuedEvent> eventQueue;
};

/**
 * @brief Scripting engine interface for custom game logic
 *
 * Provides a way to define game behaviors without modifying C++ code.
 * Can be extended with Lua, Python, or custom scripting language.
 */
class ScriptEngine
{
public:
    static ScriptEngine& getInstance();

    /**
     * @brief Initialize scripting engine
     */
    bool initialize();

    /**
     * @brief Shutdown scripting engine
     */
    void shutdown();

    /**
     * @brief Load and execute script file
     * @param path Path to script file
     * @return true if successful
     */
    bool loadScript(const std::string& path);

    /**
     * @brief Execute script string
     * @param script Script code to execute
     * @return true if successful
     */
    bool executeString(const std::string& script);

    /**
     * @brief Call a script function
     * @param functionName Name of function in script
     * @param args Arguments to pass
     * @return Result as any type
     */
    template <typename... Args>
    std::any callFunction(const std::string& functionName, Args&&... args);

    /**
     * @brief Register C++ function callable from scripts
     */
    template <typename Func>
    void registerFunction(const std::string& name, Func&& func);

    /**
     * @brief Register C++ class for use in scripts
     */
    template <typename T>
    void registerClass(const std::string& name);

    /**
     * @brief Set global variable in script environment
     */
    template <typename T>
    void setGlobal(const std::string& name, const T& value);

    /**
     * @brief Get global variable from script environment
     */
    template <typename T>
    T getGlobal(const std::string& name, const T& defaultValue = T());

    /**
     * @brief Check if script has error
     */
    bool hasError() const
    {
        return !lastError.empty();
    }

    /**
     * @brief Get last error message
     */
    const std::string& getLastError() const
    {
        return lastError;
    }

private:
    ScriptEngine() = default;
    ~ScriptEngine() = default;
    ScriptEngine(const ScriptEngine&) = delete;
    ScriptEngine& operator=(const ScriptEngine&) = delete;

    bool initialized = false;
    std::string lastError;

    // TODO: Add actual scripting backend (Lua, Python, etc.)
    std::map<std::string, std::any> globals;
    std::map<std::string, std::function<std::any()>> functions;
};

// Common event types
namespace Events
{
// Player events
constexpr const char* PLAYER_SPAWNED = "player_spawned";
constexpr const char* PLAYER_DIED = "player_died";

// World events
constexpr const char* CHUNK_LOADED = "chunk_loaded";
constexpr const char* CHUNK_UNLOADED = "chunk_unloaded";
constexpr const char* BLOCK_PLACED = "block_placed";
constexpr const char* BLOCK_BROKEN = "block_broken";

// Item events
constexpr const char* ITEM_PICKED_UP = "item_picked_up";
constexpr const char* ITEM_USED = "item_used";

// Game state events
constexpr const char* GAME_PAUSED = "game_paused";
constexpr const char* GAME_RESUMED = "game_resumed";
constexpr const char* SCENE_LOADED = "scene_loaded";
constexpr const char* RESOURCE_LOADED = "resource_loaded";

// Entity/Combat events (for event-driven gameplay)
constexpr const char* ENTITY_DAMAGED = "entity_damaged";
constexpr const char* ENTITY_HEALED = "entity_healed";
constexpr const char* ENTITY_DIED = "entity_died";
constexpr const char* ENTITY_REVIVED = "entity_revived";

// Environmental events (for emergent gameplay)
constexpr const char* DOOR_OPENED = "door_opened";
constexpr const char* DOOR_CLOSED = "door_closed";
constexpr const char* ENEMY_SPOTTED = "enemy_spotted";
constexpr const char* NOISE_MADE = "noise_made";
constexpr const char* TRAP_TRIGGERED = "trap_triggered";
} // namespace Events

} // namespace fresh
