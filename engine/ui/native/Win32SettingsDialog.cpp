#ifdef _WIN32

#include "ui/native/Win32SettingsDialog.h"
#include "ui/native/UnrealStyleTheme.h"
#include "core/Logger.h"
#include <commctrl.h>
#include <sstream>
#include <iomanip>

// Dialog resource ID (would normally be defined in a resource file)
#define IDD_SETTINGS 101

namespace fresh
{

// Available resolutions
const Win32SettingsDialog::Resolution Win32SettingsDialog::RESOLUTIONS[] = {
    { 1280, 720, L"1280 x 720 (HD)" },
    { 1600, 900, L"1600 x 900 (HD+)" },
    { 1920, 1080, L"1920 x 1080 (Full HD)" },
    { 2560, 1440, L"2560 x 1440 (2K)" },
    { 3840, 2160, L"3840 x 2160 (4K)" }
};

const int Win32SettingsDialog::RESOLUTION_COUNT = sizeof(RESOLUTIONS) / sizeof(RESOLUTIONS[0]);

Win32SettingsDialog::Win32SettingsDialog()
    : m_hwnd(nullptr),
      m_applied(false),
      m_dialogRunning(false),
      m_dialogResult(0),
      m_tabControl(nullptr),
      m_mouseSensSlider(nullptr),
      m_mouseSensLabel(nullptr),
      m_invertYCheck(nullptr),
      m_fovSlider(nullptr),
      m_fovLabel(nullptr),
      m_resolutionCombo(nullptr),
      m_vsyncCheck(nullptr),
      m_fpsLimitEdit(nullptr),
      m_fullscreenCheck(nullptr),
      m_masterVolSlider(nullptr),
      m_masterVolLabel(nullptr),
      m_musicVolSlider(nullptr),
      m_musicVolLabel(nullptr),
      m_sfxVolSlider(nullptr),
      m_sfxVolLabel(nullptr),
      m_renderDistSlider(nullptr),
      m_renderDistLabel(nullptr)
{
}

Win32SettingsDialog::~Win32SettingsDialog()
{
}

bool Win32SettingsDialog::show(HWND parentHwnd, const Settings& currentSettings)
{
    m_settings = currentSettings;
    m_originalSettings = currentSettings;
    m_applied = false;
    
    // Register window class if not already registered
    static bool classRegistered = false;
    if (!classRegistered) {
        WNDCLASSEXW wc = {};
        wc.cbSize = sizeof(WNDCLASSEXW);
        wc.lpfnWndProc = WindowProc;
        wc.hInstance = GetModuleHandle(nullptr);
        wc.lpszClassName = L"FreshSettingsDialog";
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wc.hbrBackground = CreateSolidBrush(UnrealStyleTheme::PanelBackground);
        wc.style = CS_HREDRAW | CS_VREDRAW;
        
        if (!RegisterClassExW(&wc)) {
            LOG_ERROR_C("Failed to register settings dialog window class", "Win32SettingsDialog");
            return false;
        }
        classRegistered = true;
    }
    
    // Create the dialog window
    m_hwnd = CreateWindowExW(
        WS_EX_DLGMODALFRAME | WS_EX_WINDOWEDGE,
        L"FreshSettingsDialog",
        L"Settings",
        WS_CAPTION | WS_SYSMENU | WS_VISIBLE | WS_POPUP,
        CW_USEDEFAULT, CW_USEDEFAULT, 600, 500,
        parentHwnd,
        nullptr,
        GetModuleHandle(nullptr),
        this // Pass this pointer to WM_CREATE
    );
    
    if (!m_hwnd) {
        LOG_ERROR_C("Failed to create settings dialog window", "Win32SettingsDialog");
        return false;
    }
    
    // Center the dialog
    centerDialog(parentHwnd);
    
    // Initialize dialog
    onInitDialog(m_hwnd);
    
    // Disable parent window to make this modal
    EnableWindow(parentHwnd, FALSE);
    
    // Message loop for modal dialog
    MSG msg = {};
    m_dialogResult = 0;
    m_dialogRunning = true;
    
    while (m_dialogRunning && GetMessageW(&msg, nullptr, 0, 0)) {
        if (!IsDialogMessageW(m_hwnd, &msg)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }
    
    // Re-enable parent window
    EnableWindow(parentHwnd, TRUE);
    SetForegroundWindow(parentHwnd);
    
    // Clean up
    if (m_hwnd) {
        DestroyWindow(m_hwnd);
        m_hwnd = nullptr;
    }
    
    LOG_INFO_C("Settings dialog closed", "Win32SettingsDialog");
    return m_dialogResult == IDOK || m_applied;
}

LRESULT CALLBACK Win32SettingsDialog::WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    Win32SettingsDialog* dialog = nullptr;
    
    if (msg == WM_CREATE) {
        CREATESTRUCT* cs = (CREATESTRUCT*)lParam;
        dialog = (Win32SettingsDialog*)cs->lpCreateParams;
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)dialog);
    } else {
        dialog = (Win32SettingsDialog*)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
    }
    
    if (dialog) {
        return dialog->handleWindowMessage(hwnd, msg, wParam, lParam);
    }
    
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

