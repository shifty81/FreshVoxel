#pragma once

#include <memory>

namespace fresh
{

class VoxelWorld;
class TerraformingSystem;

/**
 * @brief World editing tools
 *
 * Provides high-level world editing functionality including
 * terraforming, object placement, and world manipulation.
 */
class WorldEditor
{
public:
    WorldEditor();
    ~WorldEditor();

    /**
     * @brief Initialize world editor
     * @param world Voxel world to edit
     * @return true if successful
     */
    bool initialize(VoxelWorld* world);

    /**
     * @brief Update editor state
     * @param deltaTime Time since last update
     */
    void update(float deltaTime);

    /**
     * @brief Get the terraforming system
     * @return Pointer to terraforming system
     */
    TerraformingSystem* getTerraformingSystem() const
    {
        return m_terraformingSystem.get();
    }

    /**
     * @brief Enable or disable editor mode
     * @param enabled true to enable, false to disable
     */
    void setEnabled(bool enabled)
    {
        m_enabled = enabled;
    }

    /**
     * @brief Check if editor is enabled
     * @return true if enabled, false otherwise
     */
    bool isEnabled() const
    {
        return m_enabled;
    }

private:
    VoxelWorld* m_world;
    std::unique_ptr<TerraformingSystem> m_terraformingSystem;
    bool m_enabled;
};

} // namespace fresh
