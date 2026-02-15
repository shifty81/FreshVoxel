#include "core/Engine.h"

// Suppress C4244 warning from MSVC's xutility header (line 4813)
// The warning "conversion from 'wchar_t' to 'char', possible loss of data" occurs in system
// headers when STL containers/algorithms are instantiated with types from Windows API.
// This is triggered by Win32Window's WINDOW_CLASS_NAME (wchar_t*) interacting with STL.
// Since we use ANSI versions of Windows APIs (GetModuleFileNameA, MessageBoxA, etc.),
// suppressing this system header warning is safe.
#ifdef _MSC_VER
    #pragma warning(push)
    #pragma warning(disable: 4244)
#endif

#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream>
#include <thread>
#include <vector>

#ifdef _WIN32
    #include <windows.h>
    #include <shellapi.h>  // For ShellExecuteA
#elif defined(__APPLE__)
    #include <mach-o/dyld.h>
    #include <limits.h>
#else
    #include <unistd.h>
    #include <linux/limits.h>
#endif

#ifdef _WIN32
    #include "core/Win32Window.h"
    #include "core/GamePlayWindow.h"
    #include "input/Win32InputManager.h"
    #include "ui/native/Win32MenuBar.h"
    #include "ui/native/Win32Toolbar.h"
    #include "ui/native/Win32HUD.h"
    #include "ui/native/Win32ViewportPanel.h"
#else
    #include <GLFW/glfw3.h>
    #include "core/Window.h"
    #include "input/InputManager.h"
#endif

#ifdef _MSC_VER
    #pragma warning(pop)
#endif

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "ai/AISystem.h"
#include "core/Logger.h"
#include "core/Project.h"
#include "ecs/EntityManager.h"
#include "ecs/TransformComponent.h"
#include "ecs/RendererComponent.h"
#include "ecs/MaterialComponent.h"
#include "editor/EditorGUI.h"
#include "editor/EditorManager.h"
#include "editor/TerraformingSystem.h"
#include "editor/TransformGizmo.h"
#include "editor/WorldEditor.h"
#include "gameplay/Player.h"
#include "gameplay/TimeManager.h"
#include "gameplay/SeasonManager.h"
#include "gameplay/WeatherManager.h"
#include "gameplay/Raft.h"
#include "generation/TerrainGenerator.h"
#include "generation/IWorldGenerator.h"
#include "interaction/VoxelInteraction.h"
#include "physics/PhysicsComponent.h"
#include "physics/PhysicsSystem.h"
#include "renderer/GraphicsAPI.h"
#include "renderer/RenderContext.h"
#include "scripting/lua/LuaScriptingEngine.h"
#include "scripting/lua/LuaECSBindings.h"
#include "scripting/lua/LuaTimeBindings.h"
#include "serialization/WorldSerializer.h"
#include "ui/EditorToolbar.h"
#include "ui/HotbarPanel.h"
#include "ui/MainMenu.h"
// Removed ImGui-based panel: #include "ui/MainMenuPanel.h"
#include "ui/SceneHierarchyPanel.h"
#include "ui/VoxelToolPalette.h"
#include "voxel/Chunk.h"
#include "voxel/VoxelTypes.h"
#include "voxel/VoxelWorld.h"

#if defined(FRESH_OPENGL_SUPPORT) && defined(FRESH_GLEW_AVAILABLE)
    #include <GL/glew.h>
#endif

#ifdef _WIN32
    #include "renderer/backends/DirectX11RenderContext.h"
    #include "renderer/backends/DirectX12RenderContext.h"
#endif

