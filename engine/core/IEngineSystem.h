#pragma once

#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
#include <functional>

namespace fresh
{

class IRenderContext;
class VoxelWorld;

namespace ecs
{
class EntityManager;
}

/**
 * @brief Metadata describing a pluggable engine system
 *
 * Used by the editor to display system information and manage
 * system lifecycle through the UI.
 */
struct EngineSystemInfo
{
    std::string id;             ///< Unique system identifier (e.g., "physics", "ai")
    std::string name;           ///< Display name (e.g., "Physics System")
    std::string description;    ///< Human-readable description
    std::string version;        ///< System version string
    bool isEditorSystem;        ///< True if system runs in editor mode
    bool isPlayModeSystem;      ///< True if system runs in play mode
};

/**
 * @brief Interface for pluggable engine systems
 *
 * Game systems (physics, AI, weather, time-of-day, audio, etc.) implement
 * this interface so they can be registered with the engine at runtime.
 * The editor manages their lifecycle: systems marked as play-mode-only
 * are only active during play testing, while editor systems run
 * continuously.
 *
 * This decouples gameplay logic from the editor, allowing users to
 * add, remove, and configure systems through the editor UI without
 * modifying engine code.
 *
 * Example implementation:
 * @code
 * class PhysicsEngineSystem : public IEngineSystem {
 * public:
 *     EngineSystemInfo getInfo() const override {
 *         return {"physics", "Physics System", "3D voxel physics",
 *                 "1.0.0", false, true};
 *     }
 *     bool initialize() override { return true; }
 *     void shutdown() override { }
 *     void update(float dt) override { stepSimulation(dt); }
 *     void setEnabled(bool e) override { m_enabled = e; }
 *     bool isEnabled() const override { return m_enabled; }
 * private:
 *     bool m_enabled = true;
 * };
 * @endcode
 */
class IEngineSystem
{
public:
    virtual ~IEngineSystem() = default;

    /** @brief Retrieve metadata about this system */
    virtual EngineSystemInfo getInfo() const = 0;

    /** @brief Initialize the system. Called once after registration. */
    virtual bool initialize() = 0;

    /** @brief Shutdown and release resources */
    virtual void shutdown() = 0;

    /** @brief Per-frame update.  Only called when the system is enabled. */
    virtual void update(float deltaTime) = 0;

    /** @brief Enable or disable the system at runtime */
    virtual void setEnabled(bool enabled) = 0;

    /** @brief Check whether the system is currently enabled */
    virtual bool isEnabled() const = 0;
};

/**
 * @brief Registry that owns and manages pluggable engine systems
 *
 * The registry provides ordered update, lifetime management, and
 * convenient lookup by system id.  The editor uses this registry to
 * discover available systems and toggle them during play mode.
 */
class EngineSystemRegistry
{
public:
    /**
     * @brief Register a system with the engine
     * @param system Owning pointer – the registry takes ownership
     * @return true if registration succeeded (id was unique)
     */
    bool registerSystem(std::unique_ptr<IEngineSystem> system)
    {
        if (!system) return false;
        const std::string id = system->getInfo().id;
        auto [it, inserted] = m_lookup.try_emplace(id, system.get());
        if (!inserted) return false;                     // duplicate id
        m_systems.push_back(std::move(system));
        return true;
    }

    /** @brief Retrieve a system by id (nullptr if not found) */
    IEngineSystem* getSystem(const std::string& id) const
    {
        auto it = m_lookup.find(id);
        return it != m_lookup.end() ? it->second : nullptr;
    }

    /** @brief Initialize all registered systems in registration order */
    bool initializeAll()
    {
        for (auto& sys : m_systems) {
            if (!sys->initialize()) return false;
        }
        return true;
    }

    /** @brief Shutdown all systems in reverse registration order */
    void shutdownAll()
    {
        for (auto it = m_systems.rbegin(); it != m_systems.rend(); ++it) {
            (*it)->shutdown();
        }
    }

    /**
     * @brief Update all enabled systems whose mode matches
     * @param deltaTime Frame delta
     * @param isPlayMode True when the engine is in play mode
     */
    void updateAll(float deltaTime, bool isPlayMode)
    {
        for (auto& sys : m_systems) {
            if (!sys->isEnabled()) continue;
            const auto info = sys->getInfo();
            if (isPlayMode && info.isPlayModeSystem) {
                sys->update(deltaTime);
            } else if (!isPlayMode && info.isEditorSystem) {
                sys->update(deltaTime);
            }
        }
    }

    /** @brief Get a read-only view of all systems */
    const std::vector<std::unique_ptr<IEngineSystem>>& getSystems() const
    {
        return m_systems;
    }

private:
    std::vector<std::unique_ptr<IEngineSystem>> m_systems;
    std::unordered_map<std::string, IEngineSystem*> m_lookup;
};

} // namespace fresh
