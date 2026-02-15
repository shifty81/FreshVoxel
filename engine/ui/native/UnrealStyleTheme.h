#pragma once

#ifdef _WIN32

// Prevent Windows.h from defining min/max macros
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>
#include <dwmapi.h>

#pragma comment(lib, "dwmapi.lib")

namespace fresh
{

/**
 * @brief Unreal Engine-inspired color theme for native Win32 UI
 * 
 * Colors based on Unreal Engine 5 Editor's dark theme with professional
 * blue accents and excellent contrast ratios for readability.
 */
struct UnrealStyleTheme
{
    // Main background colors (very dark charcoal with slight blue tint)
    static constexpr COLORREF WindowBackground = RGB(32, 32, 32);           // #202020
    static constexpr COLORREF PanelBackground = RGB(37, 37, 38);            // #252526
    static constexpr COLORREF DarkBackground = RGB(28, 28, 28);             // #1C1C1C
    static constexpr COLORREF MenuBarBackground = RGB(45, 45, 48);          // #2D2D30
    static constexpr COLORREF ToolbarBackground = RGB(45, 45, 48);          // #2D2D30
    
    // Border and separator colors
    static constexpr COLORREF BorderLight = RGB(60, 60, 60);                // #3C3C3C
    static constexpr COLORREF BorderDark = RGB(45, 45, 45);                 // #2D2D2D
    static constexpr COLORREF Separator = RGB(51, 51, 51);                  // #333333
    
    // Text colors (high contrast white/gray)
    static constexpr COLORREF TextPrimary = RGB(241, 241, 241);             // #F1F1F1
    static constexpr COLORREF TextNormal = RGB(241, 241, 241);              // #F1F1F1 - Alias for TextPrimary
    static constexpr COLORREF TextSecondary = RGB(180, 180, 180);           // #B4B4B4
    static constexpr COLORREF TextDisabled = RGB(108, 108, 108);            // #6C6C6C
    static constexpr COLORREF TextHighlight = RGB(255, 255, 255);           // #FFFFFF
    
    // Unreal Blue accent colors (signature blue from UE5)
    static constexpr COLORREF AccentBlue = RGB(0, 122, 204);                // #007ACC - Unreal signature blue
    static constexpr COLORREF AccentBlueHover = RGB(28, 151, 234);          // #1C97EA
    static constexpr COLORREF AccentBlueActive = RGB(0, 98, 163);           // #0062A3
    static constexpr COLORREF AccentBlueDim = RGB(0, 84, 141);              // #00548D
    
    // Button colors
    static constexpr COLORREF ButtonNormal = RGB(60, 60, 60);               // #3C3C3C
    static constexpr COLORREF ButtonHover = RGB(80, 80, 80);                // #505050
    static constexpr COLORREF ButtonActive = RGB(48, 48, 48);               // #303030
    static constexpr COLORREF ButtonPressed = RGB(48, 48, 48);              // #303030 - Alias for ButtonActive
    static constexpr COLORREF ButtonDisabled = RGB(45, 45, 45);             // #2D2D2D
    
    // Input field colors
    static constexpr COLORREF InputBackground = RGB(51, 51, 51);            // #333333
    static constexpr COLORREF InputBackgroundHover = RGB(60, 60, 60);       // #3C3C3C
    static constexpr COLORREF InputBackgroundActive = RGB(45, 45, 45);      // #2D2D2D
    static constexpr COLORREF InputBorder = RGB(65, 65, 65);                // #414141
    
    // Selection colors
    static constexpr COLORREF SelectionBackground = RGB(0, 122, 204);       // #007ACC
    static constexpr COLORREF SelectionBackgroundInactive = RGB(60, 60, 60);// #3C3C3C
    static constexpr COLORREF SelectionText = RGB(255, 255, 255);           // #FFFFFF
    
    // Tab colors
    static constexpr COLORREF TabActive = RGB(45, 45, 48);                  // #2D2D30
    static constexpr COLORREF TabInactive = RGB(37, 37, 38);                // #252526
    static constexpr COLORREF TabHover = RGB(52, 52, 54);                   // #343436
    static constexpr COLORREF TabBorder = RGB(0, 122, 204);                 // #007ACC - Active tab indicator
    
