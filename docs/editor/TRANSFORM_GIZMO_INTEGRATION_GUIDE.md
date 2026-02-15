# Transform Gizmo Integration Guide

This guide shows how to integrate the TransformGizmo into the EditorManager and connect it to the existing UI.

## Step 1: Add TransformGizmo to EditorManager.h

Add the following to `engine/editor/EditorManager.h`:

```cpp
// Add include at top
#include "editor/TransformGizmo.h"

// Add to private members section
private:
    std::unique_ptr<TransformGizmo> m_transformGizmo;
    bool m_gizmoEnabled;
    
// Add to getters section  
public:
    /**
     * @brief Get transform gizmo instance
     * @return Pointer to transform gizmo
     */
    TransformGizmo* getTransformGizmo() { return m_transformGizmo.get(); }
```

## Step 2: Initialize TransformGizmo in EditorManager.cpp

Add to `EditorManager::initialize()` after other components:

```cpp
// Initialize transform gizmo
m_transformGizmo = std::make_unique<TransformGizmo>();
m_transformGizmo->setMode(TransformGizmo::Mode::Translate);
m_transformGizmo->setSpace(TransformGizmo::Space::World);
m_transformGizmo->setSnapEnabled(true);
m_transformGizmo->setSnapValue(1.0f);
m_transformGizmo->setSize(1.0f);
m_gizmoEnabled = true;

LOG_INFO_C("Transform Gizmo initialized", "EditorManager");
```

## Step 3: Wire Up Toolbar Callbacks

Modify the toolbar initialization in `EditorManager::initialize()`:

```cpp
m_toolbar = std::make_unique<EditorToolbar>();
if (!m_toolbar->initialize()) {
    LOG_ERROR_C("Failed to initialize Toolbar", "EditorManager");
    return false;
}

// Set toolbar tool callback to control gizmo mode
m_toolbar->setToolCallback([this](EditorToolbar::Tool tool) {
    if (!m_transformGizmo) return;
    
    switch (tool) {
        case EditorToolbar::Tool::Select:
            m_gizmoEnabled = false;
            LOG_INFO_C("Gizmo disabled - Select mode", "EditorManager");
            break;
            
        case EditorToolbar::Tool::Move:
            m_gizmoEnabled = true;
            m_transformGizmo->setMode(TransformGizmo::Mode::Translate);
            LOG_INFO_C("Gizmo mode: Translate", "EditorManager");
            break;
            
        case EditorToolbar::Tool::Rotate:
            m_gizmoEnabled = true;
            m_transformGizmo->setMode(TransformGizmo::Mode::Rotate);
            LOG_INFO_C("Gizmo mode: Rotate", "EditorManager");
            break;
            
        case EditorToolbar::Tool::Scale:
            m_gizmoEnabled = true;
            m_transformGizmo->setMode(TransformGizmo::Mode::Scale);
            LOG_INFO_C("Gizmo mode: Scale", "EditorManager");
            break;
            
        default:
            // Other tools don't use gizmo
            m_gizmoEnabled = false;
            break;
    }
});
```

## Step 4: Update Gizmo Transform on Selection Change

Add a method to update the gizmo position:

```cpp
void EditorManager::updateGizmoTransform() {
    if (!m_transformGizmo || !m_selectionManager) {
        return;
    }
    
    // If there's a selection, position gizmo at its center
    if (m_selectionManager->hasSelection()) {
        glm::ivec3 min, max;
        if (m_selectionManager->getSelectionBounds(min, max)) {
            glm::vec3 center = (glm::vec3(min) + glm::vec3(max)) * 0.5f;
            glm::mat4 transform = glm::translate(glm::mat4(1.0f), center);
            m_transformGizmo->setTransform(transform);
        }
    }
}
```

Call this whenever selection changes. In `SelectionManager` integration:

```cpp
// After finalizing selection
m_selectionManager->finalizeSelection(m_world);
updateGizmoTransform();
```

## Step 5: Handle Gizmo Input

Add input handling in `EditorManager::handleInput()` or create a new method:

```cpp
void EditorManager::handleGizmoInput(const glm::vec2& mousePos, bool mouseDown, 
                                     bool mouseDrag, bool mouseUp, Camera* camera) {
    if (!m_gizmoEnabled || !m_transformGizmo || !camera) {
        return;
    }
    
    // Don't interact if mouse is over UI
    if (ImGui::GetIO().WantCaptureMouse) {
        return;
    }
    
    // Hit test for hover highlighting
    if (!m_transformGizmo->isManipulating()) {
        m_transformGizmo->hitTest(mousePos, *camera);
    }
    
    // Begin manipulation on mouse down
    if (mouseDown && !m_transformGizmo->isManipulating()) {
        m_transformGizmo->beginManipulation(mousePos, *camera);
    }
    
    // Update manipulation on drag
    if (mouseDrag && m_transformGizmo->isManipulating()) {
        if (m_transformGizmo->updateManipulation(mousePos, *camera)) {
            // Transform changed - apply to selection
            applyGizmoTransformToSelection();
        }
    }
    
    // End manipulation on mouse up
    if (mouseUp && m_transformGizmo->isManipulating()) {
        m_transformGizmo->endManipulation();
    }
}
```