namespace fresh
{

// Platform-specific key code definitions
#ifdef _WIN32
    // Win32 virtual key codes
    constexpr int KEY_0 = '0';
    constexpr int KEY_1 = '1';
    constexpr int KEY_2 = '2';
    constexpr int KEY_3 = '3';
    constexpr int KEY_4 = '4';
    constexpr int KEY_5 = '5';
    constexpr int KEY_6 = '6';
    constexpr int KEY_7 = '7';
    constexpr int KEY_8 = '8';
    constexpr int KEY_9 = '9';
    constexpr int KEY_E = 'E';
    constexpr int KEY_F = 'F';
    constexpr int KEY_R = 'R';
    constexpr int KEY_B = 'B';  // Boat/raft interaction
    constexpr int KEY_W = 'W';
    constexpr int KEY_Y = 'Y';
    constexpr int KEY_Z = 'Z';
    constexpr int KEY_ESCAPE = VK_ESCAPE;
    constexpr int KEY_LEFT_CONTROL = VK_LCONTROL;
    constexpr int KEY_RIGHT_CONTROL = VK_RCONTROL;
    constexpr int KEY_LEFT_SHIFT = VK_LSHIFT;
    constexpr int KEY_RIGHT_SHIFT = VK_RSHIFT;
    constexpr int MOUSE_BUTTON_LEFT = VK_LBUTTON;
    constexpr int MOUSE_BUTTON_RIGHT = VK_RBUTTON;
#else
    // GLFW key codes
    constexpr int KEY_0 = GLFW_KEY_0;
    constexpr int KEY_1 = GLFW_KEY_1;
    constexpr int KEY_2 = GLFW_KEY_2;
    constexpr int KEY_3 = GLFW_KEY_3;
    constexpr int KEY_4 = GLFW_KEY_4;
    constexpr int KEY_5 = GLFW_KEY_5;
    constexpr int KEY_6 = GLFW_KEY_6;
    constexpr int KEY_7 = GLFW_KEY_7;
    constexpr int KEY_8 = GLFW_KEY_8;
    constexpr int KEY_9 = GLFW_KEY_9;
    constexpr int KEY_E = GLFW_KEY_E;
    constexpr int KEY_F = GLFW_KEY_F;
    constexpr int KEY_R = GLFW_KEY_R;
    constexpr int KEY_B = GLFW_KEY_B;  // Boat/raft interaction
    constexpr int KEY_W = GLFW_KEY_W;
    constexpr int KEY_Y = GLFW_KEY_Y;
    constexpr int KEY_Z = GLFW_KEY_Z;
    constexpr int KEY_ESCAPE = GLFW_KEY_ESCAPE;
    constexpr int KEY_LEFT_CONTROL = GLFW_KEY_LEFT_CONTROL;
    constexpr int KEY_RIGHT_CONTROL = GLFW_KEY_RIGHT_CONTROL;
    constexpr int KEY_LEFT_SHIFT = GLFW_KEY_LEFT_SHIFT;
    constexpr int KEY_RIGHT_SHIFT = GLFW_KEY_RIGHT_SHIFT;
    constexpr int MOUSE_BUTTON_LEFT = GLFW_MOUSE_BUTTON_LEFT;
    constexpr int MOUSE_BUTTON_RIGHT = GLFW_MOUSE_BUTTON_RIGHT;
#endif // Rendering constants
namespace
{
constexpr float MAX_INTERACTION_DISTANCE = 5.0f;
constexpr float CROSSHAIR_SIZE = 0.02f;
constexpr float CROSSHAIR_LINE_WIDTH = 2.0f;
constexpr int SHADER_INFO_LOG_SIZE = 512;

// Camera and spawn constants
constexpr float DEFAULT_CAMERA_YAW = -90.0f;    // Look forward (negative X direction)
constexpr float DEFAULT_CAMERA_PITCH = 0.0f;    // Level horizon
constexpr glm::vec3 DEFAULT_SPAWN_POSITION(0.0f, 100.0f, 0.0f);  // Center, above terrain

#if defined(FRESH_OPENGL_SUPPORT) && defined(FRESH_GLEW_AVAILABLE)
[[maybe_unused]] const char* VOXEL_VERTEX_SHADER = "shaders/voxel.vert";
[[maybe_unused]] const char* VOXEL_FRAGMENT_SHADER = "shaders/voxel.frag";
[[maybe_unused]] const char* CROSSHAIR_VERTEX_SHADER = "shaders/crosshair.vert";
[[maybe_unused]] const char* CROSSHAIR_FRAGMENT_SHADER = "shaders/crosshair.frag";
#endif

/**
 * @brief Get the directory path where the executable is located
 * @return Path to the executable's directory, or empty string on failure
 */
std::string getExecutableDirectory()
{
#ifdef _WIN32
    char buffer[MAX_PATH];
    DWORD length = GetModuleFileNameA(NULL, buffer, MAX_PATH);
    if (length == 0 || length == MAX_PATH) {
        return "";
    }
    std::string exePath(buffer);
    // Find last slash or backslash
    size_t lastSlash = exePath.find_last_of("\\/");
    if (lastSlash != std::string::npos) {
        return exePath.substr(0, lastSlash);
    }
    return "";
#elif defined(__APPLE__)
    char buffer[PATH_MAX];
    uint32_t size = PATH_MAX;
    if (_NSGetExecutablePath(buffer, &size) != 0) {
        return "";
    }
    std::string exePath(buffer);
    size_t lastSlash = exePath.find_last_of('/');
    if (lastSlash != std::string::npos) {
        return exePath.substr(0, lastSlash);
    }
    return "";
#else // Linux
    char buffer[PATH_MAX];
    ssize_t length = readlink("/proc/self/exe", buffer, PATH_MAX - 1);
    if (length == -1) {
        return "";
    }
    buffer[length] = '\0';
    std::string exePath(buffer);
    size_t lastSlash = exePath.find_last_of('/');
    if (lastSlash != std::string::npos) {
        return exePath.substr(0, lastSlash);
    }
    return "";
#endif
}

} // namespace

Engine::Engine() : m_running(false), m_inGame(false), m_selectedBlockType(VoxelType::Stone), 
                   m_lastCursorCaptured(false), m_rightMouseHeldForCamera(false) 
{
    LOG_INFO("Engine constructor called");
}

Engine::~Engine()
{
    shutdown();
}

bool Engine::initialize()
{
    std::cout << "Initializing Fresh Voxel Engine..." << std::endl;
    LOG_INFO_C("Initializing Fresh Voxel Engine...", "Engine");

    // Initialize project manager
    m_projectManager = std::make_unique<ProjectManager>();
    LOG_INFO_C("Project manager initialized", "Engine");

    // Initialize world generator plugin system
    WorldGeneratorFactory::registerBuiltInGenerators();
    LOG_INFO_C("World generator plugin system initialized", "Engine");

    // Create renderer using the abstraction layer first to determine API
    // Auto-select best graphics API for the platform
    m_renderer = RenderContextFactory::createBest();
    if (!m_renderer) {
        std::cerr << "Failed to create render context" << std::endl;
        LOG_ERROR_C("Failed to create render context", "Engine");
        return false;
    }
    std::cout << "Render context created: " << getGraphicsAPIName(m_renderer->getAPI())
              << std::endl;
    LOG_INFO_C("Render context created: " + std::string(getGraphicsAPIName(m_renderer->getAPI())),
               "Engine");

    // Determine if we need OpenGL context for the window based on selected API
    bool useOpenGL = (m_renderer->getAPI() == GraphicsAPI::OpenGL);

    // Create window FIRST so we can show the GUI
    m_window = std::make_unique<WindowType>(1280, 720, "Fresh Voxel Engine");
    if (!m_window->initialize(useOpenGL)) {
        std::cerr << "Failed to initialize window" << std::endl;
        LOG_ERROR_C("Failed to initialize window", "Engine");
        return false;
    }
    std::cout << "Window created" << std::endl;
    LOG_INFO_C("Window created", "Engine");

#ifdef _WIN32
    // Set up native Win32 menu bar
    setupNativeMenuBar();
    // Set up native Win32 toolbar
    setupNativeToolbar();
#endif // Initialize renderer with the window
    if (!m_renderer->initialize(m_window.get())) {
        std::cerr << "Failed to initialize renderer" << std::endl;
        LOG_ERROR_C("Failed to initialize renderer", "Engine");
        return false;
    }
    std::cout << "Renderer initialized with " << getGraphicsAPIName(m_renderer->getAPI())
              << std::endl;
    LOG_INFO_C("Renderer initialized with " + std::string(getGraphicsAPIName(m_renderer->getAPI())),
               "Engine");

    // Create input manager and set up callbacks
    m_inputManager = std::make_unique<InputManagerType>();
#ifdef _WIN32
    // Win32: Initialize with window pointer and set up callbacks
    m_inputManager->initialize(m_window.get());
    
    m_window->setKeyCallback([this](int vk, bool isDown) {
        m_inputManager->processKeyEvent(vk, isDown);
    });
    
    m_window->setMouseMoveCallback([this](int x, int y) {
        m_inputManager->processMouseMovement(x, y);
    });
    
    m_window->setMouseButtonCallback([this](int button, bool isDown) {
        m_inputManager->processMouseButton(button, isDown);
    });
#else
    // GLFW: Initialize with window handle
    m_inputManager->initialize(m_window->getHandle());
#endif
    m_inputManager->setInputMode(InputMode::UIMode); // Start in UI mode for menu
    setupInputCallbacks();
    std::cout << "Input manager initialized" << std::endl;
    LOG_INFO_C("Input manager initialized", "Engine");

    // EDITOR-FIRST DESIGN: Console main menu deprecated
    // The editor handles scene/world creation through native Win32 dialogs
    // Use File > New Scene or File > Open Scene from the menu bar
    std::cout << "\n========================================" << std::endl;
    std::cout << "  Fresh Voxel Engine - Game Editor" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Editor-First Workflow:" << std::endl;
    std::cout << "  - Create scenes via File > New Scene" << std::endl;
    std::cout << "  - Open scenes via File > Open Scene" << std::endl;
    std::cout << "  - All tools accessible in editor" << std::endl;
    std::cout << "========================================\n" << std::endl;
    
    // Keep main menu instance for backward compatibility (not actively used)
    m_mainMenu = std::make_unique<MainMenu>();
    // Skip main menu initialization - editor handles everything now
    LOG_INFO_C("Editor-first mode: Using native Win32 editor for scene management", "Engine");

    // Create entity manager for ECS
    m_entityManager = std::make_unique<ecs::EntityManager>();
    std::cout << "Entity manager initialized" << std::endl;
    LOG_INFO_C("Entity manager initialized", "Engine");

    // Initialize scripting engine with hot-reload support
    m_scriptingEngine = std::make_unique<scripting::LuaScriptingEngine>();
    if (m_scriptingEngine->initialize()) {
        // Enable hot-reload for development builds
        m_scriptingEngine->enableHotReload(true);
        std::cout << "Lua scripting engine initialized (hot-reload enabled)" << std::endl;
        LOG_INFO_C("Lua scripting engine initialized with hot-reload support", "Engine");
        
        // Register ECS bindings with the scripting engine
        scripting::LuaECSBindings::registerBindings(m_scriptingEngine.get(), m_entityManager.get());
        LOG_INFO_C("ECS bindings registered with Lua", "Engine");
        
        // Register Time bindings with the scripting engine
        scripting::LuaTimeBindings::registerBindings(m_scriptingEngine.get(), m_timeManager.get());
        LOG_INFO_C("Time bindings registered with Lua", "Engine");
    } else {
        std::cerr << "Warning: Failed to initialize Lua scripting engine" << std::endl;
        LOG_WARNING_C("Failed to initialize Lua scripting engine", "Engine");
    }

    // Create comprehensive editor manager (uses Windows Native Win32 UI) - show immediately
    m_editorManager = std::make_unique<EditorManager>();
    
    // Set project manager reference in editor manager
    m_editorManager->setProjectManager(m_projectManager.get());
    
    // Initialize with nullptr for world and worldEditor initially
    if (!m_editorManager->initialize(m_window.get(), m_renderer.get(), nullptr, nullptr,
                                     m_inputManager.get(), m_entityManager.get())) {
        std::cerr << "Failed to initialize editor manager" << std::endl;
        LOG_ERROR_C("Failed to initialize editor manager", "Engine");
        return false;
    }
    
    // Set callbacks for world operations so EditorManager can trigger them
    m_editorManager->setWorldCreationCallback([this](const WorldCreationParams& params) {
        LOG_INFO_C("World creation callback triggered: " + params.name, "Engine");
        this->createNewWorld(params);
    });
    
    m_editorManager->setWorldLoadCallback([this](const std::string& name) {
        LOG_INFO_C("World load callback triggered: " + name, "Engine");
        this->loadWorld(name);
    });
    
    m_editorManager->setEnterPlayModeCallback([this]() {
        LOG_INFO_C("Enter play mode callback triggered", "Engine");
        this->enterPlayMode();
    });
    
    m_editorManager->setExitPlayModeCallback([this]() {
        LOG_INFO_C("Exit play mode callback triggered", "Engine");
        this->exitPlayMode();
    });
    
#ifdef _WIN32
    // VIEWPORT INTEGRATION - Set up viewport rendering after EditorManager creation
    // This is the ONLY place we configure the viewport for rendering during initialization
    if (m_editorManager && m_editorManager->getViewportPanel() && m_renderer) {
        auto* viewportPanel = m_editorManager->getViewportPanel();
        HWND viewportHwnd = viewportPanel->getHandle();
        
        if (viewportHwnd) {
            int vpWidth = viewportPanel->getWidth();
            int vpHeight = viewportPanel->getHeight();
            
            LOG_INFO_C("Initial viewport configuration: " + 
                       std::to_string(vpWidth) + "x" + std::to_string(vpHeight), "Engine");
            
            // CRITICAL FIX: Validate dimensions before attempting swap chain creation
            if (vpWidth <= 0 || vpHeight <= 0) {
                LOG_WARNING_C("Viewport has invalid dimensions at initialization, forcing window resize event", "Engine");
                
                // Force a window resize event to recalculate panel dimensions
                Win32Window* win32Window = dynamic_cast<Win32Window*>(m_window.get());
                if (win32Window) {
                    RECT clientRect;
                    HWND hwnd = win32Window->getHandle();
                    if (hwnd && GetClientRect(hwnd, &clientRect)) {
                        int clientWidth = clientRect.right - clientRect.left;
                        int clientHeight = clientRect.bottom - clientRect.top;
                        m_editorManager->onWindowResize(clientWidth, clientHeight);
                        
                        // Re-read viewport dimensions after resize
                        vpWidth = viewportPanel->getWidth();
                        vpHeight = viewportPanel->getHeight();
                        LOG_INFO_C("Viewport dimensions after resize: " + 
                                  std::to_string(vpWidth) + "x" + std::to_string(vpHeight), "Engine");
                    }
                }
            }
            
            if (vpWidth > 0 && vpHeight > 0) {
                // Set viewport window handle and create swap chain
                LOG_INFO_C("Setting viewport window handle and creating swap chain...", "Engine");
                
                if (m_renderer->setViewportWindow(viewportHwnd)) {
                    LOG_INFO_C("Viewport window handle set successfully", "Engine");
                    
                    if (m_renderer->recreateSwapChain(vpWidth, vpHeight)) {
                        LOG_INFO_C("✓ Viewport swap chain created successfully: " + 
                                  std::to_string(vpWidth) + "x" + std::to_string(vpHeight), "Engine");
                        m_viewportSwapChainReady = true;
                        
                        // Update camera aspect ratio if player exists
                        if (m_player) {
                            float aspectRatio = static_cast<float>(vpWidth) / static_cast<float>(vpHeight);
                            m_player->getCamera().setAspectRatio(aspectRatio);
                        }
                    } else {
                        LOG_ERROR_C("✗ Failed to create viewport swap chain - will retry in updateEditor", "Engine");
                    }
                } else {
                    LOG_ERROR_C("✗ Failed to set viewport window handle - will retry in updateEditor", "Engine");
                }
            } else {
                LOG_WARNING_C("Invalid viewport dimensions after retry - will retry in updateEditor: " + 
                           std::to_string(vpWidth) + "x" + std::to_string(vpHeight), "Engine");
            }
        } else {
            LOG_WARNING_C("Viewport has no window handle yet - will retry in updateEditor", "Engine");
        }
    } else {
        LOG_WARNING_C("Viewport panel or renderer not available yet - will retry in updateEditor", "Engine");
    }
#endif
    
    m_editorManager->setVisible(true); // Show editor immediately
    std::cout << "Editor manager initialized" << std::endl;
    LOG_INFO_C("Editor manager initialized", "Engine");

#ifdef _WIN32
    // Trigger initial panel layout after EditorManager initialization
    // This ensures all panels are properly positioned based on the current window size
    Win32Window* win32Window = dynamic_cast<Win32Window*>(m_window.get());
    if (win32Window && m_editorManager) {
        RECT clientRect;
        HWND hwnd = win32Window->getHandle();
        if (hwnd && GetClientRect(hwnd, &clientRect)) {
            int clientWidth = clientRect.right - clientRect.left;
            int clientHeight = clientRect.bottom - clientRect.top;
            m_editorManager->onWindowResize(clientWidth, clientHeight);
            LOG_INFO_C("Triggered initial panel layout update: " + 
                      std::to_string(clientWidth) + "x" + std::to_string(clientHeight), "Engine");
            
            // Refresh all panels to ensure they are visible after positioning
            m_editorManager->refreshAllPanels();
            LOG_INFO_C("Refreshed all panels for initial visibility", "Engine");
            
            // Process any pending Windows messages to ensure panels are painted
            m_window->pollEvents();
            LOG_INFO_C("Processed initial window messages", "Engine");
        }
    }
#endif

    m_running = true;
    m_inGame = false; // Start in menu mode
    return true;
}

void Engine::createNewWorld(const WorldCreationParams& params)
{
    const char* styleNames[] = {"Platformer/Terraria", "Top-down/Zelda"};
    const char* sizeNames[] = {"Small", "Medium", "Large", "Huge"};
    const char* terrainNames[] = {"Flat", "Hills", "Mountains", "Islands"};
    const char* biomeNames[] = {"Forest", "Desert", "Snow", "Jungle", "Mixed"};
    
    std::cout << "\nCreating new " << (params.is3D ? "3D" : "2D") << " world: " << params.name;
    if (!params.is3D) {
        std::cout << " (Style: " << styleNames[params.gameStyle2D] << ")";
    }
    std::cout << " (seed: " << params.seed << ")" << std::endl;
    std::cout << "  Size: " << sizeNames[static_cast<int>(params.worldSize)] << std::endl;
    std::cout << "  Terrain: " << terrainNames[static_cast<int>(params.terrainType)] << std::endl;
    std::cout << "  Biome: " << biomeNames[static_cast<int>(params.biomeType)] << std::endl;
    
    // Delegate to old implementation for now (will be enhanced later)
    createNewWorld(params.name, params.seed, params.is3D, params.gameStyle2D);
}

void Engine::createNewWorld(const std::string& name, int seed, bool is3D, int gameStyle2D)
{
    const char* styleNames[] = {"Platformer/Terraria", "Top-down/Zelda"};
    std::cout << "\n[Legacy method] Creating new " << (is3D ? "3D" : "2D") << " world: " << name;
    if (!is3D) {
        std::cout << " (Style: " << styleNames[gameStyle2D] << ")";
    }
    std::cout << " (seed: " << seed << ")" << std::endl;

    // Store world type and style for camera setup
    m_isWorld3D = is3D;
    m_world2DStyle = gameStyle2D;

    // Create voxel world
    m_world = std::make_unique<VoxelWorld>();
    if (!m_world->initialize()) {
        std::cerr << "Failed to initialize voxel world" << std::endl;
        return;
    }
    std::cout << "Voxel world initialized" << std::endl;

    // Set custom seed for terrain generator
    m_world->setSeed(seed);
    
    // Set 2D world style if applicable
    if (!is3D) {
        m_world->set2DStyle(gameStyle2D);
    }

    // Generate initial chunks around spawn
    std::cout << "Generating initial terrain..." << std::endl;
    LOG_INFO_C("Starting world generation - entering generation mode", "Engine");
    
    // Set world generation flag to prevent world rendering during generation
    m_isGeneratingWorld = true;
    
    int chunkRadius = 3;
    int totalChunks = (2 * chunkRadius + 1) * (2 * chunkRadius + 1);
    int chunksGenerated = 0;

    for (int x = -chunkRadius; x <= chunkRadius; ++x) {
        for (int z = -chunkRadius; z <= chunkRadius; ++z) {
            m_world->loadChunk(ChunkPos(x, z));
            chunksGenerated++;

            if (chunksGenerated % 10 == 0 || chunksGenerated == totalChunks) {
                std::cout << "  Generated " << chunksGenerated << "/" << totalChunks << " chunks..."
                          << std::endl;
                LOG_INFO_C("World generation progress: " + std::to_string(chunksGenerated) + "/" + 
                          std::to_string(totalChunks) + " chunks", "Engine");
                
                // Process Windows messages periodically during generation to keep UI responsive
                // This prevents "Not Responding" and allows cancel operations
#ifdef _WIN32
                if (m_window) {
                    m_window->pollEvents();
                }
#endif
            }
        }
    }
    
    // Clear world generation flag - world is now ready to render
    m_isGeneratingWorld = false;
    LOG_INFO_C("World generation complete - exiting generation mode", "Engine");

    std::cout << "World '" << name << "' created successfully!" << std::endl;
    std::cout << "Total chunks: " << m_world->getChunks().size() << std::endl;

    // Now initialize game systems after world is created
    initializeGameSystems();

    // Save the world
    WorldSerializer serializer;
    std::string savePath = std::string("saves/") + name + ".world";
    if (serializer.saveWorld(m_world.get(), savePath)) {
        std::cout << "World saved to: " << savePath << std::endl;
    }
}

void Engine::loadWorld(const std::string& name)
{
    std::cout << "\nLoading world: " << name << std::endl;

    // Create voxel world
    m_world = std::make_unique<VoxelWorld>();
    if (!m_world->initialize()) {
        std::cerr << "Failed to initialize voxel world" << std::endl;
        return;
    }

    // Load the world from file
    WorldSerializer serializer;
    std::string loadPath = std::string("saves/") + name + ".world";

    if (serializer.loadWorld(m_world.get(), loadPath)) {
        std::cout << "World '" << name << "' loaded successfully!" << std::endl;
    } else {
        std::cerr << "Failed to load world. Creating new world instead..." << std::endl;
        // Fall back to creating a new world
        createNewWorld(name, static_cast<int>(std::time(nullptr)));
        return; // createNewWorld will call initializeGameSystems
    }

    // Now initialize game systems after world is loaded
    initializeGameSystems();
}

void Engine::initializeGameSystems()
{
    // This function sets up all game systems after a world is created/loaded

    // Create physics system
    if (!m_physics) {
        m_physics = std::make_unique<PhysicsSystem>();
        if (!m_physics->initialize()) {
            LOG_ERROR_C("Failed to initialize physics system", "Engine");
            return;
        }
        std::cout << "Physics system initialized" << std::endl;
    }

    // Create AI system
    if (!m_aiSystem) {
        m_aiSystem = std::make_unique<AISystem>();
        if (!m_aiSystem->initialize()) {
            LOG_ERROR_C("Failed to initialize AI system", "Engine");
            return;
        }
        std::cout << "AI system initialized" << std::endl;
    }

    // Create/update world editor
    if (!m_worldEditor) {
        m_worldEditor = std::make_unique<WorldEditor>();
    }
    if (m_world) {
        m_worldEditor->initialize(m_world.get());
        m_worldEditor->setEnabled(true);
        std::cout << "World editor initialized" << std::endl;
    }

    // Create legacy editor GUI (for backward compatibility)
    if (!m_editor) {
        m_editor = std::make_unique<EditorGUI>();
        if (m_worldEditor && !m_editor->initialize(m_worldEditor.get())) {
            LOG_ERROR_C("Failed to initialize editor GUI", "Engine");
        } else {
            m_editor->setVisible(false); // Hide by default
            std::cout << "Editor GUI initialized" << std::endl;
        }
    }

    // Update editor manager with the world instead of shutting down and re-initializing
    if (m_editorManager && m_world && m_worldEditor) {
        // Update the editor manager with world and world editor references
        if (!m_editorManager->updateWorld(m_world.get(), m_worldEditor.get())) {
            LOG_ERROR_C("Failed to update editor manager with world", "Engine");
            LOG_WARNING_C("Editor will be unavailable. Game will continue without editor UI.", "Engine");
            std::cerr << "Warning: Editor manager update failed. Continuing without editor UI." << std::endl;
        } else {
            m_editorManager->setVisible(true);
            std::cout << "Editor manager updated with world" << std::endl;
            
            // Set up play mode callback to toggle between editor mode and separate game window
            auto* toolbar = m_editorManager->getToolbar();
            if (toolbar) {
                toolbar->setPlayModeCallback([this](EditorToolbar::PlayMode mode) {
                    switch (mode) {
                        case EditorToolbar::PlayMode::Playing:
                            // Enter play mode - open separate game window
                            enterPlayMode();
                            break;
                        case EditorToolbar::PlayMode::Stopped:
                        case EditorToolbar::PlayMode::Paused:
                            // Exit play mode - close game window, return to editor
                            exitPlayMode();
                            break;
                    }
                });
            }
            
            // Connect voxel tool palette voxel type changes to engine's selected block type
            auto* voxelToolPalette = m_editorManager->getVoxelToolPalette();
            if (voxelToolPalette) {
                voxelToolPalette->setVoxelTypeCallback([this](VoxelType type) {
                    m_selectedBlockType = type;
                    LOG_INFO_C("Selected block type from tool palette: " + 
                              std::to_string(static_cast<int>(type)), "Engine");
                });
            }
        }
    }
    
    // Initialize rendering based on API
#if defined(FRESH_OPENGL_SUPPORT) && defined(FRESH_GLEW_AVAILABLE)
    if (m_renderer->getAPI() == GraphicsAPI::OpenGL) {
        initializeRendering();
    }
#endif // DirectX 11 rendering confirmation
    if (m_renderer->getAPI() == GraphicsAPI::DirectX11) {
        std::cout << "DirectX 11 voxel rendering ready" << std::endl;
    }

    // DirectX 12 confirmation
    if (m_renderer->getAPI() == GraphicsAPI::DirectX12) {
        std::cout << "DirectX 12 voxel rendering ready" << std::endl;
    }

    // Create voxel interaction system
    if (!m_voxelInteraction) {
        m_voxelInteraction = std::make_unique<VoxelInteraction>();
    }
    m_voxelInteraction->initialize(m_world.get());
    std::cout << "Voxel interaction initialized" << std::endl;

    // Create player
    if (!m_player) {
        m_player = std::make_unique<Player>();
    }
    m_player->setWorld(m_world.get());
    m_player->setPosition(glm::vec3(0.0f, 80.0f, 0.0f));
    std::cout << "Player initialized" << std::endl;
    
    // Create time manager
    if (!m_timeManager) {
        m_timeManager = std::make_unique<TimeManager>();
    }
    std::cout << "Time manager initialized" << std::endl;
    
    // Create season manager
    if (!m_seasonManager) {
        m_seasonManager = std::make_unique<SeasonManager>();
    }
    m_seasonManager->setSeason(Season::Spring);  // Start with spring
    std::cout << "Season manager initialized" << std::endl;
    LOG_INFO_C("Season manager initialized with Spring season", "Engine");
    
    // Create weather manager
    if (!m_weatherManager) {
        m_weatherManager = std::make_unique<WeatherManager>();
    }
    m_weatherManager->setWeather(WeatherType::Clear);  // Start with clear weather
    m_weatherManager->setCurrentSeason(0);  // Spring = 0
    std::cout << "Weather manager initialized" << std::endl;
    LOG_INFO_C("Weather manager initialized with Clear weather", "Engine");
    
    // Set player reference in editor manager for camera-based operations like paste
    if (m_editorManager) {
        m_editorManager->setPlayer(m_player.get());
        LOG_INFO_C("Editor manager updated with player reference", "Engine");
    }
    
    // Set up camera mode based on world type
    if (!m_isWorld3D) {
        Camera& camera = m_player->getCamera();
        if (m_world2DStyle == 0) {
            // Platformer/Terraria style - side-scrolling
            camera.setCameraMode(CameraMode::Orthographic2D);
            camera.setOrthographicZoom(1.0f);
            // Position camera for side view
            m_player->setPosition(glm::vec3(0.0f, 10.0f, 50.0f));
            std::cout << "Camera set to 2D side-scrolling mode (Platformer/Terraria)" << std::endl;
        } else {
            // Top-down Zelda style
            camera.setCameraMode(CameraMode::OrthographicTopDown);
            camera.setOrthographicZoom(1.0f);
            // Position camera above the world
            m_player->setPosition(glm::vec3(0.0f, 50.0f, 0.0f));
            std::cout << "Camera set to 2D top-down mode (Zelda-like)" << std::endl;
        }
    } else {
        // Standard 3D perspective camera
        Camera& camera = m_player->getCamera();
        camera.setCameraMode(CameraMode::Perspective3D);
        m_player->setPosition(glm::vec3(0.0f, 80.0f, 0.0f));
        std::cout << "Camera set to 3D perspective mode" << std::endl;
    }

    // Initialize hotbar with common block types
    if (m_editorManager && m_editorManager->getHotbar()) {
        auto* hotbar = m_editorManager->getHotbar();
        
        // Set up common block types in hotbar slots
        std::vector<VoxelType> commonBlocks = {
            VoxelType::Stone,    // Slot 1
            VoxelType::Dirt,     // Slot 2
            VoxelType::Grass,    // Slot 3
            VoxelType::Wood,     // Slot 4
            VoxelType::Sand,     // Slot 5
            VoxelType::Brick,    // Slot 6
            VoxelType::Glass,    // Slot 7
            VoxelType::Leaves,   // Slot 8
            VoxelType::Water,    // Slot 9
            VoxelType::Snow      // Slot 0
        };
        
        for (size_t i = 0; i < commonBlocks.size() && i < HotbarPanel::HOTBAR_SIZE; ++i) {
            HotbarPanel::HotbarSlot slot;
            slot.isEmpty = false;
            slot.voxelType = commonBlocks[i];
            slot.count = 1;
            hotbar->setSlot(static_cast<int>(i), slot);
        }
        
        // Connect hotbar slot selection to block type change
        hotbar->setSlotActivatedCallback([this](int slotIndex) {
            if (m_editorManager && m_editorManager->getHotbar()) {
                const auto& slot = m_editorManager->getHotbar()->getSlot(slotIndex);
                if (!slot.isEmpty) {
                    m_selectedBlockType = slot.voxelType;
                    LOG_INFO_C("Selected block type: " + std::to_string(static_cast<int>(m_selectedBlockType)), 
                              "Engine");
                }
            }
        });
        
        // Set initial selected block to match slot 0 (Stone)
        if (!commonBlocks.empty()) {
            m_selectedBlockType = commonBlocks[0];
            hotbar->setSelectedSlot(0);
        }
        
        LOG_INFO_C("Hotbar initialized with " + std::to_string(commonBlocks.size()) + " block types", 
                  "Engine");
    }
    m_inputManager->setInputMode(InputMode::UIMode);
    
    // Set the editor manager visible after world creation
    if (m_editorManager) {
        m_editorManager->setVisible(true);
        LOG_INFO_C("Editor manager set visible after viewport swap chain configuration", "Engine");
        
        // Force window refresh to ensure all panels are properly displayed
        #ifdef _WIN32
        Win32Window* win32Window = dynamic_cast<Win32Window*>(m_window.get());
        if (win32Window) {
            HWND hwnd = win32Window->getHandle();
            if (hwnd) {
                InvalidateRect(hwnd, nullptr, TRUE);
                UpdateWindow(hwnd);
                LOG_INFO_C("Main window refreshed after world creation", "Engine");
            }
        }
        
        // Refresh all panels to ensure they are visible after world creation
        m_editorManager->refreshAllPanels();
        LOG_INFO_C("Refreshed all panels after world creation", "Engine");
        #endif
    }
    
    // Create demo entities for Inspector demonstration
    createDemoEntities();
    // Note: Do NOT set m_inGame = true here. The editor should remain visible.
    // m_inGame should only be set when explicitly entering play mode via enterPlayMode().
}

void Engine::run()
{
    auto lastTime = std::chrono::high_resolution_clock::now();
    int frameCount = 0;
    float fpsTimer = 0.0f;

    // Frame timing constants
    const float TARGET_FPS = 60.0f;
    const float TARGET_FRAME_TIME = 1.0f / TARGET_FPS; // 16.67ms
    const float MAX_DELTA_TIME = 0.1f;                 // Cap at 100ms to prevent physics issues

    while (m_running) {
        // Check if window should close
        if (m_window && m_window->shouldClose()) {
            m_running = false;
            break;
        }

        // Poll events
        if (m_window) {
            m_window->pollEvents();
        }

        // Calculate delta time
        auto currentTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        deltaTime = std::min(deltaTime, MAX_DELTA_TIME); // Clamp to prevent physics explosions
        lastTime = currentTime;

        // FPS counter
        frameCount++;
        fpsTimer += deltaTime;
        if (fpsTimer >= 1.0f) {
            // Update FPS in window title or status bar
            fpsTimer = 0.0f;
            frameCount = 0;
        }

        // UNREAL ENGINE WORKFLOW: Always update and render editor, regardless of world state
        // The viewport should show an empty scene with grid when no world exists
        // This allows immediate editor interaction and world creation from within the viewport
        
        // Update input
        processInput();
        
        // EDITOR-GAME SEPARATION: Update systems based on mode
        if (m_world) {
#ifdef _WIN32
            if (m_inGame && m_gamePlayWindow && m_gamePlayWindow->isOpen()) {
                // In play mode with separate game window:
                // Poll game window events and update game systems
                if (!m_gamePlayWindow->pollEvents()) {
                    // Game window was closed - return to editor
                    exitPlayMode();
                } else {
                    update(deltaTime);
                    renderGamePlayWindow();
                }
            } else if (m_inGame) {
                // Play mode but game window gone - exit play mode
                exitPlayMode();
            } else {
                // In editor mode: update only editor-relevant systems
                updateEditor(deltaTime);
            }
#else
            if (m_inGame) {
                // In play mode: update all game systems (physics, AI, player, etc.)
                update(deltaTime);
            } else {
                // In editor mode: update only editor-relevant systems
                // Editor camera movement, world editor tools, but not gameplay systems
                updateEditor(deltaTime);
            }
#endif
        } else {
            // No world yet - still update editor for viewport resize handling and UI
            updateEditor(deltaTime);
        }
        
        // Always render editor and viewport
        render();
        
        // Frame rate limiting - hybrid sleep approach for better accuracy
        auto frameEndTime = std::chrono::high_resolution_clock::now();
        float frameTime = std::chrono::duration<float>(frameEndTime - currentTime).count();

        if (frameTime < TARGET_FRAME_TIME) {
            float sleepTime = TARGET_FRAME_TIME - frameTime;

            // Sleep for most of the remaining time (leave 1ms for spin-wait)
            if (sleepTime > 0.002f) { // 2ms threshold
                std::this_thread::sleep_for(std::chrono::duration<float>(sleepTime - 0.001f));
            }

            // Busy-wait for the remaining time for precision
            while (std::chrono::duration<float>(std::chrono::high_resolution_clock::now() -
                                                currentTime)
                       .count() < TARGET_FRAME_TIME) {
                // Spin-wait for precise timing
            }
        }
    }
}

void Engine::shutdown()
{
#if defined(FRESH_OPENGL_SUPPORT) && defined(FRESH_GLEW_AVAILABLE)
    // Cleanup OpenGL rendering resources
    if (m_renderer && m_renderer->getAPI() == GraphicsAPI::OpenGL) {
        shutdownRendering();
    }
#endif

    if (m_renderer) {
        m_renderer->waitIdle();
    }

#ifndef _WIN32
    // Clean up GLFW callback user data
    m_callbackUserData.reset();
#endif
    // Explicitly shutdown editor manager BEFORE resetting renderer
    // This ensures proper cleanup during shutdown
    if (m_editorManager) {
        m_editorManager->shutdown();
        m_editorManager.reset();
    }

    // Shutdown scripting engine
    if (m_scriptingEngine) {
        m_scriptingEngine->shutdown();
        m_scriptingEngine.reset();
    }

    // Shutdown all pluggable systems in reverse order
    m_systemRegistry.shutdownAll();

    m_player.reset();
    m_inputManager.reset();
    m_editor.reset();
    m_aiSystem.reset();
    m_physics.reset();
    m_world.reset();
    m_renderer.reset();
    m_window.reset();

    m_running = false;
}

void Engine::processInput()
{
    // Reset mouse delta from previous frame BEFORE polling new events
    if (m_inputManager) {
        m_inputManager->resetMouseDelta();
    }

    if (m_window) {
        m_window->pollEvents();
    }

    // Update input manager state (handles Alt-hold mode switching)
    if (m_inputManager) {
        m_inputManager->update();

        // Check if GUI wants input (only process world input if GUI doesn't need it)
        bool guiCapturesMouse = m_editorManager && m_editorManager->wantCaptureMouse();
        (void)guiCapturesMouse; // May be unused in this scope
        bool guiCapturesKeyboard = m_editorManager && m_editorManager->wantCaptureKeyboard();
        
        // Handle hotbar key presses (1-0) when not in editor mode and GUI doesn't capture keyboard
        if (!guiCapturesKeyboard && m_editorManager && m_editorManager->getHotbar()) {
            auto* hotbar = m_editorManager->getHotbar();
            if (hotbar->isVisible()) {
                // Check keys 1-9 and 0
                for (int key = KEY_1; key <= KEY_9; ++key) {
                    if (m_inputManager->isKeyJustPressed(key)) {
                        hotbar->handleKeyPress(key);
                    }
                }
                if (m_inputManager->isKeyJustPressed(KEY_0)) {
                    hotbar->handleKeyPress(KEY_0);
                }
            }
        }
        
        // Handle keyboard shortcuts for editor actions (Ctrl+Z, Ctrl+Y, etc.)
        if (!guiCapturesKeyboard) {
            bool ctrlPressed = m_inputManager->isKeyPressed(KEY_LEFT_CONTROL) || 
                              m_inputManager->isKeyPressed(KEY_RIGHT_CONTROL);
            bool shiftPressed = m_inputManager->isKeyPressed(KEY_LEFT_SHIFT) || 
                               m_inputManager->isKeyPressed(KEY_RIGHT_SHIFT);
            
            // Ctrl+Z for Undo
            if (ctrlPressed && !shiftPressed && m_inputManager->isKeyJustPressed(KEY_Z)) {
                if (m_worldEditor && m_worldEditor->getTerraformingSystem()) {
                    if (m_worldEditor->getTerraformingSystem()->undo()) {
                        LOG_INFO_C("Undo performed (Ctrl+Z)", "Engine");
                    } else {
                        LOG_INFO_C("Nothing to undo", "Engine");
                    }
                }
            }
            
            // Ctrl+Y for Redo (or Ctrl+Shift+Z)
            if ((ctrlPressed && m_inputManager->isKeyJustPressed(KEY_Y)) ||
                (ctrlPressed && shiftPressed && m_inputManager->isKeyJustPressed(KEY_Z))) {
                if (m_worldEditor && m_worldEditor->getTerraformingSystem()) {
                    if (m_worldEditor->getTerraformingSystem()->redo()) {
                        LOG_INFO_C("Redo performed (Ctrl+Y)", "Engine");
                    } else {
                        LOG_INFO_C("Nothing to redo", "Engine");
                    }
                }
            }
            
            // Transform Gizmo mode shortcuts (W/E/R) - only when editor is active
            if (m_editorManager && m_editorManager->getTransformGizmo()) {
                auto* gizmo = m_editorManager->getTransformGizmo();
                
                // W key - Translate mode
                if (m_inputManager->isKeyJustPressed(KEY_W)) {
                    gizmo->setMode(TransformGizmo::Mode::Translate);
                    LOG_INFO_C("Transform Gizmo: Translate mode (W)", "Engine");
                    
                    // Update toolbar to reflect the change
                    if (m_editorManager->getToolbar()) {
                        m_editorManager->getToolbar()->setActiveTool(EditorToolbar::Tool::Move);
                    }
                }
                
                // E key - Rotate mode
                if (m_inputManager->isKeyJustPressed(KEY_E)) {
                    gizmo->setMode(TransformGizmo::Mode::Rotate);
                    LOG_INFO_C("Transform Gizmo: Rotate mode (E)", "Engine");
                    
                    // Update toolbar to reflect the change
                    if (m_editorManager->getToolbar()) {
                        m_editorManager->getToolbar()->setActiveTool(EditorToolbar::Tool::Rotate);
                    }
                }
                
                // R key - Scale mode
                if (m_inputManager->isKeyJustPressed(KEY_R)) {
                    gizmo->setMode(TransformGizmo::Mode::Scale);
                    LOG_INFO_C("Transform Gizmo: Scale mode (R)", "Engine");
                    
                    // Update toolbar to reflect the change
                    if (m_editorManager->getToolbar()) {
                        m_editorManager->getToolbar()->setActiveTool(EditorToolbar::Tool::Scale);
                    }
                }
                
                // F key - Focus on selection
                if (m_inputManager->isKeyJustPressed(KEY_F)) {
                    m_editorManager->focusOnSelection();
                    LOG_INFO_C("Camera: Focus on selection (F)", "Engine");
                }
            }
        }
        
        // F key to toggle mouse cursor capture (camera freelook vs GUI mode)
        if (!guiCapturesKeyboard && m_inputManager->isKeyJustPressed(KEY_F)) {
            m_inputManager->toggleCursorCapture();
            m_userToggledCursor = true;  // Track that user explicitly toggled
            if (m_inputManager->isCursorCaptured()) {
                LOG_INFO_C("Mouse captured - Camera freelook enabled", "Engine");
            } else {
                LOG_INFO_C("Mouse released - GUI mode enabled", "Engine");
            }
        }
        
        // ESC key to exit play mode (Unreal-style)
        if (!guiCapturesKeyboard && m_inputManager->isKeyJustPressed(KEY_ESCAPE)) {
            if (m_inGame) {
                LOG_INFO_C("ESC pressed - Exiting play mode", "Engine");
                exitPlayMode();
            }
        }
        
        // B key - Raft/Boat interaction (place or mount/dismount)
        if (!guiCapturesKeyboard && m_inputManager->isKeyJustPressed(KEY_B)) {
            if (m_raft && m_raft->isMounted()) {
                // Dismount from raft
                m_raft->dismount();
                if (m_player) {
                    m_player->setFreeFlightMode(false);  // Return to normal walking mode
                }
                LOG_INFO_C("Dismounted from raft (B key)", "Engine");
            } else if (m_raft && !m_raft->isMounted() && m_player) {
                // Try to mount raft if player is near it
                glm::vec3 playerPos = m_player->getPosition();
                glm::vec3 raftPos = m_raft->getPosition();
                float distance = glm::length(playerPos - raftPos);
                
                if (distance < 3.0f) {  // Within 3 blocks
                    m_raft->mount(m_player.get());
                    LOG_INFO_C("Mounted raft (B key)", "Engine");
                } else {
                    LOG_INFO_C("Too far from raft to mount", "Engine");
                }
            } else if (!m_raft && m_player && m_world) {
                // Place a new raft at player's position
                glm::vec3 playerPos = m_player->getPosition();
                glm::vec3 raftPlacePos = playerPos;
                raftPlacePos.y -= 1.5f;  // Place below player's feet
                
                if (Raft::canPlaceAt(raftPlacePos, m_world.get())) {
                    m_raft = std::make_unique<Raft>();
                    m_raft->initialize(raftPlacePos, m_world.get());
                    LOG_INFO_C("Placed raft in water (B key)", "Engine");
                } else {
                    LOG_INFO_C("Cannot place raft here - must be over water", "Engine");
                }
            }
        }

        // T key disabled - editor is always visible in new editor-first mode
        // Keeping this code commented out for reference
        // if (!guiCapturesKeyboard &&
        //     m_inputManager->isActionJustPressed(InputAction::ToggleEditor)) {
        //         //     if (m_editorManager) {
        //         m_editorManager->toggle();
        //         // Switch to UI mode when editor is visible, game mode when hidden
        //         if (m_editorManager->isVisible()) {
        //             m_inputManager->setInputMode(InputMode::UIMode);
        //         } else {
        //             m_inputManager->setInputMode(InputMode::GameMode);
        //         }
        //         LOG_INFO_C("Editor toggled", "Engine");
        //     }
        // #else
        //     LOG_INFO_C("Editor not available (ImGui required)", "Engine");
        // #endif // }

        // Allow ESC to close the window (only if GUI doesn't want keyboard)
        if (!guiCapturesKeyboard && m_inputManager->isActionJustPressed(InputAction::OpenMenu)) {
            // In editor mode: ESC exits play mode
            // In play mode: ESC would show pause menu (future enhancement)
            if (m_inGame) {
                // Exit play mode and return to editor
                exitPlayMode();
                LOG_INFO_C("ESC pressed - exiting play mode", "Engine");
            } else {
                // In editor mode: ESC exits application
                // Future: Could show main menu instead
                m_running = false;
                LOG_INFO_C("ESC pressed - closing application", "Engine");
            }
        }
    }
}

void Engine::update(float deltaTime)
{
    // GAME MODE UPDATE: Full gameplay simulation with physics, AI, and all game systems
    // This is called when in play mode (m_inGame == true)
    
    if (!m_world) {
        return;
    }

#ifdef _WIN32
    // Skip editor window/viewport resize handling during play mode.
    // The renderer's swap chain targets the game play window, not the editor viewport.
    if (!m_inGame) {
    // WINDOW RESIZE HANDLING - Update panel layouts when main window is resized
    if (m_window && m_editorManager) {
        Win32Window* win32Window = dynamic_cast<Win32Window*>(m_window.get());
        if (win32Window && win32Window->wasFramebufferResized()) {
            win32Window->resetFramebufferResizedFlag();
            
            RECT clientRect;
            HWND hwnd = win32Window->getHandle();
            if (hwnd && GetClientRect(hwnd, &clientRect)) {
                int clientWidth = clientRect.right - clientRect.left;
                int clientHeight = clientRect.bottom - clientRect.top;
                m_editorManager->onWindowResize(clientWidth, clientHeight);
            }
        }
    }
    
    // VIEWPORT RESIZE HANDLING - Check if viewport was resized and recreate swap chain
    if (m_editorManager && m_editorManager->getViewportPanel() && m_renderer) {
        auto* viewportPanel = m_editorManager->getViewportPanel();
        
        if (viewportPanel->wasResized()) {
            int vpWidth = viewportPanel->getWidth();
            int vpHeight = viewportPanel->getHeight();
            
            viewportPanel->clearResizedFlag();
            
            if (vpWidth > 0 && vpHeight > 0) {
                if (m_renderer->recreateSwapChain(vpWidth, vpHeight)) {
                    if (m_player) {
                        float aspectRatio = static_cast<float>(vpWidth) / static_cast<float>(vpHeight);
                        m_player->getCamera().setAspectRatio(aspectRatio);
                    }
                }
            }
        }
    }
    } // end if (!m_inGame)
#endif

    // Check if GUI wants input before processing player updates
    bool guiCapturesMouse = m_editorManager && m_editorManager->wantCaptureMouse();
    bool guiCapturesKeyboard = m_editorManager && m_editorManager->wantCaptureKeyboard();
    
    // In game mode, input manager is in GameMode, so cursor is always captured
    // Handle player input (game mode typically doesn't check GUI capture as UI is hidden)
    if (m_player && m_inputManager && !guiCapturesMouse && !guiCapturesKeyboard) {
        m_player->handleInput(*m_inputManager, deltaTime);

        // In game mode, mouse always controls camera
        glm::vec2 mouseDelta = m_inputManager->getMouseDelta();
        if (glm::length(mouseDelta) > 0.0f) {
            m_player->handleMouseMovement(mouseDelta.x, mouseDelta.y);
        }

        // Handle block placement/breaking in game mode
        if (m_voxelInteraction) {
            RayHit hit = m_voxelInteraction->performRaycast(m_player->getCamera(), MAX_INTERACTION_DISTANCE);

            // Left click to break block
            if (m_inputManager->isMouseButtonPressed(MOUSE_BUTTON_LEFT) && hit.hit) {
                m_voxelInteraction->breakBlock(hit);
            }

            // Right click to place block
            if (m_inputManager->isMouseButtonPressed(MOUSE_BUTTON_RIGHT) && hit.hit) {
                m_voxelInteraction->placeBlock(hit, m_selectedBlockType);
            }
        }
    }

    // Update player (physics, collision, gameplay)
    if (m_player) {
        m_player->update(deltaTime);
    }
    
    // Update time manager (day/night cycle)
    if (m_timeManager) {
        m_timeManager->update(deltaTime);
    }
    
    // Update season manager
    if (m_seasonManager) {
        m_seasonManager->update(deltaTime);
    }
    
    // Update weather manager
    if (m_weatherManager) {
        m_weatherManager->update(deltaTime);
    }
    
    // Update raft if it exists
    if (m_raft) {
        // Handle raft movement input if mounted
        if (m_raft->isMounted() && m_player && !guiCapturesKeyboard) {
            float forward = 0.0f;
            float strafe = 0.0f;
            float turn = 0.0f;
            
            if (m_inputManager->isActionActive(InputAction::MoveForward)) forward = 1.0f;
            if (m_inputManager->isActionActive(InputAction::MoveBackward)) forward = -1.0f;
            if (m_inputManager->isActionActive(InputAction::MoveRight)) strafe = 1.0f;
            if (m_inputManager->isActionActive(InputAction::MoveLeft)) strafe = -1.0f;
            
            // Use A/D or arrow keys for turning the raft
            if (m_inputManager->isActionActive(InputAction::MoveLeft)) turn = -1.0f;
            if (m_inputManager->isActionActive(InputAction::MoveRight)) turn = 1.0f;
            
            m_raft->handleInput(forward, strafe, turn, deltaTime);
        }
        
        m_raft->update(deltaTime);
    }

    // Update world with actual player position for chunk streaming
    if (m_player) {
        glm::vec3 playerPos = m_player->getPosition();
        m_world->update(WorldPos(static_cast<int>(playerPos.x), static_cast<int>(playerPos.y),
                                 static_cast<int>(playerPos.z)));
    } else {
        // Fallback to default position
        m_world->update(WorldPos(0, 64, 0));
    }

    // Update physics
    if (m_physics) {
        m_physics->update(deltaTime);
    }

    // Update AI
    if (m_aiSystem) {
        m_aiSystem->update(deltaTime);
    }

    // Check for script changes (hot-reload)
    if (m_scriptingEngine && m_scriptingEngine->isHotReloadEnabled()) {
        m_scriptingEngine->checkForScriptChanges();
    }

    // Update world editor
    if (m_worldEditor) {
        m_worldEditor->update(deltaTime);
    }

    // Update all registered pluggable systems (play mode)
    m_systemRegistry.updateAll(deltaTime, true);

    // Update editor manager (for HUD and other UI state)
    if (m_editorManager) {
        m_editorManager->update(deltaTime);
    }
}

void Engine::updateEditor(float deltaTime)
{
    // EDITOR MODE UPDATE: Updates editor-relevant systems
    // This is called when NOT in play mode (m_inGame == false)
    // Game systems like physics and AI do NOT update in editor mode

#ifdef _WIN32
    // WINDOW RESIZE HANDLING - Update panel layouts when main window is resized.
    // This MUST run even before a world is created so the viewport swap chain stays
    // in sync with the actual viewport dimensions.
    if (m_window && m_editorManager) {
        Win32Window* win32Window = dynamic_cast<Win32Window*>(m_window.get());
        if (win32Window && win32Window->wasFramebufferResized()) {
            win32Window->resetFramebufferResizedFlag();
            
            RECT clientRect;
            HWND hwnd = win32Window->getHandle();
            if (hwnd && GetClientRect(hwnd, &clientRect)) {
                int clientWidth = clientRect.right - clientRect.left;
                int clientHeight = clientRect.bottom - clientRect.top;
                m_editorManager->onWindowResize(clientWidth, clientHeight);
                LOG_INFO_C("Window resized, updated panel layouts: " + 
                          std::to_string(clientWidth) + "x" + std::to_string(clientHeight), "Engine");
            }
        }
    }
    
    // VIEWPORT RESIZE HANDLING - Check if viewport was resized and recreate swap chain.
    // This MUST run even before a world is created so the viewport renders correctly.
    if (m_editorManager && m_editorManager->getViewportPanel() && m_renderer) {
        auto* viewportPanel = m_editorManager->getViewportPanel();
        
        // DEFERRED VIEWPORT INIT: If the swap chain wasn't created during Engine::initialize()
        // (e.g., because the viewport wasn't fully ready), retry here every frame until it succeeds.
        // This is the primary fix for the "blank viewport" issue.
        if (!m_viewportSwapChainReady) {
            HWND viewportHwnd = viewportPanel->getHandle();
            int vpWidth = viewportPanel->getWidth();
            int vpHeight = viewportPanel->getHeight();
            
            if (viewportHwnd && IsWindow(viewportHwnd) && vpWidth > 0 && vpHeight > 0) {
                if (m_renderer->setViewportWindow(viewportHwnd)) {
                    if (m_renderer->recreateSwapChain(vpWidth, vpHeight)) {
                        m_viewportSwapChainReady = true;
                        LOG_INFO_C("✓ Deferred viewport swap chain created: " +
                                  std::to_string(vpWidth) + "x" + std::to_string(vpHeight), "Engine");
                        if (m_player) {
                            float aspectRatio = static_cast<float>(vpWidth) / static_cast<float>(vpHeight);
                            m_player->getCamera().setAspectRatio(aspectRatio);
                        }
                    }
                }
            }
        }
        
        if (viewportPanel->wasResized()) {
            int vpWidth = viewportPanel->getWidth();
            int vpHeight = viewportPanel->getHeight();
            
            viewportPanel->clearResizedFlag();
            
            LOG_INFO_C("Viewport resized: " + std::to_string(vpWidth) + "x" + std::to_string(vpHeight), "Engine");
            
            // Recreate swap chain with new viewport dimensions
            if (vpWidth > 0 && vpHeight > 0) {
                if (m_renderer->recreateSwapChain(vpWidth, vpHeight)) {
                    m_viewportSwapChainReady = true;
                    // Update camera aspect ratio
                    if (m_player) {
                        float aspectRatio = static_cast<float>(vpWidth) / static_cast<float>(vpHeight);
                        m_player->getCamera().setAspectRatio(aspectRatio);
                    }
                } else {
                    LOG_ERROR_C("Failed to recreate swap chain during viewport resize", "Engine");
                }
            } else {
                LOG_ERROR_C("Invalid viewport dimensions during resize: " + 
                           std::to_string(vpWidth) + "x" + std::to_string(vpHeight), "Engine");
            }
        }
    }
#endif

    // Update editor manager UI even without a world (panel state, status bar, etc.)
    if (m_editorManager) {
        m_editorManager->update(deltaTime);
    }

    if (!m_world) {
        return;
    }

    // Check if GUI wants input
    bool guiCapturesMouse = m_editorManager && m_editorManager->wantCaptureMouse();

    // Check if mouse is within the viewport for scene interaction
    // On non-Windows platforms, default to true (no viewport panel separation)
    bool mouseInViewport = true;
#ifdef _WIN32
    if (m_editorManager && m_editorManager->getViewportPanel()) {
        POINT pt;
        if (GetCursorPos(&pt)) {
            mouseInViewport = m_editorManager->getViewportPanel()->isMouseInViewport(pt.x, pt.y);
        } else {
            mouseInViewport = false;
        }
    }
#endif

    // Editor camera movement (free-flight) and Unreal-style right-click-drag look
    // Only process camera mouse input when cursor is in the viewport and GUI doesn't want it
    bool canProcessViewportMouse = m_player && m_inputManager && !guiCapturesMouse && mouseInViewport;
    if (canProcessViewportMouse) {
        // Unreal-style: hold right mouse button to look around
        if (m_inputManager->isMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
            glm::vec2 mouseDelta = m_inputManager->getMouseDelta();
            if (glm::length(mouseDelta) > 0.0f) {
                m_player->handleMouseMovement(mouseDelta.x, mouseDelta.y);
            }
        }
    }

    // Always allow keyboard-based editor camera movement (WASD) when not captured by GUI
    if (m_player) {
        m_player->update(deltaTime);
    }
    
    // Update world streaming around editor camera
    if (m_player) {
        glm::vec3 playerPos = m_player->getPosition();
        m_world->update(WorldPos(static_cast<int>(playerPos.x), static_cast<int>(playerPos.y),
                                 static_cast<int>(playerPos.z)));
    } else {
        m_world->update(WorldPos(0, 64, 0));
    }

    // Check for script changes (hot-reload works in editor)
    if (m_scriptingEngine && m_scriptingEngine->isHotReloadEnabled()) {
        m_scriptingEngine->checkForScriptChanges();
    }

    // Update world editor (terraforming, selection, etc.)
    if (m_worldEditor) {
        m_worldEditor->update(deltaTime);
    }

    // Update all registered pluggable systems (editor mode)
    m_systemRegistry.updateAll(deltaTime, false);

    // NOTE: Physics, AI, weather, time manager, etc. DO NOT update in editor mode
    // They only update during play mode (in the regular update() method)
}

bool Engine::validateViewportState()
{
#ifdef _WIN32
    // Validate viewport panel exists and is properly configured
    if (!m_editorManager || !m_editorManager->getViewportPanel()) {
        return false;
    }
    
    auto* viewportPanel = m_editorManager->getViewportPanel();
    
    // Check if viewport has valid window handle
    HWND viewportHwnd = viewportPanel->getHandle();
    if (!viewportHwnd || !IsWindow(viewportHwnd)) {
        LOG_WARNING_C("Viewport window handle invalid", "Engine");
        return false;
    }
    
    // Check if viewport is visible
    if (!IsWindowVisible(viewportHwnd)) {
        LOG_WARNING_C("Viewport window not visible", "Engine");
        return false;
    }
    
    // Check if viewport has valid dimensions
    int vpWidth = viewportPanel->getWidth();
    int vpHeight = viewportPanel->getHeight();
    if (vpWidth <= 0 || vpHeight <= 0) {
        LOG_WARNING_C("Viewport has invalid dimensions: " + 
                     std::to_string(vpWidth) + "x" + std::to_string(vpHeight), "Engine");
        return false;
    }
    
    // All checks passed
    return true;
#else
    // Non-Windows platforms don't have viewport panel
    return true;
#endif
}

void Engine::render()
{
    if (!m_renderer) {
        return;
    }

    // EDITOR-GAME SEPARATION: Choose rendering path based on current mode
    // - In play mode (m_inGame): game renders in separate window via renderGamePlayWindow()
    //   The editor viewport is not rendered while the game window owns the swap chain.
    // - In editor mode: render editor preview for scene editing
#ifdef _WIN32
    if (m_inGame && m_gamePlayWindow && m_gamePlayWindow->isOpen()) {
        // Game window has the renderer - skip editor viewport rendering
        return;
    }
#endif
    if (m_inGame) {
        renderGame();
    } else {
        renderEditor();
    }
}

void Engine::renderGame()
{
    // GAME MODE RENDERING: Renders the game world to viewport for play-in-editor testing
    // This is used when user clicks "Play" button to test their game
    
    if (!m_renderer) {
        return;
    }

    // Set clear color for game (sky blue)
    m_renderer->clearColor(0.53f, 0.81f, 0.92f, 1.0f);

    if (!m_renderer->beginFrame()) {
        // Swap chain not ready yet (viewport not initialized)
        return;
    }

    // Set viewport to match swap chain size (viewport panel)
    m_renderer->setViewport(0, 0, m_renderer->getSwapchainWidth(),
                            m_renderer->getSwapchainHeight());

#if defined(FRESH_OPENGL_SUPPORT) && defined(FRESH_GLEW_AVAILABLE)
    if (m_renderer->getAPI() == GraphicsAPI::OpenGL) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Render game world
        if (m_world && !m_isGeneratingWorld) {
            renderVoxelWorld();
        }

        // Render crosshair in game mode
        if (m_world) {
            renderCrosshair();
        }
    }
#endif 

#ifdef _WIN32
    // DirectX 11 game rendering
    if (m_renderer->getAPI() == GraphicsAPI::DirectX11) {
        if (m_world && m_player && !m_isGeneratingWorld) {
            auto* dx11Context = dynamic_cast<DirectX11RenderContext*>(m_renderer.get());
            if (dx11Context) {
                dx11Context->renderVoxelWorld(m_world.get(), m_player.get());
            }
        }
    }

