#pragma once

#ifdef _WIN32

// Prevent Windows.h from defining min/max macros
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>
#include <string>
#include <memory>
#include <functional>

namespace fresh
{

/**
 * @brief Native Win32 Settings Dialog
 * 
 * Provides a professional Windows property sheet-style settings dialog with:
 * - Mouse sensitivity slider
 * - Resolution dropdown
 * - Video settings (VSync, FPS limit, fullscreen)
 * - Audio settings (master, music, SFX volumes)
 * - Controls settings (FOV, invert Y-axis)
 * - Gameplay settings (render distance)
 * - Apply, OK, and Cancel buttons
 * - Unreal Engine-like dark theme styling
 */
class Win32SettingsDialog
{
public:
    struct Settings {
        // Controls
        float mouseSensitivity = 0.5f;
        bool invertMouseY = false;
        float fov = 75.0f;

        // Video
        int resolutionIndex = 3; // Default to 1920x1080
        bool vsync = true;
        int fpsLimit = 60;
        bool fullscreen = false;

        // Audio
        float masterVolume = 1.0f;
        float musicVolume = 0.7f;
        float sfxVolume = 0.8f;

        // Gameplay
        float renderDistance = 150.0f;
    };

    struct Resolution {
        int width;
        int height;
        std::wstring name;
    };

    using ApplyCallback = std::function<void(const Settings&)>;

    Win32SettingsDialog();
    ~Win32SettingsDialog();

    /**
     * @brief Show the settings dialog
     * @param parentHwnd Parent window handle
     * @param currentSettings Current settings values
     * @return true if user clicked OK/Apply, false if cancelled
     */
    bool show(HWND parentHwnd, const Settings& currentSettings);

    /**
     * @brief Set callback for when settings are applied
     */
    void setApplyCallback(ApplyCallback callback) { m_applyCallback = callback; }

    /**
     * @brief Get the current settings
     */
    const Settings& getSettings() const { return m_settings; }

private:
    // Window procedure
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    
    // Instance message handler
    LRESULT handleWindowMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    // Message handlers
    void onInitDialog(HWND hwnd);
    void onCommand(HWND hwnd, WPARAM wParam);
    void onHScroll(HWND hwnd, WPARAM wParam, LPARAM lParam);
    void onApply(HWND hwnd);
    void onOK(HWND hwnd);
    void onCancel(HWND hwnd);

    // Helper functions
    void centerDialog(HWND parentHwnd);
    void createControlsTab(HWND hwnd);
    void createVideoTab(HWND hwnd);
    void createAudioTab(HWND hwnd);
    void createGameplayTab(HWND hwnd);
    void createButtons(HWND hwnd);
    void showTab(int tabIndex);
    void loadSettings(HWND hwnd);
    void saveSettings(HWND hwnd);
    void updateSliderLabel(HWND slider, HWND label, float min, float max);
    void initializeResolutionList(HWND hwnd);

    // Control IDs
    enum ControlID {
        // Tab control
        IDC_TAB_CONTROL = 1000,
        
        // Controls tab
        IDC_MOUSE_SENSITIVITY_SLIDER = 1001,
        IDC_MOUSE_SENSITIVITY_LABEL = 1002,
        IDC_INVERT_Y_CHECK = 1003,
        IDC_FOV_SLIDER = 1004,
        IDC_FOV_LABEL = 1005,

        // Video tab
        IDC_RESOLUTION_COMBO = 2001,
        IDC_VSYNC_CHECK = 2002,
        IDC_FPS_LIMIT_EDIT = 2003,
        IDC_FULLSCREEN_CHECK = 2004,

        // Audio tab
        IDC_MASTER_VOLUME_SLIDER = 3001,
        IDC_MASTER_VOLUME_LABEL = 3002,
        IDC_MUSIC_VOLUME_SLIDER = 3003,
        IDC_MUSIC_VOLUME_LABEL = 3004,
        IDC_SFX_VOLUME_SLIDER = 3005,
        IDC_SFX_VOLUME_LABEL = 3006,

        // Gameplay tab
        IDC_RENDER_DISTANCE_SLIDER = 4001,
        IDC_RENDER_DISTANCE_LABEL = 4002,

        // Buttons
        IDAPPLY = 5001
    };

    Settings m_settings;
    Settings m_originalSettings;
    ApplyCallback m_applyCallback;
    HWND m_hwnd;
    bool m_applied;
    
    // Dialog state
    bool m_dialogRunning;
    int m_dialogResult;
    
    // Control handles
    HWND m_tabControl;
    
    // Controls tab
    HWND m_mouseSensSlider;
    HWND m_mouseSensLabel;
    HWND m_invertYCheck;
    HWND m_fovSlider;
    HWND m_fovLabel;
    
    // Video tab
    HWND m_resolutionCombo;
    HWND m_vsyncCheck;
    HWND m_fpsLimitEdit;
    HWND m_fullscreenCheck;
    
    // Audio tab
    HWND m_masterVolSlider;
    HWND m_masterVolLabel;
    HWND m_musicVolSlider;
    HWND m_musicVolLabel;
    HWND m_sfxVolSlider;
    HWND m_sfxVolLabel;
    
    // Gameplay tab
    HWND m_renderDistSlider;
    HWND m_renderDistLabel;

    // Available resolutions
    static const Resolution RESOLUTIONS[];
    static const int RESOLUTION_COUNT;
};

} // namespace fresh

#endif // _WIN32
