#ifdef _WIN32

#include "ui/native/Win32HUD.h"
#include "ui/native/UnrealStyleTheme.h"
#include "core/Logger.h"
#include <algorithm>
#include <sstream>
#include <iomanip>

namespace fresh
{

Win32HUD::Win32HUD()
    : m_parentHwnd(nullptr)
    , m_visible(false)
    , m_initialized(false)
    , m_showCrosshair(true)
    , m_showHealthBar(true)
    , m_showStaminaBar(true)
    , m_showHotbar(true)
    , m_showMinimap(false)
    , m_showDebugInfo(true)
    , m_selectedSlot(0)
    , m_backgroundBrush(nullptr)
    , m_selectedBrush(nullptr)
    , m_healthBrush(nullptr)
    , m_staminaBrush(nullptr)
    , m_borderPen(nullptr)
    , m_crosshairPen(nullptr)
    , m_textFont(nullptr)
    , m_debugFont(nullptr)
{
    clearAllSlots();
}

Win32HUD::~Win32HUD()
{
    // Clean up GDI resources
    if (m_backgroundBrush) DeleteObject(m_backgroundBrush);
    if (m_selectedBrush) DeleteObject(m_selectedBrush);
    if (m_healthBrush) DeleteObject(m_healthBrush);
    if (m_staminaBrush) DeleteObject(m_staminaBrush);
    if (m_borderPen) DeleteObject(m_borderPen);
    if (m_crosshairPen) DeleteObject(m_crosshairPen);
    if (m_textFont) DeleteObject(m_textFont);
    if (m_debugFont) DeleteObject(m_debugFont);
}

bool Win32HUD::initialize(HWND parentHwnd)
{
    if (m_initialized) {
        return true;
    }

    m_parentHwnd = parentHwnd;
    
    // Create GDI resources with Unreal Engine theme colors
    m_backgroundBrush = CreateSolidBrush(UnrealStyleTheme::HUDOverlay);
    m_selectedBrush = CreateSolidBrush(UnrealStyleTheme::AccentBlue);
    m_healthBrush = CreateSolidBrush(UnrealStyleTheme::HUDHealthBar);
    m_staminaBrush = CreateSolidBrush(UnrealStyleTheme::HUDStaminaBar);
    m_borderPen = CreatePen(PS_SOLID, 1, UnrealStyleTheme::BorderLight);
    m_crosshairPen = CreatePen(PS_SOLID, 2, UnrealStyleTheme::Crosshair);
    
    // Create fonts - Unreal uses Roboto, but we'll use Segoe UI on Windows
    m_textFont = CreateFontW(
        14, 0, 0, 0, FW_MEDIUM, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI"
    );
    
    m_debugFont = CreateFontW(
        12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Consolas"
    );
    
    m_initialized = true;
    LOG_INFO_C("Win32HUD initialized with Unreal Engine styling", "Win32HUD");
    return true;
}

void Win32HUD::render(HDC hdc, const RECT& clientRect)
{
    if (!m_visible || !m_initialized) {
        return;
    }

    // Set background mode for transparent text
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, UnrealStyleTheme::TextPrimary);
    
    // Render HUD elements in Unreal-style order
    if (m_showCrosshair) {
        renderCrosshair(hdc, clientRect);
    }
    
    if (m_showHealthBar) {
        renderHealthBar(hdc, clientRect);
    }
    
    if (m_showStaminaBar) {
        renderStaminaBar(hdc, clientRect);
    }
    
    if (m_showHotbar) {
        renderHotbar(hdc, clientRect);
    }
    
    if (m_showMinimap) {
        renderMinimap(hdc, clientRect);
    }
    
    if (m_showDebugInfo) {
        renderDebugInfo(hdc, clientRect);
    }
}