    // Status and feedback colors
    static constexpr COLORREF StatusSuccess = RGB(73, 190, 70);             // #49BE46 - Green
    static constexpr COLORREF StatusWarning = RGB(255, 194, 10);            // #FFC20A - Yellow/Gold
    static constexpr COLORREF StatusError = RGB(232, 17, 35);               // #E81123 - Red
    static constexpr COLORREF StatusInfo = RGB(0, 122, 204);                // #007ACC - Blue
    
    // Console-specific color aliases
    static constexpr COLORREF INFO_COLOR = StatusInfo;
    static constexpr COLORREF WARNING_COLOR = StatusWarning;
    static constexpr COLORREF ERROR_COLOR = StatusError;
    static constexpr COLORREF PRIMARY_TEXT = TextPrimary;
    static constexpr COLORREF DARK_BACKGROUND = DarkBackground;
    
    // Special UI elements
    static constexpr COLORREF ScrollbarBackground = RGB(37, 37, 38);        // #252526
    static constexpr COLORREF ScrollbarThumb = RGB(104, 104, 104);          // #686868
    static constexpr COLORREF ScrollbarThumbHover = RGB(158, 158, 158);     // #9E9E9E
    
    // Grid and viewport colors
    static constexpr COLORREF GridMajor = RGB(51, 51, 51);                  // #333333
    static constexpr COLORREF GridMinor = RGB(40, 40, 40);                  // #282828
    static constexpr COLORREF ViewportBackground = RGB(28, 28, 28);         // #1C1C1C
    
    // HUD overlay colors (for in-game UI)
    static constexpr COLORREF HUDBackground = RGB(20, 20, 20);              // #141414 - Semi-transparent in practice
    static constexpr COLORREF HUDOverlay = RGB(40, 40, 45);                 // #28282D
    static constexpr COLORREF HUDAccent = RGB(0, 122, 204);                 // #007ACC
    static constexpr COLORREF HUDHealthBar = RGB(220, 50, 50);              // #DC3232 - Red
    static constexpr COLORREF HUDStaminaBar = RGB(120, 200, 80);            // #78C850 - Green
    static constexpr COLORREF HUDManaBar = RGB(80, 120, 200);               // #5078C8 - Blue
    
    // Crosshair and targeting
    static constexpr COLORREF Crosshair = RGB(255, 255, 255);               // #FFFFFF
    static constexpr COLORREF CrosshairTarget = RGB(255, 100, 100);         // #FF6464 - Red when targeting
    
    // Helper function to create semi-transparent colors (for layering)
    static COLORREF WithAlpha(COLORREF color, int alpha) {
        // Note: Use AlphaBlend or layered windows for true transparency
        // This is a helper for documentation purposes
        (void)alpha;  // Mark as unused - functionality not implemented yet
        return color;
    }
    
    // Font sizes (in points)
    static constexpr int FontSizeNormal = 9;
    static constexpr int FontSizeSmall = 8;
    static constexpr int FontSizeLarge = 10;
    static constexpr int FontSizeTitle = 12;
    
    // Spacing and sizing constants (in pixels)
    static constexpr int PaddingSmall = 4;
    static constexpr int PaddingMedium = 8;
    static constexpr int PaddingLarge = 12;
    static constexpr int BorderRadius = 0;  // Unreal uses sharp corners
    static constexpr int BorderWidth = 1;
    static constexpr int ButtonHeight = 24;
    static constexpr int InputHeight = 22;
    static constexpr int TabHeight = 24;
    
    // Icon sizes
    static constexpr int IconSmall = 16;
    static constexpr int IconMedium = 24;
    static constexpr int IconLarge = 32;
    
    // Panel title bar colors
    static constexpr COLORREF TitleBarBackground = RGB(45, 45, 48);         // #2D2D30
    static constexpr COLORREF TitleBarText = RGB(241, 241, 241);            // #F1F1F1
    static constexpr COLORREF TitleBarAccent = RGB(0, 122, 204);            // #007ACC
    static constexpr int TitleBarHeight = 26;
    
    // Hover and focus states
    static constexpr COLORREF FocusBorder = RGB(0, 122, 204);               // #007ACC
    static constexpr COLORREF HoverOverlay = RGB(255, 255, 255);            // 10% white overlay effect
    
