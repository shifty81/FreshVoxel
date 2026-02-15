#include "ui/NativeMainMenu.h"

#include <filesystem>
#include <sstream>
#include <cstring>
#include <vector>

#include "core/Logger.h"

#include <dwmapi.h>

#pragma comment(lib, "dwmapi.lib")

namespace fs = std::filesystem;

namespace fresh
{

#ifdef _WIN32
// Helper function to convert wide string to narrow string safely
static std::string toNarrowString(const std::wstring& wstr)
{
    if (wstr.empty()) return std::string();
    
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), nullptr, 0, nullptr, nullptr);
    if (size_needed <= 0) return std::string();
    
    std::string strTo(size_needed, 0);
    int result = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), &strTo[0], size_needed, nullptr, nullptr);
    if (result <= 0) return std::string();
    
    return strTo;
}

/**
 * @brief Creates an in-memory dialog template for use with DialogBoxIndirectParam
 * 
 * This function creates a minimal dialog template structure that can be used
 * to create modal dialogs without resource files.
 * 
 * @param width Dialog width in dialog units
 * @param height Dialog height in dialog units
 * @param title Dialog title (wide string)
 * @return Vector containing the dialog template bytes
 */
static std::vector<BYTE> createDialogTemplate(WORD width, WORD height, const wchar_t* title)
{
    // Calculate template size
    // DLGTEMPLATE + menu (null) + class (null) + title + font size + font name
    size_t titleLen = wcslen(title);
    size_t templateSize = sizeof(DLGTEMPLATE) + 
                          sizeof(WORD) * 2 +  // menu and class (both null = 0)
                          (titleLen + 1) * sizeof(wchar_t);  // title including null
    
    // Align to DWORD boundary
    templateSize = (templateSize + 3) & ~3;
    
    std::vector<BYTE> buffer(templateSize, 0);
    
    // Fill in the DLGTEMPLATE structure
    DLGTEMPLATE* dlgTemplate = reinterpret_cast<DLGTEMPLATE*>(buffer.data());
    dlgTemplate->style = WS_POPUP | WS_VISIBLE | WS_CAPTION | WS_SYSMENU | DS_MODALFRAME | DS_CENTER;
    dlgTemplate->dwExtendedStyle = 0;
    dlgTemplate->cdit = 0;  // No controls defined in template (we create them dynamically)
    dlgTemplate->x = 0;
    dlgTemplate->y = 0;
    dlgTemplate->cx = width;
    dlgTemplate->cy = height;
    
    // After DLGTEMPLATE comes:
    // 1. Menu (sz_Or_Ord) - 0 for no menu
    // 2. Class (sz_Or_Ord) - 0 for default dialog class
    // 3. Title (sz_Or_Ord) - the dialog title
    WORD* pMenu = reinterpret_cast<WORD*>(dlgTemplate + 1);
    *pMenu++ = 0;  // No menu
    *pMenu++ = 0;  // Default class
    
    // Copy title safely with bounds checking
    wchar_t* pTitle = reinterpret_cast<wchar_t*>(pMenu);
    
    // Calculate how much space remains in the buffer for the title
    BYTE* titleStart = reinterpret_cast<BYTE*>(pTitle);
    size_t bytesUsed = titleStart - buffer.data();
    size_t remainingBytes = buffer.size() - bytesUsed;
    size_t maxTitleChars = remainingBytes / sizeof(wchar_t);
    
    // Verify title fits in buffer (title length + 1 for null terminator)
    if (titleLen + 1 > maxTitleChars) {
        // Buffer too small - should not happen with correct size calculation
        return buffer;
    }
    
    // Use wcscpy_s for safe string copy
    wcscpy_s(pTitle, maxTitleChars, title);
    
    return buffer;
}
#endif


NativeMainMenu::NativeMainMenu()
    : m_parentWindow(nullptr),
      m_mainDialog(nullptr),
      m_createNewWorld(false),
      m_loadWorld(false),
      m_isWorld3D(true),
      m_world2DStyle(WorldStyle2D::Platformer),
      m_worldSize(WorldSize::Medium),
      m_terrainType(TerrainType::Hills),
      m_biomeType(BiomeType::Mixed),
      m_worldSeed(0)
{
}

NativeMainMenu::~NativeMainMenu() {}

bool NativeMainMenu::initialize(HWND parentWindow)
{
    m_parentWindow = parentWindow;
    LOG_INFO_C("Initializing Native Main Menu", "NativeMainMenu");
    scanWorldSaves();
    return true;
}

