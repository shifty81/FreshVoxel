#pragma once

#ifdef _WIN32
#include "ui/native/Win32SceneHierarchyPanel.h"
#else
#include <string>
#include <memory>
#include <vector>
#include <functional>

namespace fresh
{

class VoxelWorld;

/**
 * @brief Hierarchy node structure for scene graph representation (cross-platform)
 */
struct HierarchyNode
{
    std::string name;
    int type = 0;
    void* data = nullptr;
    std::vector<std::shared_ptr<HierarchyNode>> children;
    bool visible = true;
    bool selected = false;
    void* userData = nullptr;

    explicit HierarchyNode(const std::string& nodeName = "")
        : name(nodeName), type(0), data(nullptr), visible(true), selected(false), userData(nullptr) {}
};

/**
 * @brief Stub SceneHierarchyPanel for non-Win32 platforms
 */
class SceneHierarchyPanel
{
public:
    using SelectionCallback = std::function<void(HierarchyNode*)>;

    SceneHierarchyPanel() = default;
    virtual ~SceneHierarchyPanel() = default;

    bool initialize(VoxelWorld*) { return true; }
    void setWorld(VoxelWorld*) {}
    void render() {}
    void refresh() {}
    HierarchyNode* getSelectedNode() const { return nullptr; }
    void setSelectionCallback(SelectionCallback) {}
    void selectAll() {}
    void deselectAll() {}
    HierarchyNode* addNode(const std::string& name, HierarchyNode* = nullptr) {
        auto node = std::make_shared<HierarchyNode>(name);
        m_nodes.push_back(node);
        return node.get();
    }

private:
    std::vector<std::shared_ptr<HierarchyNode>> m_nodes;
};

} // namespace fresh

#endif

#ifdef _WIN32
namespace fresh
{
    // Windows native UI implementation
    using SceneHierarchyPanel = Win32SceneHierarchyPanel;
}
#endif