LRESULT Win32SettingsDialog::handleWindowMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
        case WM_CREATE:
            return 0;
            
        case WM_COMMAND:
            onCommand(hwnd, wParam);
            return 0;
            
        case WM_NOTIFY:
            {
                LPNMHDR pnmh = (LPNMHDR)lParam;
                if (pnmh->code == TCN_SELCHANGE && pnmh->hwndFrom == m_tabControl) {
                    int selectedTab = TabCtrl_GetCurSel(m_tabControl);
                    showTab(selectedTab);
                }
            }
            return 0;
            
        case WM_HSCROLL:
            onHScroll(hwnd, wParam, lParam);
            return 0;
            
        case WM_CLOSE:
            onCancel(hwnd);
            return 0;
            
        case WM_DESTROY:
            m_dialogRunning = false;
            return 0;
            
        case WM_CTLCOLORSTATIC:
        case WM_CTLCOLORBTN:
        case WM_CTLCOLOREDIT:
            {
                HDC hdc = (HDC)wParam;
                SetTextColor(hdc, UnrealStyleTheme::TextPrimary);
                SetBkColor(hdc, UnrealStyleTheme::PanelBackground);
                return (LRESULT)GetStockObject(NULL_BRUSH);
            }
    }
    
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

void Win32SettingsDialog::onInitDialog(HWND hwnd)
{
    m_hwnd = hwnd;
    
    // Apply theme
    UnrealStyleTheme::ApplyToWindow(hwnd);
    
    // Create tab control
    m_tabControl = CreateWindowExW(
        0, WC_TABCONTROLW, L"",
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
        10, 10, 570, 400,
        hwnd, (HMENU)IDC_TAB_CONTROL, GetModuleHandle(nullptr), nullptr
    );
    
    // Add tabs
    TCITEMW tie = {};
    tie.mask = TCIF_TEXT;
    
    tie.pszText = (LPWSTR)L"Controls";
    TabCtrl_InsertItem(m_tabControl, 0, &tie);
    
    tie.pszText = (LPWSTR)L"Video";
    TabCtrl_InsertItem(m_tabControl, 1, &tie);
    
    tie.pszText = (LPWSTR)L"Audio";
    TabCtrl_InsertItem(m_tabControl, 2, &tie);
    
    tie.pszText = (LPWSTR)L"Gameplay";
    TabCtrl_InsertItem(m_tabControl, 3, &tie);
    
    // Create controls for all tabs
    createControlsTab(hwnd);
    createVideoTab(hwnd);
    createAudioTab(hwnd);
    createGameplayTab(hwnd);
    
    // Create buttons
    createButtons(hwnd);
    
    // Load current settings into controls
    loadSettings(hwnd);
    
    // Show first tab
    showTab(0);
}