bool NativeMainMenu::show()
{
    // Create in-memory dialog template for the main menu
    // Size in dialog units (approximately 1.5x pixels for width, 1.75x for height)
    auto dialogTemplate = createDialogTemplate(400, 230, L"Fresh Voxel Engine");
    
    // Create and show modal dialog using the in-memory template
    INT_PTR result =
        DialogBoxIndirectParam(GetModuleHandle(nullptr),
                               reinterpret_cast<LPCDLGTEMPLATE>(dialogTemplate.data()),
                               m_parentWindow, mainMenuDialogProc, reinterpret_cast<LPARAM>(this));

    return result == IDOK;
}

bool NativeMainMenu::showCreateWorldDialog()
{
    // Reset state
    m_createNewWorld = false;
    
    // Show create project dialog directly using in-memory template
    auto dialogTemplate = createDialogTemplate(370, 290, L"Create New Project");
    INT_PTR result = DialogBoxIndirectParam(GetModuleHandle(nullptr), 
                                    reinterpret_cast<LPCDLGTEMPLATE>(dialogTemplate.data()), 
                                    m_parentWindow, createWorldDialogProc, reinterpret_cast<LPARAM>(this));

    if (result == IDOK) {
        m_createNewWorld = true;
        return true;
    }
    return false;
}

bool NativeMainMenu::showLoadWorldDialog()
{
    // Reset state and rescan for saved worlds
    m_loadWorld = false;
    scanWorldSaves();
    
    // Show load world dialog directly using in-memory template
    auto dialogTemplate = createDialogTemplate(340, 260, L"Load World");
    INT_PTR result = DialogBoxIndirectParam(GetModuleHandle(nullptr), 
                                    reinterpret_cast<LPCDLGTEMPLATE>(dialogTemplate.data()), 
                                    m_parentWindow, loadWorldDialogProc, reinterpret_cast<LPARAM>(this));

    if (result == IDOK) {
        m_loadWorld = true;
        return true;
    }
    return false;
}

void NativeMainMenu::applyDarkTheme(HWND hwnd)
{
    // Enable Windows 11 dark mode
    BOOL useDarkMode = TRUE;
    DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &useDarkMode, sizeof(useDarkMode));

    // Set window background to dark
    SetClassLongPtr(hwnd, GCLP_HBRBACKGROUND, (LONG_PTR)CreateSolidBrush(UnrealColors::WindowBg));
}

void NativeMainMenu::applyDarkThemeToControl(HWND control)
{
    // Set control colors for dark theme
    // This is a simplified version - full implementation would need custom drawing
    SetClassLongPtr(control, GCLP_HBRBACKGROUND, (LONG_PTR)CreateSolidBrush(UnrealColors::InputBg));
}

void NativeMainMenu::scanWorldSaves()
{
    m_worldSaves.clear();

    const std::string savesDir = "saves";

    if (!fs::exists(savesDir)) {
        LOG_INFO_C("Saves directory does not exist, creating it", "NativeMainMenu");
        try {
            fs::create_directory(savesDir);
        } catch (const std::exception& e) {
            LOG_ERROR_C("Failed to create saves directory: " + std::string(e.what()),
                        "NativeMainMenu");
            return;
        }
    }

    try {
        for (const auto& entry : fs::directory_iterator(savesDir)) {
            if (entry.is_regular_file() && entry.path().extension() == ".world") {
                std::wstring filename = entry.path().stem().wstring();
                m_worldSaves.push_back(filename);
            }
        }
    } catch (const std::exception& e) {
        LOG_ERROR_C("Failed to scan saves directory: " + std::string(e.what()), "NativeMainMenu");
    }

    LOG_INFO_C("Found " + std::to_string(m_worldSaves.size()) + " saved worlds", "NativeMainMenu");
}

