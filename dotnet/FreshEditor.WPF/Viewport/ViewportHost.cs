using System;
using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Windows;
using System.Windows.Interop;
using System.Windows.Media;
using FreshEngine.Managed;

namespace FreshEditor.WPF.Viewport;

/// <summary>
/// WPF HwndHost that hosts a native Win32 child window into which the
/// Fresh Voxel Engine renders via DirectX 11.
///
/// Usage:
///   1. Create an instance and add it to the visual tree.
///   2. Call <see cref="AttachEngine"/> after <c>Engine_Initialize</c>.
///   3. Drive the frame loop from <c>CompositionTarget.Rendering</c>.
/// </summary>
public sealed class ViewportHost : HwndHost
{
    // -------------------------------------------------------------------------
    // Win32 P/Invoke
    // -------------------------------------------------------------------------

    private const int WS_CHILD   = 0x40000000;
    private const int WS_VISIBLE = 0x10000000;
    private const int WS_CLIPCHILDREN = 0x02000000;
    private const int WS_CLIPSIBLINGS = 0x04000000;

    [DllImport("user32.dll", SetLastError = true)]
    private static extern IntPtr CreateWindowEx(
        int    dwExStyle,
        string lpClassName,
        string lpWindowName,
        int    dwStyle,
        int x, int y, int nWidth, int nHeight,
        IntPtr hWndParent,
        IntPtr hMenu,
        IntPtr hInstance,
        IntPtr lpParam);

    [DllImport("user32.dll", SetLastError = true)]
    private static extern bool DestroyWindow(IntPtr hwnd);

    // -------------------------------------------------------------------------
    // State
    // -------------------------------------------------------------------------

    private IntPtr   _hwnd;
    private Engine?  _engine;
    private readonly Stopwatch _stopwatch = Stopwatch.StartNew();
    private double   _lastMs;

    /// <summary>Gets the Win32 HWND of the child window.</summary>
    public IntPtr ChildHwnd => _hwnd;

    // -------------------------------------------------------------------------
    // HwndHost overrides
    // -------------------------------------------------------------------------

    protected override HandleRef BuildWindowCore(HandleRef hwndParent)
    {
        _hwnd = CreateWindowEx(
            dwExStyle:   0,
            lpClassName: "STATIC",
            lpWindowName: "",
            dwStyle:     WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
            x: 0, y: 0,
            nWidth:  (int)ActualWidth  == 0 ? 1 : (int)ActualWidth,
            nHeight: (int)ActualHeight == 0 ? 1 : (int)ActualHeight,
            hWndParent: hwndParent.Handle,
            hMenu:      IntPtr.Zero,
            hInstance:  IntPtr.Zero,
            lpParam:    IntPtr.Zero);

        if (_hwnd == IntPtr.Zero)
            throw new InvalidOperationException("Failed to create viewport child window");

        return new HandleRef(this, _hwnd);
    }

    protected override void DestroyWindowCore(HandleRef hwnd)
    {
        if (hwnd.Handle != IntPtr.Zero)
            DestroyWindow(hwnd.Handle);
    }

    protected override IntPtr WndProc(IntPtr hwnd, int msg, IntPtr wParam, IntPtr lParam, ref bool handled)
    {
        const int WM_SIZE = 0x0005;
        if (msg == WM_SIZE && _engine != null)
        {
            int w = (int)lParam & 0xFFFF;
            int h = (int)((uint)lParam >> 16) & 0xFFFF;
            if (w > 0 && h > 0)
                _engine.ResizeViewport(w, h);
            handled = false; // let default proc handle painting
        }
        return base.WndProc(hwnd, msg, wParam, lParam, ref handled);
    }

    // -------------------------------------------------------------------------
    // Public API
    // -------------------------------------------------------------------------

    /// <summary>
    /// Attach the engine and redirect its DirectX swap chain to this viewport.
    /// Call once after <c>Engine.Initialize()</c> and after the HWND has been
    /// created (i.e. after this control is loaded into the visual tree).
    /// </summary>
    public void AttachEngine(Engine engine)
    {
        _engine = engine;
        if (_hwnd != IntPtr.Zero)
            _engine.SetViewportWindow(_hwnd);

        // Start per-frame tick driven by WPF's composition thread
        CompositionTarget.Rendering += OnRendering;
    }

    /// <summary>Detach the engine and stop the render loop.</summary>
    public void DetachEngine()
    {
        CompositionTarget.Rendering -= OnRendering;
        _engine = null;
    }

    private void OnRendering(object? sender, EventArgs e)
    {
        double now   = _stopwatch.Elapsed.TotalMilliseconds;
        float  delta = (float)(now - _lastMs);
        _lastMs      = now;

        _engine?.Tick(delta);
    }

    protected override void Dispose(bool disposing)
    {
        DetachEngine();
        base.Dispose(disposing);
    }
}