    // DirectX 12 game rendering
    if (m_renderer->getAPI() == GraphicsAPI::DirectX12) {
        if (m_world && m_player && !m_isGeneratingWorld) {
            auto* dx12Context = dynamic_cast<DirectX12RenderContext*>(m_renderer.get());
            if (dx12Context) {
                dx12Context->renderVoxelWorld(m_world.get(), m_player.get());
            }
        }
    }
#endif

    m_renderer->endFrame();
}

void Engine::renderEditor()
{
    // EDITOR MODE RENDERING: Renders editor preview to viewport and editor UI
    // This is the default mode where users build and edit their game
    
    if (!m_renderer) {
        return;
    }

    // Set clear color for editor (sky blue)
    m_renderer->clearColor(0.53f, 0.81f, 0.92f, 1.0f);

    if (!m_renderer->beginFrame()) {
        // Swap chain not ready yet (viewport not initialized).
        // Mark the viewport as not actively rendering so its WM_PAINT
        // handler paints a solid background instead of leaving artifacts.
#ifdef _WIN32
        if (m_editorManager && m_editorManager->getViewportPanel()) {
            m_editorManager->getViewportPanel()->setRenderingActive(false);
        }
#endif
        return;
    }

#ifdef _WIN32
    // DirectX is presenting to the viewport – tell the panel so WM_PAINT
    // only validates the region instead of doing GDI painting.
    if (m_editorManager && m_editorManager->getViewportPanel()) {
        m_editorManager->getViewportPanel()->setRenderingActive(true);
    }
#endif

    // Set viewport to match swap chain size (viewport panel)
    m_renderer->setViewport(0, 0, m_renderer->getSwapchainWidth(),
                            m_renderer->getSwapchainHeight());

#if defined(FRESH_OPENGL_SUPPORT) && defined(FRESH_GLEW_AVAILABLE)
    if (m_renderer->getAPI() == GraphicsAPI::OpenGL) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Render editor preview of world (if exists)
        if (m_world && !m_isGeneratingWorld) {
            renderVoxelWorld();
        }
        // Note: If no world exists, viewport shows clear color (empty scene)
        // This is correct for blank projects - use File > New Level to add content
    }