    // Helper functions for applying theme
    /**
     * @brief Apply theme colors to a window
     * @param hwnd Window handle to apply theme to
     */
    static void ApplyToWindow(HWND hwnd) {
        if (!hwnd) return;
        
        // Set background color
        SetClassLongPtrW(hwnd, GCLP_HBRBACKGROUND, (LONG_PTR)CreateSolidBrush(PanelBackground));
        
        // Apply dark theme to window
        BOOL useDarkMode = TRUE;
        DwmSetWindowAttribute(hwnd, 20 /*DWMWA_USE_IMMERSIVE_DARK_MODE*/, &useDarkMode, sizeof(useDarkMode));
        
        // Set text color for the window
        InvalidateRect(hwnd, nullptr, TRUE);
    }
    
    /**
     * @brief Calculate font height for a given point size
     * Uses system DPI for proper scaling on high-DPI displays
     * @param pointSize Font size in points
     * @return Font height in pixels (negative for character height)
     */
    static int GetFontHeight(int pointSize) {
        // Get system DPI - default to 96 if unable to retrieve
        HDC hdc = GetDC(nullptr);
        int dpi = hdc ? GetDeviceCaps(hdc, LOGPIXELSY) : 96;
        if (hdc) ReleaseDC(nullptr, hdc);
        
        // Formula: height = -(pointSize * DPI / 72)
        return -MulDiv(pointSize, dpi, 72);
    }
    
    /**
     * @brief Get default font for UI elements
     * @return Handle to default font (9pt, DPI-aware)
     */
    static HFONT GetFont() {
        static HFONT hFont = nullptr;
        if (!hFont) {
            hFont = CreateFontW(
                GetFontHeight(FontSizeNormal),  // 9pt, DPI-aware
                0, 0, 0,
                FW_NORMAL,
                FALSE, FALSE, FALSE,
                DEFAULT_CHARSET,
                OUT_DEFAULT_PRECIS,
                CLIP_DEFAULT_PRECIS,
                CLEARTYPE_QUALITY,
                DEFAULT_PITCH | FF_DONTCARE,
                L"Segoe UI"
            );
        }
        return hFont;
    }
    
    /**
     * @brief Get bold font for section headers
     * @return Handle to bold font (9pt, DPI-aware)
     */
    static HFONT GetBoldFont() {
        static HFONT hBoldFont = nullptr;
        if (!hBoldFont) {
            hBoldFont = CreateFontW(
                GetFontHeight(FontSizeNormal),  // 9pt, DPI-aware
                0, 0, 0,
                FW_SEMIBOLD,
                FALSE, FALSE, FALSE,
                DEFAULT_CHARSET,
                OUT_DEFAULT_PRECIS,
                CLIP_DEFAULT_PRECIS,
                CLEARTYPE_QUALITY,
                DEFAULT_PITCH | FF_DONTCARE,
                L"Segoe UI"
            );
        }
        return hBoldFont;
    }
    
    /**
     * @brief Get title font for panel headers
     * @return Handle to title font (12pt, DPI-aware)
     */
    static HFONT GetTitleFont() {
        static HFONT hTitleFont = nullptr;
        if (!hTitleFont) {
            hTitleFont = CreateFontW(
                GetFontHeight(FontSizeTitle),  // 12pt, DPI-aware
                0, 0, 0,
                FW_SEMIBOLD,
                FALSE, FALSE, FALSE,
                DEFAULT_CHARSET,
                OUT_DEFAULT_PRECIS,
                CLIP_DEFAULT_PRECIS,
                CLEARTYPE_QUALITY,
                DEFAULT_PITCH | FF_DONTCARE,
                L"Segoe UI"
            );
        }
        return hTitleFont;
    }
    
    /**
     * @brief Get monospace font for console/code display
     * @return Handle to monospace font (9pt, DPI-aware)
     */
    static HFONT GetMonospaceFont() {
        static HFONT hMonoFont = nullptr;
        if (!hMonoFont) {
            hMonoFont = CreateFontW(
                GetFontHeight(FontSizeNormal),  // 9pt, DPI-aware
                0, 0, 0,
                FW_NORMAL,
                FALSE, FALSE, FALSE,
                DEFAULT_CHARSET,
                OUT_DEFAULT_PRECIS,
                CLIP_DEFAULT_PRECIS,
                CLEARTYPE_QUALITY,
                FIXED_PITCH | FF_MODERN,
                L"Consolas"
            );
        }
        return hMonoFont;
    }
    
