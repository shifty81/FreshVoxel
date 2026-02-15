#pragma once

#ifdef _WIN32

// Prevent Windows.h from defining min/max macros
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "ui/native/Win32Panel.h"
#include "editor/TerraformingSystem.h"
#include "ui/native/UnrealStyleTheme.h"
#include <windows.h>
#include <string>
#include <functional>

namespace fresh
{

class WorldEditor;

/**
 * @brief Native Win32 Terraforming Panel
 * 
 * Provides a visual interface for terraforming tools with Unreal Engine styling:
 * - Tool selection (Single Block, Brush, Sphere, Cube, etc.)
 * - Mode selection (Place, Remove, Replace)
 * - Material/voxel type picker
 * - Tool size/radius control
 * - Undo/Redo status display
 * - Professional dark theme integration
 */
class Win32TerraformingPanel : public Win32Panel
{
public:
    Win32TerraformingPanel();
    ~Win32TerraformingPanel() override;

    /**
     * @brief Initialize the terraforming panel
     * @param parent Parent window handle
     * @param worldEditor World editor instance
     * @return true if successful
     */
    bool initialize(HWND parent, WorldEditor* worldEditor);

    /**
     * @brief Update panel display (call when terraforming state changes)
     */
    void update();

protected:
    void onCreate() override;
    void onPaint(HDC hdc) override;
    void onResize(int width, int height) override;
    bool handleMessage(UINT msg, WPARAM wParam, LPARAM lParam, LRESULT& result) override;

private:
    // UI Creation
    void createControls();
    void createToolButtons();
    void createModeButtons();
    void createMaterialPicker();
    void createSizeControl();
    void createUndoRedoDisplay();
    void createShortcutsDisplay();

    // Update functions
    void updateToolSelection();
    void updateModeSelection();
    void updateMaterialSelection();
    void updateSizeDisplay();
    void updateUndoRedoDisplay();

    // Event handlers
    void onToolButtonClick(TerraformTool tool);
    void onModeButtonClick(TerraformMode mode);
    void onMaterialSelect(VoxelType type);
    void onSizeChange(int delta);
    void onUndoClick();
    void onRedoClick();

    // Drawing helpers
    void drawSection(HDC hdc, int& yPos, const wchar_t* title, int contentHeight);
    void drawButton(HDC hdc, RECT rect, const wchar_t* text, bool selected, bool enabled);
    void drawMaterialIcon(HDC hdc, RECT rect, VoxelType type, bool selected);
    std::wstring getToolName(TerraformTool tool) const;
    std::wstring getModeName(TerraformMode mode) const;
    std::wstring getMaterialName(VoxelType type) const;
    COLORREF getVoxelColor(VoxelType type) const;  // Get representative color for voxel type

private:
    WorldEditor* m_worldEditor;
    TerraformingSystem* m_terraformingSystem;

    // UI State
    TerraformTool m_currentTool;
    TerraformMode m_currentMode;
    VoxelType m_currentMaterial;
    int m_currentSize;
    bool m_canUndo;
    bool m_canRedo;

    // Control handles
    HWND m_toolGroup;
    HWND m_modeGroup;
    HWND m_materialGroup;
    HWND m_sizeGroup;
    HWND m_undoRedoGroup;

    // Button handles
    HWND m_toolButtons[10];  // One for each tool type
    HWND m_modeButtons[3];   // Place, Remove, Replace
    HWND m_materialButtons[15]; // Common voxel types
    HWND m_sizeUpButton;
    HWND m_sizeDownButton;
    HWND m_sizeLabel;
    HWND m_undoButton;
    HWND m_redoButton;
    HWND m_tooltipControl;  // Tooltip control for hover hints
    HWND m_shortcutsLabel;   // Label displaying keyboard shortcuts

    // GDI resources for custom drawing
    HBRUSH m_bgBrush;
    HBRUSH m_selectedBrush;
    HBRUSH m_buttonBrush;
    HPEN m_borderPen;
    HFONT m_titleFont;
    HFONT m_textFont;
    HFONT m_shortcutsFont;  // Small font for shortcuts display

    // Layout constants
    static constexpr int MARGIN = 10;
    static constexpr int BUTTON_HEIGHT = 30;
    static constexpr int SECTION_SPACING = 15;
    static constexpr int BUTTON_SPACING = 5;
    static constexpr int NUM_MATERIALS = 10;
    
    // Material type mapping for buttons (matches button indices)
    static const VoxelType s_materialTypes[NUM_MATERIALS];
};

} // namespace fresh

#endif // _WIN32