## Step 6: Apply Transform to Selection

```cpp
void EditorManager::applyGizmoTransformToSelection() {
    if (!m_transformGizmo || !m_selectionManager || !m_world) {
        return;
    }
    
    // Get delta based on gizmo mode
    switch (m_transformGizmo->getMode()) {
        case TransformGizmo::Mode::Translate:
        case TransformGizmo::Mode::Universal: {
            glm::vec3 delta = m_transformGizmo->getDeltaTranslation();
            
            // Only apply if delta is significant (avoid micro-movements)
            if (glm::length(delta) > 0.01f) {
                glm::ivec3 intDelta = glm::round(delta);
                m_selectionManager->moveSelection(intDelta, m_world);
                
                // Update gizmo position
                updateGizmoTransform();
            }
            break;
        }
        
        case TransformGizmo::Mode::Rotate:
            // Rotation for voxels - could implement later
            LOG_INFO_C("Voxel rotation not yet implemented", "EditorManager");
            break;
            
        case TransformGizmo::Mode::Scale:
            // Scaling for voxels - could implement later
            LOG_INFO_C("Voxel scaling not yet implemented", "EditorManager");
            break;
            
        default:
            break;
    }
}
```

## Step 7: Render Gizmo

Add to `EditorManager::render()` after rendering UI panels:

```cpp
void EditorManager::render() {
    // ... existing panel rendering ...
    
    // Render transform gizmo in 3D viewport
    if (m_gizmoEnabled && m_transformGizmo && m_selectionManager && 
        m_selectionManager->hasSelection()) {
        // Get camera from scene/viewport
        Camera* camera = getCurrentCamera();
        if (camera) {
            m_transformGizmo->render(*camera);
        }
    }
}
```

## Step 8: Add Keyboard Shortcuts

In input handling, add keyboard shortcuts:

```cpp
void EditorManager::handleKeyboardInput() {
    // Toggle between gizmo modes with W/E/R keys
    if (ImGui::IsKeyPressed(ImGuiKey_W)) {
        m_toolbar->setActiveTool(EditorToolbar::Tool::Move);
        m_gizmoEnabled = true;
        m_transformGizmo->setMode(TransformGizmo::Mode::Translate);
        LOG_INFO_C("Keyboard shortcut: W - Translate mode", "EditorManager");
    }
    
    if (ImGui::IsKeyPressed(ImGuiKey_E)) {
        m_toolbar->setActiveTool(EditorToolbar::Tool::Rotate);
        m_gizmoEnabled = true;
        m_transformGizmo->setMode(TransformGizmo::Mode::Rotate);
        LOG_INFO_C("Keyboard shortcut: E - Rotate mode", "EditorManager");
    }
    
    if (ImGui::IsKeyPressed(ImGuiKey_R)) {
        m_toolbar->setActiveTool(EditorToolbar::Tool::Scale);
        m_gizmoEnabled = true;
        m_transformGizmo->setMode(TransformGizmo::Mode::Scale);
        LOG_INFO_C("Keyboard shortcut: R - Scale mode", "EditorManager");
    }
    
    // Toggle snap with Shift key
    if (ImGui::IsKeyDown(ImGuiKey_LeftShift) || ImGui::IsKeyDown(ImGuiKey_RightShift)) {
        if (!m_transformGizmo->isSnapEnabled()) {
            m_transformGizmo->setSnapEnabled(true);
        }
    }
    
    // Toggle world/local space with spacebar
    if (ImGui::IsKeyPressed(ImGuiKey_Space)) {
        auto currentSpace = m_transformGizmo->getSpace();
        auto newSpace = (currentSpace == TransformGizmo::Space::World) 
                        ? TransformGizmo::Space::Local 
                        : TransformGizmo::Space::World;
        m_transformGizmo->setSpace(newSpace);
        LOG_INFO_C("Gizmo space toggled", "EditorManager");
    }
}
```

## Step 9: Add UI Settings Panel

In EngineConfigPanel or SettingsPanel, add gizmo settings:

