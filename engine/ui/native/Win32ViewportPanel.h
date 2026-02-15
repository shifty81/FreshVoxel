#pragma once

#ifdef _WIN32

// Prevent Windows.h from defining min/max macros
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>
#include <string>

namespace fresh
{

class IRenderContext;
class VoxelWorld;
class Player;

/**
 * @brief Win32 native viewport panel for 3D rendering
 * 
 * Provides a dedicated viewport window for DirectX rendering.
 * This panel contains the 3D world view and handles mouse/camera input
 * within its bounds, separate from the editor UI panels.
 */
class Win32ViewportPanel
{
public:
    Win32ViewportPanel();
    ~Win32ViewportPanel();

    // Prevent copying
    Win32ViewportPanel(const Win32ViewportPanel&) = delete;
    Win32ViewportPanel& operator=(const Win32ViewportPanel&) = delete;

    /**
     * @brief Create the viewport panel
     * @param parent Parent window handle
     * @param x X position relative to parent
     * @param y Y position relative to parent
     * @param width Viewport width in pixels
     * @param height Viewport height in pixels
     * @return true if successful
     */
    bool create(HWND parent, int x, int y, int width, int height);

    /**
     * @brief Destroy the viewport panel
     */
    void destroy();

    /**
     * @brief Show or hide the viewport
     * @param show true to show, false to hide
     */
    void setVisible(bool show);

    /**
     * @brief Check if viewport is visible
     */
    bool isVisible() const;

    /**
     * @brief Get the viewport window handle
     * Use this handle to create the DirectX swap chain
     */
    HWND getHandle() const { return m_hwnd; }

    /**
     * @brief Set viewport position relative to parent
     */
    void setPosition(int x, int y);

    /**
     * @brief Set viewport size
     */
    void setSize(int width, int height);

    /**
     * @brief Get viewport width
     */
    int getWidth() const { return m_width; }

    /**
     * @brief Get viewport height
     */
    int getHeight() const { return m_height; }

    /**
     * @brief Check if the viewport was resized this frame
     */
    bool wasResized() const { return m_wasResized; }

    /**
     * @brief Clear the resized flag (call after handling resize)
     */
    void clearResizedFlag() { m_wasResized = false; }

    /**
     * @brief Check if mouse is within viewport bounds
     * @param screenX Mouse X position in screen coordinates
     * @param screenY Mouse Y position in screen coordinates
     * @return true if mouse is inside viewport
     */
    bool isMouseInViewport(int screenX, int screenY) const;

    /**
     * @brief Convert screen coordinates to viewport-relative coordinates
     * @param screenX Mouse X position in screen coordinates
     * @param screenY Mouse Y position in screen coordinates
     * @param outViewportX Output viewport-relative X coordinate
     * @param outViewportY Output viewport-relative Y coordinate
     */
    void screenToViewport(int screenX, int screenY, int& outViewportX, int& outViewportY) const;

    /**
     * @brief Notify the viewport that DirectX rendering is active
     *
     * When true, WM_PAINT simply validates the region (DirectX owns the
     * surface).  When false, WM_PAINT fills the viewport with a solid
     * background colour to prevent ghost images from dragged windows.
     */
    void setRenderingActive(bool active) { m_renderingActive = active; }
    bool isRenderingActive() const { return m_renderingActive; }

private:
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    static bool registerWindowClass();

    HWND m_hwnd;
    HWND m_parent;
    int m_width;
    int m_height;
    bool m_wasResized;
    bool m_renderingActive = false;  ///< True when DirectX swap chain is presenting to this window

    static const wchar_t* WINDOW_CLASS_NAME;
    static bool s_classRegistered;
};

} // namespace fresh

#endif // _WIN32
