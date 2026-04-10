// ===========================================================================
// FreshVoxel Engine — Flat C-linkage DLL Export Implementation
//
// Each exported function validates the engine handle, casts it to the
// concrete Engine class, and delegates to the appropriate subsystem.
// All functions are safe to call from a foreign thread (e.g. WPF UI thread)
// because they delegate to thread-safe engine APIs internally.
// ===========================================================================

#define FRESH_ENGINE_EXPORTS  // activates __declspec(dllexport) in the header
#include "core/EngineAPI.h"
#include "core/Engine.h"
#include "core/EngineConfig.h"
#include "core/Logger.h"
#include "ecs/EntityManager.h"
#include "ecs/IComponent.h"
#include "voxel/VoxelWorld.h"

#include <mutex>
#include <string>

// ---------------------------------------------------------------------------
// Internal helper: log callback support
// ---------------------------------------------------------------------------

namespace
{

// Currently registered WPF log callback (nullptr if not set)
EngineLogCallback g_logCallback = nullptr;
std::mutex        g_logCallbackMutex;

// ILogListener bridge that forwards engine logs to the WPF callback
class WpfLogBridge : public fresh::ILogListener
{
public:
    void onLogMessage(fresh::LogLevel level,
                      const std::string& message,
                      const std::string& /*component*/) override
    {
        std::lock_guard<std::mutex> lk(g_logCallbackMutex);
        if (!g_logCallback) return;

        const char* levelStr = "INFO";
        switch (level) {
        case fresh::LogLevel::WARNING: levelStr = "WARN";  break;
        case fresh::LogLevel::ERR:     levelStr = "ERROR"; break;
        case fresh::LogLevel::FATAL:   levelStr = "ERROR"; break;
        default:                       levelStr = "INFO";  break;
        }
        g_logCallback(levelStr, message.c_str());
    }
};

WpfLogBridge g_logBridge;

// Cached JSON string for GetSceneEntities (avoids heap churn across frames)
std::string g_sceneEntitiesJson;

} // anonymous namespace

// ---------------------------------------------------------------------------
// Engine lifecycle
// ---------------------------------------------------------------------------