#endif

#ifdef _WIN32
    // DirectX 11 editor preview rendering
    if (m_renderer->getAPI() == GraphicsAPI::DirectX11) {
        if (m_world && m_player && !m_isGeneratingWorld) {
            auto* dx11Context = dynamic_cast<DirectX11RenderContext*>(m_renderer.get());
            if (dx11Context) {
                dx11Context->renderVoxelWorld(m_world.get(), m_player.get());
            }
        }
    }

    // DirectX 12 editor preview rendering
    if (m_renderer->getAPI() == GraphicsAPI::DirectX12) {
        if (m_world && m_player && !m_isGeneratingWorld) {
            auto* dx12Context = dynamic_cast<DirectX12RenderContext*>(m_renderer.get());
            if (dx12Context) {
                dx12Context->renderVoxelWorld(m_world.get(), m_player.get());
            }
        }
    }
#endif
    
    // Render editor UI (panels, gizmos, selection)
    // This happens AFTER viewport content is rendered
    if (m_editorManager && m_editorManager->isInitialized()) {
        m_editorManager->beginFrame();
    }

    if (m_editorManager && m_editorManager->isInitialized() && m_editorManager->isVisible()) {
        m_editorManager->render();
    }

    if (m_editorManager && m_editorManager->isInitialized()) {
        m_editorManager->endFrame();
    }

    m_renderer->endFrame();
}

