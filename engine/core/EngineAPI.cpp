// ===========================================================================
// FreshVoxel Engine — Flat C-linkage DLL Export Implementation
//
// Each exported function validates the engine handle, casts it to the
// concrete Engine class, and delegates to the appropriate subsystem.
// All functions are safe to call from a foreign thread (e.g. WPF UI thread)
// because they delegate to thread-safe engine APIs internally.
// ===========================================================================

#ifndef FRESH_ENGINE_EXPORTS
#define FRESH_ENGINE_EXPORTS  // activates __declspec(dllexport) in the header
#endif
#include "core/EngineAPI.h"
#include "core/Engine.h"
#include "core/EngineConfig.h"
#include "core/Logger.h"
#include "ecs/EntityManager.h"
#include "ecs/IComponent.h"
#include "editor/EditorManager.h"
#include "editor/PrefabSystem.h"
#include "gameplay/Camera.h"
#include "physics/CollisionDetection.h"
#include "renderer/RenderContext.h"
#include "viewport/ViewportContext.h"
#include "voxel/VoxelWorld.h"

#ifdef FRESH_VOX_AVAILABLE
#include "assets/vox/VoxImporter.h"
#endif

#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <mutex>
#include <sstream>
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

    // DLL-hosted: the external process (e.g. WPF HwndHost) owns the window.
    // The engine must NOT create its own Win32 window, native menu/toolbar, or
    // native EditorManager panels.  Rendering is deferred until the host calls
    // Engine_SetViewportWindow().
    cfg.dllHosted = true;

    bool ok = eng->initialize(cfg);

    if (ok) {
        // Register the WPF log bridge so all engine log calls reach WPF
        fresh::Logger::getInstance().addListener(&g_logBridge);
        fresh::Logger::getInstance().info("Engine initialised in DLL mode", "EngineAPI");
    }
    return ok ? 1 : 0;
}

FRESH_API void Engine_Tick(void* engine, float deltaMs)
{
    if (!engine) return;
    static_cast<fresh::Engine*>(engine)->tickOnce(deltaMs / 1000.0f);
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
    auto* eng = static_cast<fresh::Engine*>(engine);

    // Route directly through the render context so that DX11/DX12/GL can
    // create a swap chain bound to the WPF child HWND.
    fresh::IRenderContext* rc = eng->getRenderer();
    if (!rc) {
        fresh::Logger::getInstance().warning(
            "Engine_SetViewportWindow: renderer not available", "EngineAPI");
        return 0;
    }
    bool ok = rc->setViewportWindow(viewportHwnd);
    if (ok) {
        fresh::Logger::getInstance().info(
            "Engine_SetViewportWindow: HWND bound to renderer", "EngineAPI");
    }
    return ok ? 1 : 0;
}

FRESH_API void Engine_ResizeViewport(void* engine, int width, int height)
{
    if (!engine || width <= 0 || height <= 0) return;
    auto* eng = static_cast<fresh::Engine*>(engine);

    // Resize via ViewportContext when available; fall back to renderer directly.
    fresh::ViewportContext* vc = eng->getViewportContext();
    if (vc) {
        vc->resize(width, height);
        return;
    }
    fresh::IRenderContext* rc = eng->getRenderer();
    if (rc) {
        rc->recreateSwapChain(width, height);
    }
}

FRESH_API void Engine_SetWindowTitle(void* engine, const char* title)
{
    if (!engine || !title) return;
    static_cast<fresh::Engine*>(engine)->setWindowTitle(title);
}

// ---------------------------------------------------------------------------
// Editor / play-mode control
// ---------------------------------------------------------------------------

FRESH_API void Engine_SetEditorMode(void* engine, int editorMode)
{
    if (!engine) return;
    static_cast<fresh::Engine*>(engine)->setEditorMode(editorMode != 0);
}

FRESH_API void Engine_Undo(void* engine)
{
    if (!engine) return;
    fresh::EditorManager* em = static_cast<fresh::Engine*>(engine)->getEditorManager();
    if (em) {
        em->undo();
    }
}

FRESH_API void Engine_Redo(void* engine)
{
    if (!engine) return;
    fresh::EditorManager* em = static_cast<fresh::Engine*>(engine)->getEditorManager();
    if (em) {
        em->redo();
    }
}

FRESH_API void Engine_FrameSelection(void* engine)
{
    if (!engine) return;
    fresh::EditorManager* em = static_cast<fresh::Engine*>(engine)->getEditorManager();
    if (em) {
        em->frameSelection();
    }
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
    fresh::ecs::EntityManager* em = eng->getEntityManager();

    std::ostringstream ss;
    ss << "{\"entities\":[";

    if (em) {
        const std::vector<fresh::ecs::Entity> entities = em->getAllEntities();
        bool first = true;
        for (const auto& entity : entities) {
            if (!first) ss << ",";
            first = false;
            // Entity IDs are numeric — safe to embed directly.
            // Name uses only ASCII digits — no JSON escaping needed.
            ss << "{\"id\":" << entity.getId()
               << ",\"name\":\"Entity_" << entity.getId()
               << "\",\"components\":[]}";
        }
    }

    ss << "]}";
    g_sceneEntitiesJson = ss.str();
    return g_sceneEntitiesJson.c_str();
}

