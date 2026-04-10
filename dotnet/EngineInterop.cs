using System;
using System.Runtime.InteropServices;

namespace FreshEngine.Managed;

/// <summary>
/// P/Invoke declarations for native Fresh Voxel Engine API (FreshVoxelEngine.dll)
/// </summary>
internal static class NativeMethods
{
    private const string DllName = "FreshVoxelEngine.dll";

    // -------------------------------------------------------------------------
    // Engine lifecycle
    // -------------------------------------------------------------------------

    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern IntPtr Engine_Create();

    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern void Engine_Destroy(IntPtr engine);

    /// <param name="editorMode">Non-zero for editor mode; zero for runtime mode.</param>
    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern int Engine_Initialize(IntPtr engine, int editorMode);

    /// <summary>
    /// Advance the engine one frame.  Call every frame from
    /// <c>CompositionTarget.Rendering</c>.
    /// </summary>
    /// <param name="deltaMs">Elapsed milliseconds since previous call.</param>
    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern void Engine_Tick(IntPtr engine, float deltaMs);

    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern void Engine_Shutdown(IntPtr engine);

    // -------------------------------------------------------------------------
    // Window / viewport
    // -------------------------------------------------------------------------

    /// <summary>
    /// Redirect DirectX rendering to a Win32 child HWND created by
    /// <c>HwndHost.BuildWindowCore()</c>.
    /// </summary>
    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern int Engine_SetViewportWindow(IntPtr engine, IntPtr viewportHwnd);

    /// <summary>Notify the engine that the viewport was resized.</summary>
    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern void Engine_ResizeViewport(IntPtr engine, int width, int height);

    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern void Engine_SetWindowTitle(IntPtr engine,
        [MarshalAs(UnmanagedType.LPStr)] string title);

    // -------------------------------------------------------------------------
    // Editor / play-mode control
    // -------------------------------------------------------------------------

    /// <param name="editorMode">Non-zero for editor; zero for play mode.</param>
    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern void Engine_SetEditorMode(IntPtr engine, int editorMode);

    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern void Engine_Undo(IntPtr engine);

    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern void Engine_Redo(IntPtr engine);

    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern void Engine_FrameSelection(IntPtr engine);

    // -------------------------------------------------------------------------
    // Scene / entity queries
    // -------------------------------------------------------------------------

    /// <summary>
    /// Returns a JSON string of all entities in the current scene.
    /// The pointer is valid until the next call or engine shutdown.
    /// </summary>
    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern IntPtr Engine_GetSceneEntities(IntPtr engine);

    /// <summary>
    /// Cast a ray from viewport coordinates (u,v) in [0,1].
    /// </summary>
    /// <returns>Entity ID of the hit object, or -1 if none.</returns>
    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern int Engine_RaycastViewport(IntPtr engine, float u, float v);

    /// <summary>Set a component property on a specific entity.</summary>
    /// <returns>Non-zero if the property was updated.</returns>
    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern int Engine_SetComponentProperty(IntPtr engine, int entityId,
        [MarshalAs(UnmanagedType.LPStr)] string component,
        [MarshalAs(UnmanagedType.LPStr)] string key,
        [MarshalAs(UnmanagedType.LPStr)] string value);

    // -------------------------------------------------------------------------
    // .vox file support
    // -------------------------------------------------------------------------

    /// <summary>Load a MagicaVoxel .vox file into the world at the given origin.</summary>
    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern int Engine_LoadVoxFile(IntPtr engine,
        [MarshalAs(UnmanagedType.LPStr)] string path,
        int worldX, int worldY, int worldZ);

    // -------------------------------------------------------------------------
    // Logging
    // -------------------------------------------------------------------------

    /// <summary>
    /// Delegate type for engine log callbacks.
    /// Both strings are valid only for the duration of the call.
    /// </summary>
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void EngineLogCallback(
        [MarshalAs(UnmanagedType.LPStr)] string level,
        [MarshalAs(UnmanagedType.LPStr)] string message);

    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern void Engine_SetLogCallback(IntPtr engine, EngineLogCallback? callback);