void Win32SettingsDialog::loadSettings(HWND hwnd)
{
    (void)hwnd; // Use member variables instead
    
    // Controls tab
    SendMessageW(m_mouseSensSlider, TBM_SETPOS, TRUE, (LPARAM)(m_settings.mouseSensitivity * 100));
    updateSliderLabel(m_mouseSensSlider, m_mouseSensLabel, 0.0f, 1.0f);
    
    SendMessageW(m_invertYCheck, BM_SETCHECK, m_settings.invertMouseY ? BST_CHECKED : BST_UNCHECKED, 0);
    
    SendMessageW(m_fovSlider, TBM_SETPOS, TRUE, (LPARAM)m_settings.fov);
    std::wstringstream fovSs;
    fovSs << (int)m_settings.fov;
    SetWindowTextW(m_fovLabel, fovSs.str().c_str());
    
    // Video tab
    initializeResolutionList(hwnd);
    SendMessageW(m_vsyncCheck, BM_SETCHECK, m_settings.vsync ? BST_CHECKED : BST_UNCHECKED, 0);
    
    std::wstringstream fpsSs;
    fpsSs << m_settings.fpsLimit;
    SetWindowTextW(m_fpsLimitEdit, fpsSs.str().c_str());
    
    SendMessageW(m_fullscreenCheck, BM_SETCHECK, m_settings.fullscreen ? BST_CHECKED : BST_UNCHECKED, 0);
    
    // Audio tab
    SendMessageW(m_masterVolSlider, TBM_SETPOS, TRUE, (LPARAM)(m_settings.masterVolume * 100));
    std::wstringstream masterSs;
    masterSs << (int)(m_settings.masterVolume * 100) << L"%";
    SetWindowTextW(m_masterVolLabel, masterSs.str().c_str());
    
    SendMessageW(m_musicVolSlider, TBM_SETPOS, TRUE, (LPARAM)(m_settings.musicVolume * 100));
    std::wstringstream musicSs;
    musicSs << (int)(m_settings.musicVolume * 100) << L"%";
    SetWindowTextW(m_musicVolLabel, musicSs.str().c_str());
    
    SendMessageW(m_sfxVolSlider, TBM_SETPOS, TRUE, (LPARAM)(m_settings.sfxVolume * 100));
    std::wstringstream sfxSs;
    sfxSs << (int)(m_settings.sfxVolume * 100) << L"%";
    SetWindowTextW(m_sfxVolLabel, sfxSs.str().c_str());
    
    // Gameplay tab
    SendMessageW(m_renderDistSlider, TBM_SETPOS, TRUE, (LPARAM)m_settings.renderDistance);
    std::wstringstream distSs;
    distSs << (int)m_settings.renderDistance << L"m";
    SetWindowTextW(m_renderDistLabel, distSs.str().c_str());
}

void Win32SettingsDialog::saveSettings(HWND hwnd)
{
    (void)hwnd; // Use member variables instead
    
    // Controls tab
    m_settings.mouseSensitivity = (float)SendMessageW(m_mouseSensSlider, TBM_GETPOS, 0, 0) / 100.0f;
    m_settings.invertMouseY = (SendMessageW(m_invertYCheck, BM_GETCHECK, 0, 0) == BST_CHECKED);
    m_settings.fov = (float)SendMessageW(m_fovSlider, TBM_GETPOS, 0, 0);
    
    // Video tab
    m_settings.resolutionIndex = (int)SendMessageW(m_resolutionCombo, CB_GETCURSEL, 0, 0);
    m_settings.vsync = (SendMessageW(m_vsyncCheck, BM_GETCHECK, 0, 0) == BST_CHECKED);
    
    wchar_t fpsText[32] = {};
    GetWindowTextW(m_fpsLimitEdit, fpsText, 32);
    m_settings.fpsLimit = _wtoi(fpsText);
    
    m_settings.fullscreen = (SendMessageW(m_fullscreenCheck, BM_GETCHECK, 0, 0) == BST_CHECKED);
    
    // Audio tab
    m_settings.masterVolume = (float)SendMessageW(m_masterVolSlider, TBM_GETPOS, 0, 0) / 100.0f;
    m_settings.musicVolume = (float)SendMessageW(m_musicVolSlider, TBM_GETPOS, 0, 0) / 100.0f;
    m_settings.sfxVolume = (float)SendMessageW(m_sfxVolSlider, TBM_GETPOS, 0, 0) / 100.0f;
    
    // Gameplay tab
    m_settings.renderDistance = (float)SendMessageW(m_renderDistSlider, TBM_GETPOS, 0, 0);
}