void Win32HUD::renderHotbar(HDC hdc, const RECT& clientRect)
{
    const int slotSize = 50;
    const int spacing = 4;
    const int totalWidth = (slotSize + spacing) * HOTBAR_SIZE - spacing;
    const int bottomPadding = 20;
    
    int startX = (clientRect.right - totalWidth) / 2;
    int startY = clientRect.bottom - slotSize - bottomPadding;
    
    // Render each slot
    for (int i = 0; i < HOTBAR_SIZE; ++i) {
        int x = startX + i * (slotSize + spacing);
        int slotNumber = (i + 1) % 10; // 1-9, 0 for last slot
        renderSlot(hdc, x, startY, slotSize, m_slots[i], 
                   i == m_selectedSlot, slotNumber);
    }
}

void Win32HUD::renderSlot(HDC hdc, int x, int y, int size, 
                          const HotbarSlot& slot, bool isSelected, int slotNumber)
{
    // Unreal-style slot rendering with sharp corners and accent color
    COLORREF bgColor = isSelected ? UnrealStyleTheme::HUDOverlay : UnrealStyleTheme::DarkBackground;
    COLORREF borderColor = isSelected ? UnrealStyleTheme::AccentBlue : UnrealStyleTheme::BorderLight;
    
    // Draw slot background with Unreal style (sharp corners, no rounding)
    RECT slotRect = { x, y, x + size, y + size };
    HBRUSH bgBrush = CreateSolidBrush(bgColor);
    HPEN borderPen = CreatePen(PS_SOLID, isSelected ? 2 : 1, borderColor);
    
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, bgBrush);
    HPEN oldPen = (HPEN)SelectObject(hdc, borderPen);
    
    Rectangle(hdc, x, y, x + size, y + size);
    
    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(bgBrush);
    DeleteObject(borderPen);
    
    // Draw accent bar at top for selected slot (Unreal-style)
    if (isSelected) {
        HBRUSH accentBrush = CreateSolidBrush(UnrealStyleTheme::AccentBlue);
        RECT accentRect = { x, y, x + size, y + 3 };
        FillRect(hdc, &accentRect, accentBrush);
        DeleteObject(accentBrush);
    }
    
    // Draw slot number in top-left corner
    HFONT oldFont = (HFONT)SelectObject(hdc, m_textFont);
    SetTextColor(hdc, UnrealStyleTheme::TextSecondary);
    
    WCHAR slotNumText[3];
    swprintf_s(slotNumText, 3, L"%d", slotNumber);
    TextOutW(hdc, x + 4, y + 4, slotNumText, (int)wcslen(slotNumText));
    
    // Draw item if slot is not empty
    if (!slot.isEmpty) {
        // Draw icon if available
        if (slot.icon) {
            // Create a compatible DC for the bitmap
            HDC hdcMem = CreateCompatibleDC(hdc);
            if (hdcMem) {
                HBITMAP oldBitmap = (HBITMAP)SelectObject(hdcMem, slot.icon);
                
                // Get bitmap dimensions
                BITMAP bm;
                GetObject(slot.icon, sizeof(BITMAP), &bm);
                
                // Calculate icon position (centered in slot)
                int iconSize = size - 16;  // Leave margin for slot number and count
                int iconX = x + (size - iconSize) / 2;
                int iconY = y + (size - iconSize) / 2 + 8;  // Offset for slot number
                
                // Draw the bitmap with stretching/shrinking to fit
                SetStretchBltMode(hdc, HALFTONE);
                SetBrushOrgEx(hdc, 0, 0, nullptr);
                StretchBlt(hdc, iconX, iconY, iconSize, iconSize,
                          hdcMem, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
                
                SelectObject(hdcMem, oldBitmap);
                DeleteDC(hdcMem);
            }
        }
        
        // Draw item name with Unreal text color
        SetTextColor(hdc, UnrealStyleTheme::TextPrimary);
        std::wstring itemName(slot.itemName.begin(), slot.itemName.end());
        SIZE textSize;
        GetTextExtentPoint32W(hdc, itemName.c_str(), (int)itemName.length(), &textSize);
        
        int textX = x + (size - textSize.cx) / 2;
        int textY = y + (size - textSize.cy) / 2;
        TextOutW(hdc, textX, textY, itemName.c_str(), (int)itemName.length());
        
        // Draw count badge in bottom-right (Unreal-style)
        if (slot.count > 1) {
            SetTextColor(hdc, UnrealStyleTheme::TextHighlight);
            WCHAR countText[16];
            swprintf_s(countText, 16, L"x%d", slot.count);
            SIZE countSize;
            GetTextExtentPoint32W(hdc, countText, (int)wcslen(countText), &countSize);
            
            // Draw semi-transparent background for count
            int countX = x + size - countSize.cx - 4;
            int countY = y + size - countSize.cy - 4;
            RECT countBg = { countX - 2, countY - 1, countX + countSize.cx + 2, countY + countSize.cy + 1 };
            HBRUSH countBgBrush = CreateSolidBrush(UnrealStyleTheme::HUDBackground);
            FillRect(hdc, &countBg, countBgBrush);
            DeleteObject(countBgBrush);
            
            TextOutW(hdc, countX, countY, countText, (int)wcslen(countText));
        }
    }
    
    SelectObject(hdc, oldFont);
}