INT_PTR CALLBACK NativeMainMenu::mainMenuDialogProc(HWND hwnd, UINT msg, WPARAM wParam,
                                                    LPARAM lParam)
{
    NativeMainMenu* menu = nullptr;

    if (msg == WM_INITDIALOG) {
        // Store pointer to our class instance
        SetWindowLongPtr(hwnd, GWLP_USERDATA, lParam);
        menu = reinterpret_cast<NativeMainMenu*>(lParam);
        menu->m_mainDialog = hwnd;
    } else {
        menu = reinterpret_cast<NativeMainMenu*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }

    if (!menu)
        return FALSE;

    switch (msg) {
    case WM_INITDIALOG: {
        // Center dialog on screen
        RECT rc;
        GetWindowRect(hwnd, &rc);
        int width = rc.right - rc.left;
        int height = rc.bottom - rc.top;
        int screenWidth = GetSystemMetrics(SM_CXSCREEN);
        int screenHeight = GetSystemMetrics(SM_CYSCREEN);
        SetWindowPos(hwnd, nullptr, (screenWidth - width) / 2, (screenHeight - height) / 2, width,
                     height, SWP_NOZORDER);

        // Apply dark theme
        menu->applyDarkTheme(hwnd);

        // Set window title
        SetWindowText(hwnd, L"Fresh Voxel Engine");

        // Create controls
        menu->createMainMenuDialog(hwnd);

        return TRUE;
    }

    case WM_COMMAND: {
        switch (LOWORD(wParam)) {
        case ID_BTN_CREATE_WORLD: {
            // Show create project dialog using in-memory template
            auto dialogTemplate = createDialogTemplate(370, 290, L"Create New Project");
            INT_PTR result = DialogBoxIndirectParam(GetModuleHandle(nullptr), 
                                            reinterpret_cast<LPCDLGTEMPLATE>(dialogTemplate.data()), 
                                            hwnd, createWorldDialogProc, reinterpret_cast<LPARAM>(menu));

            if (result == IDOK) {
                menu->m_createNewWorld = true;
                EndDialog(hwnd, IDOK);
            }
            return TRUE;
        }

        case ID_BTN_LOAD_WORLD: {
            // Show load world dialog using in-memory template
            auto dialogTemplate = createDialogTemplate(340, 260, L"Load World");
            INT_PTR result = DialogBoxIndirectParam(GetModuleHandle(nullptr), 
                                            reinterpret_cast<LPCDLGTEMPLATE>(dialogTemplate.data()), 
                                            hwnd, loadWorldDialogProc, reinterpret_cast<LPARAM>(menu));

            if (result == IDOK) {
                menu->m_loadWorld = true;
                EndDialog(hwnd, IDOK);
            }
            return TRUE;
        }

        case ID_BTN_EXIT:
            EndDialog(hwnd, IDCANCEL);
            return TRUE;
        }
        break;
    }

    case WM_CLOSE:
        EndDialog(hwnd, IDCANCEL);
        return TRUE;

    case WM_CTLCOLORDLG:
    case WM_CTLCOLORSTATIC:
        // Make static text and dialog background dark
        SetBkColor((HDC)wParam, UnrealColors::WindowBg);
        SetTextColor((HDC)wParam, UnrealColors::TextNormal);
        return (INT_PTR)CreateSolidBrush(UnrealColors::WindowBg);
    }

    return FALSE;
}