void Engine::setupInputCallbacks()
{
    if (!m_window || !m_inputManager) {
        return;
    }

#ifdef _WIN32
    // Win32: Callbacks are already set up in initialize() with lambdas
    // No additional setup needed
    LOG_INFO_C("Win32 input callbacks already configured", "Engine");
#else
    // GLFW: Set up traditional GLFW callbacks
    GLFWwindow* window = m_window->getHandle();

    // Create user data for callbacks
    m_callbackUserData = std::make_unique<CallbackUserData>();
    m_callbackUserData->inputManager = m_inputManager.get();
    m_callbackUserData->window = m_window.get();
    glfwSetWindowUserPointer(window, m_callbackUserData.get());

    // Keyboard callback
    glfwSetKeyCallback(window, [](GLFWwindow* win, int key, int scancode, int action, int mods) {
        (void)scancode; // Unused
        (void)mods;     // Unused
        CallbackUserData* data = static_cast<CallbackUserData*>(glfwGetWindowUserPointer(win));
        if (data && data->inputManager) {
            data->inputManager->processKeyEvent(key, action);
        }
    });

    // Mouse movement callback
    glfwSetCursorPosCallback(window, [](GLFWwindow* win, double xpos, double ypos) {
        CallbackUserData* data = static_cast<CallbackUserData*>(glfwGetWindowUserPointer(win));
        if (data && data->inputManager) {
            data->inputManager->processMouseMovement(xpos, ypos);
        }
    });

    // Mouse button callback
    glfwSetMouseButtonCallback(window, [](GLFWwindow* win, int button, int action, int mods) {
        (void)mods; // Unused
        CallbackUserData* data = static_cast<CallbackUserData*>(glfwGetWindowUserPointer(win));
        if (data && data->inputManager) {
            data->inputManager->processMouseButton(button, action);
        }
    });

    // Framebuffer resize callback
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow* win, int width, int height) {
        CallbackUserData* data = static_cast<CallbackUserData*>(glfwGetWindowUserPointer(win));
        if (data && data->window) {
            // Update window dimensions (Window class handles this internally via its own tracking)
            // The renderer will need to handle the resize in its next frame
            (void)width;
            (void)height;
        }
    });
#endif
}

#if defined(FRESH_OPENGL_SUPPORT) && defined(FRESH_GLEW_AVAILABLE)

std::string Engine::loadShaderFile(const std::string& filepath)
{
    // Try multiple paths to find the shader file
    std::vector<std::string> searchPaths;
    
    // 1. Try relative to current working directory (for development builds)
    searchPaths.push_back(filepath);
    
    // 2. Try relative to executable directory (for installed/deployed builds)
    std::string exeDir = getExecutableDirectory();
    if (!exeDir.empty()) {
        searchPaths.push_back(exeDir + "/" + filepath);
    }
    
    // Try each path
    for (const auto& path : searchPaths) {
        std::ifstream file(path);
        if (file.is_open()) {
            std::stringstream buffer;
            buffer << file.rdbuf();
            LOG_INFO_C("Loaded shader from: " + path, "Engine");
            return buffer.str();
        }
    }
    
    // If we get here, shader file was not found in any location
    LOG_ERROR_C("Failed to open shader file: " + filepath, "Engine");
    LOG_ERROR_C("Searched in the following locations:", "Engine");
    for (const auto& path : searchPaths) {
        LOG_ERROR_C("  - " + path, "Engine");
    }
    LOG_ERROR_C("Make sure the shaders/ folder exists in one of these locations", "Engine");
    return "";
}

GLuint Engine::compileShader(const std::string& source, GLenum shaderType)
{
    GLuint shader = glCreateShader(shaderType);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    // Check compilation
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[SHADER_INFO_LOG_SIZE];
        glGetShaderInfoLog(shader, SHADER_INFO_LOG_SIZE, nullptr, infoLog);
        LOG_ERROR_C(std::string("Shader compilation failed: ") + infoLog, "Engine");
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

GLuint Engine::createShaderProgram(const std::string& vertPath, const std::string& fragPath)
{
    std::string vertSource = loadShaderFile(vertPath);
    std::string fragSource = loadShaderFile(fragPath);

    if (vertSource.empty() || fragSource.empty()) {
        return 0;
    }

    GLuint vertShader = compileShader(vertSource, GL_VERTEX_SHADER);
    GLuint fragShader = compileShader(fragSource, GL_FRAGMENT_SHADER);

    if (vertShader == 0 || fragShader == 0) {
        if (vertShader)
            glDeleteShader(vertShader);
        if (fragShader)
            glDeleteShader(fragShader);
        return 0;
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vertShader);
    glAttachShader(program, fragShader);
    glLinkProgram(program);

    // Check linking
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[SHADER_INFO_LOG_SIZE];
        glGetProgramInfoLog(program, SHADER_INFO_LOG_SIZE, nullptr, infoLog);
        LOG_ERROR_C(std::string("Shader program linking failed: ") + infoLog, "Engine");
        glDeleteProgram(program);
        program = 0;
    }

    glDeleteShader(vertShader);
    glDeleteShader(fragShader);

    return program;
}

void Engine::initializeRendering()
{
    if (m_renderer->getAPI() != GraphicsAPI::OpenGL) {
        LOG_INFO_C("Rendering initialization skipped - not using OpenGL", "Engine");
        return;
    }

    // Create shader program
    m_shaderProgram = createShaderProgram(VOXEL_VERTEX_SHADER, VOXEL_FRAGMENT_SHADER);
    if (m_shaderProgram == 0) {
        LOG_ERROR_C("Failed to create voxel shader program", "Engine");
        return;
    }

    // Create crosshair shader program
    m_crosshairShader = createShaderProgram(CROSSHAIR_VERTEX_SHADER, CROSSHAIR_FRAGMENT_SHADER);
    if (m_crosshairShader == 0) {
        LOG_ERROR_C("Failed to create crosshair shader program", "Engine");
        // Continue without crosshair
    }

    LOG_INFO_C("OpenGL voxel rendering initialized", "Engine");

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // Enable back-face culling for performance
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    // Create crosshair geometry (simple cross)
    float crosshairVertices[] = {// Horizontal line
                                 -CROSSHAIR_SIZE, 0.0f, CROSSHAIR_SIZE, 0.0f,
                                 // Vertical line
                                 0.0f, -CROSSHAIR_SIZE, 0.0f, CROSSHAIR_SIZE};

    glGenVertexArrays(1, &m_crosshairVAO);
    glGenBuffers(1, &m_crosshairVBO);

    glBindVertexArray(m_crosshairVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_crosshairVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(crosshairVertices), crosshairVertices, GL_STATIC_DRAW);

    // Position attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

    glBindVertexArray(0);
}

void Engine::shutdownRendering()
{
    if (m_renderer->getAPI() != GraphicsAPI::OpenGL) {
        return;
    }

    // Delete crosshair buffers
    if (m_crosshairVAO) {
        glDeleteVertexArrays(1, &m_crosshairVAO);
        m_crosshairVAO = 0;
    }
    if (m_crosshairVBO) {
        glDeleteBuffers(1, &m_crosshairVBO);
        m_crosshairVBO = 0;
    }

    // Delete all VAOs/VBOs/EBOs
    for (auto& pair : m_chunkVAOs) {
        glDeleteVertexArrays(1, &pair.second);
    }
    for (auto& pair : m_chunkVBOs) {
        glDeleteBuffers(1, &pair.second);
    }
    for (auto& pair : m_chunkEBOs) {
        glDeleteBuffers(1, &pair.second);
    }

    m_chunkVAOs.clear();
    m_chunkVBOs.clear();
    m_chunkEBOs.clear();
    m_chunkIndexCounts.clear();

    if (m_shaderProgram) {
        glDeleteProgram(m_shaderProgram);
        m_shaderProgram = 0;
    }
    if (m_crosshairShader) {
        glDeleteProgram(m_crosshairShader);
        m_crosshairShader = 0;
    }
}

void Engine::renderVoxelWorld()
{
    if (!m_world || !m_player || m_shaderProgram == 0) {
        return;
    }

    glUseProgram(m_shaderProgram);

    // Calculate matrices
    glm::mat4 view = m_player->getCamera().getViewMatrix();
    float aspectRatio = static_cast<float>(m_renderer->getSwapchainWidth()) /
                        static_cast<float>(m_renderer->getSwapchainHeight());
    glm::mat4 projection = m_player->getCamera().getProjectionMatrix(aspectRatio);

    // Render each chunk
    for (const auto& chunkPair : m_world->getChunks()) {
        const ChunkPos& chunkPos = chunkPair.first;
        Chunk* chunk = chunkPair.second.get();

        if (!chunk)
            continue;

        // Check if we need to upload mesh data to GPU
        // Upload if chunk is dirty (modified) OR if we haven't created GPU buffers yet
        bool needsUpload = chunk->isDirty() || (m_chunkVAOs.find(chunkPos) == m_chunkVAOs.end());

        if (needsUpload) {
            // Generate mesh if dirty
            if (chunk->isDirty()) {
                chunk->generateMesh();
                chunk->clearDirty();
            }

            // Update OpenGL buffers
            const auto& vertices = chunk->getMeshVertices();
            const auto& indices = chunk->getMeshIndices();

            if (vertices.empty() || indices.empty()) {
                continue;
            }

            // Create or get VAO/VBO/EBO
            GLuint& vao = m_chunkVAOs[chunkPos];
            GLuint& vbo = m_chunkVBOs[chunkPos];
            GLuint& ebo = m_chunkEBOs[chunkPos];

            if (vao == 0) {
                glGenVertexArrays(1, &vao);
                glGenBuffers(1, &vbo);
                glGenBuffers(1, &ebo);
            }

            glBindVertexArray(vao);

            // Upload vertex data
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(),
                         GL_STATIC_DRAW);

            // Upload index data
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), indices.data(),
                         GL_STATIC_DRAW);

            // Set up vertex attributes
            // Position (vec3)
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);

            // Normal (vec3)
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                                  (void*)(3 * sizeof(float)));

            glBindVertexArray(0);

            m_chunkIndexCounts[chunkPos] = indices.size();
        }

        // Render the chunk if it has a VAO
        if (m_chunkVAOs.find(chunkPos) != m_chunkVAOs.end()) {
            GLuint vao = m_chunkVAOs[chunkPos];
            size_t indexCount = m_chunkIndexCounts[chunkPos];

            if (indexCount == 0)
                continue;

            // Calculate model matrix (chunk position)
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model,
                                   glm::vec3(chunkPos.x * CHUNK_SIZE, 0, chunkPos.z * CHUNK_SIZE));

            // Calculate MVP matrix
            glm::mat4 mvp = projection * view * model;

            // Set uniform
            GLint mvpLoc = glGetUniformLocation(m_shaderProgram, "modelViewProj");
            glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(mvp));

            // Draw
            glBindVertexArray(vao);
            glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indexCount), GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);
        }
    }

    glUseProgram(0);
}

void Engine::renderCrosshair()
{
    if (m_crosshairVAO == 0 || m_crosshairShader == 0) {
        return;
    }

    // Disable depth test for 2D overlay
    glDisable(GL_DEPTH_TEST);

    // Use crosshair shader
    glUseProgram(m_crosshairShader);

    // Draw crosshair
    glBindVertexArray(m_crosshairVAO);
    glLineWidth(CROSSHAIR_LINE_WIDTH);
    glDrawArrays(GL_LINES, 0, 4);
    glBindVertexArray(0);

    glUseProgram(0);

    // Re-enable depth test
    glEnable(GL_DEPTH_TEST);
}