void Win32HUD::renderHealthBar(HDC hdc, const RECT& clientRect)
{
    (void)clientRect; // Not used in current implementation
    const int barWidth = 250;
    const int barHeight = 18;
    const int padding = 15;
    
    int x = padding;
    int y = padding;
    
    // Unreal-style health bar with gradient effect
    drawProgressBar(hdc, x, y, barWidth, barHeight, 
                   m_stats.health, m_stats.maxHealth,
                   UnrealStyleTheme::HUDHealthBar, UnrealStyleTheme::DarkBackground);
    
    // Draw health text with Unreal font style
    HFONT oldFont = (HFONT)SelectObject(hdc, m_textFont);
    SetTextColor(hdc, UnrealStyleTheme::TextHighlight);
    
    WCHAR healthText[32];
    swprintf_s(healthText, 32, L"HEALTH  %.0f / %.0f", m_stats.health, m_stats.maxHealth);
    TextOutW(hdc, x + 8, y + 2, healthText, (int)wcslen(healthText));
    
    SelectObject(hdc, oldFont);
}

void Win32HUD::renderStaminaBar(HDC hdc, const RECT& clientRect)
{
    (void)clientRect; // Not used in current implementation
    const int barWidth = 250;
    const int barHeight = 18;
    const int padding = 15;
    
    int x = padding;
    int y = padding + 23;  // Below health bar with Unreal spacing
    
    // Unreal-style stamina bar
    drawProgressBar(hdc, x, y, barWidth, barHeight, 
                   m_stats.stamina, m_stats.maxStamina,
                   UnrealStyleTheme::HUDStaminaBar, UnrealStyleTheme::DarkBackground);
    
    // Draw stamina text
    HFONT oldFont = (HFONT)SelectObject(hdc, m_textFont);
    SetTextColor(hdc, UnrealStyleTheme::TextHighlight);
    
    WCHAR staminaText[32];
    swprintf_s(staminaText, 32, L"STAMINA  %.0f / %.0f", m_stats.stamina, m_stats.maxStamina);
    TextOutW(hdc, x + 8, y + 2, staminaText, (int)wcslen(staminaText));
    
    SelectObject(hdc, oldFont);
}

void Win32HUD::renderCrosshair(HDC hdc, const RECT& clientRect)
{
    int centerX = clientRect.right / 2;
    int centerY = clientRect.bottom / 2;
    const int size = 10;
    const int thickness = 2;
    
    HPEN oldPen = (HPEN)SelectObject(hdc, m_crosshairPen);
    
    // Draw crosshair lines
    // Horizontal line
    MoveToEx(hdc, centerX - size, centerY, nullptr);
    LineTo(hdc, centerX - thickness, centerY);
    MoveToEx(hdc, centerX + thickness, centerY, nullptr);
    LineTo(hdc, centerX + size, centerY);
    
    // Vertical line
    MoveToEx(hdc, centerX, centerY - size, nullptr);
    LineTo(hdc, centerX, centerY - thickness);
    MoveToEx(hdc, centerX, centerY + thickness, nullptr);
    LineTo(hdc, centerX, centerY + size);
    
    SelectObject(hdc, oldPen);
}