void NativeMainMenu::createMainMenuDialog(HWND hwnd)
{
    // Set dialog size
    SetWindowPos(hwnd, nullptr, 0, 0, 600, 400, SWP_NOMOVE | SWP_NOZORDER);

    // Title
    HWND title = CreateWindowEx(0, L"STATIC", L"FRESH VOXEL ENGINE",
                                WS_CHILD | WS_VISIBLE | SS_CENTER, 150, 30, 300, 40, hwnd,
                                (HMENU)ID_STATIC_TITLE, GetModuleHandle(nullptr), nullptr);

    HFONT titleFont = CreateFontW(28, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                 OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                 DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    SendMessage(title, WM_SETFONT, (WPARAM)titleFont, TRUE);

    // Buttons
    int buttonY = 120;
    int buttonWidth = 300;
    int buttonHeight = 50;
    int spacing = 20;
    int centerX = (600 - buttonWidth) / 2;

    // Create New Project button
    CreateWindowEx(0, L"BUTTON", L"Create New Project", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                   centerX, buttonY, buttonWidth, buttonHeight, hwnd, (HMENU)ID_BTN_CREATE_WORLD,
                   GetModuleHandle(nullptr), nullptr);

    buttonY += buttonHeight + spacing;

    // Load Project button
    CreateWindowEx(0, L"BUTTON", L"Load Existing Project", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                   centerX, buttonY, buttonWidth, buttonHeight, hwnd, (HMENU)ID_BTN_LOAD_WORLD,
                   GetModuleHandle(nullptr), nullptr);

    buttonY += buttonHeight + spacing;

    // Exit button
    CreateWindowEx(0, L"BUTTON", L"Exit", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, centerX, buttonY,
                   buttonWidth, buttonHeight, hwnd, (HMENU)ID_BTN_EXIT, GetModuleHandle(nullptr),
                   nullptr);

    // Version info at bottom
    CreateWindowEx(0, L"STATIC", L"Version 0.1.0 - C++20 Voxel Engine",
                   WS_CHILD | WS_VISIBLE | SS_CENTER, 100, 350, 400, 20, hwnd, nullptr,
                   GetModuleHandle(nullptr), nullptr);
}

INT_PTR CALLBACK NativeMainMenu::createWorldDialogProc(HWND hwnd, UINT msg, WPARAM wParam,
                                                       LPARAM lParam)
{
    NativeMainMenu* menu = nullptr;

    if (msg == WM_INITDIALOG) {
        SetWindowLongPtr(hwnd, GWLP_USERDATA, lParam);
        menu = reinterpret_cast<NativeMainMenu*>(lParam);
    } else {
        menu = reinterpret_cast<NativeMainMenu*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }

    if (!menu)
        return FALSE;

    switch (msg) {
    case WM_INITDIALOG: {
        // Center on parent
        RECT rc;
        GetWindowRect(hwnd, &rc);
        int width = rc.right - rc.left;
        int height = rc.bottom - rc.top;
        int screenWidth = GetSystemMetrics(SM_CXSCREEN);
        int screenHeight = GetSystemMetrics(SM_CYSCREEN);
        SetWindowPos(hwnd, nullptr, (screenWidth - width) / 2, (screenHeight - height) / 2, width,
                     height, SWP_NOZORDER);

        menu->applyDarkTheme(hwnd);
        SetWindowText(hwnd, L"Create New Project");
        menu->createWorldCreationDialog(hwnd);
        return TRUE;
    }

    case WM_COMMAND: {
        switch (LOWORD(wParam)) {
        case ID_BTN_CREATE: {
            // Get world name
            wchar_t worldName[256];
            GetDlgItemText(hwnd, ID_EDIT_WORLD_NAME, worldName, 256);
            menu->m_newWorldName = worldName;

            // Get seed
            wchar_t seedText[32];
            GetDlgItemText(hwnd, ID_EDIT_SEED, seedText, 32);
            if (wcslen(seedText) > 0) {
                menu->m_worldSeed = _wtoi(seedText);
            } else {
                menu->m_worldSeed = static_cast<int>(time(nullptr));
            }

            // Get world type
            menu->m_isWorld3D = (IsDlgButtonChecked(hwnd, ID_RADIO_3D) == BST_CHECKED);
            
            // Get 2D style if 2D world is selected
            if (!menu->m_isWorld3D) {
                bool isPlatformer = (IsDlgButtonChecked(hwnd, ID_RADIO_2D_PLATFORMER) == BST_CHECKED);
                menu->m_world2DStyle = isPlatformer ? WorldStyle2D::Platformer : WorldStyle2D::TopDown;
            }

            // Get world size
            HWND sizeCombo = GetDlgItem(hwnd, ID_COMBO_WORLD_SIZE);
            int sizeIndex = (int)SendMessage(sizeCombo, CB_GETCURSEL, 0, 0);
            menu->m_worldSize = static_cast<WorldSize>(sizeIndex);

            // Get terrain type
            HWND terrainCombo = GetDlgItem(hwnd, ID_COMBO_TERRAIN_TYPE);
            int terrainIndex = (int)SendMessage(terrainCombo, CB_GETCURSEL, 0, 0);
            menu->m_terrainType = static_cast<TerrainType>(terrainIndex);

            // Get biome type
            HWND biomeCombo = GetDlgItem(hwnd, ID_COMBO_BIOME_TYPE);
            int biomeIndex = (int)SendMessage(biomeCombo, CB_GETCURSEL, 0, 0);
            menu->m_biomeType = static_cast<BiomeType>(biomeIndex);

            LOG_INFO_C("Creating new world: " + toNarrowString(menu->m_newWorldName),
                       "NativeMainMenu");

            EndDialog(hwnd, IDOK);
            return TRUE;
        }

        case ID_BTN_CANCEL:
            EndDialog(hwnd, IDCANCEL);
            return TRUE;
        }
        break;
    }

    case WM_CLOSE:
        EndDialog(hwnd, IDCANCEL);
        return TRUE;

    case WM_CTLCOLORDLG:
    case WM_CTLCOLORSTATIC:
    case WM_CTLCOLOREDIT:
        SetBkColor((HDC)wParam, UnrealColors::WindowBg);
        SetTextColor((HDC)wParam, UnrealColors::TextNormal);
        return (INT_PTR)CreateSolidBrush(UnrealColors::WindowBg);
    }

    return FALSE;
}

void NativeMainMenu::createWorldCreationDialog(HWND hwnd)
{
    // Make dialog larger to accommodate new controls
    SetWindowPos(hwnd, nullptr, 0, 0, 550, 650, SWP_NOMOVE | SWP_NOZORDER);

    int y = 20;
    int labelX = 20;
    int controlX = 150;
    int controlWidth = 360;

    // World Name label
    CreateWindowEx(0, L"STATIC", L"Project Name:", WS_CHILD | WS_VISIBLE, labelX, y, 120, 20, hwnd,
                   nullptr, GetModuleHandle(nullptr), nullptr);

    // Project Name input
    CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"New Project", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
                   controlX, y, controlWidth, 24, hwnd, (HMENU)ID_EDIT_WORLD_NAME,
                   GetModuleHandle(nullptr), nullptr);

    y += 40;

    // World Seed label
    CreateWindowEx(0, L"STATIC", L"World Seed:", WS_CHILD | WS_VISIBLE, labelX, y, 120, 20, hwnd,
                   nullptr, GetModuleHandle(nullptr), nullptr);

    // World Seed input
    CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"",
                   WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_NUMBER, controlX, y, controlWidth,
                   24, hwnd, (HMENU)ID_EDIT_SEED, GetModuleHandle(nullptr), nullptr);

    y += 30;

    CreateWindowEx(0, L"STATIC", L"(Leave empty for random)", WS_CHILD | WS_VISIBLE, controlX, y,
                   300, 20, hwnd, nullptr, GetModuleHandle(nullptr), nullptr);

    y += 40;

    // World Type label
    CreateWindowEx(0, L"STATIC", L"World Type:", WS_CHILD | WS_VISIBLE, labelX, y, 120, 20, hwnd,
                   nullptr, GetModuleHandle(nullptr), nullptr);

    y += 25;

    // 3D World radio button
    CreateWindowEx(0, L"BUTTON", L"3D World (Full Voxel)",
                   WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | WS_GROUP, controlX, y, 300, 20,
                   hwnd, (HMENU)ID_RADIO_3D, GetModuleHandle(nullptr), nullptr);
    CheckDlgButton(hwnd, ID_RADIO_3D, BST_CHECKED);

    y += 30;

    // 2D World radio button
    CreateWindowEx(0, L"BUTTON", L"2D World", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON, controlX,
                   y, 300, 20, hwnd, (HMENU)ID_RADIO_2D, GetModuleHandle(nullptr), nullptr);

    y += 40;

    // 2D Style label (indented)
    CreateWindowEx(0, L"STATIC", L"2D World Style:", WS_CHILD | WS_VISIBLE, controlX + 20, y, 120, 20, hwnd,
                   (HMENU)ID_STATIC_2D_STYLE, GetModuleHandle(nullptr), nullptr);

    y += 25;

    // 2D Platformer radio button (start of 2D style group)
    CreateWindowEx(0, L"BUTTON", L"Platformer / Terraria-style (Side View)",
                   WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | WS_GROUP, controlX + 40, y, 300, 20,
                   hwnd, (HMENU)ID_RADIO_2D_PLATFORMER, GetModuleHandle(nullptr), nullptr);
    CheckDlgButton(hwnd, ID_RADIO_2D_PLATFORMER, BST_CHECKED);

    y += 25;

    // 2D Top-down radio button (in same group as platformer)
    CreateWindowEx(0, L"BUTTON", L"Top-down / Zelda-style",
                   WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON, controlX + 40, y, 300, 20,
                   hwnd, (HMENU)ID_RADIO_2D_TOPDOWN, GetModuleHandle(nullptr), nullptr);

    y += 40;

    // World Size label
    CreateWindowEx(0, L"STATIC", L"World Size:", WS_CHILD | WS_VISIBLE, labelX, y, 120, 20, hwnd,
                   nullptr, GetModuleHandle(nullptr), nullptr);

    // World Size combo box
    HWND sizeCombo = CreateWindowEx(WS_EX_CLIENTEDGE, L"COMBOBOX", nullptr,
                   WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
                   controlX, y, controlWidth, 200, hwnd, (HMENU)ID_COMBO_WORLD_SIZE,
                   GetModuleHandle(nullptr), nullptr);
    SendMessage(sizeCombo, CB_ADDSTRING, 0, (LPARAM)L"Small (8 chunk radius)");
    SendMessage(sizeCombo, CB_ADDSTRING, 0, (LPARAM)L"Medium (16 chunk radius)");
    SendMessage(sizeCombo, CB_ADDSTRING, 0, (LPARAM)L"Large (24 chunk radius)");
    SendMessage(sizeCombo, CB_ADDSTRING, 0, (LPARAM)L"Huge (32 chunk radius)");
    SendMessage(sizeCombo, CB_SETCURSEL, 1, 0); // Default to Medium

    y += 35;

    // Terrain Type label
    CreateWindowEx(0, L"STATIC", L"Terrain Type:", WS_CHILD | WS_VISIBLE, labelX, y, 120, 20, hwnd,
                   nullptr, GetModuleHandle(nullptr), nullptr);

    // Terrain Type combo box
    HWND terrainCombo = CreateWindowEx(WS_EX_CLIENTEDGE, L"COMBOBOX", nullptr,
                   WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
                   controlX, y, controlWidth, 200, hwnd, (HMENU)ID_COMBO_TERRAIN_TYPE,
                   GetModuleHandle(nullptr), nullptr);
    SendMessage(terrainCombo, CB_ADDSTRING, 0, (LPARAM)L"Flat");
    SendMessage(terrainCombo, CB_ADDSTRING, 0, (LPARAM)L"Hills");
    SendMessage(terrainCombo, CB_ADDSTRING, 0, (LPARAM)L"Mountains");
    SendMessage(terrainCombo, CB_ADDSTRING, 0, (LPARAM)L"Islands");
    SendMessage(terrainCombo, CB_SETCURSEL, 1, 0); // Default to Hills

    y += 35;

    // Biome Type label
    CreateWindowEx(0, L"STATIC", L"Biome:", WS_CHILD | WS_VISIBLE, labelX, y, 120, 20, hwnd,
                   nullptr, GetModuleHandle(nullptr), nullptr);

    // Biome Type combo box
    HWND biomeCombo = CreateWindowEx(WS_EX_CLIENTEDGE, L"COMBOBOX", nullptr,
                   WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
                   controlX, y, controlWidth, 200, hwnd, (HMENU)ID_COMBO_BIOME_TYPE,
                   GetModuleHandle(nullptr), nullptr);
    SendMessage(biomeCombo, CB_ADDSTRING, 0, (LPARAM)L"Forest");
    SendMessage(biomeCombo, CB_ADDSTRING, 0, (LPARAM)L"Desert");
    SendMessage(biomeCombo, CB_ADDSTRING, 0, (LPARAM)L"Snow");
    SendMessage(biomeCombo, CB_ADDSTRING, 0, (LPARAM)L"Jungle");
    SendMessage(biomeCombo, CB_ADDSTRING, 0, (LPARAM)L"Mixed");
    SendMessage(biomeCombo, CB_SETCURSEL, 4, 0); // Default to Mixed

    y = 570;

    // Buttons
    int buttonWidth = 150;
    int buttonHeight = 40;
    int centerX = (550 - (buttonWidth * 2 + 20)) / 2;

    CreateWindowEx(0, L"BUTTON", L"Create",
                   WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_DEFPUSHBUTTON, centerX, y,
                   buttonWidth, buttonHeight, hwnd, (HMENU)ID_BTN_CREATE, GetModuleHandle(nullptr),
                   nullptr);

    CreateWindowEx(0, L"BUTTON", L"Cancel", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                   centerX + buttonWidth + 20, y, buttonWidth, buttonHeight, hwnd,
                   (HMENU)ID_BTN_CANCEL, GetModuleHandle(nullptr), nullptr);
}

