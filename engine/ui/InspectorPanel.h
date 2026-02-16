#pragma once

#ifdef _WIN32
#include "ui/native/Win32InspectorPanel.h"

namespace fresh
{
    // Windows native UI implementation
    using InspectorPanel = Win32InspectorPanel;
}
#else
#include <string>

namespace fresh
{

struct HierarchyNode;

namespace ecs { class EntityManager; }

/**
 * @brief Stub InspectorPanel for non-Win32 platforms
 */
class InspectorPanel
{
public:
    InspectorPanel() = default;
    virtual ~InspectorPanel() = default;

    bool initialize(ecs::EntityManager* = nullptr) { return true; }
    void render() {}
    void setInspectedNode(HierarchyNode*) {}
    HierarchyNode* getInspectedNode() const { return nullptr; }
    void refresh() {}
};

} // namespace fresh
#endif