void Win32SettingsDialog::onCommand(HWND hwnd, WPARAM wParam)
{
    (void)hwnd;
    int cmdId = LOWORD(wParam);
    
    switch (cmdId) {
        case IDOK:
            onOK(m_hwnd);
            break;
        case IDCANCEL:
            onCancel(m_hwnd);
            break;
        case IDAPPLY:
            onApply(m_hwnd);
            break;
    }
}

void Win32SettingsDialog::onHScroll(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    (void)hwnd;
    (void)wParam;
    
    HWND hSlider = (HWND)lParam;
    
    // Update labels for sliders
    if (hSlider == m_mouseSensSlider) {
        updateSliderLabel(m_mouseSensSlider, m_mouseSensLabel, 0.0f, 1.0f);
    } else if (hSlider == m_fovSlider) {
        int fov = (int)SendMessageW(m_fovSlider, TBM_GETPOS, 0, 0);
        std::wstringstream ss;
        ss << fov;
        SetWindowTextW(m_fovLabel, ss.str().c_str());
    } else if (hSlider == m_masterVolSlider) {
        int vol = (int)SendMessageW(m_masterVolSlider, TBM_GETPOS, 0, 0);
        std::wstringstream ss;
        ss << vol << L"%";
        SetWindowTextW(m_masterVolLabel, ss.str().c_str());
    } else if (hSlider == m_musicVolSlider) {
        int vol = (int)SendMessageW(m_musicVolSlider, TBM_GETPOS, 0, 0);
        std::wstringstream ss;
        ss << vol << L"%";
        SetWindowTextW(m_musicVolLabel, ss.str().c_str());
    } else if (hSlider == m_sfxVolSlider) {
        int vol = (int)SendMessageW(m_sfxVolSlider, TBM_GETPOS, 0, 0);
        std::wstringstream ss;
        ss << vol << L"%";
        SetWindowTextW(m_sfxVolLabel, ss.str().c_str());
    } else if (hSlider == m_renderDistSlider) {
        int dist = (int)SendMessageW(m_renderDistSlider, TBM_GETPOS, 0, 0);
        std::wstringstream ss;
        ss << dist << L"m";
        SetWindowTextW(m_renderDistLabel, ss.str().c_str());
    }
}

void Win32SettingsDialog::onApply(HWND hwnd)
{
    (void)hwnd;
    saveSettings(m_hwnd);
    m_applied = true;
    
    if (m_applyCallback) {
        m_applyCallback(m_settings);
    }
    
    LOG_INFO_C("Settings applied", "Win32SettingsDialog");
}

void Win32SettingsDialog::onOK(HWND hwnd)
{
    (void)hwnd;
    saveSettings(m_hwnd);
    m_applied = true;
    
    if (m_applyCallback) {
        m_applyCallback(m_settings);
    }
    
    m_dialogResult = IDOK;
    m_dialogRunning = false;
}

void Win32SettingsDialog::onCancel(HWND hwnd)
{
    (void)hwnd;
    // Revert to original settings if not applied
    if (!m_applied) {
        m_settings = m_originalSettings;
    }
    
    m_dialogResult = IDCANCEL;
    m_dialogRunning = false;
}

void Win32SettingsDialog::updateSliderLabel(HWND slider, HWND label, float min, float max)
{
    int pos = (int)SendMessageW(slider, TBM_GETPOS, 0, 0);
    float value = min + (max - min) * (pos / 100.0f);
    
    std::wstringstream ss;
    ss << std::fixed << std::setprecision(2) << value;
    SetWindowTextW(label, ss.str().c_str());
}

