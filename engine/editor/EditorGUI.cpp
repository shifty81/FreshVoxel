#include "editor/EditorGUI.h"

#include <iostream>

#include "core/Logger.h"
#include "editor/TerraformingSystem.h"
#include "editor/WorldEditor.h"

namespace fresh
{

EditorGUI::EditorGUI()
    : m_worldEditor(nullptr), m_visible(false), m_showTerraformingPanel(true), m_showWorldInfo(true)
{
}

EditorGUI::~EditorGUI() {}

bool EditorGUI::initialize(WorldEditor* worldEditor)
{
    if (!worldEditor) {
        LOG_ERROR_C("Invalid world editor", "EditorGUI");
        return false;
    }

    m_worldEditor = worldEditor;

    LOG_INFO_C("EditorGUI initialized", "EditorGUI");
    LOG_INFO_C("Note: EditorGUI is now a legacy wrapper. Native Win32 UI panels are used via EditorManager.", "EditorGUI");
    return true;
}

void EditorGUI::render()
{
    if (!m_visible || !m_worldEditor) {
        return;
    }

    // Legacy EditorGUI is now a lightweight wrapper
    // The actual GUI is handled by native Win32 panels in EditorManager
    // This method is kept for backward compatibility but does minimal work
    
    // If EditorManager's Win32TerraformingPanel is available, it handles all UI
    // Otherwise, provide basic console feedback for debugging
    #ifndef FRESH_WIN32_UI
    // Only show console output if native Win32 UI is not available
    renderTerraformingPanel();
    renderWorldInfoPanel();
    #endif
}

void EditorGUI::renderTerraformingPanel()
{
    if (!m_showTerraformingPanel) {
        return;
    }

    TerraformingSystem* terraform = m_worldEditor->getTerraformingSystem();
    if (!terraform) {
        return;
    }

    // Console-based UI fallback (only used when Win32 UI is not available)
    std::cout << "\n--- Terraforming Tools (Console Mode) ---\n";
    std::cout << "Note: For full GUI, use native Win32 panels via EditorManager\n";
    std::cout << "Current Tool: ";

    switch (terraform->getTool()) {
    case TerraformTool::SingleBlock:
        std::cout << "Single Block\n";
        break;
    case TerraformTool::Brush:
        std::cout << "Brush (Radius: " << terraform->getToolSize() << ")\n";
        break;
    case TerraformTool::Sphere:
        std::cout << "Hollow Sphere (Radius: " << terraform->getToolSize() << ")\n";
        break;
    case TerraformTool::FilledSphere:
        std::cout << "Filled Sphere (Radius: " << terraform->getToolSize() << ")\n";
        break;
    case TerraformTool::Cube:
        std::cout << "Hollow Cube (Size: " << terraform->getToolSize() << ")\n";
        break;
    case TerraformTool::FilledCube:
        std::cout << "Filled Cube (Size: " << terraform->getToolSize() << ")\n";
        break;
    case TerraformTool::Line:
        std::cout << "Line\n";
        break;
    case TerraformTool::Flatten:
        std::cout << "Flatten (Radius: " << terraform->getToolSize() << ")\n";
        break;
    case TerraformTool::Smooth:
        std::cout << "Smooth (Radius: " << terraform->getToolSize() << ")\n";
        break;
    case TerraformTool::Paint:
        std::cout << "Paint (Radius: " << terraform->getToolSize() << ")\n";
        break;
    }

    std::cout << "Current Mode: ";
    switch (terraform->getMode()) {
    case TerraformMode::Place:
        std::cout << "Place\n";
        break;
    case TerraformMode::Remove:
        std::cout << "Remove\n";
        break;
    case TerraformMode::Replace:
        std::cout << "Replace\n";
        break;
    }

    std::cout << "Selected Material: ";
    switch (terraform->getVoxelType()) {
    case VoxelType::Stone:
        std::cout << "Stone\n";
        break;
    case VoxelType::Dirt:
        std::cout << "Dirt\n";
        break;
    case VoxelType::Grass:
        std::cout << "Grass\n";
        break;
    case VoxelType::Sand:
        std::cout << "Sand\n";
        break;
    case VoxelType::Water:
        std::cout << "Water\n";
        break;
    default:
        std::cout << "Unknown\n";
        break;
    }

    std::cout << "Undo available: " << (terraform->canUndo() ? "Yes" : "No") << "\n";
    std::cout << "Redo available: " << (terraform->canRedo() ? "Yes" : "No") << "\n";
}

void EditorGUI::renderWorldInfoPanel()
{
    if (!m_showWorldInfo) {
        return;
    }

    // Console-based UI fallback
    std::cout << "\n--- World Info (Console Mode) ---\n";
    std::cout << "Press 'E' to toggle editor\n";
    std::cout << "Press 'T' to change tool\n";
    std::cout << "Press 'M' to change material\n";
    std::cout << "Press 'U' to undo\n";
    std::cout << "Press 'R' to redo\n";
    std::cout << "Press '+/-' to change tool size\n";
    std::cout << "Note: Use Win32TerraformingPanel for full GUI experience\n";
}

void EditorGUI::renderToolSelector()
{
    // Tool selection UI would go here
    // Now handled by Win32TerraformingPanel
}

void EditorGUI::renderMaterialSelector()
{
    // Material selection UI would go here
    // Now handled by Win32TerraformingPanel
}

} // namespace fresh
