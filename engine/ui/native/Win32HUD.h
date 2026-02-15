#pragma once

#ifdef _WIN32

// Prevent Windows.h from defining min/max macros
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>
#include <memory>
#include <string>
#include <array>
#include <functional>
#include "ui/native/UnrealStyleTheme.h"

namespace fresh
{

/**
 * @brief Native Win32 Heads-Up Display (HUD) for in-game overlay
 * 
 * Provides a native Windows implementation of the game HUD with Unreal Engine styling:
 * - Hotbar with 10 slots (1-0 keys) - Unreal-style design
 * - Health and stamina bars - Professional gradient bars
 * - Crosshair - Clean, modern design
 * - Minimap - Integrated viewport-style minimap
 * - FPS counter and debug info - Console-style overlay
 * - All rendered using native Win32 GDI/GDI+ with Unreal color theme
 */
class Win32HUD
{
public:
    static constexpr int HOTBAR_SIZE = 10;
    
    struct HotbarSlot {
        bool isEmpty = true;
        std::string itemName;
        int count = 0;
        HBITMAP icon = nullptr; // Optional icon bitmap
    };
    
    struct HUDStats {
        float health = 100.0f;      // 0-100
        float maxHealth = 100.0f;
        float stamina = 100.0f;     // 0-100
        float maxStamina = 100.0f;
        int fps = 0;
        float posX = 0.0f;
        float posY = 0.0f;
        float posZ = 0.0f;
    };
    
    using SlotActivatedCallback = std::function<void(int slotIndex)>;

    Win32HUD();
    ~Win32HUD();

    /**
     * @brief Initialize the HUD system
     * @param parentHwnd Parent window handle
     * @return true if successful
     */
    bool initialize(HWND parentHwnd);

    /**
     * @brief Render the HUD overlay
     * @param hdc Device context to render to
     * @param clientRect Client area rectangle
     */
    void render(HDC hdc, const RECT& clientRect);

    /**
     * @brief Set HUD visibility
     * @param visible true to show, false to hide
     */
    void setVisible(bool visible) { m_visible = visible; }

    /**
     * @brief Check if HUD is visible
     * @return true if visible
     */
    bool isVisible() const { return m_visible; }

    /**
     * @brief Update HUD statistics
     * @param stats Current game statistics
     */
    void updateStats(const HUDStats& stats) { m_stats = stats; }

    /**
     * @brief Set the currently selected hotbar slot
     * @param slotIndex Slot index (0-9, where 9 is the '0' key)
     */
    void setSelectedSlot(int slotIndex);

    /**
     * @brief Get the currently selected slot
     * @return Slot index (0-9)
     */
    int getSelectedSlot() const { return m_selectedSlot; }

    /**
     * @brief Set the contents of a hotbar slot
     * @param slotIndex Slot index (0-9)
     * @param slot Slot data
     */
    void setSlot(int slotIndex, const HotbarSlot& slot);

    /**
     * @brief Get the contents of a hotbar slot
     * @param slotIndex Slot index (0-9)
     * @return Slot data
     */
    const HotbarSlot& getSlot(int slotIndex) const;

    /**
     * @brief Clear a hotbar slot
     * @param slotIndex Slot index (0-9)
     */
    void clearSlot(int slotIndex);

    /**
     * @brief Clear all hotbar slots
     */
    void clearAllSlots();

    /**
     * @brief Set callback for slot activation
     */
    void setSlotActivatedCallback(SlotActivatedCallback callback) {
        m_slotActivatedCallback = callback;
    }

    /**
     * @brief Handle keyboard input for hotbar
     * @param key Virtual key code
     */
    void handleKeyPress(int key);

    /**
     * @brief Enable/disable specific HUD elements
     */
    void setShowCrosshair(bool show) { m_showCrosshair = show; }
    void setShowHealthBar(bool show) { m_showHealthBar = show; }
    void setShowStaminaBar(bool show) { m_showStaminaBar = show; }
    void setShowHotbar(bool show) { m_showHotbar = show; }
    void setShowMinimap(bool show) { m_showMinimap = show; }
    void setShowDebugInfo(bool show) { m_showDebugInfo = show; }

private:
    // Rendering functions
    void renderHotbar(HDC hdc, const RECT& clientRect);
    void renderHealthBar(HDC hdc, const RECT& clientRect);
    void renderStaminaBar(HDC hdc, const RECT& clientRect);
    void renderCrosshair(HDC hdc, const RECT& clientRect);
    void renderMinimap(HDC hdc, const RECT& clientRect);
    void renderDebugInfo(HDC hdc, const RECT& clientRect);
    void renderSlot(HDC hdc, int x, int y, int size, const HotbarSlot& slot, 
                    bool isSelected, int slotNumber);
    
    // Helper functions
    HBRUSH createSolidBrush(COLORREF color);
    HPEN createPen(COLORREF color, int width = 1);
    void drawRoundRect(HDC hdc, int x, int y, int width, int height, int radius, 
                      COLORREF fillColor, COLORREF borderColor);
    void drawProgressBar(HDC hdc, int x, int y, int width, int height, 
                        float value, float maxValue, 
                        COLORREF fillColor, COLORREF bgColor);

private:
    HWND m_parentHwnd;
    bool m_visible;
    bool m_initialized;
    
    // HUD element visibility flags
    bool m_showCrosshair;
    bool m_showHealthBar;
    bool m_showStaminaBar;
    bool m_showHotbar;
    bool m_showMinimap;
    bool m_showDebugInfo;
    
    // Hotbar data
    int m_selectedSlot;
    std::array<HotbarSlot, HOTBAR_SIZE> m_slots;
    SlotActivatedCallback m_slotActivatedCallback;
    
    // Stats
    HUDStats m_stats;
    
    // GDI resources (brushes, pens, fonts)
    HBRUSH m_backgroundBrush;
    HBRUSH m_selectedBrush;
    HBRUSH m_healthBrush;
    HBRUSH m_staminaBrush;
    HPEN m_borderPen;
    HPEN m_crosshairPen;
    HFONT m_textFont;
    HFONT m_debugFont;
};

} // namespace fresh

#endif // _WIN32