void Win32SettingsDialog::initializeResolutionList(HWND hwnd)
{
    HWND combo = GetDlgItem(hwnd, IDC_RESOLUTION_COMBO);
    if (!combo) {
        return;
    }
    
    for (int i = 0; i < RESOLUTION_COUNT; ++i) {
        SendMessageW(combo, CB_ADDSTRING, 0, (LPARAM)RESOLUTIONS[i].name.c_str());
    }
    
    SendMessageW(combo, CB_SETCURSEL, m_settings.resolutionIndex, 0);
}

void Win32SettingsDialog::centerDialog(HWND parentHwnd)
{
    RECT rcParent, rcDialog;
    if (parentHwnd) {
        GetWindowRect(parentHwnd, &rcParent);
    } else {
        // Center on screen
        rcParent.left = 0;
        rcParent.top = 0;
        rcParent.right = GetSystemMetrics(SM_CXSCREEN);
        rcParent.bottom = GetSystemMetrics(SM_CYSCREEN);
    }
    
    GetWindowRect(m_hwnd, &rcDialog);
    
    int width = rcDialog.right - rcDialog.left;
    int height = rcDialog.bottom - rcDialog.top;
    int x = rcParent.left + (rcParent.right - rcParent.left - width) / 2;
    int y = rcParent.top + (rcParent.bottom - rcParent.top - height) / 2;
    
    SetWindowPos(m_hwnd, nullptr, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}

void Win32SettingsDialog::createControlsTab(HWND hwnd)
{
    HINSTANCE hInst = GetModuleHandle(nullptr);
    int y = 50;
    int x = 30;
    
    // Mouse Sensitivity
    CreateWindowW(L"STATIC", L"Mouse Sensitivity:", WS_CHILD, 
                  x, y, 150, 20, hwnd, (HMENU)1010, hInst, nullptr);
    
    m_mouseSensSlider = CreateWindowW(TRACKBAR_CLASSW, L"", 
        WS_CHILD | WS_VISIBLE | TBS_HORZ | TBS_AUTOTICKS,
        x + 160, y, 200, 30, hwnd, (HMENU)IDC_MOUSE_SENSITIVITY_SLIDER, hInst, nullptr);
    SendMessageW(m_mouseSensSlider, TBM_SETRANGE, TRUE, MAKELPARAM(0, 100));
    
    m_mouseSensLabel = CreateWindowW(L"STATIC", L"0.50", WS_CHILD | WS_VISIBLE,
                                     x + 370, y + 5, 50, 20, hwnd, (HMENU)IDC_MOUSE_SENSITIVITY_LABEL, hInst, nullptr);
    
    y += 40;
    
    // Invert Y
    m_invertYCheck = CreateWindowW(L"BUTTON", L"Invert Mouse Y-Axis", 
        WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
        x, y, 200, 25, hwnd, (HMENU)IDC_INVERT_Y_CHECK, hInst, nullptr);
    
    y += 40;
    
    // FOV
    CreateWindowW(L"STATIC", L"Field of View:", WS_CHILD | WS_VISIBLE,
                  x, y, 150, 20, hwnd, (HMENU)1011, hInst, nullptr);
    
    m_fovSlider = CreateWindowW(TRACKBAR_CLASSW, L"",
        WS_CHILD | WS_VISIBLE | TBS_HORZ | TBS_AUTOTICKS,
        x + 160, y, 200, 30, hwnd, (HMENU)IDC_FOV_SLIDER, hInst, nullptr);
    SendMessageW(m_fovSlider, TBM_SETRANGE, TRUE, MAKELPARAM(60, 120));
    
    m_fovLabel = CreateWindowW(L"STATIC", L"75", WS_CHILD | WS_VISIBLE,
                               x + 370, y + 5, 50, 20, hwnd, (HMENU)IDC_FOV_LABEL, hInst, nullptr);
}

void Win32SettingsDialog::createVideoTab(HWND hwnd)
{
    HINSTANCE hInst = GetModuleHandle(nullptr);
    int y = 50;
    int x = 30;
    
    // Resolution
    CreateWindowW(L"STATIC", L"Resolution:", WS_CHILD,
                  x, y, 150, 20, hwnd, (HMENU)2010, hInst, nullptr);
    
    m_resolutionCombo = CreateWindowW(WC_COMBOBOXW, L"",
        WS_CHILD | CBS_DROPDOWNLIST | CBS_HASSTRINGS,
        x + 160, y, 250, 200, hwnd, (HMENU)IDC_RESOLUTION_COMBO, hInst, nullptr);
    
    y += 40;
    
    // VSync
    m_vsyncCheck = CreateWindowW(L"BUTTON", L"Enable VSync",
        WS_CHILD | BS_AUTOCHECKBOX,
        x, y, 200, 25, hwnd, (HMENU)IDC_VSYNC_CHECK, hInst, nullptr);
    
    y += 40;
    
    // FPS Limit
    CreateWindowW(L"STATIC", L"FPS Limit:", WS_CHILD,
                  x, y, 150, 20, hwnd, (HMENU)2011, hInst, nullptr);
    
    m_fpsLimitEdit = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"60",
        WS_CHILD | ES_NUMBER,
        x + 160, y, 100, 22, hwnd, (HMENU)IDC_FPS_LIMIT_EDIT, hInst, nullptr);
    
    y += 40;
    
    // Fullscreen
    m_fullscreenCheck = CreateWindowW(L"BUTTON", L"Fullscreen",
        WS_CHILD | BS_AUTOCHECKBOX,
        x, y, 200, 25, hwnd, (HMENU)IDC_FULLSCREEN_CHECK, hInst, nullptr);
}