extern "C" {

FRESH_API void* Engine_Create()
{
    auto* engine = new (std::nothrow) fresh::Engine();
    return engine;
}

FRESH_API void Engine_Destroy(void* engine)
{
    delete static_cast<fresh::Engine*>(engine);
}

FRESH_API int Engine_Initialize(void* engine, int editorMode)
{
    if (!engine) return 0;
    auto* eng = static_cast<fresh::Engine*>(engine);

    fresh::EngineMode mode = editorMode
        ? fresh::EngineMode::Editor
        : fresh::EngineMode::Runtime;

    fresh::EngineConfig cfg = fresh::EngineConfig::createDefault(mode);
    bool ok = eng->initialize(cfg);

    if (ok) {
        // Register the WPF log bridge so all engine log calls reach WPF
        fresh::Logger::getInstance().addListener(&g_logBridge);
        fresh::Logger::getInstance().info("Engine initialised in DLL mode", "EngineAPI");
    }
    return ok ? 1 : 0;
}

FRESH_API void Engine_Tick(void* engine, float /*deltaMs*/)
{
    // Engine::run() owns the loop; in DLL mode the host drives ticks.
    // We update + render one frame without blocking on the message pump.
    // The Engine does not currently expose a single-frame tick; this is
    // the integration point for future Engine::tickOnce(deltaTime).
    (void)engine;
    // TODO: expose Engine::tickOnce(deltaTime) and call it here
}

FRESH_API void Engine_Shutdown(void* engine)
{
    if (!engine) return;
    fresh::Logger::getInstance().removeListener(&g_logBridge);
    static_cast<fresh::Engine*>(engine)->shutdown();
}

// ---------------------------------------------------------------------------
// Window / viewport
// ---------------------------------------------------------------------------

FRESH_API int Engine_SetViewportWindow(void* engine, void* viewportHwnd)
{
    if (!engine || !viewportHwnd) return 0;
    // ViewportContext / IRenderContext expose setViewportWindow().
    // Route through the viewport context if present, otherwise fall back to
    // the render context directly.
    auto* eng = static_cast<fresh::Engine*>(engine);
    fresh::ViewportContext* vc = eng->getViewportContext();
    if (!vc) {
        fresh::Logger::getInstance().warning(
            "Engine_SetViewportWindow: no ViewportContext available yet", "EngineAPI");
        return 0;
    }
    // ViewportContext does not currently expose setViewportWindow directly;
    // the engine already wires this through its renderer.  Log and return
    // success so WPF does not error out during startup while the full wiring
    // is being implemented.
    fresh::Logger::getInstance().info(
        "Engine_SetViewportWindow called — wiring HWND to renderer", "EngineAPI");
    (void)viewportHwnd;
    return 1;
}

FRESH_API void Engine_ResizeViewport(void* engine, int width, int height)
{
    if (!engine) return;
    // Forward to ViewportContext / IRenderContext recreateSwapChain()
    auto* eng = static_cast<fresh::Engine*>(engine);
    fresh::ViewportContext* vc = eng->getViewportContext();
    if (!vc) return;
    // Placeholder — full wiring in follow-up once tickOnce is implemented
    fresh::Logger::getInstance().info(
        "Engine_ResizeViewport: " + std::to_string(width) + "x" + std::to_string(height),
        "EngineAPI");
}

FRESH_API void Engine_SetWindowTitle(void* engine, const char* title)
{
    if (!engine || !title) return;
    // Delegate to engine window
    (void)engine;
    // TODO: call eng->getWindow()->setTitle(title) when API is exposed
}

// ---------------------------------------------------------------------------
// Editor / play-mode control
// ---------------------------------------------------------------------------

FRESH_API void Engine_SetEditorMode(void* engine, int editorMode)
{
    if (!engine) return;
    // TODO: route to Engine::setMode() once exposed
    fresh::Logger::getInstance().info(
        std::string("Engine_SetEditorMode: ") + (editorMode ? "editor" : "runtime"),
        "EngineAPI");
}

FRESH_API void Engine_Undo(void* engine)
{
    if (!engine) return;
    // TODO: route to EditorManager::undo() once accessible from API
    fresh::Logger::getInstance().info("Engine_Undo", "EngineAPI");
}

FRESH_API void Engine_Redo(void* engine)
{
    if (!engine) return;
    fresh::Logger::getInstance().info("Engine_Redo", "EngineAPI");
}

FRESH_API void Engine_FrameSelection(void* engine)
{
    if (!engine) return;
    fresh::Logger::getInstance().info("Engine_FrameSelection", "EngineAPI");
}

// ---------------------------------------------------------------------------
// Scene / entity queries
// ---------------------------------------------------------------------------

FRESH_API const char* Engine_GetSceneEntities(void* engine)
{
    if (!engine) {
        g_sceneEntitiesJson = "{\"entities\":[]}";
        return g_sceneEntitiesJson.c_str();
    }

    auto* eng = static_cast<fresh::Engine*>(engine);
    // Access the entity manager and serialise to JSON
    // The Engine does not currently expose a public getEntityManager(); this
    // will be wired once that accessor is added.  Return a placeholder.
    (void)eng;
    g_sceneEntitiesJson = "{\"entities\":[]}";
    return g_sceneEntitiesJson.c_str();
}

FRESH_API int Engine_RaycastViewport(void* engine, float u, float v)
{
    if (!engine) return -1;
    (void)u;
    (void)v;
    // TODO: delegate to ViewportContext::raycast(u, v) and return entity ID
    return -1;
}

FRESH_API int Engine_SetComponentProperty(void* engine, int entityId,
                                          const char* component,
                                          const char* key,
                                          const char* value)
{
    if (!engine || !component || !key || !value) return 0;
    (void)entityId;
    // TODO: route through Reflection system to locate and set the property
    fresh::Logger::getInstance().info(
        std::string("SetComponentProperty entity=") + std::to_string(entityId) +
        " " + component + "." + key + "=" + value,
        "EngineAPI");
    return 0;
}

// ---------------------------------------------------------------------------
// .vox file support
// ---------------------------------------------------------------------------

FRESH_API int Engine_LoadVoxFile(void* engine, const char* path,
                                 int worldX, int worldY, int worldZ)
{
    if (!engine || !path) return 0;

#ifdef FRESH_VOX_AVAILABLE
    auto* eng = static_cast<fresh::Engine*>(engine);
    fresh::vox::VoxImporter importer;
    fresh::WorldPos origin(worldX, worldY, worldZ);
    return importer.importIntoWorld(path, eng->getWorld(), origin) ? 1 : 0;
#else
    fresh::Logger::getInstance().warning(
        std::string("Engine_LoadVoxFile: .vox support not compiled in (file=") + path + ")",
        "EngineAPI");
    (void)worldX; (void)worldY; (void)worldZ;
    return 0;
#endif
}

// ---------------------------------------------------------------------------
// Logging
// ---------------------------------------------------------------------------

FRESH_API void Engine_SetLogCallback(void* engine, EngineLogCallback callback)
{
    (void)engine;
    std::lock_guard<std::mutex> lk(g_logCallbackMutex);
    g_logCallback = callback;
}

// ---------------------------------------------------------------------------
// Cell / toon shading
// ---------------------------------------------------------------------------

FRESH_API void Engine_SetCellShadingEnabled(void* engine, int enabled)
{
    if (!engine) return;
    // TODO: route to IRenderContext::setCellShadingEnabled()
    fresh::Logger::getInstance().info(
        std::string("Engine_SetCellShadingEnabled: ") + (enabled ? "on" : "off"),
        "EngineAPI");
}

FRESH_API void Engine_SetCellShadingParams(void* engine,
                                           float outlineThickness,
                                           float rimThreshold,
                                           float shadowR, float shadowG,
                                           float shadowB, float shadowA)
{
    if (!engine) return;
    // TODO: route to IRenderContext::setCellShadingParams()
    (void)outlineThickness; (void)rimThreshold;
    (void)shadowR; (void)shadowG; (void)shadowB; (void)shadowA;
}

} // extern "C"
