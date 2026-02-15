#pragma once

#include <string>
#include <memory>
#include <vector>

namespace fresh
{

class IRenderContext;
class VoxelWorld;
class Camera;

namespace ecs
{
class EntityManager;
}

/**
 * @brief Engine mode metadata
 * 
 * Contains information about the engine mode plugin.
 */
struct EngineModeMetadata
{
    std::string name;           // Display name (e.g., "3D Voxel Mode")
    std::string id;             // Unique identifier (e.g., "mode3d")
    std::string description;    // User-facing description
    std::string version;        // Plugin version
    std::string author;         // Plugin author
    bool supportsVoxels;        // True if mode supports voxel rendering
    bool supportsSprites;       // True if mode supports sprite rendering
    int dimensions;             // 2 for 2D, 3 for 3D
};

/**
 * @brief Interface for engine mode plugins
 * 
 * Engine modes define how the game engine renders and processes game logic.
 * This allows switching between 2D and 3D modes, or custom rendering modes.
 * 
 * Engine modes control:
 * - Rendering pipeline (2D sprites vs 3D voxels)
 * - Physics simulation (2D vs 3D)
 * - Camera behavior (orthographic vs perspective)
 * - Input handling (screen space vs world space)
 * 
 * Example modes:
 * - 3D Voxel Mode: Minecraft-style voxel rendering with 3D physics
 * - 2D Platformer Mode: Side-scrolling with 2D physics
 * - 2D Top-Down Mode: Zelda-style overhead view with 2D physics
 * - Hybrid Mode: Mix of 2D and 3D elements
 * 
 * Example implementation:
 * @code
 * class Mode3D : public IEngineMode {
 * public:
 *     EngineModeMetadata getMetadata() const override {
 *         return {
 *             "3D Voxel Mode",        // name
 *             "mode3d",               // id
 *             "Full 3D voxel rendering with perspective camera",
 *             "1.0.0",                // version
 *             "Fresh Engine",         // author
 *             true,                   // supportsVoxels
 *             false,                  // supportsSprites
 *             3                       // dimensions
 *         };
 *     }
 *     
 *     bool initialize(IRenderContext* renderer, ecs::EntityManager* entityMgr) override {
 *         // Setup 3D rendering pipeline
 *         return true;
 *     }
 *     
 *     void update(float deltaTime) override {
 *         // Update 3D physics and logic
 *     }
 *     
 *     void render(IRenderContext* renderer) override {
 *         // Render 3D scene
 *     }
 * };
 * @endcode
 */
class IEngineMode
{
public:
    virtual ~IEngineMode() = default;
    
    /**
     * @brief Get plugin metadata
     * @return Metadata structure with plugin information
     */
    virtual EngineModeMetadata getMetadata() const = 0;
    
    /**
     * @brief Initialize the engine mode
     * @param renderer Render context
     * @param entityMgr Entity manager
     * @return true if initialization succeeded
     */
    virtual bool initialize(IRenderContext* renderer, ecs::EntityManager* entityMgr) = 0;
    
    /**
     * @brief Shutdown the engine mode and cleanup resources
     */
    virtual void shutdown() = 0;
    
    /**
     * @brief Update game logic for this mode
     * @param deltaTime Time since last frame in seconds
     */
    virtual void update(float deltaTime) = 0;
    
    /**
     * @brief Render the scene for this mode
     * @param renderer Render context
     */
    virtual void render(IRenderContext* renderer) = 0;
    
    /**
     * @brief Handle window resize
     * @param width New window width
     * @param height New window height
     */
    virtual void onResize(int width, int height) = 0;
    
    /**
     * @brief Get the camera for this mode
     * @return Pointer to camera or nullptr if not applicable
     */
    virtual Camera* getCamera() = 0;
    
    /**
     * @brief Check if this mode uses 2D physics
     * @return true if 2D physics should be active
     */
    virtual bool uses2DPhysics() const = 0;
    
    /**
     * @brief Check if this mode uses 3D physics
     * @return true if 3D physics should be active
     */
    virtual bool uses3DPhysics() const = 0;
    
    /**
     * @brief Get gravity vector for this mode
     * @return Gravity vector (2D or 3D depending on mode)
     */
    virtual glm::vec3 getGravity() const = 0;
};

/**
 * @brief Factory for creating engine mode instances
 * 
 * Allows registration of custom engine modes at runtime.
 */
class EngineModeFactory
{
public:
    using ModeCreator = std::unique_ptr<IEngineMode>(*)();
    
    /**
     * @brief Register an engine mode
     * @param id Mode ID (must be unique)
     * @param creator Factory function to create instances
     */
    static void registerMode(const std::string& id, ModeCreator creator);
    
    /**
     * @brief Create an engine mode instance by ID
     * @param id Mode ID
     * @return Mode instance or nullptr if not found
     */
    static std::unique_ptr<IEngineMode> createMode(const std::string& id);
    
    /**
     * @brief Get list of all registered mode IDs
     * @return Vector of mode IDs
     */
    static std::vector<std::string> getRegisteredModes();
    
    /**
     * @brief Get metadata for a registered mode
     * @param id Mode ID
     * @return Mode metadata or empty struct if not found
     */
    static EngineModeMetadata getModeMetadata(const std::string& id);
    
    /**
     * @brief Register built-in engine modes
     * 
     * Registers the default modes:
     * - "mode3d" (3D voxel rendering)
     * - "mode2d_platformer" (2D side-scrolling)
     * - "mode2d_topdown" (2D overhead view)
     */
    static void registerBuiltInModes();

private:
    struct ModeRegistration {
        ModeCreator creator;
        EngineModeMetadata metadata;
    };
    
    static std::unordered_map<std::string, ModeRegistration>& getRegistry();
};

} // namespace fresh