INT_PTR CALLBACK NativeMainMenu::loadWorldDialogProc(HWND hwnd, UINT msg, WPARAM wParam,
                                                     LPARAM lParam)
{
    NativeMainMenu* menu = nullptr;

    if (msg == WM_INITDIALOG) {
        SetWindowLongPtr(hwnd, GWLP_USERDATA, lParam);
        menu = reinterpret_cast<NativeMainMenu*>(lParam);
    } else {
        menu = reinterpret_cast<NativeMainMenu*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }

    if (!menu)
        return FALSE;

    switch (msg) {
    case WM_INITDIALOG: {
        RECT rc;
        GetWindowRect(hwnd, &rc);
        int width = rc.right - rc.left;
        int height = rc.bottom - rc.top;
        int screenWidth = GetSystemMetrics(SM_CXSCREEN);
        int screenHeight = GetSystemMetrics(SM_CYSCREEN);
        SetWindowPos(hwnd, nullptr, (screenWidth - width) / 2, (screenHeight - height) / 2, width,
                     height, SWP_NOZORDER);

        menu->applyDarkTheme(hwnd);
        SetWindowText(hwnd, L"Load World");
        menu->createWorldLoadingDialog(hwnd);
        return TRUE;
    }

    case WM_COMMAND: {
        switch (LOWORD(wParam)) {
        case ID_BTN_LOAD: {
            // Get selected world
            HWND listBox = GetDlgItem(hwnd, ID_LIST_WORLDS);
            int sel = (int)SendMessage(listBox, LB_GETCURSEL, 0, 0);

            if (sel != LB_ERR) {
                wchar_t worldName[256];
                SendMessage(listBox, LB_GETTEXT, sel, (LPARAM)worldName);
                menu->m_loadWorldName = worldName;

                LOG_INFO_C("Loading world: " + toNarrowString(menu->m_loadWorldName),
                           "NativeMainMenu");

                EndDialog(hwnd, IDOK);
            }
            return TRUE;
        }

        case ID_BTN_CANCEL:
            EndDialog(hwnd, IDCANCEL);
            return TRUE;
        }
        break;
    }

    case WM_CLOSE:
        EndDialog(hwnd, IDCANCEL);
        return TRUE;

    case WM_CTLCOLORDLG:
    case WM_CTLCOLORSTATIC:
    case WM_CTLCOLORLISTBOX:
        SetBkColor((HDC)wParam, UnrealColors::WindowBg);
        SetTextColor((HDC)wParam, UnrealColors::TextNormal);
        return (INT_PTR)CreateSolidBrush(UnrealColors::WindowBg);
    }

    return FALSE;
}