```cpp
void EngineConfigPanel::renderGizmoSettings() {
    if (!m_editorManager || !m_editorManager->getTransformGizmo()) {
        return;
    }
    
    auto gizmo = m_editorManager->getTransformGizmo();
    
    ImGui::Text("Transform Gizmo Settings");
    ImGui::Separator();
    
    // Snap settings
    bool snapEnabled = gizmo->isSnapEnabled();
    if (ImGui::Checkbox("Snap to Grid", &snapEnabled)) {
        gizmo->setSnapEnabled(snapEnabled);
    }
    
    if (snapEnabled) {
        float snapValue = gizmo->getSnapValue();
        if (ImGui::DragFloat("Snap Value", &snapValue, 0.1f, 0.1f, 10.0f)) {
            gizmo->setSnapValue(snapValue);
        }
        ImGui::SameLine();
        ImGui::TextDisabled("(?)");
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Distance units for translate, degrees for rotate, factor for scale");
        }
    }
    
    // Gizmo size
    float size = gizmo->getSize();
    if (ImGui::SliderFloat("Gizmo Size", &size, 0.5f, 3.0f)) {
        gizmo->setSize(size);
    }
    
    // Coordinate space
    ImGui::Text("Coordinate Space:");
    ImGui::SameLine();
    auto space = gizmo->getSpace();
    if (ImGui::RadioButton("World", space == TransformGizmo::Space::World)) {
        gizmo->setSpace(TransformGizmo::Space::World);
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Local", space == TransformGizmo::Space::Local)) {
        gizmo->setSpace(TransformGizmo::Space::Local);
    }
}
```

## Step 10: Connect Rendering to DebugRenderer

Replace the placeholder rendering functions in `TransformGizmo.cpp`:

```cpp
void TransformGizmo::drawArrow(const glm::vec3& position, const glm::vec3& direction, 
                               const glm::vec3& color, bool highlighted) {
    if (!m_debugRenderer) return;
    
    float thickness = highlighted ? 3.0f : 2.0f;
    glm::vec4 drawColor = glm::vec4(highlighted ? COLOR_HIGHLIGHT : color, 1.0f);
    
    // Draw shaft
    glm::vec3 end = position + direction * ARROW_LENGTH * size_;
    m_debugRenderer->drawLine(position, end, drawColor, thickness);
    
    // Draw arrow head
    glm::vec3 headBase = end - direction * ARROW_HEAD_LENGTH * size_;
    m_debugRenderer->drawCone(headBase, direction, ARROW_HEAD_RADIUS * size_, 
                             ARROW_HEAD_LENGTH * size_, drawColor);
}

void TransformGizmo::drawCircle(const glm::vec3& position, const glm::vec3& normal, 
                                float radius, const glm::vec3& color, bool highlighted) {
    if (!m_debugRenderer) return;
    
    float thickness = highlighted ? 3.0f : 2.0f;
    glm::vec4 drawColor = glm::vec4(highlighted ? COLOR_HIGHLIGHT : color, 1.0f);
    
    m_debugRenderer->drawCircle(position, normal, radius, CIRCLE_SEGMENTS, 
                                drawColor, thickness);
}

void TransformGizmo::drawBox(const glm::vec3& position, const glm::vec3& size, 
                             const glm::vec3& color, bool highlighted) {
    if (!m_debugRenderer) return;
    
    float thickness = highlighted ? 3.0f : 2.0f;
    glm::vec4 drawColor = glm::vec4(highlighted ? COLOR_HIGHLIGHT : color, 1.0f);
    
    m_debugRenderer->drawWireBox(position - size * 0.5f, position + size * 0.5f, 
                                 drawColor, thickness);
}
```

And add a setter:

```cpp
// In TransformGizmo.h
void setDebugRenderer(devtools::DebugRenderer* renderer) { m_debugRenderer = renderer; }

// In private members
devtools::DebugRenderer* m_debugRenderer = nullptr;
```

Then in EditorManager initialization:

```cpp
// After creating debug renderer
m_transformGizmo->setDebugRenderer(m_debugRenderer.get());
```

## Testing Checklist

- [ ] Transform gizmo appears when voxels are selected
- [ ] Clicking toolbar buttons changes gizmo mode
- [ ] W/E/R keyboard shortcuts work
- [ ] Dragging gizmo arrows moves selection
- [ ] Snap-to-grid works with configurable values
- [ ] Space key toggles world/local space
- [ ] Gizmo disappears in select mode
- [ ] Gizmo size can be adjusted in settings
- [ ] Hover highlights axes in yellow
- [ ] Active manipulation highlights in yellow
- [ ] Multiple selections show gizmo at center

## Common Issues

### Gizmo Not Visible
- Check that `m_gizmoEnabled` is true
- Check that selection has voxels
- Check that camera is passed to render
- Check that DebugRenderer is initialized

### Gizmo Not Responding to Mouse
- Check that ImGui isn't capturing mouse
- Check that hit test is being called
- Check that ray-casting math is correct
- Check that camera matrices are valid

### Transform Not Applied
- Check that `applyGizmoTransformToSelection` is called
- Check that SelectionManager and World are valid
- Check that delta is being calculated correctly
- Check that undo/redo is not blocking changes

## Next Steps

1. Implement plane handles for 2D movement
2. Add entity transform support (not just voxels)
3. Add rotation/scale support for voxel selections
4. Improve hit testing accuracy
5. Add gizmo state to undo/redo history
6. Add numerical input during manipulation
7. Add duplicate-on-drag (hold Alt)