    // -------------------------------------------------------------------------
    // Cell / toon shading
    // -------------------------------------------------------------------------

    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern void Engine_SetCellShadingEnabled(IntPtr engine, int enabled);

    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern void Engine_SetCellShadingParams(IntPtr engine,
        float outlineThickness, float rimThreshold,
        float shadowR, float shadowG, float shadowB, float shadowA);

    // -------------------------------------------------------------------------
    // Prefab system
    // -------------------------------------------------------------------------

    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern int Engine_SavePrefab(IntPtr engine, uint entityId,
        [MarshalAs(UnmanagedType.LPStr)] string filePath);

    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern uint Engine_SpawnPrefab(IntPtr engine,
        [MarshalAs(UnmanagedType.LPStr)] string filePath);

    /// <summary>
    /// Returns a pointer to a C string containing a JSON array of prefab paths.
    /// The string is owned by the native DLL — do not free it.
    /// </summary>
    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern IntPtr Engine_ListPrefabs(IntPtr engine,
        [MarshalAs(UnmanagedType.LPStr)] string directory);
}

/// <summary>
/// Managed wrapper for the Fresh Voxel Engine.
/// Wraps <c>FreshVoxelEngine.dll</c> via P/Invoke for use in WPF or
/// any .NET 9 host application.
/// </summary>
public class Engine : IDisposable
{
    private IntPtr _nativeHandle;
    private bool _disposed;

    // Keep a strong reference to prevent the GC from collecting the delegate
    // while the native code holds a raw function pointer to it.  The delegate
    // must remain assigned for the entire lifetime of the callback registration
    // and must be cleared (by calling SetLogCallback(null)) before the Engine
    // is disposed, otherwise native code may invoke a collected delegate.
    private NativeMethods.EngineLogCallback? _logCallbackDelegate;

    /// <summary>
    /// Initializes a new instance of the Engine class.
    /// </summary>
    /// <exception cref="InvalidOperationException">
    /// Thrown when the native engine instance cannot be created.
    /// </exception>
    public Engine()
    {
        _nativeHandle = NativeMethods.Engine_Create();
        if (_nativeHandle == IntPtr.Zero)
        {
            throw new InvalidOperationException("Failed to create native engine instance");
        }
    }

    /// <summary>
    /// Initializes the engine and its subsystems.
    /// </summary>
    /// <param name="editorMode">True to start in editor mode.</param>
    /// <returns>True if initialization succeeded.</returns>
    public bool Initialize(bool editorMode = true)
    {
        ThrowIfDisposed();
        return NativeMethods.Engine_Initialize(_nativeHandle, editorMode ? 1 : 0) != 0;
    }

    /// <summary>
    /// Advance the engine one frame.  Call every frame from
    /// <c>CompositionTarget.Rendering</c>.
    /// </summary>
    /// <param name="deltaMs">Elapsed milliseconds since last call.</param>
    public void Tick(float deltaMs)
    {
        ThrowIfDisposed();
        NativeMethods.Engine_Tick(_nativeHandle, deltaMs);
    }

    /// <summary>Shuts down the engine and releases resources.</summary>
    public void Shutdown()
    {
        ThrowIfDisposed();
        NativeMethods.Engine_Shutdown(_nativeHandle);
    }

    // -------------------------------------------------------------------------
    // Window / viewport
    // -------------------------------------------------------------------------

    /// <summary>
    /// Redirect DirectX rendering to the child HWND produced by
    /// <c>HwndHost.BuildWindowCore()</c>.
    /// </summary>
    public bool SetViewportWindow(IntPtr hwnd)
    {
        ThrowIfDisposed();
        return NativeMethods.Engine_SetViewportWindow(_nativeHandle, hwnd) != 0;
    }