void NativeMainMenu::createWorldLoadingDialog(HWND hwnd)
{
    SetWindowPos(hwnd, nullptr, 0, 0, 500, 400, SWP_NOMOVE | SWP_NOZORDER);

    // Title
    CreateWindowEx(0, L"STATIC", L"Select a world to load:", WS_CHILD | WS_VISIBLE, 20, 20, 460, 20,
                   hwnd, nullptr, GetModuleHandle(nullptr), nullptr);

    // List box for worlds
    HWND listBox =
        CreateWindowEx(WS_EX_CLIENTEDGE, L"LISTBOX", nullptr,
                       WS_CHILD | WS_VISIBLE | LBS_NOTIFY | WS_VSCROLL | WS_BORDER, 20, 50, 460,
                       250, hwnd, (HMENU)ID_LIST_WORLDS, GetModuleHandle(nullptr), nullptr);

    // Populate list
    for (const auto& world : m_worldSaves) {
        SendMessage(listBox, LB_ADDSTRING, 0, (LPARAM)world.c_str());
    }

    if (m_worldSaves.empty()) {
        SendMessage(listBox, LB_ADDSTRING, 0, (LPARAM)L"No saved worlds found");
        EnableWindow(GetDlgItem(hwnd, ID_BTN_LOAD), FALSE);
    } else {
        SendMessage(listBox, LB_SETCURSEL, 0, 0);
    }

    // Buttons
    int buttonY = 320;
    int buttonWidth = 150;
    int buttonHeight = 40;
    int centerX = (500 - (buttonWidth * 2 + 20)) / 2;

    CreateWindowEx(0, L"BUTTON", L"Load", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_DEFPUSHBUTTON,
                   centerX, buttonY, buttonWidth, buttonHeight, hwnd, (HMENU)ID_BTN_LOAD,
                   GetModuleHandle(nullptr), nullptr);

    CreateWindowEx(0, L"BUTTON", L"Cancel", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                   centerX + buttonWidth + 20, buttonY, buttonWidth, buttonHeight, hwnd,
                   (HMENU)ID_BTN_CANCEL, GetModuleHandle(nullptr), nullptr);
}

} // namespace fresh
