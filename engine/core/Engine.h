#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#if defined(FRESH_OPENGL_SUPPORT) && defined(FRESH_GLEW_AVAILABLE)
    #include <GL/glew.h>
#endif

#include "core/IEngineSystem.h"
#include "voxel/VoxelTypes.h"

namespace fresh
{

// Forward declare WorldCreationParams
struct WorldCreationParams;

#ifdef _WIN32
class Win32Window;
class Win32InputManager;
class GamePlayWindow;
using WindowType = Win32Window;
using InputManagerType = Win32InputManager;
#else
class Window;
class InputManager;
using WindowType = Window;
using InputManagerType = InputManager;
#endif

class IRenderContext;
class VoxelWorld;
class PhysicsSystem;
class AISystem;
class EditorGUI;
class EditorManager;
class MainMenu;
class WorldEditor;
class Player;
class VoxelInteraction;
class TimeManager;
class SeasonManager;
class WeatherManager;
class Raft;
class ProjectManager;

namespace ecs
{
class EntityManager;
}

namespace scripting
{
class LuaScriptingEngine;
}

/**
 * @brief Main engine class that orchestrates all subsystems
 *
 * This is the central component that manages the lifecycle of the voxel engine,
 * including initialization, main loop, and cleanup of all subsystems.
 */
class Engine
{
public:
    Engine();
    ~Engine();

    // Prevent copying
    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;

    /**
     * @brief Initialize the engine and all subsystems
     * @return true if initialization was successful, false otherwise
     */
    bool initialize();

    /**
     * @brief Run the main engine loop
     */
    void run();

    /**
     * @brief Shutdown the engine and cleanup resources
     */
    void shutdown();

    /**
     * @brief Check if the engine is running
     * @return true if the engine is running, false otherwise
     */
    bool isRunning() const
    {
        return m_running;
    }

    /**
     * @brief Get the project manager
     * @return Pointer to project manager
     */
    ProjectManager* getProjectManager() const { return m_projectManager.get(); }

    /**
     * @brief Get the pluggable system registry
     *
     * Allows external code (editor UI, scripts) to discover and manage
     * registered engine systems at runtime.
     * @return Reference to system registry
     */
    EngineSystemRegistry& getSystemRegistry() { return m_systemRegistry; }
    const EngineSystemRegistry& getSystemRegistry() const { return m_systemRegistry; }

private:
    void processInput();
    void update(float deltaTime);
    void updateEditor(float deltaTime);  // Updates editor-mode systems only
    void render();
    
    // Separate rendering methods for editor and game modes
    void renderGame();      // Renders game world to viewport (used in play mode)
    void renderEditor();    // Renders editor preview to viewport (used in editor mode)

    // Viewport validation helper
    bool validateViewportState();

    // Rendering helpers (private implementation details)
    void initializeRendering();
    void shutdownRendering();
    void renderVoxelWorld();
    void renderCrosshair();
#if defined(FRESH_OPENGL_SUPPORT) && defined(FRESH_GLEW_AVAILABLE)
    std::string loadShaderFile(const std::string& filepath);
    GLuint compileShader(const std::string& source, GLenum shaderType);
    GLuint createShaderProgram(const std::string& vertPath, const std::string& fragPath);
#endif

#ifdef _WIN32
    // Native Win32 menu bar setup
    void setupNativeMenuBar();
    void setupNativeToolbar();
#endif

    // Play mode methods
    void enterPlayMode();
    void exitPlayMode();
    void togglePlayMode();
    bool isInPlayMode() const { return m_inGame; }

#ifdef _WIN32
    // Separate game window play mode helpers
    void updateGamePlayWindow(float deltaTime);
    void renderGamePlayWindow();
    void restoreEditorViewport();
#endif

private:
    bool m_running;
    bool m_inGame;
    std::unique_ptr<WindowType> m_window;
    std::unique_ptr<IRenderContext> m_renderer;
    std::unique_ptr<VoxelWorld> m_world;
    std::unique_ptr<PhysicsSystem> m_physics;
    std::unique_ptr<AISystem> m_aiSystem;
    std::unique_ptr<ecs::EntityManager> m_entityManager;
    std::unique_ptr<EditorGUI> m_editor;
    std::unique_ptr<EditorManager> m_editorManager;
    std::unique_ptr<MainMenu> m_mainMenu;
    std::unique_ptr<WorldEditor> m_worldEditor;
    std::unique_ptr<Player> m_player;
    std::unique_ptr<InputManagerType> m_inputManager;
    std::unique_ptr<VoxelInteraction> m_voxelInteraction;
    std::unique_ptr<TimeManager> m_timeManager;
    std::unique_ptr<SeasonManager> m_seasonManager;
    std::unique_ptr<WeatherManager> m_weatherManager;
    std::unique_ptr<Raft> m_raft;
    std::unique_ptr<scripting::LuaScriptingEngine> m_scriptingEngine;
    std::unique_ptr<ProjectManager> m_projectManager;
    EngineSystemRegistry m_systemRegistry;
#ifdef _WIN32
    std::unique_ptr<GamePlayWindow> m_gamePlayWindow;
    bool m_viewportSwapChainReady = false;  // Tracks whether viewport swap chain was successfully created
#endif
    VoxelType m_selectedBlockType;
    // Track world type for camera setup
    bool m_isWorld3D = true;
    int m_world2DStyle = 0; // 0 = Platformer, 1 = Top-down
    
    // Track cursor capture state to avoid unnecessary mode switches
    bool m_lastCursorCaptured = false;
    
    // Track if user explicitly toggled cursor mode with F key
    // Prevents automatic cursor management from overriding user's explicit choice
    bool m_userToggledCursor = false;
    
    // Unreal-style mouse control: track if right mouse button is held for free look
    bool m_rightMouseHeldForCamera = false;
    
    // World generation state tracking
    bool m_isGeneratingWorld = false;

#if defined(FRESH_OPENGL_SUPPORT) && defined(FRESH_GLEW_AVAILABLE)
    // OpenGL rendering state
    GLuint m_shaderProgram = 0;
    GLuint m_crosshairShader = 0;
    GLuint m_crosshairVAO = 0;
    GLuint m_crosshairVBO = 0;
    std::unordered_map<ChunkPos, GLuint> m_chunkVAOs;
    std::unordered_map<ChunkPos, GLuint> m_chunkVBOs;
    std::unordered_map<ChunkPos, GLuint> m_chunkEBOs;
    std::unordered_map<ChunkPos, size_t> m_chunkIndexCounts;
#endif

    void createNewWorld(const WorldCreationParams& params);
    void createNewWorld(const std::string& name, int seed, bool is3D = true, int gameStyle2D = 0); // Deprecated, for backward compatibility
    void loadWorld(const std::string& name);
    void setupInputCallbacks();
    void initializeGameSystems(); // Helper for common initialization
    void createDemoEntities();    // Create demo entities for Inspector demonstration

#ifndef _WIN32
    // User data for GLFW callbacks (not needed for Win32)
    struct CallbackUserData {
        InputManagerType* inputManager;
        WindowType* window;
    };
    std::unique_ptr<CallbackUserData> m_callbackUserData;
#endif
};

} // namespace fresh