    /**
     * @brief Create a brush for the specified color
     * @param color The color for the brush
     * @return Handle to the created brush (caller should manage lifetime)
     */
    static HBRUSH CreateBrush(COLORREF color) {
        return CreateSolidBrush(color);
    }
    
    /**
     * @brief Get a cached brush for panel background
     * @return Handle to panel background brush
     */
    static HBRUSH GetPanelBackgroundBrush() {
        static HBRUSH hBrush = nullptr;
        if (!hBrush) {
            hBrush = CreateSolidBrush(PanelBackground);
        }
        return hBrush;
    }
    
    /**
     * @brief Get a cached brush for dark background
     * @return Handle to dark background brush
     */
    static HBRUSH GetDarkBackgroundBrush() {
        static HBRUSH hBrush = nullptr;
        if (!hBrush) {
            hBrush = CreateSolidBrush(DarkBackground);
        }
        return hBrush;
    }
    
    /**
     * @brief Get a cached brush for input background
     * @return Handle to input background brush
     */
    static HBRUSH GetInputBackgroundBrush() {
        static HBRUSH hBrush = nullptr;
        if (!hBrush) {
            hBrush = CreateSolidBrush(InputBackground);
        }
        return hBrush;
    }
    
    /**
     * @brief Get a cached brush for title bar background
     * @return Handle to title bar background brush
     */
    static HBRUSH GetTitleBarBackgroundBrush() {
        static HBRUSH hBrush = nullptr;
        if (!hBrush) {
            hBrush = CreateSolidBrush(TitleBarBackground);
        }
        return hBrush;
    }
    
    /**
     * @brief Get a cached pen for accent line
     * @return Handle to accent pen
     */
    static HPEN GetAccentPen() {
        static HPEN hPen = nullptr;
        if (!hPen) {
            hPen = CreatePen(PS_SOLID, 2, TitleBarAccent);
        }
        return hPen;
    }
    
    /**
     * @brief Get a cached pen for dark borders
     * @return Handle to dark border pen
     */
    static HPEN GetBorderDarkPen() {
        static HPEN hPen = nullptr;
        if (!hPen) {
            hPen = CreatePen(PS_SOLID, 1, BorderDark);
        }
        return hPen;
    }
    
    /**
     * @brief Draw a panel title bar with accent line
     * @param hdc Device context
     * @param rect Rectangle for the title bar
     * @param title Title text to display
     * @param showAccent Whether to show the accent line
     */
    static void DrawPanelTitleBar(HDC hdc, const RECT& rect, const wchar_t* title, bool showAccent = true) {
        // Fill title bar background using cached brush
        FillRect(hdc, &rect, GetTitleBarBackgroundBrush());
        
        // Draw accent line at top using cached pen
        if (showAccent) {
            HPEN oldPen = (HPEN)SelectObject(hdc, GetAccentPen());
            MoveToEx(hdc, rect.left, rect.top, nullptr);
            LineTo(hdc, rect.right, rect.top);
            SelectObject(hdc, oldPen);
        }
        
        // Draw title text
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, TitleBarText);
        HFONT oldFont = (HFONT)SelectObject(hdc, GetTitleFont());
        
        RECT textRect = rect;
        textRect.left += PaddingMedium;
        textRect.top += 2;  // Offset for accent line
        DrawTextW(hdc, title, -1, &textRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        
        SelectObject(hdc, oldFont);
    }
    
    /**
     * @brief Get a cached brush for button background
     * @return Handle to button background brush
     */
    static HBRUSH GetButtonBackgroundBrush() {
        static HBRUSH hBrush = nullptr;
        if (!hBrush) {
            hBrush = CreateSolidBrush(ButtonNormal);
        }
        return hBrush;
    }
    