#endif // FRESH_OPENGL_SUPPORT && FRESH_GLEW_AVAILABLE

#ifdef _WIN32
void Engine::setupNativeMenuBar()
{
    if (!m_window) {
        LOG_ERROR_C("Cannot setup menu bar: window not initialized", "Engine");
        return;
    }

    auto* menuBar = m_window->getMenuBar();
    if (!menuBar) {
        LOG_ERROR_C("Failed to create menu bar", "Engine");
        return;
    }

    // ========== FILE MENU (Unreal-style Project Workflow) ==========
    int fileMenu = menuBar->addMenu("File");
    menuBar->addMenuItem(fileMenu, "New Project...\tCtrl+N", [this]() {
        LOG_INFO_C("New Project menu item clicked", "Engine");
        if (m_editorManager) {
            m_editorManager->newProject(); // Creates new project
        }
    });
    menuBar->addMenuItem(fileMenu, "Open Project...\tCtrl+O", [this]() {
        LOG_INFO_C("Open Project menu item clicked", "Engine");
        if (m_editorManager) {
            m_editorManager->openProject(); // Opens existing project
        }
    });
    menuBar->addSeparator(fileMenu);
    menuBar->addMenuItem(fileMenu, "New Level...\tCtrl+Shift+N", [this]() {
        LOG_INFO_C("New Level menu item clicked (creates new world in project)", "Engine");
        if (m_editorManager) {
            m_editorManager->newWorld(); // Creates new level/world within project
        }
    });
    menuBar->addMenuItem(fileMenu, "Open Level...\tCtrl+Shift+O", [this]() {
        LOG_INFO_C("Open Level menu item clicked (loads world in project)", "Engine");
        if (m_editorManager) {
            m_editorManager->loadWorld(); // Loads existing level/world within project
        }
    });
    menuBar->addSeparator(fileMenu);
    menuBar->addMenuItem(fileMenu, "Save\tCtrl+S", [this]() {
        LOG_INFO_C("Save menu item clicked", "Engine");
        if (m_editorManager) {
            m_editorManager->saveWorld();
        }
    });
    menuBar->addMenuItem(fileMenu, "Save As...\tCtrl+Shift+S", [this]() {
        LOG_INFO_C("Save As menu item clicked", "Engine");
        if (m_editorManager) {
            m_editorManager->saveWorldAs();
        }
    });
    menuBar->addMenuItem(fileMenu, "Save All\tCtrl+Shift+A", [this]() {
        LOG_INFO_C("Save All menu item clicked", "Engine");
        // Save all assets and project settings
        if (m_projectManager && m_projectManager->isProjectOpen()) {
            m_projectManager->saveProject();
        }
        if (m_editorManager) {
            m_editorManager->saveWorld();  // Also save current world if exists
        }
    });
    menuBar->addSeparator(fileMenu);
    menuBar->addMenuItem(fileMenu, "Import...\tCtrl+I", [this]() {
        LOG_INFO_C("Import menu item clicked", "Engine");
        if (m_editorManager) {
            m_editorManager->showImportAssets();
        }
    });
    menuBar->addMenuItem(fileMenu, "Export...\tCtrl+E", [this]() {
        LOG_INFO_C("Export menu item clicked", "Engine");
#ifdef _WIN32
        // Future: Export world/scene to various formats
        // This could include exporting to OBJ, FBX, or custom game format
        LOG_INFO_C("Export functionality requested", "Engine");
        MessageBoxA(
            static_cast<HWND>(m_window ? m_window->getNativeWindowHandle() : nullptr),
            "Export functionality is not yet implemented.\n\nPlanned export formats:\n- OBJ (3D mesh)\n- FBX (3D scene)\n- Voxel format (custom)\n- Image atlas (texture pack)",
            "Export",
            MB_OK | MB_ICONINFORMATION
        );
        // Future: Win32ExportDialog dialog(m_window->getNativeWindowHandle(), m_world);
        // if (dialog.show()) { /* Perform export */ }
#else
        LOG_WARNING_C("Export functionality requires Windows implementation", "Engine");
#endif
    });
    menuBar->addSeparator(fileMenu);
    menuBar->addMenuItem(fileMenu, "Project Settings...", [this]() {
        LOG_INFO_C("Project Settings menu item clicked", "Engine");
        if (m_editorManager) {
            m_editorManager->showEngineConfig();
        }
    });
    menuBar->addSeparator(fileMenu);
    menuBar->addMenuItem(fileMenu, "Exit\tAlt+F4", [this]() {
        LOG_INFO_C("Exit menu item clicked", "Engine");
        m_running = false;
    });

    // ========== EDIT MENU (Unreal-style) ==========
    int editMenu = menuBar->addMenu("Edit");
    menuBar->addMenuItem(editMenu, "Undo\tCtrl+Z", [this]() {
        LOG_INFO_C("Undo menu item clicked", "Engine");
        if (m_editorManager) {
            m_editorManager->undo();
        }
    });
    menuBar->addMenuItem(editMenu, "Redo\tCtrl+Y", [this]() {
        LOG_INFO_C("Redo menu item clicked", "Engine");
        if (m_editorManager) {
            m_editorManager->redo();
        }
    });
    menuBar->addSeparator(editMenu);
    menuBar->addMenuItem(editMenu, "Cut\tCtrl+X", [this]() {
        LOG_INFO_C("Cut menu item clicked", "Engine");
        if (m_editorManager) {
            m_editorManager->cut();
        }
    });
    menuBar->addMenuItem(editMenu, "Copy\tCtrl+C", [this]() {
        LOG_INFO_C("Copy menu item clicked", "Engine");
        if (m_editorManager) {
            m_editorManager->copy();
        }
    });
    menuBar->addMenuItem(editMenu, "Paste\tCtrl+V", [this]() {
        LOG_INFO_C("Paste menu item clicked", "Engine");
        if (m_editorManager) {
            m_editorManager->paste();
        }
    });
    menuBar->addMenuItem(editMenu, "Delete\tDel", [this]() {
        LOG_INFO_C("Delete menu item clicked", "Engine");
        if (m_editorManager) {
            m_editorManager->deleteSelection();
        }
    });
    menuBar->addSeparator(editMenu);
    menuBar->addMenuItem(editMenu, "Select All\tCtrl+A", [this]() {
        LOG_INFO_C("Select All menu item clicked", "Engine");
        if (m_editorManager) {
            m_editorManager->selectAll();
        }
    });
    menuBar->addMenuItem(editMenu, "Deselect All\tCtrl+D", [this]() {
        LOG_INFO_C("Deselect All menu item clicked", "Engine");
        if (m_editorManager) {
            m_editorManager->deselectAll();
        }
    });
    menuBar->addSeparator(editMenu);
    menuBar->addMenuItem(editMenu, "Find...\tCtrl+F", [this]() {
        LOG_INFO_C("Find menu item clicked", "Engine");
#ifdef _WIN32
        // Future: Find objects/entities in scene by name or type
        // This would open a search dialog to locate specific entities
        LOG_INFO_C("Find functionality requested", "Engine");
        MessageBoxA(
            static_cast<HWND>(m_window ? m_window->getNativeWindowHandle() : nullptr),
            "Find functionality is not yet implemented.\n\nPlanned features:\n- Find entities by name\n- Find by type\n- Find by tag\n- Search scene hierarchy",
            "Find",
            MB_OK | MB_ICONINFORMATION
        );
        // Future: Win32FindDialog dialog(m_window->getNativeWindowHandle(), m_entityManager.get());
        // if (dialog.show()) { /* Select found entities */ }
#else
        LOG_WARNING_C("Find functionality requires Windows implementation", "Engine");
#endif
    });
    menuBar->addSeparator(editMenu);
    menuBar->addMenuItem(editMenu, "Editor Preferences...", [this]() {
        LOG_INFO_C("Editor Preferences menu item clicked", "Engine");
        if (m_editorManager) {
            m_editorManager->showSettings();
        }
    });

    // ========== VIEW MENU (Unreal-style) ==========
    int viewMenu = menuBar->addMenu("View");
    menuBar->addMenuItem(viewMenu, "Perspective\t1", [this]() {
        LOG_INFO_C("Perspective view selected", "Engine");
        if (m_editorManager) {
            m_editorManager->setCameraViewMode("Perspective");
        }
    });
    menuBar->addMenuItem(viewMenu, "Top\t2", [this]() {
        LOG_INFO_C("Top view selected", "Engine");
        if (m_editorManager) {
            m_editorManager->setCameraViewMode("Top");
        }
    });
    menuBar->addMenuItem(viewMenu, "Front\t3", [this]() {
        LOG_INFO_C("Front view selected", "Engine");
        if (m_editorManager) {
            m_editorManager->setCameraViewMode("Front");
        }
    });
    menuBar->addMenuItem(viewMenu, "Side\t4", [this]() {
        LOG_INFO_C("Side view selected", "Engine");
        if (m_editorManager) {
            m_editorManager->setCameraViewMode("Side");
        }
    });
    menuBar->addMenuItem(viewMenu, "Reset Camera\tHome", [this]() {
        LOG_INFO_C("Reset Camera menu item clicked", "Engine");
        if (m_editorManager) {
            m_editorManager->setCameraViewMode("Perspective");
        }
        // Also reset player position if available
        if (m_player) {
            m_player->setPosition(glm::vec3(0.0f, 70.0f, 0.0f));
        }
    });
    menuBar->addSeparator(viewMenu);
    menuBar->addMenuItem(viewMenu, "Content Browser\tCtrl+B", [this]() {
        LOG_INFO_C("Content Browser toggled", "Engine");
        if (m_editorManager) {
            m_editorManager->toggleContentBrowser();
        }
    });
    menuBar->addMenuItem(viewMenu, "Scene Hierarchy\tCtrl+H", [this]() {
        LOG_INFO_C("Scene Hierarchy toggled", "Engine");
        if (m_editorManager) {
            m_editorManager->toggleSceneHierarchy();
        }
    });
    menuBar->addMenuItem(viewMenu, "Inspector\tCtrl+I", [this]() {
        LOG_INFO_C("Inspector toggled", "Engine");
        if (m_editorManager) {
            m_editorManager->toggleInspector();
        }
    });
    menuBar->addMenuItem(viewMenu, "Console\t~", [this]() {
        LOG_INFO_C("Console toggled", "Engine");
        if (m_editorManager) {
            m_editorManager->toggleConsole();
        }
    });
    menuBar->addSeparator(viewMenu);
    menuBar->addMenuItem(viewMenu, "Toggle Fullscreen\tF11", [this]() {
        LOG_INFO_C("Toggle Fullscreen menu item clicked", "Engine");
#ifdef _WIN32
        if (m_window) {
            m_window->toggleFullscreen();
        }
#else
        LOG_WARNING_C("Fullscreen toggle requires Windows implementation", "Engine");
#endif
    });
    menuBar->addMenuItem(viewMenu, "Toggle UI\tU", [this]() {
        LOG_INFO_C("Toggle UI menu item clicked", "Engine");
        if (m_editorManager) {
            m_editorManager->toggle();
        }
    });
    menuBar->addMenuItem(viewMenu, "Show Stats\tCtrl+Shift+S", [this]() {
        LOG_INFO_C("Show Stats toggled", "Engine");
        if (m_editorManager) {
            m_editorManager->showEngineConfig();
        }
    });

    // ========== WORLD MENU (Unreal-style) ==========
    int worldMenu = menuBar->addMenu("World");
    menuBar->addMenuItem(worldMenu, "Play\tAlt+P", [this]() {
        LOG_INFO_C("Play mode started", "Engine");
        enterPlayMode();
    });
    menuBar->addMenuItem(worldMenu, "Pause\tAlt+Pause", [this]() {
        if (m_timeManager) {
            m_timeManager->togglePause();
            bool isPaused = m_timeManager->isPaused();
            LOG_INFO_C(isPaused ? "Play mode paused" : "Play mode resumed", "Engine");
        } else {
            LOG_WARNING_C("Cannot pause - TimeManager not initialized", "Engine");
        }
    });
    menuBar->addMenuItem(worldMenu, "Stop\tEsc", [this]() {
        LOG_INFO_C("Play mode stopped", "Engine");
        exitPlayMode();
    });
    menuBar->addSeparator(worldMenu);
    menuBar->addMenuItem(worldMenu, "Generate Terrain...", [this]() {
        LOG_INFO_C("Generate Terrain dialog requested", "Engine");
#ifdef _WIN32
        // Future: Show terrain generation dialog for procedural world generation
        // This would allow users to configure generation parameters
        MessageBoxA(
            static_cast<HWND>(m_window ? m_window->getNativeWindowHandle() : nullptr),
            "Terrain Generation dialog is not yet implemented.\n\nPlanned features:\n- Seed selection\n- Terrain type (flat, hills, mountains, islands)\n- Biome selection\n- World size\n- Generation preview",
            "Generate Terrain",
            MB_OK | MB_ICONINFORMATION
        );
        // Future: Win32TerrainGenerationDialog dialog(m_window->getNativeWindowHandle());
        // if (dialog.show()) { 
        //     m_world->regenerateWithParams(dialog.getParams());
        // }
#else
        LOG_WARNING_C("Terrain generation dialog requires Windows implementation", "Engine");
#endif
    });
    menuBar->addMenuItem(worldMenu, "Clear Scene", [this]() {
        LOG_INFO_C("Clear Scene selected (clears world)", "Engine");
#ifdef _WIN32
        // Show confirmation dialog using native Windows MessageBox
        int result = MessageBoxA(
            static_cast<HWND>(m_window ? m_window->getNativeWindowHandle() : nullptr),
            "Are you sure you want to clear the entire scene?\n\nThis will remove all voxels and cannot be undone.",
            "Clear Scene Confirmation",
            MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2
        );
        
        if (result == IDYES) {
            if (m_world) {
                LOG_INFO_C("Clearing all chunks in world", "Engine");
                m_world->clearAllChunks();
                LOG_INFO_C("World cleared successfully", "Engine");
            } else {
                LOG_WARNING_C("Cannot clear scene - no world loaded", "Engine");
            }
        } else {
            LOG_INFO_C("Clear scene cancelled by user", "Engine");
        }
#else
        LOG_WARNING_C("Clear scene confirmation requires Windows MessageBox", "Engine");
#endif
    });
    menuBar->addMenuItem(worldMenu, "Regenerate Chunks", [this]() {
        LOG_INFO_C("Regenerate Chunks selected", "Engine");
        if (m_world) {
            LOG_INFO_C("Regenerating all loaded chunks", "Engine");
            m_world->regenerateLoadedChunks();
            LOG_INFO_C("Chunk regeneration complete", "Engine");
        } else {
            LOG_WARNING_C("Cannot regenerate chunks - no world loaded", "Engine");
        }
    });
    menuBar->addSeparator(worldMenu);
    menuBar->addMenuItem(worldMenu, "Scene Settings...", [this]() {
        LOG_INFO_C("Scene Settings selected (world settings)", "Engine");
#ifdef _WIN32
        // Future: Show native Win32 settings dialog for world/scene parameters
        // This would include seed, generation type, biome settings, etc.
        LOG_INFO_C("World settings dialog requested", "Engine");
        MessageBoxA(
            static_cast<HWND>(m_window ? m_window->getNativeWindowHandle() : nullptr),
            "World Settings dialog is not yet implemented.\n\nFuture features:\n- World seed\n- Generation parameters\n- Biome settings\n- Physics settings\n- Lighting settings",
            "Scene Settings",
            MB_OK | MB_ICONINFORMATION
        );
        // Future: Win32WorldSettingsDialog dialog(m_window->getNativeWindowHandle(), m_world);
        // if (dialog.show()) { /* Apply settings */ }
#else
        LOG_WARNING_C("World settings dialog requires Windows implementation", "Engine");
#endif
    });

    // ========== TOOLS MENU (Unreal-style) ==========
    int toolsMenu = menuBar->addMenu("Tools");
    menuBar->addMenuItem(toolsMenu, "Brush\tB", [this]() {
        LOG_INFO_C("Brush tool selected", "Engine");
        if (m_worldEditor && m_worldEditor->getTerraformingSystem()) {
            m_worldEditor->getTerraformingSystem()->setTool(TerraformTool::Brush);
        }
    });
    menuBar->addMenuItem(toolsMenu, "Paint\tP", [this]() {
        LOG_INFO_C("Paint tool selected", "Engine");
        if (m_worldEditor && m_worldEditor->getTerraformingSystem()) {
            m_worldEditor->getTerraformingSystem()->setTool(TerraformTool::Paint);
        }
    });
    menuBar->addMenuItem(toolsMenu, "Sculpt\tS", [this]() {
        LOG_INFO_C("Sculpt tool selected (using FilledSphere)", "Engine");
        if (m_worldEditor && m_worldEditor->getTerraformingSystem()) {
            m_worldEditor->getTerraformingSystem()->setTool(TerraformTool::FilledSphere);
        }
    });
    menuBar->addMenuItem(toolsMenu, "Smooth\tM", [this]() {
        LOG_INFO_C("Smooth tool selected", "Engine");
        if (m_worldEditor && m_worldEditor->getTerraformingSystem()) {
            m_worldEditor->getTerraformingSystem()->setTool(TerraformTool::Smooth);
        }
    });
    menuBar->addSeparator(toolsMenu);
    menuBar->addMenuItem(toolsMenu, "Select\tV", [this]() {
        LOG_INFO_C("Select tool activated", "Engine");
        if (m_worldEditor && m_worldEditor->getTerraformingSystem()) {
            m_worldEditor->getTerraformingSystem()->setTool(TerraformTool::SingleBlock);
        }
    });
    menuBar->addMenuItem(toolsMenu, "Move\tW", [this]() {
        LOG_INFO_C("Move tool activated", "Engine");
        if (m_editorManager && m_editorManager->getTransformGizmo()) {
            m_editorManager->getTransformGizmo()->setMode(TransformGizmo::Mode::Translate);
        }
    });
    menuBar->addMenuItem(toolsMenu, "Rotate\tE", [this]() {
        LOG_INFO_C("Rotate tool activated", "Engine");
        if (m_editorManager && m_editorManager->getTransformGizmo()) {
            m_editorManager->getTransformGizmo()->setMode(TransformGizmo::Mode::Rotate);
        }
    });
    menuBar->addMenuItem(toolsMenu, "Scale\tR", [this]() {
        LOG_INFO_C("Scale tool activated", "Engine");
        if (m_editorManager && m_editorManager->getTransformGizmo()) {
            m_editorManager->getTransformGizmo()->setMode(TransformGizmo::Mode::Scale);
        }
    });
    menuBar->addSeparator(toolsMenu);
    menuBar->addMenuItem(toolsMenu, "Asset Manager...", [this]() {
        LOG_INFO_C("Asset Manager opened", "Engine");
        if (m_editorManager) {
            m_editorManager->toggleContentBrowser();
        }
    });
    menuBar->addMenuItem(toolsMenu, "Material Editor...", [this]() {
        LOG_INFO_C("Material Editor requested", "Engine");
#ifdef _WIN32
        // Future: Show material editor for creating/editing voxel materials
        MessageBoxA(
            static_cast<HWND>(m_window ? m_window->getNativeWindowHandle() : nullptr),
            "Material Editor is not yet implemented.\n\nPlanned features:\n- Material properties editor\n- Texture assignment\n- Material preview\n- Shader parameters\n- Physics properties",
            "Material Editor",
            MB_OK | MB_ICONINFORMATION
        );
        // Future: Win32MaterialEditorDialog dialog(m_window->getNativeWindowHandle());
        // if (dialog.show()) { /* Apply material changes */ }
#else
        LOG_WARNING_C("Material Editor requires Windows implementation", "Engine");
#endif
    });

    // ========== WINDOW MENU (Unreal-style) ==========
    int windowMenu = menuBar->addMenu("Window");
    menuBar->addMenuItem(windowMenu, "Content Browser\tCtrl+B", [this]() {
        LOG_INFO_C("Content Browser toggled", "Engine");
        if (m_editorManager) {
            m_editorManager->toggleContentBrowser();
        }
    });
    menuBar->addMenuItem(windowMenu, "Scene Hierarchy\tCtrl+H", [this]() {
        LOG_INFO_C("Scene Hierarchy toggled", "Engine");
        if (m_editorManager) {
            m_editorManager->toggleSceneHierarchy();
        }
    });
    menuBar->addMenuItem(windowMenu, "Inspector\tCtrl+I", [this]() {
        LOG_INFO_C("Inspector toggled", "Engine");
        if (m_editorManager) {
            m_editorManager->toggleInspector();
        }
    });
    menuBar->addMenuItem(windowMenu, "Console\t~", [this]() {
        LOG_INFO_C("Console toggled", "Engine");
        if (m_editorManager) {
            m_editorManager->toggleConsole();
        }
    });
    menuBar->addSeparator(windowMenu);
    menuBar->addMenuItem(windowMenu, "Reset Layout", [this]() {
        LOG_INFO_C("Layout reset requested", "Engine");
#ifdef _WIN32
        if (m_editorManager) {
            // Reset all panel positions and sizes to default layout
            LOG_INFO_C("Resetting editor layout to default", "Engine");
            m_editorManager->resetLayout();
        } else {
            LOG_WARNING_C("Cannot reset layout - EditorManager not initialized", "Engine");
        }
#else
        LOG_WARNING_C("Layout reset requires Windows EditorManager", "Engine");
#endif
    });
    menuBar->addSeparator(windowMenu);
    menuBar->addMenuItem(windowMenu, "Preferences...", [this]() {
        LOG_INFO_C("Preferences opened", "Engine");
        if (m_editorManager) {
            m_editorManager->showSettings();
        }
    });

    // ========== HELP MENU (Unreal-style) ==========
    int helpMenu = menuBar->addMenu("Help");
    menuBar->addMenuItem(helpMenu, "Documentation\tF1", [this]() {
        LOG_INFO_C("Documentation menu item clicked", "Engine");
#ifdef _WIN32
        // Open documentation in default browser
        ShellExecuteA(NULL, "open", "https://github.com/shifty81/fresh/blob/main/DOCUMENTATION.md", NULL, NULL, SW_SHOWNORMAL);
#endif
    });
    menuBar->addMenuItem(helpMenu, "Keyboard Shortcuts\tCtrl+?", [this]() {
        LOG_INFO_C("Keyboard Shortcuts displayed", "Engine");
#ifdef _WIN32
        if (m_editorManager && m_editorManager->getEditorSettingsDialog()) {
            m_editorManager->showEditorSettings();
        } else {
            MessageBoxA(NULL, 
                "Editor Shortcuts:\n\n"
                "F - Toggle cursor capture\n"
                "ESC - Exit play mode\n"
                "W/E/R - Transform tool (Move/Rotate/Scale)\n"
                "Ctrl+Z - Undo\n"
                "Ctrl+Y - Redo\n"
                "Ctrl+S - Save\n"
                "Alt+P - Play mode\n\n"
                "Camera:\n"
                "Right-click + drag - Free look\n"
                "WASD - Move camera\n"
                "Space/Ctrl - Up/Down\n"
                "Shift - Speed boost",
                "Keyboard Shortcuts", MB_OK | MB_ICONINFORMATION);
        }
#endif
    });
    menuBar->addSeparator(helpMenu);
    menuBar->addMenuItem(helpMenu, "Report Bug...", [this]() {
        LOG_INFO_C("Report Bug selected", "Engine");
#ifdef _WIN32
        ShellExecuteA(NULL, "open", "https://github.com/shifty81/fresh/issues/new?template=bug_report.md", NULL, NULL, SW_SHOWNORMAL);
#endif
    });
    menuBar->addMenuItem(helpMenu, "Feature Request...", [this]() {
        LOG_INFO_C("Feature Request selected", "Engine");
#ifdef _WIN32
        ShellExecuteA(NULL, "open", "https://github.com/shifty81/fresh/issues/new?template=feature_request.md", NULL, NULL, SW_SHOWNORMAL);
#endif
    });
    menuBar->addSeparator(helpMenu);
    menuBar->addMenuItem(helpMenu, "Check for Updates...", [this]() {
        LOG_INFO_C("Check for Updates selected", "Engine");
#ifdef _WIN32
        ShellExecuteA(NULL, "open", "https://github.com/shifty81/fresh/releases", NULL, NULL, SW_SHOWNORMAL);
#endif
    });
    menuBar->addMenuItem(helpMenu, "Release Notes", [this]() {
        LOG_INFO_C("Release Notes selected", "Engine");
#ifdef _WIN32
        ShellExecuteA(NULL, "open", "https://github.com/shifty81/fresh/blob/main/CHANGELOG.md", NULL, NULL, SW_SHOWNORMAL);
#endif
    });
    menuBar->addSeparator(helpMenu);
    menuBar->addMenuItem(helpMenu, "About Fresh Voxel Engine", [this]() {
        LOG_INFO_C("About menu item clicked", "Engine");
#ifdef _WIN32
        MessageBoxA(NULL,
            "Fresh Voxel Engine v0.1.0\n\n"
            "A professional Windows-exclusive voxel game development platform\n"
            "built with C++20, featuring DirectX 12/11 rendering and an\n"
            "Unreal Engine-like editor.\n\n"
            "© 2024 Fresh Voxel Engine Project\n"
            "Licensed under the MIT License\n\n"
            "https://github.com/shifty81/fresh",
            "About Fresh Voxel Engine", MB_OK | MB_ICONINFORMATION);
#endif
    });

    LOG_INFO_C("Unreal-style native Win32 menu bar initialized with comprehensive menus", "Engine");
}

