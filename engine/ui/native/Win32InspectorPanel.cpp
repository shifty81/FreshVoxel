#ifdef _WIN32

#include "ui/native/Win32InspectorPanel.h"
#include "ui/native/Win32SceneHierarchyPanel.h"
#include "ui/native/UnrealStyleTheme.h"
#include "ui/SceneHierarchyPanel.h"
#include "ecs/EntityManager.h"
#include "ecs/TransformComponent.h"
#include "ecs/RendererComponent.h"
#include "ecs/MaterialComponent.h"
#include "physics/PhysicsComponent.h"
#include "core/Logger.h"
#include <windowsx.h>
#include <sstream>
#include <iomanip>

namespace fresh
{

Win32InspectorPanel::Win32InspectorPanel()
    : Win32Panel(),
      m_inspectedNode(nullptr),
      m_entityManager(nullptr),
      m_scrollWindow(nullptr),
      m_noSelectionLabel(nullptr),
      m_visibleCheckbox(nullptr),
      m_posXEdit(nullptr), m_posYEdit(nullptr), m_posZEdit(nullptr),
      m_rotXEdit(nullptr), m_rotYEdit(nullptr), m_rotZEdit(nullptr),
      m_scaleXEdit(nullptr), m_scaleYEdit(nullptr), m_scaleZEdit(nullptr),
      m_scrollPos(0),
      m_contentHeight(0)
{
}

Win32InspectorPanel::~Win32InspectorPanel()
{
    // Controls are automatically destroyed with parent window
}

bool Win32InspectorPanel::create(HWND parent, int x, int y, int width, int height, ecs::EntityManager* entityManager)
{
    m_entityManager = entityManager;
    
    if (!Win32Panel::create(parent, x, y, width, height, L"Inspector")) {
        return false;
    }
    
    // Create "no selection" label
    m_noSelectionLabel = CreateWindowExW(
        0, L"STATIC", L"No object selected\n\nSelect an object in the Scene Hierarchy",
        WS_CHILD | WS_VISIBLE | SS_CENTER,
        PADDING, 50, width - 2 * PADDING, 100,
        m_hwnd, nullptr, GetModuleHandle(nullptr), nullptr
    );
    
    // Apply theme
    UnrealStyleTheme::ApplyToWindow(m_noSelectionLabel);
    SendMessageW(m_noSelectionLabel, WM_SETFONT, (WPARAM)UnrealStyleTheme::GetFont(), TRUE);
    
    LOG_INFO_C("Win32InspectorPanel created", "Win32InspectorPanel");
    return true;
}

bool Win32InspectorPanel::initialize(ecs::EntityManager* entityManager)
{
    // For standalone initialization without parent window
    // Store the entity manager reference for later use when create() is called
    m_entityManager = entityManager;
    LOG_INFO_C("Inspector panel initialized (stub)", "Win32InspectorPanel");
    return true;
}

void Win32InspectorPanel::setInspectedNode(HierarchyNode* node)
{
    m_inspectedNode = node;
    refresh();
}

void Win32InspectorPanel::refresh()
{
    if (!m_hwnd) {
        return;
    }
    
    // Clear existing property controls
    clearProperties();
    
    if (!m_inspectedNode) {
        // Show "no selection" message
        ShowWindow(m_noSelectionLabel, SW_SHOW);
        return;
    }
    
    // Hide "no selection" message
    ShowWindow(m_noSelectionLabel, SW_HIDE);
    
    // Create property controls for the inspected node
    createPropertyControls();
    
    // Force redraw
    InvalidateRect(m_hwnd, nullptr, TRUE);
}

void Win32InspectorPanel::createPropertyControls()
{
    if (!m_inspectedNode) {
        return;
    }
    
    RECT clientRect;
    GetClientRect(m_hwnd, &clientRect);
    int width = clientRect.right - clientRect.left;
    int yPos = PADDING;
    
    // Object name header
    std::wstring objectName = L"Object: ";
    objectName += std::wstring(m_inspectedNode->name.begin(), m_inspectedNode->name.end());
    addSectionHeader(objectName, yPos);
    
    // Visibility checkbox
    m_visibleCheckbox = CreateWindowExW(
        0, L"BUTTON", L"Visible",
        WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
        PADDING, yPos, width - 2 * PADDING, 20,
        m_hwnd, (HMENU)1001, GetModuleHandle(nullptr), nullptr
    );
    SendMessageW(m_visibleCheckbox, BM_SETCHECK, m_inspectedNode->visible ? BST_CHECKED : BST_UNCHECKED, 0);
    UnrealStyleTheme::ApplyToWindow(m_visibleCheckbox);
    SendMessageW(m_visibleCheckbox, WM_SETFONT, (WPARAM)UnrealStyleTheme::GetFont(), TRUE);
    yPos += PROPERTY_HEIGHT + 5;
    
    // Transform section
    renderTransformSection();
    
    // Physics section (if physics component exists)
    if (m_entityManager && m_inspectedNode->userData) {
        ecs::Entity* entity = static_cast<ecs::Entity*>(m_inspectedNode->userData);
        if (entity && entity->isValid() && m_entityManager->hasComponent<physics::PhysicsComponent>(*entity)) {
            renderPhysicsSection();
        }
    }
    
    // Renderer section (if renderer component exists)
    if (m_entityManager && m_inspectedNode->userData) {
        ecs::Entity* entity = static_cast<ecs::Entity*>(m_inspectedNode->userData);
        if (entity && entity->isValid() && m_entityManager->hasComponent<ecs::RendererComponent>(*entity)) {
            renderRendererSection();
        }
    }
    
    // Material section (if material component exists)
    if (m_entityManager && m_inspectedNode->userData) {
        ecs::Entity* entity = static_cast<ecs::Entity*>(m_inspectedNode->userData);
        if (entity && entity->isValid() && m_entityManager->hasComponent<ecs::MaterialComponent>(*entity)) {
            renderMaterialSection();
        }
    }
    
    // Components section
    renderComponentSection();
}

void Win32InspectorPanel::updatePropertyValues()
{
    // Update control values from inspected node
    // This would be called when external changes occur
}

void Win32InspectorPanel::clearProperties()
{
    // Destroy all dynamically created controls (except m_noSelectionLabel)
    HWND child = GetWindow(m_hwnd, GW_CHILD);
    while (child) {
        HWND next = GetWindow(child, GW_HWNDNEXT);
        if (child != m_noSelectionLabel) {
            DestroyWindow(child);
        }
        child = next;
    }
    
    // Reset control handles
    m_visibleCheckbox = nullptr;
    m_posXEdit = m_posYEdit = m_posZEdit = nullptr;
    m_rotXEdit = m_rotYEdit = m_rotZEdit = nullptr;
    m_scaleXEdit = m_scaleYEdit = m_scaleZEdit = nullptr;
}

void Win32InspectorPanel::addSectionHeader(const std::wstring& text, int& yPos)
{
    RECT clientRect;
    GetClientRect(m_hwnd, &clientRect);
    int width = clientRect.right - clientRect.left;
    
    HWND header = CreateWindowExW(
        0, L"STATIC", text.c_str(),
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        PADDING, yPos, width - 2 * PADDING, SECTION_HEIGHT,
        m_hwnd, nullptr, GetModuleHandle(nullptr), nullptr
    );
    
    // Use the theme's bold font for headers
    SendMessageW(header, WM_SETFONT, (WPARAM)UnrealStyleTheme::GetBoldFont(), TRUE);
    UnrealStyleTheme::ApplyToWindow(header);
    
    yPos += SECTION_HEIGHT + 5;
}

void Win32InspectorPanel::addFloatEdit(const std::wstring& label, float value, int& yPos, HWND& controlOut)
{
    RECT clientRect;
    GetClientRect(m_hwnd, &clientRect);
    
    // Label
    HWND labelCtrl = CreateWindowExW(
        0, L"STATIC", label.c_str(),
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        PADDING, yPos + 2, LABEL_WIDTH, 20,
        m_hwnd, nullptr, GetModuleHandle(nullptr), nullptr
    );
    UnrealStyleTheme::ApplyToWindow(labelCtrl);
    SendMessageW(labelCtrl, WM_SETFONT, (WPARAM)UnrealStyleTheme::GetFont(), TRUE);
    
    // Edit control
    std::wstringstream ss;
    ss << std::fixed << std::setprecision(2) << value;
    
    controlOut = CreateWindowExW(
        WS_EX_CLIENTEDGE, L"EDIT", ss.str().c_str(),
        WS_CHILD | WS_VISIBLE | ES_LEFT | ES_AUTOHSCROLL,
        LABEL_WIDTH + PADDING + 5, yPos, EDIT_WIDTH, 20,
        m_hwnd, nullptr, GetModuleHandle(nullptr), nullptr
    );
    UnrealStyleTheme::ApplyToWindow(controlOut);
    SendMessageW(controlOut, WM_SETFONT, (WPARAM)UnrealStyleTheme::GetFont(), TRUE);
    
    yPos += PROPERTY_HEIGHT;
}

void Win32InspectorPanel::renderTransformSection()
{
    if (!m_inspectedNode) {
        return;
    }
    
    RECT clientRect;
    GetClientRect(m_hwnd, &clientRect);
    int yPos = 80; // Position after header and visible checkbox
    
    // Transform header
    addSectionHeader(L"Transform", yPos);
    
    // Get transform data from entity's transform component
    glm::vec3 position(0.0f);
    glm::vec3 rotation(0.0f);
    glm::vec3 scale(1.0f);
    
    // Try to get transform component if available
    if (m_entityManager && m_inspectedNode->userData) {
        ecs::Entity* entity = static_cast<ecs::Entity*>(m_inspectedNode->userData);
        if (entity && entity->isValid() && m_entityManager->hasComponent<ecs::TransformComponent>(*entity)) {
            auto* transform = m_entityManager->getComponent<ecs::TransformComponent>(*entity);
            position = transform->position;
            rotation = transform->getEulerAngles();
            scale = transform->scale;
        }
    }
    
    // Position
    addFloatEdit(L"Position X", position.x, yPos, m_posXEdit);
    addFloatEdit(L"Position Y", position.y, yPos, m_posYEdit);
    addFloatEdit(L"Position Z", position.z, yPos, m_posZEdit);
    
    yPos += 10; // Spacer
    
    // Rotation
    addFloatEdit(L"Rotation X", rotation.x, yPos, m_rotXEdit);
    addFloatEdit(L"Rotation Y", rotation.y, yPos, m_rotYEdit);
    addFloatEdit(L"Rotation Z", rotation.z, yPos, m_rotZEdit);
    
    yPos += 10; // Spacer
    
    // Scale
    addFloatEdit(L"Scale X", scale.x, yPos, m_scaleXEdit);
    addFloatEdit(L"Scale Y", scale.y, yPos, m_scaleYEdit);
    addFloatEdit(L"Scale Z", scale.z, yPos, m_scaleZEdit);
}

void Win32InspectorPanel::renderPhysicsSection()
{
    // Placeholder for physics properties
    RECT clientRect;
    GetClientRect(m_hwnd, &clientRect);
    int yPos = 400; // Position after transform section
    
    addSectionHeader(L"Physics", yPos);
    
    // Add physics properties here (mass, velocity, etc.)
}

void Win32InspectorPanel::renderRendererSection()
{
    // Placeholder for renderer properties
}

void Win32InspectorPanel::renderMaterialSection()
{
    // Placeholder for material properties
}

void Win32InspectorPanel::renderComponentSection()
{
    // Placeholder for component list
}

bool Win32InspectorPanel::handleMessage(UINT msg, WPARAM wParam, LPARAM lParam, LRESULT& result)
{
    (void)lParam;  // Unused parameter
    switch (msg) {
        case WM_COMMAND: {
            int controlId = LOWORD(wParam);
            int notificationCode = HIWORD(wParam);
            
            if (controlId == 1001 && notificationCode == BN_CLICKED) {
                // Visibility checkbox changed
                if (m_inspectedNode) {
                    LRESULT checked = SendMessageW(m_visibleCheckbox, BM_GETCHECK, 0, 0);
                    m_inspectedNode->visible = (checked == BST_CHECKED);
                    LOG_INFO_C("Object visibility changed", "Win32InspectorPanel");
                }
            }
            result = 0;
            return true;
        }
        
        case WM_CTLCOLORSTATIC: {
            HDC hdcStatic = (HDC)wParam;
            SetTextColor(hdcStatic, UnrealStyleTheme::TextPrimary);
            SetBkColor(hdcStatic, UnrealStyleTheme::DarkBackground);
            result = (LRESULT)GetStockObject(DC_BRUSH);
            return true;
        }
    }
    
    return false; // Let base class handle it
}

} // namespace fresh

#endif // _WIN32