FRESH_API int Engine_RaycastViewport(void* engine, float u, float v)
{
    if (!engine) return -1;

    auto* eng = static_cast<fresh::Engine*>(engine);
    fresh::ViewportContext* vc = eng->getViewportContext();
    if (!vc) return -1;

    fresh::Camera* cam = vc->getCamera();
    if (!cam) return -1;

    // Build a world-space ray from normalised viewport coordinates [0,1].
    // Convert to NDC: x in [-1,1], y in [-1,1] (flip Y because screen Y is top-down).
    const float ndcX =  2.0f * u - 1.0f;
    const float ndcY =  1.0f - 2.0f * v;

    const glm::mat4 proj = vc->getProjectionMatrix();
    const glm::mat4 view = vc->getViewMatrix();
    const glm::mat4 invProjView = glm::inverse(proj * view);

    const glm::vec4 nearClip = invProjView * glm::vec4(ndcX, ndcY, -1.0f, 1.0f);
    const glm::vec4 farClip  = invProjView * glm::vec4(ndcX, ndcY,  1.0f, 1.0f);

    // Guard against a degenerate projection matrix (w == 0 → perspective divide undefined)
    static constexpr float W_EPSILON = 1e-6f;
    if (std::abs(nearClip.w) < W_EPSILON || std::abs(farClip.w) < W_EPSILON) return -1;

    const glm::vec3 rayOrigin = glm::vec3(nearClip) / nearClip.w;
    const glm::vec3 rayDir    = glm::normalize(glm::vec3(farClip) / farClip.w - rayOrigin);

    fresh::VoxelWorld* world = eng->getWorld();
    if (!world) return -1;

    const fresh::Ray ray(rayOrigin, rayDir);
    const fresh::RayHit hit = fresh::CollisionDetection::raycastVoxel(ray, world, 200.0f);
    if (!hit.hit) return -1;

    // No entity spatial index yet — report the voxel hit position encoded as a
    // negative sentinel so callers can distinguish "hit voxel" from "hit entity".
    // Return -1 (no entity) until entity AABB picking is implemented.
    return -1;
}

FRESH_API int Engine_SetComponentProperty(void* engine, int entityId,
                                          const char* component,
                                          const char* key,
                                          const char* value)
{
    if (!engine || !component || !key || !value) return 0;
    // Full reflection-based property setting requires a registered component
    // property map that does not yet exist.  Log the request so that it is
    // visible in the Output Log and return 0 (not applied) until the
    // reflection system is wired.
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
    fresh::IRenderContext* rc = static_cast<fresh::Engine*>(engine)->getRenderer();
    if (rc) {
        rc->setCellShadingEnabled(enabled != 0);
    }
}

FRESH_API void Engine_SetCellShadingParams(void* engine,
                                           float outlineThickness,
                                           float rimThreshold,
                                           float shadowR, float shadowG,
                                           float shadowB, float shadowA)
{
    if (!engine) return;
    fresh::IRenderContext* rc = static_cast<fresh::Engine*>(engine)->getRenderer();
    if (rc) {
        fresh::IRenderContext::CellShadingParams p;
        p.outlineThickness = outlineThickness;
        p.rimThreshold     = rimThreshold;
        p.shadowR          = shadowR;
        p.shadowG          = shadowG;
        p.shadowB          = shadowB;
        p.shadowA          = shadowA;
        rc->setCellShadingParams(p);
    }
}

// ---------------------------------------------------------------------------
// Prefab system
// ---------------------------------------------------------------------------

// Thread-local scratch buffer for string return values (avoids malloc per call)
static thread_local std::string g_prefabReturnBuffer;

FRESH_API int Engine_SavePrefab(void* engine, unsigned int entityId, const char* filePath)
{
    if (!engine || !filePath) return 0;
    auto* eng = static_cast<fresh::Engine*>(engine);
    auto* em  = eng->getEntityManager();
    if (!em) return 0;

    fresh::ecs::Entity entity(static_cast<fresh::ecs::Entity::ID>(entityId));
    if (!em->isEntityValid(entity)) {
        fresh::Logger::getInstance().error(
            "Engine_SavePrefab: entity " + std::to_string(entityId) + " is not valid",
            "EngineAPI");
        return 0;
    }

    fresh::PrefabSystem ps;
    return ps.saveEntityAsPrefab(*em, entity, filePath) ? 1 : 0;
}

FRESH_API unsigned int Engine_SpawnPrefab(void* engine, const char* filePath)
{
    if (!engine || !filePath) return 0;
    auto* eng = static_cast<fresh::Engine*>(engine);
    auto* em  = eng->getEntityManager();
    if (!em) return 0;

    fresh::PrefabSystem ps;
    fresh::ecs::Entity spawned = ps.spawnPrefab(*em, filePath);
    if (!em->isEntityValid(spawned)) return 0;
    return static_cast<unsigned int>(spawned.getId());
}

FRESH_API const char* Engine_ListPrefabs(void* engine, const char* directory)
{
    if (!directory) { g_prefabReturnBuffer = "[]"; return g_prefabReturnBuffer.c_str(); }
    (void)engine; // does not need the engine instance

    fresh::PrefabSystem ps;
    const auto paths = ps.listPrefabs(directory);

    std::ostringstream ss;
    ss << "[";
    for (size_t i = 0; i < paths.size(); ++i) {
        if (i > 0) ss << ", ";
        ss << "\"";
        // Escape backslashes (Windows paths)
        for (char c : paths[i]) {
            if (c == '\\') ss << "\\\\";
            else if (c == '"') ss << "\\\"";
            else ss << c;
        }
        ss << "\"";
    }
    ss << "]";
    g_prefabReturnBuffer = ss.str();
    return g_prefabReturnBuffer.c_str();
}

} // extern "C"