void Engine::setupNativeToolbar()
{
    if (!m_window) {
        LOG_ERROR_C("Cannot setup toolbar: window not initialized", "Engine");
        return;
    }

    auto* toolbar = m_window->getToolbar();
    if (!toolbar) {
        LOG_ERROR_C("Failed to create toolbar", "Engine");
        return;
    }

    // Add toolbar buttons in Unreal-style grouping with icons
    // Using Windows shell icons for a professional look
    
    // Build shell32.dll path once for efficiency and safety
    wchar_t shell32Path[MAX_PATH];
    UINT pathLen = GetSystemDirectoryW(shell32Path, MAX_PATH - 20);  // Reserve space for concat
    if (pathLen == 0 || pathLen >= MAX_PATH - 20) {
        LOG_ERROR_C("Failed to get system directory for toolbar icons", "Engine");
        // Fallback: buttons will work but without icons
    } else {
        wcscat_s(shell32Path, MAX_PATH, L"\\shell32.dll");
    }
    
    // Helper lambda to load shell32 icons (path already constructed)
    auto loadShellIcon = [&shell32Path, pathLen](int iconIndex) -> HICON {
        if (pathLen == 0 || pathLen >= MAX_PATH - 20) {
            return nullptr;  // Path construction failed, return no icon
        }
        return ExtractIconW(GetModuleHandle(nullptr), shell32Path, iconIndex);
    };
    
    // ========== FILE OPERATIONS GROUP ==========
    HICON newIcon = loadShellIcon(0);  // New document icon
    toolbar->addButton(5001, "New", newIcon, [this]() {
        LOG_INFO_C("New button clicked", "Engine");
        if (m_editorManager) {
            m_editorManager->newWorld();
        }
    });
    
    HICON openIcon = loadShellIcon(4);  // Open folder icon
    toolbar->addButton(5002, "Open", openIcon, [this]() {
        LOG_INFO_C("Open button clicked", "Engine");
        if (m_editorManager) {
            m_editorManager->loadWorld();
        }
    });
    
    HICON saveIcon = loadShellIcon(259);  // Save/floppy disk icon
    toolbar->addButton(5003, "Save", saveIcon, [this]() {
        LOG_INFO_C("Save button clicked", "Engine");
        if (m_editorManager) {
            m_editorManager->saveWorld();
        }
    });
    
    toolbar->addSeparator();
    
    // ========== EDIT OPERATIONS GROUP ==========
    HICON undoIcon = loadShellIcon(240);  // Undo/back arrow
    toolbar->addButton(5004, "Undo", undoIcon, [this]() {
        LOG_INFO_C("Undo button clicked", "Engine");
        if (m_editorManager) {
            m_editorManager->undo();
        }
    });
    
    HICON redoIcon = loadShellIcon(241);  // Redo/forward arrow
    toolbar->addButton(5005, "Redo", redoIcon, [this]() {
        LOG_INFO_C("Redo button clicked", "Engine");
        if (m_editorManager) {
            m_editorManager->redo();
        }
    });
    
    toolbar->addSeparator();
    
    // ========== PLAY CONTROLS GROUP (Unreal-style) ==========
    HICON playIcon = loadShellIcon(137);  // Play/right arrow icon
    toolbar->addButton(5010, "Play", playIcon, [this]() {
        LOG_INFO_C("Play button clicked - entering play mode", "Engine");
        enterPlayMode();
    });
    
    HICON pauseIcon = loadShellIcon(138);  // Pause icon
    toolbar->addButton(5011, "Pause", pauseIcon, [this]() {
        if (m_timeManager) {
            m_timeManager->togglePause();
            bool isPaused = m_timeManager->isPaused();
            LOG_INFO_C(isPaused ? "Game paused" : "Game resumed", "Engine");
        } else {
            LOG_WARNING_C("Cannot pause - TimeManager not initialized", "Engine");
        }
    });
    
    HICON stopIcon = loadShellIcon(131);  // Stop icon
    toolbar->addButton(5012, "Stop", stopIcon, [this]() {
        LOG_INFO_C("Stop button clicked - exiting play mode", "Engine");
        exitPlayMode();
    });
    
    toolbar->addSeparator();
    
    // ========== VIEW CONTROLS GROUP ==========
    HICON perspectiveIcon = loadShellIcon(165);  // 3D perspective icon
    toolbar->addButton(5020, "Perspective", perspectiveIcon, [this]() {
        LOG_INFO_C("Perspective view selected", "Engine");
        if (m_editorManager) {
            m_editorManager->setCameraViewMode("Perspective");
        }
    });
    
    HICON topIcon = loadShellIcon(24);  // Grid/top view icon
    toolbar->addButton(5021, "Top", topIcon, [this]() {
        LOG_INFO_C("Top view selected", "Engine");
        if (m_editorManager) {
            m_editorManager->setCameraViewMode("Top");
        }
    });
    
    HICON cameraIcon = loadShellIcon(176);  // Camera icon
    toolbar->addButton(5006, "Camera", cameraIcon, [this]() {
        LOG_INFO_C("Camera reset button clicked", "Engine");
        if (m_player) {
            // Reset player to default spawn position
            m_player->setPosition(DEFAULT_SPAWN_POSITION);
            // Reset camera rotation to default (look forward)
            m_player->getCamera().setYaw(DEFAULT_CAMERA_YAW);
            m_player->getCamera().setPitch(DEFAULT_CAMERA_PITCH);
            LOG_INFO_C("Camera reset to default position and rotation", "Engine");
        } else {
            LOG_WARNING_C("Cannot reset camera - Player not initialized", "Engine");
        }
    });
    
    HICON fullscreenIcon = loadShellIcon(238);  // Fullscreen/maximize icon
    toolbar->addButton(5007, "Fullscreen", fullscreenIcon, [this]() {
        LOG_INFO_C("Fullscreen toggle requested", "Engine");
        if (m_window) {
            m_window->toggleFullscreen();
        }
    });
    
    toolbar->addSeparator();
    
    // ========== TRANSFORMATION TOOLS GROUP (Unreal-style) ==========
    HICON selectIcon = loadShellIcon(210);  // Cursor/select icon
    toolbar->addButton(5030, "Select", selectIcon, [this]() {
        LOG_INFO_C("Select tool activated", "Engine");
        if (m_worldEditor && m_worldEditor->getTerraformingSystem()) {
            m_worldEditor->getTerraformingSystem()->setTool(TerraformTool::SingleBlock);
        }
    });
    
    HICON moveIcon = loadShellIcon(143);  // Move/arrows icon
    toolbar->addButton(5031, "Move", moveIcon, [this]() {
        LOG_INFO_C("Move tool activated", "Engine");
        if (m_editorManager && m_editorManager->getTransformGizmo()) {
            m_editorManager->getTransformGizmo()->setMode(TransformGizmo::Mode::Translate);
        }
    });
    
    HICON rotateIcon = loadShellIcon(239);  // Rotate/circular arrows icon
    toolbar->addButton(5032, "Rotate", rotateIcon, [this]() {
        LOG_INFO_C("Rotate tool activated", "Engine");
        if (m_editorManager && m_editorManager->getTransformGizmo()) {
            m_editorManager->getTransformGizmo()->setMode(TransformGizmo::Mode::Rotate);
        }
    });
    
    HICON scaleIcon = loadShellIcon(166);  // Scale/resize icon
    toolbar->addButton(5033, "Scale", scaleIcon, [this]() {
        LOG_INFO_C("Scale tool activated", "Engine");
        if (m_editorManager && m_editorManager->getTransformGizmo()) {
            m_editorManager->getTransformGizmo()->setMode(TransformGizmo::Mode::Scale);
        }
    });
    
    toolbar->addSeparator();
    
    // ========== VOXEL TOOLS GROUP ==========
    HICON brushIcon = loadShellIcon(22);  // Paintbrush icon
    toolbar->addButton(5040, "Brush", brushIcon, [this]() {
        LOG_INFO_C("Brush tool activated", "Engine");
        if (m_worldEditor && m_worldEditor->getTerraformingSystem()) {
            m_worldEditor->getTerraformingSystem()->setTool(TerraformTool::Brush);
        }
    });
    
    HICON paintIcon = loadShellIcon(269);  // Paint/color icon
    toolbar->addButton(5041, "Paint", paintIcon, [this]() {
        LOG_INFO_C("Paint tool activated", "Engine");
        if (m_worldEditor && m_worldEditor->getTerraformingSystem()) {
            m_worldEditor->getTerraformingSystem()->setTool(TerraformTool::Paint);
        }
    });
    
    HICON sculptIcon = loadShellIcon(135);  // Sculpt/tool icon
    toolbar->addButton(5042, "Sculpt", sculptIcon, [this]() {
        LOG_INFO_C("Sculpt tool activated", "Engine");
        if (m_worldEditor && m_worldEditor->getTerraformingSystem()) {
            m_worldEditor->getTerraformingSystem()->setTool(TerraformTool::FilledSphere);
        }
    });
    
    HICON smoothIcon = loadShellIcon(133);  // Smooth/gradient icon
    toolbar->addButton(5043, "Smooth", smoothIcon, [this]() {
        LOG_INFO_C("Smooth tool activated", "Engine");
        if (m_worldEditor && m_worldEditor->getTerraformingSystem()) {
            m_worldEditor->getTerraformingSystem()->setTool(TerraformTool::Smooth);
        }
    });

    LOG_INFO_C("Unreal-style native Win32 toolbar initialized with comprehensive tools", "Engine");
}
#endif