void Win32SettingsDialog::createAudioTab(HWND hwnd)
{
    HINSTANCE hInst = GetModuleHandle(nullptr);
    int y = 50;
    int x = 30;
    
    // Master Volume
    CreateWindowW(L"STATIC", L"Master Volume:", WS_CHILD,
                  x, y, 150, 20, hwnd, (HMENU)3010, hInst, nullptr);
    
    m_masterVolSlider = CreateWindowW(TRACKBAR_CLASSW, L"",
        WS_CHILD | TBS_HORZ | TBS_AUTOTICKS,
        x + 160, y, 200, 30, hwnd, (HMENU)IDC_MASTER_VOLUME_SLIDER, hInst, nullptr);
    SendMessageW(m_masterVolSlider, TBM_SETRANGE, TRUE, MAKELPARAM(0, 100));
    
    m_masterVolLabel = CreateWindowW(L"STATIC", L"100%", WS_CHILD,
                                     x + 370, y + 5, 50, 20, hwnd, (HMENU)IDC_MASTER_VOLUME_LABEL, hInst, nullptr);
    
    y += 40;
    
    // Music Volume
    CreateWindowW(L"STATIC", L"Music Volume:", WS_CHILD,
                  x, y, 150, 20, hwnd, (HMENU)3011, hInst, nullptr);
    
    m_musicVolSlider = CreateWindowW(TRACKBAR_CLASSW, L"",
        WS_CHILD | TBS_HORZ | TBS_AUTOTICKS,
        x + 160, y, 200, 30, hwnd, (HMENU)IDC_MUSIC_VOLUME_SLIDER, hInst, nullptr);
    SendMessageW(m_musicVolSlider, TBM_SETRANGE, TRUE, MAKELPARAM(0, 100));
    
    m_musicVolLabel = CreateWindowW(L"STATIC", L"70%", WS_CHILD,
                                    x + 370, y + 5, 50, 20, hwnd, (HMENU)IDC_MUSIC_VOLUME_LABEL, hInst, nullptr);
    
    y += 40;
    
    // SFX Volume
    CreateWindowW(L"STATIC", L"SFX Volume:", WS_CHILD,
                  x, y, 150, 20, hwnd, (HMENU)3012, hInst, nullptr);
    
    m_sfxVolSlider = CreateWindowW(TRACKBAR_CLASSW, L"",
        WS_CHILD | TBS_HORZ | TBS_AUTOTICKS,
        x + 160, y, 200, 30, hwnd, (HMENU)IDC_SFX_VOLUME_SLIDER, hInst, nullptr);
    SendMessageW(m_sfxVolSlider, TBM_SETRANGE, TRUE, MAKELPARAM(0, 100));
    
    m_sfxVolLabel = CreateWindowW(L"STATIC", L"80%", WS_CHILD,
                                  x + 370, y + 5, 50, 20, hwnd, (HMENU)IDC_SFX_VOLUME_LABEL, hInst, nullptr);
}

