#pragma once

#include <string>
#include <vector>

#include "ecs/Entity.h"
#include "ecs/EntityManager.h"

namespace fresh
{

/**
 * @brief Prefab serialization / deserialization system.
 *
 * A **prefab** is a JSON file that captures all the components of a single
 * entity so it can be:
 *   - Saved to the Content Browser as a reusable template.
 *   - Spawned back into the world by dragging from the Content Browser or
 *     by calling `Engine_SpawnPrefab`.
 *   - Used as a blueprint when importing .vox models as entities.
 *
 * ### File format (simplified JSON, no external library required)
 * ```json
 * {
 *   "prefab_version": 1,
 *   "name": "MyEntity",
 *   "components": [
 *     { "type": "TransformComponent",
 *       "position": [0.0, 0.0, 0.0],
 *       "rotation": [0.0, 0.0, 0.0, 1.0],
 *       "scale":    [1.0, 1.0, 1.0] },
 *     { "type": "HealthComponent",
 *       "maxHealth": 100.0,
 *       "currentHealth": 100.0 },
 *     { "type": "MovementComponent",
 *       "maxSpeed": 5.0,
 *       "acceleration": 10.0 }
 *   ]
 * }
 * ```
 */
class PrefabSystem
{
public:
    PrefabSystem() = default;
    ~PrefabSystem() = default;

    // Non-copyable, non-movable (stateless utility class — default is fine, but be explicit)
    PrefabSystem(const PrefabSystem&)            = delete;
    PrefabSystem& operator=(const PrefabSystem&) = delete;

    // -----------------------------------------------------------------------
    // Serialization
    // -----------------------------------------------------------------------

    /**
     * @brief Serialize a single entity to a JSON string.
     *
     * Writes the known component types (Transform, Health, Movement) for the
     * entity.  Unknown/custom components are silently ignored — only the
     * built-in component set is serializable today.
     *
     * @param em       Entity manager owning the entity.
     * @param entity   Entity to serialize.
     * @param name     Human-readable name embedded in the JSON (e.g. file stem).
     * @return Formatted JSON string, or an empty string on failure.
     */
    std::string serializeEntity(const ecs::EntityManager& em,
                                ecs::Entity entity,
                                const std::string& name = "Untitled") const;

    /**
     * @brief Save a prefab JSON file to disk.
     *
     * @param filePath  Absolute path including file name and .prefab extension.
     * @param json      JSON text returned by serializeEntity().
     * @return true on success.
     */
    bool savePrefab(const std::string& filePath, const std::string& json) const;

    /**
     * @brief Convenience: serialize then save in one call.
     */
    bool saveEntityAsPrefab(const ecs::EntityManager& em,
                            ecs::Entity entity,
                            const std::string& filePath) const;

    // -----------------------------------------------------------------------
    // Deserialization
    // -----------------------------------------------------------------------

    /**
     * @brief Load a prefab JSON file from disk.
     *
     * @param filePath  Absolute path to the .prefab file.
     * @return File contents, or an empty string on failure.
     */
    std::string loadPrefab(const std::string& filePath) const;

    /**
     * @brief Deserialize a JSON string and spawn a new entity in `em`.
     *
     * Components present in the JSON are added to the new entity.  Unknown
     * component types are logged as warnings and skipped.
     *
     * @param em    Entity manager that will own the new entity.
     * @param json  JSON text (result of loadPrefab or serializeEntity).
     * @return The newly created entity, or an invalid entity on failure.
     */
    ecs::Entity spawnFromJSON(ecs::EntityManager& em, const std::string& json) const;

    /**
     * @brief Convenience: load then spawn in one call.
     */
    ecs::Entity spawnPrefab(ecs::EntityManager& em, const std::string& filePath) const;

    // -----------------------------------------------------------------------
    // Listing
    // -----------------------------------------------------------------------

    /**
     * @brief Enumerate all .prefab files in a directory (non-recursive).
     *
     * @param directory  Path to scan.
     * @return List of absolute file paths.
     */
    std::vector<std::string> listPrefabs(const std::string& directory) const;

private:
    // Simple JSON emission helpers
    static std::string escapeString(const std::string& s);
    static std::string vec3ToJSON(float x, float y, float z);
    static std::string vec4ToJSON(float x, float y, float z, float w);

    // Simple JSON parsing helpers (no full parser — pattern-match known keys)
    static bool parseFloatArray(const std::string& json,
                                const std::string& key,
                                float* out, int count);
    static bool parseFloat(const std::string& json,
                           const std::string& key,
                           float& out);
    static std::string parseString(const std::string& json,
                                   const std::string& key);
    static std::vector<std::string> splitComponentBlocks(const std::string& json);
    static std::string extractComponentType(const std::string& block);
};

} // namespace fresh
