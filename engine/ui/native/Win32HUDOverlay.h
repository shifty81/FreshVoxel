#pragma once

#ifdef _WIN32

#include <windows.h>
#include <memory>

namespace fresh
{

class Win32HUD;

/**
 * @brief Overlay window for rendering Win32HUD using GDI
 * 
 * This class creates a transparent overlay window that sits on top of the
 * main DirectX viewport to render the HUD using native Win32 GDI calls.
 * This solves the incompatibility between DirectX rendering and GDI rendering.
 * 
 * The overlay window is:
 * - Transparent (WS_EX_LAYERED)
 * - Click-through for input (WS_EX_TRANSPARENT)
 * - Always on top of parent (child window)
 * - Automatically positioned and resized with parent
 */
class Win32HUDOverlay
{
public:
    Win32HUDOverlay();
    ~Win32HUDOverlay();

    // Prevent copying
    Win32HUDOverlay(const Win32HUDOverlay&) = delete;
    Win32HUDOverlay& operator=(const Win32HUDOverlay&) = delete;

    /**
     * @brief Initialize the overlay window
     * @param parentHwnd Parent window handle
     * @param hud Pointer to the Win32HUD instance to render
     * @return true if successful
     */
    bool initialize(HWND parentHwnd, Win32HUD* hud);

    /**
     * @brief Shutdown and destroy the overlay window
     */
    void shutdown();

    /**
     * @brief Update the overlay window position and size to match parent
     */
    void updatePosition();

    /**
     * @brief Set overlay visibility
     * @param visible true to show, false to hide
     */
    void setVisible(bool visible);

    /**
     * @brief Invalidate the overlay to trigger a repaint
     */
    void invalidate();

    /**
     * @brief Get the overlay window handle
     * @return HWND handle or nullptr if not initialized
     */
    HWND getHandle() const { return m_hwnd; }

    /**
     * @brief Check if overlay is initialized
     * @return true if initialized
     */
    bool isInitialized() const { return m_initialized; }

private:
    /**
     * @brief Window procedure for the overlay window
     */
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    /**
     * @brief Register the window class
     */
    bool registerWindowClass();

    /**
     * @brief Create the overlay window
     */
    bool createOverlayWindow();

    /**
     * @brief Handle WM_PAINT message
     */
    void onPaint();

private:
    HWND m_hwnd;              // Overlay window handle
    HWND m_parentHwnd;        // Parent window handle
    Win32HUD* m_hud;          // HUD instance to render (not owned)
    bool m_initialized;       // Initialization flag
    HBRUSH m_clearBrush;      // Cached brush for clearing background
    
    static constexpr const wchar_t* WINDOW_CLASS_NAME = L"FreshHUDOverlayWindow";
};

} // namespace fresh

#endif // _WIN32