void Win32SettingsDialog::createGameplayTab(HWND hwnd)
{
    HINSTANCE hInst = GetModuleHandle(nullptr);
    int y = 50;
    int x = 30;
    
    // Render Distance
    CreateWindowW(L"STATIC", L"Render Distance:", WS_CHILD,
                  x, y, 150, 20, hwnd, (HMENU)4010, hInst, nullptr);
    
    m_renderDistSlider = CreateWindowW(TRACKBAR_CLASSW, L"",
        WS_CHILD | TBS_HORZ | TBS_AUTOTICKS,
        x + 160, y, 200, 30, hwnd, (HMENU)IDC_RENDER_DISTANCE_SLIDER, hInst, nullptr);
    SendMessageW(m_renderDistSlider, TBM_SETRANGE, TRUE, MAKELPARAM(50, 300));
    
    m_renderDistLabel = CreateWindowW(L"STATIC", L"150m", WS_CHILD,
                                      x + 370, y + 5, 60, 20, hwnd, (HMENU)IDC_RENDER_DISTANCE_LABEL, hInst, nullptr);
}

void Win32SettingsDialog::createButtons(HWND hwnd)
{
    HINSTANCE hInst = GetModuleHandle(nullptr);
    int y = 420;
    int x = 290;
    
    // OK button
    CreateWindowW(L"BUTTON", L"OK", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
                  x, y, 80, 28, hwnd, (HMENU)IDOK, hInst, nullptr);
    
    // Apply button
    CreateWindowW(L"BUTTON", L"Apply", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                  x + 90, y, 80, 28, hwnd, (HMENU)IDAPPLY, hInst, nullptr);
    
    // Cancel button
    CreateWindowW(L"BUTTON", L"Cancel", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                  x + 180, y, 80, 28, hwnd, (HMENU)IDCANCEL, hInst, nullptr);
}

void Win32SettingsDialog::showTab(int tabIndex)
{
    // Hide all tab-specific controls (controls are created as WS_CHILD but initially hidden)
    // We need to enumerate and hide/show controls programmatically or track them
    
    // For simplicity, hide all controls on all tabs
    HWND hChild = GetWindow(m_hwnd, GW_CHILD);
    while (hChild) {
        // Don't hide tab control or buttons
        if (hChild != m_tabControl) {
            int ctrlId = GetDlgCtrlID(hChild);
            if (ctrlId < 5000) { // Not a button (OK/Apply/Cancel)
                ShowWindow(hChild, SW_HIDE);
            }
        }
        hChild = GetWindow(hChild, GW_HWNDNEXT);
    }
    
    // Show controls for selected tab based on ID ranges
    hChild = GetWindow(m_hwnd, GW_CHILD);
    while (hChild) {
        int ctrlId = GetDlgCtrlID(hChild);
        bool shouldShow = false;
        
        switch (tabIndex) {
            case 0: // Controls tab (1000-1999)
                shouldShow = (ctrlId >= 1000 && ctrlId < 2000);
                break;
            case 1: // Video tab (2000-2999)
                shouldShow = (ctrlId >= 2000 && ctrlId < 3000);
                break;
            case 2: // Audio tab (3000-3999)
                shouldShow = (ctrlId >= 3000 && ctrlId < 4000);
                break;
            case 3: // Gameplay tab (4000-4999)
                shouldShow = (ctrlId >= 4000 && ctrlId < 5000);
                break;
        }
        
        if (shouldShow) {
            ShowWindow(hChild, SW_SHOW);
        }
        
        hChild = GetWindow(hChild, GW_HWNDNEXT);
    }
}

} // namespace fresh

#endif // _WIN32