    /// <summary>Notify the engine that the viewport panel was resized.</summary>
    public void ResizeViewport(int width, int height)
    {
        ThrowIfDisposed();
        NativeMethods.Engine_ResizeViewport(_nativeHandle, width, height);
    }

    /// <summary>Sets the outer window title.</summary>
    public string WindowTitle
    {
        set
        {
            ThrowIfDisposed();
            NativeMethods.Engine_SetWindowTitle(_nativeHandle, value);
        }
    }

    // -------------------------------------------------------------------------
    // Editor / play-mode
    // -------------------------------------------------------------------------

    /// <summary>Switch between editor and play mode.</summary>
    public bool IsEditorMode
    {
        set
        {
            ThrowIfDisposed();
            NativeMethods.Engine_SetEditorMode(_nativeHandle, value ? 1 : 0);
        }
    }

    /// <summary>Undo the last editor action.</summary>
    public void Undo()
    {
        ThrowIfDisposed();
        NativeMethods.Engine_Undo(_nativeHandle);
    }

    /// <summary>Redo the last undone editor action.</summary>
    public void Redo()
    {
        ThrowIfDisposed();
        NativeMethods.Engine_Redo(_nativeHandle);
    }

    /// <summary>Frame the camera on the current selection.</summary>
    public void FrameSelection()
    {
        ThrowIfDisposed();
        NativeMethods.Engine_FrameSelection(_nativeHandle);
    }

    // -------------------------------------------------------------------------
    // Scene / entity queries
    // -------------------------------------------------------------------------

    /// <summary>
    /// Returns JSON describing all entities in the current scene.
    /// </summary>
    public string GetSceneEntitiesJson()
    {
        ThrowIfDisposed();
        IntPtr ptr = NativeMethods.Engine_GetSceneEntities(_nativeHandle);
        return Marshal.PtrToStringAnsi(ptr) ?? "{}";
    }

    /// <summary>
    /// Cast a ray from normalised viewport coordinates.
    /// </summary>
    /// <returns>Entity ID or -1 if nothing hit.</returns>
    public int RaycastViewport(float u, float v)
    {
        ThrowIfDisposed();
        return NativeMethods.Engine_RaycastViewport(_nativeHandle, u, v);
    }

    /// <summary>Set a component property on an entity.</summary>
    public bool SetComponentProperty(int entityId, string component, string key, string value)
    {
        ThrowIfDisposed();
        return NativeMethods.Engine_SetComponentProperty(
            _nativeHandle, entityId, component, key, value) != 0;
    }

    // -------------------------------------------------------------------------
    // .vox import
    // -------------------------------------------------------------------------

    /// <summary>
    /// Load a MagicaVoxel .vox file and place it in the world.
    /// </summary>
    public bool LoadVoxFile(string path, int worldX = 0, int worldY = 0, int worldZ = 0)
    {
        ThrowIfDisposed();
        return NativeMethods.Engine_LoadVoxFile(_nativeHandle, path, worldX, worldY, worldZ) != 0;
    }

    // -------------------------------------------------------------------------
    // Logging
    // -------------------------------------------------------------------------

    /// <summary>
    /// Register a callback that receives engine log messages.
    /// Pass <c>null</c> to unregister.
    /// </summary>
    public void SetLogCallback(Action<string, string>? callback)
    {
        ThrowIfDisposed();
        if (callback is null)
        {
            _logCallbackDelegate = null;
            NativeMethods.Engine_SetLogCallback(_nativeHandle, null);
        }
        else
        {
            _logCallbackDelegate = (level, msg) => callback(level, msg);
            NativeMethods.Engine_SetLogCallback(_nativeHandle, _logCallbackDelegate);
        }
    }

    // -------------------------------------------------------------------------
    // Cell / toon shading
    // -------------------------------------------------------------------------

    /// <summary>Enable or disable Borderlands-style cell shading.</summary>
    public bool CellShadingEnabled
    {
        set
        {
            ThrowIfDisposed();
            NativeMethods.Engine_SetCellShadingEnabled(_nativeHandle, value ? 1 : 0);
        }
    }