    /**
     * @brief Get a cached pen for separators
     * @return Handle to separator pen
     */
    static HPEN GetSeparatorPen() {
        static HPEN hPen = nullptr;
        if (!hPen) {
            hPen = CreatePen(PS_SOLID, 1, Separator);
        }
        return hPen;
    }
    
    /**
     * @brief Draw a separator line
     * @param hdc Device context
     * @param x1 Start X coordinate
     * @param y1 Start Y coordinate
     * @param x2 End X coordinate
     * @param y2 End Y coordinate
     */
    static void DrawSeparator(HDC hdc, int x1, int y1, int x2, int y2) {
        HPEN oldPen = (HPEN)SelectObject(hdc, GetSeparatorPen());
        MoveToEx(hdc, x1, y1, nullptr);
        LineTo(hdc, x2, y2);
        SelectObject(hdc, oldPen);
    }
    
    /**
     * @brief Draw a section header with optional collapse indicator
     * @param hdc Device context
     * @param rect Rectangle for the header
     * @param title Section title
     * @param collapsed Whether section is collapsed
     */
    static void DrawSectionHeader(HDC hdc, const RECT& rect, const wchar_t* title, bool collapsed = false) {
        // Fill background with cached brush
        FillRect(hdc, &rect, GetButtonBackgroundBrush());
        
        // Draw collapse indicator (triangle)
        SetTextColor(hdc, TextSecondary);
        SetBkMode(hdc, TRANSPARENT);
        HFONT oldFont = (HFONT)SelectObject(hdc, GetFont());
        
        RECT indicatorRect = rect;
        indicatorRect.left += PaddingSmall;
        indicatorRect.right = indicatorRect.left + IconSmall;
        const wchar_t* indicator = collapsed ? L"\x25B6" : L"\x25BC";  // Right or Down triangle
        DrawTextW(hdc, indicator, -1, &indicatorRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        
        // Draw title
        SetTextColor(hdc, TextPrimary);
        SelectObject(hdc, GetBoldFont());
        
        RECT textRect = rect;
        textRect.left += IconSmall + PaddingMedium;
        DrawTextW(hdc, title, -1, &textRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        
        SelectObject(hdc, oldFont);
    }
    
    /**
     * @brief Get RGB components from COLORREF
     * @param color The color to decompose
     * @param r Output red component (0-255)
     * @param g Output green component (0-255)
     * @param b Output blue component (0-255)
     */
    static void GetRGB(COLORREF color, BYTE& r, BYTE& g, BYTE& b) {
        r = GetRValue(color);
        g = GetGValue(color);
        b = GetBValue(color);
    }
    
    /**
     * @brief Blend two colors together
     * @param color1 First color
     * @param color2 Second color
     * @param factor Blend factor (0.0 = color1, 1.0 = color2), clamped to [0.0, 1.0]
     * @return Blended color
     */
    static COLORREF BlendColors(COLORREF color1, COLORREF color2, float factor) {
        // Clamp factor to valid range
        if (factor < 0.0f) factor = 0.0f;
        if (factor > 1.0f) factor = 1.0f;
        
        BYTE r1, g1, b1, r2, g2, b2;
        GetRGB(color1, r1, g1, b1);
        GetRGB(color2, r2, g2, b2);
        
        // Helper lambda to blend a single channel with clamping
        auto blendChannel = [factor](BYTE c1, BYTE c2) -> BYTE {
            int result = static_cast<int>(c1) + static_cast<int>((c2 - c1) * factor);
            if (result < 0) return 0;
            if (result > 255) return 255;
            return static_cast<BYTE>(result);
        };
        
        return RGB(blendChannel(r1, r2), blendChannel(g1, g2), blendChannel(b1, b2));
    }
    
    /**
     * @brief Create a hover effect by lightening a color
     * @param color Base color
     * @return Lightened color suitable for hover state
     */
    static COLORREF GetHoverColor(COLORREF color) {
        return BlendColors(color, RGB(255, 255, 255), 0.15f);
    }
    
    /**
     * @brief Create a pressed effect by darkening a color
     * @param color Base color
     * @return Darkened color suitable for pressed state
     */
    static COLORREF GetPressedColor(COLORREF color) {
        return BlendColors(color, RGB(0, 0, 0), 0.2f);
    }
};

} // namespace fresh

#endif // _WIN32
