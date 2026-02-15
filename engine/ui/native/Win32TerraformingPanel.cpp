#ifdef _WIN32

#include "ui/native/Win32TerraformingPanel.h"
#include "editor/WorldEditor.h"
#include "core/Logger.h"
#include <commctrl.h>
#include <sstream>
#include <iomanip>

namespace fresh
{

// Command IDs for buttons
#define CMD_TOOL_SINGLE_BLOCK   2000
#define CMD_TOOL_BRUSH          2001
#define CMD_TOOL_SPHERE         2002
#define CMD_TOOL_FILLED_SPHERE  2003
#define CMD_TOOL_CUBE           2004
#define CMD_TOOL_FILLED_CUBE    2005
#define CMD_TOOL_LINE           2006
#define CMD_TOOL_FLATTEN        2007
#define CMD_TOOL_SMOOTH         2008
#define CMD_TOOL_PAINT          2009

#define CMD_MODE_PLACE          2100
#define CMD_MODE_REMOVE         2101
#define CMD_MODE_REPLACE        2102

#define CMD_SIZE_UP             2200
#define CMD_SIZE_DOWN           2201
#define CMD_UNDO                2202
#define CMD_REDO                2203

#define CMD_MATERIAL_BASE       3000  // Materials start at 3000
#define NUM_MATERIAL_BUTTONS    10    // Number of material buttons to display

// Static material type mapping for button indices
const VoxelType Win32TerraformingPanel::s_materialTypes[Win32TerraformingPanel::NUM_MATERIALS] = {
    VoxelType::Stone, VoxelType::Dirt, VoxelType::Grass, VoxelType::Sand, VoxelType::Water,
    VoxelType::Wood, VoxelType::Leaves, VoxelType::Cobblestone, VoxelType::Planks, VoxelType::Glass
};

Win32TerraformingPanel::Win32TerraformingPanel()
    : Win32Panel()
    , m_worldEditor(nullptr)
    , m_terraformingSystem(nullptr)
    , m_currentTool(TerraformTool::SingleBlock)
    , m_currentMode(TerraformMode::Place)
    , m_currentMaterial(VoxelType::Stone)
    , m_currentSize(1)
    , m_canUndo(false)
    , m_canRedo(false)
    , m_toolGroup(nullptr)
    , m_modeGroup(nullptr)
    , m_materialGroup(nullptr)
    , m_sizeGroup(nullptr)
    , m_undoRedoGroup(nullptr)
    , m_sizeUpButton(nullptr)
    , m_sizeDownButton(nullptr)
    , m_sizeLabel(nullptr)
    , m_undoButton(nullptr)
    , m_redoButton(nullptr)
    , m_tooltipControl(nullptr)
    , m_shortcutsLabel(nullptr)
    , m_bgBrush(nullptr)
    , m_selectedBrush(nullptr)
    , m_buttonBrush(nullptr)
    , m_borderPen(nullptr)
    , m_titleFont(nullptr)
    , m_textFont(nullptr)
    , m_shortcutsFont(nullptr)
{
    for (int i = 0; i < 10; ++i) {
        m_toolButtons[i] = nullptr;
    }
    for (int i = 0; i < 3; ++i) {
        m_modeButtons[i] = nullptr;
    }
    for (int i = 0; i < 15; ++i) {
        m_materialButtons[i] = nullptr;
    }
}

Win32TerraformingPanel::~Win32TerraformingPanel()
{
    // Clean up tooltip control
    if (m_tooltipControl) {
        DestroyWindow(m_tooltipControl);
        m_tooltipControl = nullptr;
    }
    
    // Clean up GDI resources
    if (m_bgBrush) DeleteObject(m_bgBrush);
    if (m_selectedBrush) DeleteObject(m_selectedBrush);
    if (m_buttonBrush) DeleteObject(m_buttonBrush);
    if (m_borderPen) DeleteObject(m_borderPen);
    if (m_titleFont) DeleteObject(m_titleFont);
    if (m_textFont) DeleteObject(m_textFont);
    if (m_shortcutsFont) DeleteObject(m_shortcutsFont);
}

bool Win32TerraformingPanel::initialize(HWND parent, WorldEditor* worldEditor)
{
    if (!worldEditor) {
        LOG_ERROR_C("Invalid world editor", "Win32TerraformingPanel");
        return false;
    }

    m_worldEditor = worldEditor;
    m_terraformingSystem = worldEditor->getTerraformingSystem();

    if (!m_terraformingSystem) {
        LOG_ERROR_C("No terraforming system available", "Win32TerraformingPanel");
        return false;
    }

    // Create the panel window
    if (!create(parent, 10, 100, 250, 600, L"Terraforming Tools")) {
        LOG_ERROR_C("Failed to create terraforming panel window", "Win32TerraformingPanel");
        return false;
    }

    // Create GDI resources
    m_bgBrush = CreateSolidBrush(UnrealStyleTheme::PanelBackground);
    m_selectedBrush = CreateSolidBrush(UnrealStyleTheme::AccentBlue);
    m_buttonBrush = CreateSolidBrush(UnrealStyleTheme::ButtonNormal);
    m_borderPen = CreatePen(PS_SOLID, 1, UnrealStyleTheme::BorderLight);

    // Create fonts
    m_titleFont = CreateFontW(16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                            CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");

    m_textFont = CreateFontW(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                           DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                           CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");

    LOG_INFO_C("Win32TerraformingPanel initialized", "Win32TerraformingPanel");
    return true;
}

void Win32TerraformingPanel::onCreate()
{
    createControls();
    update();
}

void Win32TerraformingPanel::createControls()
{
    // Create tooltip control for hover hints
    m_tooltipControl = CreateWindowExW(
        WS_EX_TOPMOST,
        TOOLTIPS_CLASSW,
        nullptr,
        WS_POPUP | TTS_ALWAYSTIP | TTS_BALLOON,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        m_hwnd,
        nullptr,
        GetModuleHandle(nullptr),
        nullptr
    );
    
    if (m_tooltipControl) {
        // Set tooltip delay times (faster appearance)
        SendMessageW(m_tooltipControl, TTM_SETDELAYTIME, TTDT_AUTOPOP, MAKELONG(10000, 0));  // Display for 10 seconds
        SendMessageW(m_tooltipControl, TTM_SETDELAYTIME, TTDT_INITIAL, MAKELONG(500, 0));     // Show after 0.5 seconds
        SendMessageW(m_tooltipControl, TTM_SETDELAYTIME, TTDT_RESHOW, MAKELONG(200, 0));      // Re-show after 0.2 seconds
        
        // Set max width for multi-line tooltips
        SendMessageW(m_tooltipControl, TTM_SETMAXTIPWIDTH, 0, 300);
    }
    
    // Create tool buttons
    // Removed unused variable: int yPos = MARGIN;
    
    // Tool selection section
    createToolButtons();
    
    // Mode selection section
    createModeButtons();
    
    // Material picker section
    createMaterialPicker();
    
    // Size control section
    createSizeControl();
    
    // Undo/Redo section
    createUndoRedoDisplay();
    
    // Keyboard shortcuts display
    createShortcutsDisplay();
}

void Win32TerraformingPanel::createToolButtons()
{
    // For simplicity, create a few key tool buttons
    // In a full implementation, you'd create all tool buttons with proper icons
    
    const wchar_t* toolNames[] = {
        L"Single Block",
        L"Brush",
        L"Sphere",
        L"Filled Sphere",
        L"Cube",
        L"Filled Cube",
        L"Line",
        L"Flatten",
        L"Smooth",
        L"Paint"
    };
    
    // Tooltip descriptions for each tool
    const wchar_t* toolTooltips[] = {
        L"Single Block - Place or remove one block at a time",
        L"Brush - Paint voxels in a circular area",
        L"Sphere - Create a hollow sphere shape",
        L"Filled Sphere - Create a solid sphere shape",
        L"Cube - Create a hollow cube shape",
        L"Filled Cube - Create a solid cube shape",
        L"Line - Draw a straight line of voxels",
        L"Flatten - Level terrain to create flat surfaces",
        L"Smooth - Blend and smooth terrain irregularities",
        L"Paint - Change voxel types without affecting shape"
    };

    int cmdIds[] = {
        CMD_TOOL_SINGLE_BLOCK,
        CMD_TOOL_BRUSH,
        CMD_TOOL_SPHERE,
        CMD_TOOL_FILLED_SPHERE,
        CMD_TOOL_CUBE,
        CMD_TOOL_FILLED_CUBE,
        CMD_TOOL_LINE,
        CMD_TOOL_FLATTEN,
        CMD_TOOL_SMOOTH,
        CMD_TOOL_PAINT
    };

    int yPos = MARGIN + 30;  // After "Tools" header
    for (int i = 0; i < 10; ++i) {
        m_toolButtons[i] = CreateWindowW(
            L"BUTTON",
            toolNames[i],
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            MARGIN, yPos, m_width - 2 * MARGIN, BUTTON_HEIGHT,
            m_hwnd,
            (HMENU)(LONG_PTR)cmdIds[i],
            GetModuleHandle(nullptr),
            nullptr
        );
        
        if (m_toolButtons[i]) {
            SendMessage(m_toolButtons[i], WM_SETFONT, (WPARAM)m_textFont, TRUE);
            
            // Add tooltip for this tool button
            if (m_tooltipControl) {
                TOOLINFOW ti = { 0 };
                ti.cbSize = sizeof(TOOLINFOW);
                ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
                ti.hwnd = m_hwnd;
                ti.uId = (UINT_PTR)m_toolButtons[i];
                ti.lpszText = const_cast<LPWSTR>(toolTooltips[i]);
                SendMessageW(m_tooltipControl, TTM_ADDTOOLW, 0, (LPARAM)&ti);
            }
        }
        
        yPos += BUTTON_HEIGHT + BUTTON_SPACING;
    }
}

void Win32TerraformingPanel::createModeButtons()
{
    const wchar_t* modeNames[] = { L"Place", L"Remove", L"Replace" };
    const wchar_t* modeTooltips[] = {
        L"Place Mode - Add new voxels to the world",
        L"Remove Mode - Delete voxels from the world",
        L"Replace Mode - Change existing voxels to a new type"
    };
    int cmdIds[] = { CMD_MODE_PLACE, CMD_MODE_REMOVE, CMD_MODE_REPLACE };

    int yPos = MARGIN + 30 + (BUTTON_HEIGHT + BUTTON_SPACING) * 10 + SECTION_SPACING + 30;
    int buttonWidth = (m_width - 3 * MARGIN - 2 * BUTTON_SPACING) / 3;

    for (int i = 0; i < 3; ++i) {
        m_modeButtons[i] = CreateWindowW(
            L"BUTTON",
            modeNames[i],
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            MARGIN + i * (buttonWidth + BUTTON_SPACING), yPos,
            buttonWidth, BUTTON_HEIGHT,
            m_hwnd,
            (HMENU)(LONG_PTR)cmdIds[i],
            GetModuleHandle(nullptr),
            nullptr
        );
        
        if (m_modeButtons[i]) {
            SendMessage(m_modeButtons[i], WM_SETFONT, (WPARAM)m_textFont, TRUE);
            
            // Add tooltip for this mode button
            if (m_tooltipControl) {
                TOOLINFOW ti = { 0 };
                ti.cbSize = sizeof(TOOLINFOW);
                ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
                ti.hwnd = m_hwnd;
                ti.uId = (UINT_PTR)m_modeButtons[i];
                ti.lpszText = const_cast<LPWSTR>(modeTooltips[i]);
                SendMessageW(m_tooltipControl, TTM_ADDTOOLW, 0, (LPARAM)&ti);
            }
        }
    }
}

void Win32TerraformingPanel::createMaterialPicker()
{
    // Create material buttons with color indicators for common voxel types
    const wchar_t* materialNames[NUM_MATERIALS] = {
        L"Stone", L"Dirt", L"Grass", L"Sand", L"Water",
        L"Wood", L"Leaves", L"Cobblestone", L"Planks", L"Glass"
    };
    
    // Tooltip descriptions for each material
    const wchar_t* materialTooltips[NUM_MATERIALS] = {
        L"Stone - Basic building block from underground",
        L"Dirt - Common earth material for terrain",
        L"Grass - Surface terrain with vegetation",
        L"Sand - Desert and beach material",
        L"Water - Liquid block for lakes and oceans",
        L"Wood - Natural log material from trees",
        L"Leaves - Foliage blocks from trees",
        L"Cobblestone - Crafted stone variant",
        L"Planks - Processed wood building material",
        L"Glass - Transparent building material"
    };

    int yPos = MARGIN + 30 + (BUTTON_HEIGHT + BUTTON_SPACING) * 10 + 
               SECTION_SPACING + 30 + BUTTON_HEIGHT + SECTION_SPACING + 30;
    
    // Create buttons with owner-draw style for custom color rendering
    for (int i = 0; i < NUM_MATERIALS; ++i) {
        m_materialButtons[i] = CreateWindowW(
            L"BUTTON",
            materialNames[i],
            WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,  // Use owner-draw for custom rendering
            MARGIN, yPos, m_width - 2 * MARGIN, BUTTON_HEIGHT,
            m_hwnd,
            (HMENU)(LONG_PTR)(CMD_MATERIAL_BASE + i),
            GetModuleHandle(nullptr),
            nullptr
        );
        
        // Add tooltip for this material button
        if (m_materialButtons[i] && m_tooltipControl) {
            TOOLINFOW ti = { 0 };
            ti.cbSize = sizeof(TOOLINFOW);
            ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
            ti.hwnd = m_hwnd;
            ti.uId = (UINT_PTR)m_materialButtons[i];
            ti.lpszText = const_cast<LPWSTR>(materialTooltips[i]);
            SendMessageW(m_tooltipControl, TTM_ADDTOOLW, 0, (LPARAM)&ti);
        }
        
        yPos += BUTTON_HEIGHT + BUTTON_SPACING;
    }
}


void Win32TerraformingPanel::createSizeControl()
{
    int yPos = m_height - 150;

    // Size label
    m_sizeLabel = CreateWindowW(
        L"STATIC",
        L"Size: 1",
        WS_CHILD | WS_VISIBLE | SS_CENTER,
        MARGIN, yPos, m_width - 2 * MARGIN, 25,
        m_hwnd,
        nullptr,
        GetModuleHandle(nullptr),
        nullptr
    );

    if (m_sizeLabel) {
        SendMessage(m_sizeLabel, WM_SETFONT, (WPARAM)m_textFont, TRUE);
    }

    yPos += 30;

    // Size buttons
    int buttonWidth = (m_width - 3 * MARGIN) / 2;
    
    m_sizeDownButton = CreateWindowW(
        L"BUTTON",
        L"-",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        MARGIN, yPos, buttonWidth, BUTTON_HEIGHT,
        m_hwnd,
        (HMENU)CMD_SIZE_DOWN,
        GetModuleHandle(nullptr),
        nullptr
    );

    m_sizeUpButton = CreateWindowW(
        L"BUTTON",
        L"+",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        MARGIN + buttonWidth + BUTTON_SPACING, yPos, buttonWidth, BUTTON_HEIGHT,
        m_hwnd,
        (HMENU)CMD_SIZE_UP,
        GetModuleHandle(nullptr),
        nullptr
    );

    if (m_sizeDownButton) SendMessage(m_sizeDownButton, WM_SETFONT, (WPARAM)m_textFont, TRUE);
    if (m_sizeUpButton) SendMessage(m_sizeUpButton, WM_SETFONT, (WPARAM)m_textFont, TRUE);
}

void Win32TerraformingPanel::createUndoRedoDisplay()
{
    int yPos = m_height - 80;
    int buttonWidth = (m_width - 3 * MARGIN) / 2;

    m_undoButton = CreateWindowW(
        L"BUTTON",
        L"Undo (U)",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        MARGIN, yPos, buttonWidth, BUTTON_HEIGHT,
        m_hwnd,
        (HMENU)CMD_UNDO,
        GetModuleHandle(nullptr),
        nullptr
    );

    m_redoButton = CreateWindowW(
        L"BUTTON",
        L"Redo (R)",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        MARGIN + buttonWidth + BUTTON_SPACING, yPos, buttonWidth, BUTTON_HEIGHT,
        m_hwnd,
        (HMENU)CMD_REDO,
        GetModuleHandle(nullptr),
        nullptr
    );

    if (m_undoButton) SendMessage(m_undoButton, WM_SETFONT, (WPARAM)m_textFont, TRUE);
    if (m_redoButton) SendMessage(m_redoButton, WM_SETFONT, (WPARAM)m_textFont, TRUE);
}

void Win32TerraformingPanel::createShortcutsDisplay()
{
    // Create a multi-line static text control to display keyboard shortcuts
    int yPos = m_height - 35;  // Position at the very bottom
    
    m_shortcutsLabel = CreateWindowW(
        L"STATIC",
        L"Shortcuts: U=Undo | R=Redo | [/]=Size",
        WS_CHILD | WS_VISIBLE | SS_CENTER | SS_SUNKEN,
        MARGIN, yPos, m_width - 2 * MARGIN, 25,
        m_hwnd,
        nullptr,
        GetModuleHandle(nullptr),
        nullptr
    );
    
    if (m_shortcutsLabel) {
        // Create and store a smaller font for the shortcuts display
        m_shortcutsFont = CreateFontW(11, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                     DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                     CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
        SendMessage(m_shortcutsLabel, WM_SETFONT, (WPARAM)m_shortcutsFont, TRUE);
    }
}

void Win32TerraformingPanel::update()
{
    if (!m_terraformingSystem) {
        return;
    }

    // Update state from terraforming system
    m_currentTool = m_terraformingSystem->getTool();
    m_currentMode = m_terraformingSystem->getMode();
    m_currentMaterial = m_terraformingSystem->getVoxelType();
    m_currentSize = m_terraformingSystem->getToolSize();
    m_canUndo = m_terraformingSystem->canUndo();
    m_canRedo = m_terraformingSystem->canRedo();

    updateToolSelection();
    updateModeSelection();
    updateMaterialSelection();
    updateSizeDisplay();
    updateUndoRedoDisplay();

    // Redraw the panel
    InvalidateRect(m_hwnd, nullptr, TRUE);
}

void Win32TerraformingPanel::updateToolSelection()
{
    // Update visual selection state of tool buttons
    // In a full implementation, you'd change button styles or colors
}

void Win32TerraformingPanel::updateModeSelection()
{
    // Update visual selection state of mode buttons
}

void Win32TerraformingPanel::updateMaterialSelection()
{
    // Update visual selection state of material buttons
}

void Win32TerraformingPanel::updateSizeDisplay()
{
    if (m_sizeLabel) {
        std::wstringstream ss;
        ss << L"Size: " << m_currentSize;
        SetWindowTextW(m_sizeLabel, ss.str().c_str());
    }
}

void Win32TerraformingPanel::updateUndoRedoDisplay()
{
    if (m_undoButton) {
        EnableWindow(m_undoButton, m_canUndo ? TRUE : FALSE);
    }
    if (m_redoButton) {
        EnableWindow(m_redoButton, m_canRedo ? TRUE : FALSE);
    }
}

void Win32TerraformingPanel::onPaint(HDC hdc)
{
    RECT clientRect;
    GetClientRect(m_hwnd, &clientRect);

    // Fill background
    FillRect(hdc, &clientRect, m_bgBrush);

    // Set text properties
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, UnrealStyleTheme::TextPrimary);
    SelectObject(hdc, m_titleFont);

    // Draw section headers
    int yPos = MARGIN;

    // Tools header
    RECT headerRect = { MARGIN, yPos, m_width - MARGIN, yPos + 25 };
    DrawTextW(hdc, L"Tools", -1, &headerRect, DT_LEFT | DT_VCENTER);
    yPos += 30 + (BUTTON_HEIGHT + BUTTON_SPACING) * 10 + SECTION_SPACING;

    // Mode header
    headerRect = { MARGIN, yPos, m_width - MARGIN, yPos + 25 };
    DrawTextW(hdc, L"Mode", -1, &headerRect, DT_LEFT | DT_VCENTER);
    yPos += 30 + BUTTON_HEIGHT + SECTION_SPACING;

    // Material header
    headerRect = { MARGIN, yPos, m_width - MARGIN, yPos + 25 };
    DrawTextW(hdc, L"Material", -1, &headerRect, DT_LEFT | DT_VCENTER);
}

void Win32TerraformingPanel::onResize(int width, int height)
{
    (void)width;   // Unused parameter
    (void)height;  // Unused parameter
    // Reposition controls on resize if needed
    // For simplicity, we'll keep controls at their original positions
}

bool Win32TerraformingPanel::handleMessage(UINT msg, WPARAM wParam, LPARAM lParam, LRESULT& result)
{
    if (msg == WM_DRAWITEM) {
        // Handle owner-draw material buttons
        DRAWITEMSTRUCT* dis = reinterpret_cast<DRAWITEMSTRUCT*>(lParam);
        if (dis && dis->CtlType == ODT_BUTTON) {
            int cmdId = static_cast<int>(dis->CtlID);
            if (cmdId >= CMD_MATERIAL_BASE && cmdId < CMD_MATERIAL_BASE + NUM_MATERIALS) {
                // This is a material button - draw it with color indicator
                int materialIndex = cmdId - CMD_MATERIAL_BASE;
                VoxelType type = s_materialTypes[materialIndex]; // Use static material type array
                
                // Get button state
                bool isPressed = (dis->itemState & ODS_SELECTED) != 0;
                bool isFocused = (dis->itemState & ODS_FOCUS) != 0;
                
                HDC hdc = dis->hDC;
                RECT rect = dis->rcItem;
                
                // Draw button background
                COLORREF bgColor = isPressed ? UnrealStyleTheme::ButtonPressed : 
                                  (isFocused ? UnrealStyleTheme::ButtonHover : UnrealStyleTheme::ButtonNormal);
                HBRUSH bgBrush = CreateSolidBrush(bgColor);
                FillRect(hdc, &rect, bgBrush);
                DeleteObject(bgBrush);
                
                // Draw colored square indicator on the left
                RECT colorRect = rect;
                colorRect.left += 5;
                colorRect.top += 5;
                colorRect.right = colorRect.left + 20;
                colorRect.bottom = colorRect.top + 20;
                
                COLORREF voxelColor = getVoxelColor(type);
                HBRUSH colorBrush = CreateSolidBrush(voxelColor);
                FillRect(hdc, &colorRect, colorBrush);
                DeleteObject(colorBrush);
                
                // Draw border around color square
                HPEN borderPen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
                HPEN oldPen = (HPEN)SelectObject(hdc, borderPen);
                MoveToEx(hdc, colorRect.left, colorRect.top, nullptr);
                LineTo(hdc, colorRect.right, colorRect.top);
                LineTo(hdc, colorRect.right, colorRect.bottom);
                LineTo(hdc, colorRect.left, colorRect.bottom);
                LineTo(hdc, colorRect.left, colorRect.top);
                SelectObject(hdc, oldPen);
                DeleteObject(borderPen);
                
                // Draw material name text
                std::wstring materialName = getMaterialName(type);
                RECT textRect = rect;
                textRect.left += 35; // Leave space for color square
                
                SetBkMode(hdc, TRANSPARENT);
                SetTextColor(hdc, UnrealStyleTheme::TextNormal);
                if (m_textFont) {
                    SelectObject(hdc, m_textFont);
                }
                DrawTextW(hdc, materialName.c_str(), -1, &textRect, 
                         DT_LEFT | DT_VCENTER | DT_SINGLELINE);
                
                result = TRUE;
                return true;
            }
        }
    }
    
    if (msg == WM_COMMAND) {
        int cmdId = LOWORD(wParam);

        // Handle tool selection
        if (cmdId >= CMD_TOOL_SINGLE_BLOCK && cmdId <= CMD_TOOL_PAINT) {
            TerraformTool tool = static_cast<TerraformTool>(cmdId - CMD_TOOL_SINGLE_BLOCK);
            onToolButtonClick(tool);
            result = 0;
            return true;
        }

        // Handle mode selection
        if (cmdId >= CMD_MODE_PLACE && cmdId <= CMD_MODE_REPLACE) {
            TerraformMode mode = static_cast<TerraformMode>(cmdId - CMD_MODE_PLACE);
            onModeButtonClick(mode);
            result = 0;
            return true;
        }

        // Handle material selection
        if (cmdId >= CMD_MATERIAL_BASE && cmdId < CMD_MATERIAL_BASE + NUM_MATERIALS) {
            int materialIndex = cmdId - CMD_MATERIAL_BASE;
            VoxelType type = s_materialTypes[materialIndex];  // Use static material type array
            onMaterialSelect(type);
            result = 0;
            return true;
        }

        // Handle size controls
        switch (cmdId) {
            case CMD_SIZE_UP:
                onSizeChange(1);
                result = 0;
                return true;
            case CMD_SIZE_DOWN:
                onSizeChange(-1);
                result = 0;
                return true;
            case CMD_UNDO:
                onUndoClick();
                result = 0;
                return true;
            case CMD_REDO:
                onRedoClick();
                result = 0;
                return true;
        }
    }

    return false;
}

void Win32TerraformingPanel::onToolButtonClick(TerraformTool tool)
{
    if (m_terraformingSystem) {
        m_terraformingSystem->setTool(tool);
        update();
        LOG_INFO_C("Tool changed to: " + std::to_string(static_cast<int>(tool)), "Win32TerraformingPanel");
    }
}

void Win32TerraformingPanel::onModeButtonClick(TerraformMode mode)
{
    if (m_terraformingSystem) {
        m_terraformingSystem->setMode(mode);
        update();
        LOG_INFO_C("Mode changed to: " + std::to_string(static_cast<int>(mode)), "Win32TerraformingPanel");
    }
}

void Win32TerraformingPanel::onMaterialSelect(VoxelType type)
{
    if (m_terraformingSystem) {
        m_terraformingSystem->setVoxelType(type);
        update();
        LOG_INFO_C("Material changed to: " + std::to_string(static_cast<int>(type)), "Win32TerraformingPanel");
    }
}

void Win32TerraformingPanel::onSizeChange(int delta)
{
    if (m_terraformingSystem) {
        int newSize = m_currentSize + delta;
        if (newSize < 1) newSize = 1;
        if (newSize > 20) newSize = 20;  // Max size limit
        
        m_terraformingSystem->setToolSize(newSize);
        update();
        LOG_INFO_C("Tool size changed to: " + std::to_string(newSize), "Win32TerraformingPanel");
    }
}

void Win32TerraformingPanel::onUndoClick()
{
    if (m_terraformingSystem && m_terraformingSystem->canUndo()) {
        m_terraformingSystem->undo();
        update();
        LOG_INFO_C("Undo performed", "Win32TerraformingPanel");
    }
}

void Win32TerraformingPanel::onRedoClick()
{
    if (m_terraformingSystem && m_terraformingSystem->canRedo()) {
        m_terraformingSystem->redo();
        update();
        LOG_INFO_C("Redo performed", "Win32TerraformingPanel");
    }
}

std::wstring Win32TerraformingPanel::getToolName(TerraformTool tool) const
{
    switch (tool) {
        case TerraformTool::SingleBlock: return L"Single Block";
        case TerraformTool::Brush: return L"Brush";
        case TerraformTool::Sphere: return L"Sphere";
        case TerraformTool::FilledSphere: return L"Filled Sphere";
        case TerraformTool::Cube: return L"Cube";
        case TerraformTool::FilledCube: return L"Filled Cube";
        case TerraformTool::Line: return L"Line";
        case TerraformTool::Flatten: return L"Flatten";
        case TerraformTool::Smooth: return L"Smooth";
        case TerraformTool::Paint: return L"Paint";
        default: return L"Unknown";
    }
}

std::wstring Win32TerraformingPanel::getModeName(TerraformMode mode) const
{
    switch (mode) {
        case TerraformMode::Place: return L"Place";
        case TerraformMode::Remove: return L"Remove";
        case TerraformMode::Replace: return L"Replace";
        default: return L"Unknown";
    }
}

std::wstring Win32TerraformingPanel::getMaterialName(VoxelType type) const
{
    switch (type) {
        case VoxelType::Stone: return L"Stone";
        case VoxelType::Dirt: return L"Dirt";
        case VoxelType::Grass: return L"Grass";
        case VoxelType::Sand: return L"Sand";
        case VoxelType::Water: return L"Water";
        case VoxelType::Wood: return L"Wood";
        case VoxelType::Leaves: return L"Leaves";
        case VoxelType::Glass: return L"Glass";
        case VoxelType::Cobblestone: return L"Cobblestone";
        case VoxelType::Planks: return L"Planks";
        case VoxelType::Snow: return L"Snow";
        case VoxelType::Ice: return L"Ice";
        case VoxelType::Obsidian: return L"Obsidian";
        default: return L"Unknown";
    }
}

COLORREF Win32TerraformingPanel::getVoxelColor(VoxelType type) const
{
    // Return representative colors for each voxel type for visual display
    switch (type) {
        case VoxelType::Stone:       return RGB(128, 128, 128);  // Gray
        case VoxelType::Dirt:        return RGB(139, 69, 19);    // Brown
        case VoxelType::Grass:       return RGB(34, 139, 34);    // Green
        case VoxelType::Sand:        return RGB(238, 214, 175);  // Tan
        case VoxelType::Water:       return RGB(65, 105, 225);   // Blue
        case VoxelType::Wood:        return RGB(101, 67, 33);    // Dark brown
        case VoxelType::Leaves:      return RGB(0, 128, 0);      // Dark green
        case VoxelType::Glass:       return RGB(173, 216, 230);  // Light blue (transparent)
        case VoxelType::Cobblestone: return RGB(112, 128, 144);  // Slate gray
        case VoxelType::Planks:      return RGB(205, 133, 63);   // Peru (wood color)
        case VoxelType::Snow:        return RGB(255, 250, 250);  // Snow white
        case VoxelType::Ice:         return RGB(175, 238, 238);  // Pale turquoise
        case VoxelType::Obsidian:    return RGB(25, 25, 25);     // Nearly black
        case VoxelType::Bedrock:     return RGB(64, 64, 64);     // Dark gray
        case VoxelType::Brick:       return RGB(178, 34, 34);    // Fire brick red
        case VoxelType::Gravel:      return RGB(169, 169, 169);  // Dark gray
        case VoxelType::Coal:        return RGB(54, 54, 54);     // Coal black
        case VoxelType::Iron:        return RGB(192, 192, 192);  // Silver
        case VoxelType::Gold:        return RGB(255, 215, 0);    // Gold
        case VoxelType::Diamond:     return RGB(0, 191, 255);    // Deep sky blue
        default:                     return RGB(200, 200, 200);  // Default light gray
    }
}


} // namespace fresh

#endif // _WIN32