    /// <summary>Update cell shading parameters.</summary>
    public void SetCellShadingParams(
        float outlineThickness,
        float rimThreshold,
        float shadowR, float shadowG, float shadowB, float shadowA)
    {
        ThrowIfDisposed();
        NativeMethods.Engine_SetCellShadingParams(_nativeHandle,
            outlineThickness, rimThreshold,
            shadowR, shadowG, shadowB, shadowA);
    }

    // -------------------------------------------------------------------------
    // Prefab system
    // -------------------------------------------------------------------------

    /// <summary>
    /// Serialize the given entity to a .prefab JSON file on disk.
    /// </summary>
    /// <param name="entityId">Entity ID from <see cref="GetSceneEntitiesJson"/>.</param>
    /// <param name="filePath">Absolute output path (recommended extension: .prefab).</param>
    /// <returns>true on success.</returns>
    public bool SavePrefab(uint entityId, string filePath)
    {
        ThrowIfDisposed();
        return NativeMethods.Engine_SavePrefab(_nativeHandle, entityId, filePath) != 0;
    }

    /// <summary>
    /// Spawn (instantiate) a prefab file and return the new entity ID.
    /// </summary>
    /// <param name="filePath">Absolute path to a .prefab file.</param>
    /// <returns>Entity ID of the spawned entity, or 0 on failure.</returns>
    public uint SpawnPrefab(string filePath)
    {
        ThrowIfDisposed();
        return NativeMethods.Engine_SpawnPrefab(_nativeHandle, filePath);
    }

    /// <summary>
    /// List all .prefab files found in <paramref name="directory"/> (non-recursive).
    /// </summary>
    /// <param name="directory">Absolute directory path to scan.</param>
    /// <returns>Array of absolute prefab file paths (may be empty).</returns>
    public string[] ListPrefabs(string directory)
    {
        ThrowIfDisposed();
        var ptr = NativeMethods.Engine_ListPrefabs(_nativeHandle, directory);
        if (ptr == IntPtr.Zero)
            return Array.Empty<string>();

        var json = System.Runtime.InteropServices.Marshal.PtrToStringAnsi(ptr) ?? "[]";
        // Parse the simple JSON array: ["path1","path2",...]
        json = json.Trim();
        if (json == "[]" || json.Length < 2)
            return Array.Empty<string>();

        // Strip outer brackets and split on '", "'
        json = json.Substring(1, json.Length - 2).Trim();
        var parts = json.Split(new[] { "\", \"" }, StringSplitOptions.RemoveEmptyEntries);
        var result = new string[parts.Length];
        for (int i = 0; i < parts.Length; i++)
            result[i] = parts[i].Trim('"').Replace("\\\\", "\\");
        return result;
    }

    // -------------------------------------------------------------------------
    // IDisposable
    // -------------------------------------------------------------------------

    private void ThrowIfDisposed()
    {
        if (_disposed)
        {
            throw new ObjectDisposedException(nameof(Engine));
        }
    }

    /// <inheritdoc/>
    public void Dispose()
    {
        Dispose(true);
        GC.SuppressFinalize(this);
    }

    /// <inheritdoc/>
    protected virtual void Dispose(bool disposing)
    {
        if (!_disposed)
        {
            if (_nativeHandle != IntPtr.Zero)
            {
                // Clear the log callback first so native code cannot invoke the
                // delegate after the managed object is collected.
                if (_logCallbackDelegate != null)
                {
                    NativeMethods.Engine_SetLogCallback(_nativeHandle, null);
                    _logCallbackDelegate = null;
                }
                NativeMethods.Engine_Destroy(_nativeHandle);
                _nativeHandle = IntPtr.Zero;
            }
            _disposed = true;
        }
    }

    /// <inheritdoc/>
    ~Engine()
    {
        Dispose(false);
    }
}