void Win32HUD::renderMinimap(HDC hdc, const RECT& clientRect)
{
    const int mapSize = 160;
    const int padding = 15;
    
    int x = clientRect.right - mapSize - padding;
    int y = padding;
    
    // Draw minimap background with Unreal dark theme
    RECT mapRect = { x, y, x + mapSize, y + mapSize };
    HBRUSH bgBrush = CreateSolidBrush(UnrealStyleTheme::ViewportBackground);
    HPEN borderPen = CreatePen(PS_SOLID, 1, UnrealStyleTheme::BorderLight);
    
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, bgBrush);
    HPEN oldPen = (HPEN)SelectObject(hdc, borderPen);
    
    Rectangle(hdc, x, y, x + mapSize, y + mapSize);
    
    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(bgBrush);
    DeleteObject(borderPen);
    
    // Draw grid lines (Unreal-style)
    HPEN gridPen = CreatePen(PS_SOLID, 1, UnrealStyleTheme::GridMinor);
    SelectObject(hdc, gridPen);
    
    int gridSpacing = mapSize / 4;
    for (int i = 1; i < 4; ++i) {
        // Vertical lines
        MoveToEx(hdc, x + i * gridSpacing, y, nullptr);
        LineTo(hdc, x + i * gridSpacing, y + mapSize);
        // Horizontal lines
        MoveToEx(hdc, x, y + i * gridSpacing, nullptr);
        LineTo(hdc, x + mapSize, y + i * gridSpacing);
    }
    
    DeleteObject(gridPen);
    
    // Draw player position (center dot with Unreal blue)
    HBRUSH playerBrush = CreateSolidBrush(UnrealStyleTheme::AccentBlue);
    oldBrush = (HBRUSH)SelectObject(hdc, playerBrush);
    Ellipse(hdc, x + mapSize/2 - 4, y + mapSize/2 - 4, 
            x + mapSize/2 + 4, y + mapSize/2 + 4);
    SelectObject(hdc, oldBrush);
    DeleteObject(playerBrush);
    
    // Add "MINIMAP" label in Unreal style
    HFONT oldFont = (HFONT)SelectObject(hdc, m_debugFont);
    SetTextColor(hdc, UnrealStyleTheme::TextSecondary);
    TextOutW(hdc, x + 6, y + 6, L"MINIMAP", 7);
    SelectObject(hdc, oldFont);
}

void Win32HUD::renderDebugInfo(HDC hdc, const RECT& clientRect)
{
    int x = clientRect.right - 220;
    int y = clientRect.bottom - 60;
    
    HFONT oldFont = (HFONT)SelectObject(hdc, m_debugFont);
    
    // FPS in Unreal accent color
    SetTextColor(hdc, UnrealStyleTheme::AccentBlue);
    WCHAR fpsText[32];
    swprintf_s(fpsText, 32, L"FPS: %d", m_stats.fps);
    TextOutW(hdc, x, y, fpsText, (int)wcslen(fpsText));
    
    // Position in secondary text color
    SetTextColor(hdc, UnrealStyleTheme::TextSecondary);
    WCHAR posText[64];
    swprintf_s(posText, 64, L"X:%.1f Y:%.1f Z:%.1f", 
              m_stats.posX, m_stats.posY, m_stats.posZ);
    TextOutW(hdc, x, y + 16, posText, (int)wcslen(posText));
    
    // Memory/performance info
    WCHAR perfText[64];
    swprintf_s(perfText, 64, L"Render: %.2fms", 1000.0f / (m_stats.fps > 0 ? m_stats.fps : 1));
    TextOutW(hdc, x, y + 32, perfText, (int)wcslen(perfText));
    
    SelectObject(hdc, oldFont);
}

