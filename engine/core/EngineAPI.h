#pragma once

// ===========================================================================
// FreshVoxel Engine — Flat C-linkage DLL Export API
//
// This header declares every symbol exported by FreshVoxelEngine.dll.
// It is consumed by:
//   - dotnet/EngineInterop.cs  (WPF P/Invoke declarations)
//   - Any external host process that embeds the engine
//
// Design intent:
//   The WPF process is the host.  It creates the outer window, creates a
//   child HWND via HwndHost, hands it to Engine_SetViewportWindow(), and
//   drives the render loop by calling Engine_Tick() every frame from
//   CompositionTarget.Rendering.  The engine never blocks the calling thread
//   with its own message pump when used in DLL mode.
// ===========================================================================

#ifdef FRESH_ENGINE_EXPORTS
    #define FRESH_API __declspec(dllexport)
#elif defined(FRESH_ENGINE_IMPORTS)
    #define FRESH_API __declspec(dllimport)
#else
    // Header included from within a non-DLL build (e.g. static lib tests)
    #define FRESH_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

// ---------------------------------------------------------------------------
// Engine lifecycle
// ---------------------------------------------------------------------------

/** Create a new engine instance.  Returns an opaque handle or NULL on failure. */
FRESH_API void* Engine_Create();

/** Destroy an engine instance previously created by Engine_Create(). */
FRESH_API void  Engine_Destroy(void* engine);

/**
 * Initialize the engine and all subsystems.
 * @param editorMode  Non-zero → start in editor mode.  Zero → runtime mode.
 * @return Non-zero on success.
 */
FRESH_API int   Engine_Initialize(void* engine, int editorMode);

/**
 * Advance the engine by one frame.
 * Call this every frame from the host's render loop (e.g. WPF's
 * CompositionTarget.Rendering event).
 * @param deltaMs  Elapsed milliseconds since the previous call.
 */
FRESH_API void  Engine_Tick(void* engine, float deltaMs);

/** Shutdown and release all engine resources.  Call before Engine_Destroy(). */
FRESH_API void  Engine_Shutdown(void* engine);

// ---------------------------------------------------------------------------
// Window / viewport
// ---------------------------------------------------------------------------

/**
 * Redirect DirectX rendering to the supplied child HWND.
 * This is called once after Engine_Initialize() with the HWND produced by
 * the WPF HwndHost.BuildWindowCore() override.
 * @param viewportHwnd  Win32 HWND of the child window (cast from IntPtr).
 * @return Non-zero on success.
 */
FRESH_API int   Engine_SetViewportWindow(void* engine, void* viewportHwnd);

/**
 * Notify the engine that the viewport panel was resized.
 * Call from the WPF HwndHost.WndProc WM_SIZE handler.
 */
FRESH_API void  Engine_ResizeViewport(void* engine, int width, int height);

/** Set the outer window title (shown in the OS taskbar). */
FRESH_API void  Engine_SetWindowTitle(void* engine, const char* title);

// ---------------------------------------------------------------------------
// Editor / play-mode control
// ---------------------------------------------------------------------------

/** Enter or exit editor mode.  Non-zero → editor; zero → play mode. */
FRESH_API void  Engine_SetEditorMode(void* engine, int editorMode);

/** Undo the last recorded editor action. */
FRESH_API void  Engine_Undo(void* engine);

/** Redo the last undone editor action. */
FRESH_API void  Engine_Redo(void* engine);

/** Frame the camera on the current selection. */
FRESH_API void  Engine_FrameSelection(void* engine);

// ---------------------------------------------------------------------------
// Scene / entity queries
// ---------------------------------------------------------------------------

/**
 * Return a JSON string describing all entities in the current scene.
 *
 * The string is owned by the engine; it remains valid until the next call to
 * Engine_GetSceneEntities() or until Engine_Shutdown() is called.
 *
 * JSON format:
 * {
 *   "entities": [
 *     { "id": 1, "name": "Player", "components": ["Transform", "Player"] },
 *     ...
 *   ]
 * }
 */
FRESH_API const char* Engine_GetSceneEntities(void* engine);

/**
 * Cast a ray from the viewport pixel (u, v) into the scene.
 * u and v are in [0,1] normalised viewport coordinates.
 * @return The entity ID that was hit, or -1 if nothing was hit.
 */
FRESH_API int   Engine_RaycastViewport(void* engine, float u, float v);

/**
 * Set a component property on an entity.
 *
 * @param entityId    Entity identifier (from Engine_RaycastViewport or
 *                    the JSON returned by Engine_GetSceneEntities).
 * @param component   Component name (e.g. "Transform", "Player").
 * @param key         Property key  (e.g. "posX", "health").
 * @param value       Serialised value as a string (e.g. "3.14", "true").
 * @return Non-zero if the property was found and updated.
 */
FRESH_API int   Engine_SetComponentProperty(void* engine, int entityId,
                                            const char* component,
                                            const char* key,
                                            const char* value);

// ---------------------------------------------------------------------------
// .vox file support
// ---------------------------------------------------------------------------

/**
 * Load a MagicaVoxel .vox file and place its voxels in the world.
 * @param path    Absolute or relative path to the .vox file.
 * @param worldX  World X origin for the imported structure.
 * @param worldY  World Y origin for the imported structure.
 * @param worldZ  World Z origin for the imported structure.
 * @return Non-zero on success.
 */
FRESH_API int   Engine_LoadVoxFile(void* engine, const char* path,
                                   int worldX, int worldY, int worldZ);

// ---------------------------------------------------------------------------
// Logging
// ---------------------------------------------------------------------------

/**
 * Register a callback that receives engine log messages.
 *
 * The WPF Output Log panel subscribes here instead of polling a log file.
 *
 * @param callback  Function pointer with signature:
 *                    void callback(const char* level, const char* msg)
 *                  where level is one of "INFO", "WARN", "ERROR", "DEBUG".
 *                  The strings are valid only for the duration of the call.
 */
typedef void (*EngineLogCallback)(const char* level, const char* message);
FRESH_API void  Engine_SetLogCallback(void* engine, EngineLogCallback callback);

// ---------------------------------------------------------------------------
// Cell / toon shading
// ---------------------------------------------------------------------------

/** Enable or disable Borderlands-style cell shading. */
FRESH_API void  Engine_SetCellShadingEnabled(void* engine, int enabled);

/**
 * Update cell shading parameters.
 * @param outlineThickness  World-space extrusion for ink outlines (e.g. 0.04).
 * @param rimThreshold      Dot-product threshold for rim highlight (e.g. 0.6).
 * @param shadowR/G/B/A     RGBA of the deep-shadow colour (0–1 each).
 */
FRESH_API void  Engine_SetCellShadingParams(void* engine,
                                            float outlineThickness,
                                            float rimThreshold,
                                            float shadowR, float shadowG,
                                            float shadowB, float shadowA);

#ifdef __cplusplus
} // extern "C"
#endif
