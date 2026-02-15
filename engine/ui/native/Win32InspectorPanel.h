#pragma once

#ifdef _WIN32

// Prevent Windows.h from defining min/max macros
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>
#include <commctrl.h>
#include <string>
#include <memory>
#include <vector>
#include <functional>
#include "ui/native/Win32Panel.h"

namespace fresh
{

struct HierarchyNode;

namespace ecs
{
class EntityManager;
}

/**
 * @brief Native Win32 Inspector Panel using property grid
 * 
 * Provides a professional native Windows inspector interface with:
 * - Property grid for editing object properties
 * - Transform section (position, rotation, scale)
 * - Component sections (collapsible)
 * - Material and physics properties
 * - Native Windows controls (edit boxes, checkboxes, sliders)
 * - Unreal Engine-like dark theme styling
 */
class Win32InspectorPanel : public Win32Panel
{
public:
    Win32InspectorPanel();
    virtual ~Win32InspectorPanel();

    /**
     * @brief Initialize the inspector panel (simplified interface)
     * @param entityManager Optional entity manager for component editing
     * @return true if successful
     */
    bool initialize(ecs::EntityManager* entityManager = nullptr);

    /**
     * @brief Initialize the inspector panel
     * @param parent Parent window handle
     * @param x X position
     * @param y Y position
     * @param width Panel width
     * @param height Panel height
     * @param entityManager Optional entity manager for component editing
     * @return true if successful
     */
    bool create(HWND parent, int x, int y, int width, int height, ecs::EntityManager* entityManager = nullptr);

    /**
     * @brief Set the object to inspect
     * @param node Scene node to inspect
     */
    void setInspectedNode(HierarchyNode* node);

    /**
     * @brief Get the currently inspected node
     * @return Current node or nullptr
     */
    HierarchyNode* getInspectedNode() const { return m_inspectedNode; }

    /**
     * @brief Refresh the display with current property values
     */
    void refresh();

protected:
    // Override base class method
    bool handleMessage(UINT msg, WPARAM wParam, LPARAM lParam, LRESULT& result) override;

private:
    void createPropertyControls();
    void updatePropertyValues();
    void clearProperties();
    
    void addSectionHeader(const std::wstring& text, int& yPos);
    void addProperty(const std::wstring& label, const std::wstring& value, int& yPos, HWND& controlOut);
    void addCheckbox(const std::wstring& label, bool checked, int& yPos, HWND& controlOut);
    void addEditBox(const std::wstring& label, const std::wstring& value, int& yPos, HWND& controlOut);
    void addFloatEdit(const std::wstring& label, float value, int& yPos, HWND& controlOut);
    
    void renderTransformSection();
    void renderPhysicsSection();
    void renderRendererSection();
    void renderMaterialSection();
    void renderComponentSection();

private:
    HierarchyNode* m_inspectedNode;
    ecs::EntityManager* m_entityManager;
    
    // Control handles
    HWND m_scrollWindow;
    HWND m_noSelectionLabel;
    
    // Transform controls
    HWND m_visibleCheckbox;
    HWND m_posXEdit, m_posYEdit, m_posZEdit;
    HWND m_rotXEdit, m_rotYEdit, m_rotZEdit;
    HWND m_scaleXEdit, m_scaleYEdit, m_scaleZEdit;
    
    // Layout constants
    static constexpr int SECTION_HEIGHT = 25;
    static constexpr int PROPERTY_HEIGHT = 25;
    static constexpr int LABEL_WIDTH = 120;
    static constexpr int EDIT_WIDTH = 180;
    static constexpr int PADDING = 10;
    
    int m_scrollPos;
    int m_contentHeight;
};

} // namespace fresh

#endif // _WIN32