void Win32HUD::drawRoundRect(HDC hdc, int x, int y, int width, int height, 
                             int radius, COLORREF fillColor, COLORREF borderColor)
{
    // Create brushes and pens
    HBRUSH fillBrush = CreateSolidBrush(fillColor);
    HPEN borderPen = CreatePen(PS_SOLID, 2, borderColor);
    
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, fillBrush);
    HPEN oldPen = (HPEN)SelectObject(hdc, borderPen);
    
    // Draw rounded rectangle
    RoundRect(hdc, x, y, x + width, y + height, radius, radius);
    
    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(fillBrush);
    DeleteObject(borderPen);
}

void Win32HUD::drawProgressBar(HDC hdc, int x, int y, int width, int height, 
                               float value, float maxValue, 
                               COLORREF fillColor, COLORREF bgColor)
{
    // Unreal-style progress bar with sharp edges
    RECT bgRect = { x, y, x + width, y + height };
    
    // Draw background
    HBRUSH bgBrush = CreateSolidBrush(bgColor);
    HPEN borderPen = CreatePen(PS_SOLID, 1, UnrealStyleTheme::BorderDark);
    
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, bgBrush);
    HPEN oldPen = (HPEN)SelectObject(hdc, borderPen);
    
    Rectangle(hdc, x, y, x + width, y + height);
    
    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(bgBrush);
    DeleteObject(borderPen);
    
    // Draw fill (progress) with slight inset
    float percentage = (maxValue > 0) ? (value / maxValue) : 0.0f;
    percentage = std::max(0.0f, std::min(1.0f, percentage));
    int fillWidth = (int)((width - 4) * percentage);
    
    if (fillWidth > 0) {
        RECT fillRect = { x + 2, y + 2, x + 2 + fillWidth, y + height - 2 };
        HBRUSH fillBrush = CreateSolidBrush(fillColor);
        FillRect(hdc, &fillRect, fillBrush);
        DeleteObject(fillBrush);
    }
}

HBRUSH Win32HUD::createSolidBrush(COLORREF color)
{
    return CreateSolidBrush(color);
}

HPEN Win32HUD::createPen(COLORREF color, int width)
{
    return CreatePen(PS_SOLID, width, color);
}

void Win32HUD::setSelectedSlot(int slotIndex)
{
    if (slotIndex >= 0 && slotIndex < HOTBAR_SIZE) {
        m_selectedSlot = slotIndex;
        
        if (m_slotActivatedCallback) {
            m_slotActivatedCallback(slotIndex);
        }
    }
}

void Win32HUD::setSlot(int slotIndex, const HotbarSlot& slot)
{
    if (slotIndex >= 0 && slotIndex < HOTBAR_SIZE) {
        m_slots[slotIndex] = slot;
    }
}

const Win32HUD::HotbarSlot& Win32HUD::getSlot(int slotIndex) const
{
    static HotbarSlot emptySlot;
    if (slotIndex >= 0 && slotIndex < HOTBAR_SIZE) {
        return m_slots[slotIndex];
    }
    return emptySlot;
}

void Win32HUD::clearSlot(int slotIndex)
{
    if (slotIndex >= 0 && slotIndex < HOTBAR_SIZE) {
        m_slots[slotIndex] = HotbarSlot();
    }
}

void Win32HUD::clearAllSlots()
{
    for (auto& slot : m_slots) {
        slot = HotbarSlot();
    }
}

void Win32HUD::handleKeyPress(int key)
{
    // Map keys 1-9 to slots 0-8, and key 0 to slot 9
    int slotIndex = -1;
    
    if (key >= '1' && key <= '9') {
        slotIndex = key - '1';
    } else if (key == '0') {
        slotIndex = 9;
    }
    
    if (slotIndex >= 0) {
        setSelectedSlot(slotIndex);
    }
}

} // namespace fresh

#endif // _WIN32