void Engine::createDemoEntities()
{
    if (!m_entityManager) {
        LOG_WARNING_C("Cannot create demo entities: EntityManager not available", "Engine");
        return;
    }

    if (!m_editorManager) {
        LOG_WARNING_C("Cannot create demo entities: EditorManager not available", "Engine");
        return;
    }

    auto* sceneHierarchy = m_editorManager->getSceneHierarchy();
    if (!sceneHierarchy) {
        LOG_WARNING_C("Cannot create demo entities: SceneHierarchy not available", "Engine");
        return;
    }

    LOG_INFO_C("Creating demo entities for Inspector demonstration...", "Engine");

    // Create Entity 1: A cube with transform, renderer, and material
    {
        auto entity = m_entityManager->createEntity();
        
        // Add transform
        auto* transform = new ecs::TransformComponent(
            glm::vec3(0.0f, 85.0f, -5.0f),  // position
            glm::quat(1.0f, 0.0f, 0.0f, 0.0f),  // rotation
            glm::vec3(2.0f, 2.0f, 2.0f)      // scale
        );
        m_entityManager->addComponent(entity, transform);
        
        // Add renderer
        auto* renderer = new ecs::RendererComponent();
        renderer->castShadows = true;
        renderer->receiveShadows = true;
        renderer->renderLayer = 0;
        m_entityManager->addComponent(entity, renderer);
        
        // Add material
        auto* material = new ecs::MaterialComponent();
        material->shaderType = ecs::MaterialComponent::ShaderType::Standard;
        material->color = glm::vec4(0.8f, 0.2f, 0.2f, 1.0f);  // Red
        material->metallic = 0.5f;
        material->smoothness = 0.8f;
        m_entityManager->addComponent(entity, material);
        
        // Add to scene hierarchy
        auto node = sceneHierarchy->addNode("Red Cube", nullptr);
        if (node) {
            node->userData = new ecs::Entity(entity);
        }
        
        LOG_INFO_C("Created demo entity: Red Cube", "Engine");
    }

    // Create Entity 2: A sphere with physics
    {
        auto entity = m_entityManager->createEntity();
        
        // Add transform
        auto* transform = new ecs::TransformComponent(
            glm::vec3(5.0f, 90.0f, -5.0f),  // position
            glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
            glm::vec3(1.5f, 1.5f, 1.5f)
        );
        m_entityManager->addComponent(entity, transform);
        
        // Add physics
        auto* physics = new physics::PhysicsComponent();
        physics->mass = 10.0f;
        physics->useGravity = true;
        physics->friction = 0.6f;
        physics->restitution = 0.4f;
        m_entityManager->addComponent(entity, physics);
        
        // Add renderer
        auto* renderer = new ecs::RendererComponent();
        m_entityManager->addComponent(entity, renderer);
        
        // Add material
        auto* material = new ecs::MaterialComponent();
        material->color = glm::vec4(0.2f, 0.8f, 0.2f, 1.0f);  // Green
        material->smoothness = 0.9f;
        m_entityManager->addComponent(entity, material);
        
        // Add to scene hierarchy
        auto node = sceneHierarchy->addNode("Green Sphere (Physics)", nullptr);
        if (node) {
            node->userData = new ecs::Entity(entity);
        }
        
        LOG_INFO_C("Created demo entity: Green Sphere with Physics", "Engine");
    }

    // Create Entity 3: A light source
    {
        auto entity = m_entityManager->createEntity();
        
        // Add transform
        auto* transform = new ecs::TransformComponent(
            glm::vec3(-5.0f, 95.0f, -5.0f),
            glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
            glm::vec3(0.5f, 0.5f, 0.5f)
        );
        m_entityManager->addComponent(entity, transform);
        
        // Add material with emission
        auto* material = new ecs::MaterialComponent();
        material->shaderType = ecs::MaterialComponent::ShaderType::Unlit;
        material->color = glm::vec4(1.0f, 1.0f, 0.5f, 1.0f);  // Yellow
        material->emission = 3.0f;  // Emissive
        m_entityManager->addComponent(entity, material);
        
        // Add renderer
        auto* renderer = new ecs::RendererComponent();
        renderer->castShadows = false;
        m_entityManager->addComponent(entity, renderer);
        
        // Add to scene hierarchy
        auto node = sceneHierarchy->addNode("Yellow Light", nullptr);
        if (node) {
            node->userData = new ecs::Entity(entity);
        }
        
        LOG_INFO_C("Created demo entity: Yellow Light (Emissive)", "Engine");
    }

    LOG_INFO_C("Demo entities created successfully! Select them in Scene Hierarchy to inspect.", "Engine");
}

// Play mode management methods
void Engine::enterPlayMode()
{
    if (m_inGame) {
        LOG_WARNING_C("Already in play mode", "Engine");
        return;
    }
    
    LOG_INFO_C("Entering play mode", "Engine");

#ifdef _WIN32
    // Create a separate game window for play testing
    // The editor window is hidden to completely detach play mode from the editor
    m_gamePlayWindow = std::make_unique<GamePlayWindow>();
    
    // Use the editor viewport dimensions for the game window
    uint32_t gameWidth = 1280;
    uint32_t gameHeight = 720;
    if (m_editorManager && m_editorManager->getViewportPanel()) {
        auto* viewport = m_editorManager->getViewportPanel();
        if (viewport->getWidth() > 0 && viewport->getHeight() > 0) {
            gameWidth = static_cast<uint32_t>(viewport->getWidth());
            gameHeight = static_cast<uint32_t>(viewport->getHeight());
        }
    }
    
    if (!m_gamePlayWindow->create(gameWidth, gameHeight, "Fresh Voxel Engine - Game Preview")) {
        LOG_ERROR_C("Failed to create game play window", "Engine");
        m_gamePlayWindow.reset();
        return;
    }
    
    // Set up input callbacks for the game window
    m_gamePlayWindow->setKeyCallback([this](int vkCode, bool isDown) {
        // ESC in game window closes it and returns to editor
        if (vkCode == VK_ESCAPE && isDown) {
            exitPlayMode();
            return;
        }
        // Forward input to the input manager for gameplay
        if (m_inputManager) {
            m_inputManager->processKeyEvent(static_cast<WPARAM>(vkCode), isDown);
        }
    });
    
    m_gamePlayWindow->setMouseMoveCallback([this](int x, int y) {
        if (m_inputManager) {
            m_inputManager->processMouseMovement(x, y);
        }
    });
    
    m_gamePlayWindow->setMouseButtonCallback([this](int button, bool isDown) {
        if (m_inputManager) {
            m_inputManager->processMouseButton(button, isDown);
        }
    });
    
    // Redirect renderer to the game window
    if (m_renderer) {
        HWND gameHwnd = m_gamePlayWindow->getHandle();
        if (gameHwnd) {
            // Mark viewport as no longer receiving DirectX output
            if (m_editorManager && m_editorManager->getViewportPanel()) {
                m_editorManager->getViewportPanel()->setRenderingActive(false);
            }
            
            if (m_renderer->setViewportWindow(gameHwnd)) {
                if (m_renderer->recreateSwapChain(static_cast<int>(gameWidth), static_cast<int>(gameHeight))) {
                    LOG_INFO_C("Renderer redirected to game play window", "Engine");
                } else {
                    LOG_ERROR_C("Failed to create swap chain for game play window", "Engine");
                    m_gamePlayWindow->close();
                    m_gamePlayWindow.reset();
                    // Restore renderer to editor viewport
                    restoreEditorViewport();
                    return;
                }
            } else {
                LOG_ERROR_C("Failed to redirect renderer to game play window", "Engine");
                m_gamePlayWindow->close();
                m_gamePlayWindow.reset();
                return;
            }
        }
    }

    // Update camera aspect ratio for game window dimensions so the scene
    // renders with the correct projection.  Without this the projection
    // matrix still targets the editor viewport size and the game window
    // can appear blank or distorted.
    if (m_player && gameWidth > 0 && gameHeight > 0) {
        float aspectRatio = static_cast<float>(gameWidth) / static_cast<float>(gameHeight);
        m_player->getCamera().setAspectRatio(aspectRatio);
    }

    // Hide the editor window so the game play window is completely detached.
    // This prevents the game from rendering behind the editor GUI and being
    // visible through transparent gaps between editor panel borders.
    if (m_window) {
        m_window->hide();
    }

    // Bring game window to the foreground with focus
    if (m_gamePlayWindow && m_gamePlayWindow->getHandle()) {
        SetForegroundWindow(m_gamePlayWindow->getHandle());
        SetFocus(m_gamePlayWindow->getHandle());
    }
#endif

    m_inGame = true;
    
    // Switch to game input mode (capture cursor for FPS controls)
    if (m_inputManager) {
        m_inputManager->setInputMode(InputMode::GameMode);
    }
    
    // Disable free-flight mode for player (enable physics)
    if (m_player) {
        m_player->setFreeFlightMode(false);
    }
    
    LOG_INFO_C("Play mode started in separate window - close game window or press ESC to return to editor", "Engine");
}

void Engine::exitPlayMode()
{
    if (!m_inGame) {
        LOG_WARNING_C("Not in play mode", "Engine");
        return;
    }
    
    LOG_INFO_C("Exiting play mode", "Engine");
    m_inGame = false;

#ifdef _WIN32
    // Close the game play window
    if (m_gamePlayWindow) {
        m_gamePlayWindow->close();
        m_gamePlayWindow.reset();
        LOG_INFO_C("Game play window closed", "Engine");
    }
    
    // Restore renderer to the editor viewport
    restoreEditorViewport();

    // Restore the editor window that was hidden during play mode
    if (m_window) {
        m_window->show();
    }
#endif
    
    // Switch back to editor UI mode (release cursor for UI interaction)
    if (m_inputManager) {
        m_inputManager->setInputMode(InputMode::UIMode);
    }
    
    // Enable free-flight mode for player (disable physics for editor camera)
    if (m_player) {
        m_player->setFreeFlightMode(true);
    }
    
    LOG_INFO_C("Returned to editor mode", "Engine");
}

void Engine::togglePlayMode()
{
    if (m_inGame) {
        exitPlayMode();
    } else {
        enterPlayMode();
    }
}

#ifdef _WIN32
void Engine::renderGamePlayWindow()
{
    if (!m_renderer || !m_gamePlayWindow || !m_gamePlayWindow->isOpen()) {
        return;
    }
    
    // Handle game window resize
    if (m_gamePlayWindow->wasResized()) {
        m_gamePlayWindow->resetResizedFlag();
        int w = static_cast<int>(m_gamePlayWindow->getWidth());
        int h = static_cast<int>(m_gamePlayWindow->getHeight());
        if (w > 0 && h > 0) {
            m_renderer->recreateSwapChain(w, h);
            // Keep camera aspect ratio in sync with the resized game window
            if (m_player) {
                float aspectRatio = static_cast<float>(w) / static_cast<float>(h);
                m_player->getCamera().setAspectRatio(aspectRatio);
            }
        }
    }
    
    // Render game scene to the game window
    renderGame();
}

void Engine::restoreEditorViewport()
{
    if (!m_renderer || !m_editorManager) {
        return;
    }
    
    auto* viewportPanel = m_editorManager->getViewportPanel();
    if (!viewportPanel) {
        LOG_WARNING_C("Cannot restore editor viewport - viewport panel not available", "Engine");
        return;
    }
    
    HWND viewportHwnd = viewportPanel->getHandle();
    if (!viewportHwnd) {
        LOG_WARNING_C("Cannot restore editor viewport - no viewport HWND", "Engine");
        return;
    }
    
    int vpWidth = viewportPanel->getWidth();
    int vpHeight = viewportPanel->getHeight();
    
    if (vpWidth <= 0 || vpHeight <= 0) {
        LOG_WARNING_C("Cannot restore editor viewport - invalid dimensions", "Engine");
        return;
    }
    
    if (m_renderer->setViewportWindow(viewportHwnd)) {
        if (m_renderer->recreateSwapChain(vpWidth, vpHeight)) {
            m_viewportSwapChainReady = true;
            viewportPanel->setRenderingActive(true);
            // Restore camera aspect ratio for editor viewport dimensions
            if (m_player) {
                float aspectRatio = static_cast<float>(vpWidth) / static_cast<float>(vpHeight);
                m_player->getCamera().setAspectRatio(aspectRatio);
            }
            LOG_INFO_C("Renderer restored to editor viewport", "Engine");
        } else {
            m_viewportSwapChainReady = false;
            viewportPanel->setRenderingActive(false);
            LOG_ERROR_C("Failed to recreate swap chain for editor viewport - will retry", "Engine");
        }
    } else {
        m_viewportSwapChainReady = false;
        viewportPanel->setRenderingActive(false);
        LOG_ERROR_C("Failed to restore renderer to editor viewport", "Engine");
    }
}
#endif

} // namespace fresh
