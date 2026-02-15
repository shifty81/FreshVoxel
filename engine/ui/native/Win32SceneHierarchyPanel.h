#pragma once

#ifdef _WIN32

// Prevent Windows.h from defining min/max macros
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>
#include <string>
#include <memory>
#include <map>
#include <vector>
#include <functional>
#include "ui/native/Win32Panel.h"
#include "ui/native/Win32TreeView.h"

namespace fresh
{

class VoxelWorld;

/**
 * @brief Hierarchy node structure for scene graph representation
 */
struct HierarchyNode
{
    std::string name;                                     // Node name
    int type = 0;                                         // Node type identifier
    void* data = nullptr;                                 // Custom data pointer
    std::vector<std::shared_ptr<HierarchyNode>> children; // Child nodes
    bool visible = true;                                  // Visibility flag
    bool selected = false;                                // Selection state
    void* userData = nullptr;                             // User-defined data pointer
    
    // Constructor
    explicit HierarchyNode(const std::string& nodeName = "") 
        : name(nodeName), type(0), data(nullptr), visible(true), selected(false), userData(nullptr) {}
};

/**
 * @brief Native Win32 Scene Hierarchy Panel using TreeView
 * 
 * Provides a professional native Windows scene hierarchy interface with:
 * - TreeView control for hierarchical object display
 * - Expand/collapse nodes
 * - Selection and multi-selection
 * - Context menu for operations (rename, duplicate, delete)
 * - Drag-and-drop for reparenting (future)
 * - Unreal Engine-like dark theme styling
 */
class Win32SceneHierarchyPanel : public Win32Panel
{
public:
    using SelectionCallback = std::function<void(HierarchyNode*)>;
    
    Win32SceneHierarchyPanel();
    virtual ~Win32SceneHierarchyPanel();

    /**
     * @brief Initialize the scene hierarchy panel (simplified interface)
     * @param world Voxel world to display
     * @return true if successful
     */
    bool initialize(VoxelWorld* world);

    /**
     * @brief Initialize the scene hierarchy panel
     * @param parent Parent window handle
     * @param x X position
     * @param y Y position
     * @param width Panel width
     * @param height Panel height
     * @param world Voxel world to display
     * @return true if successful
     */
    bool create(HWND parent, int x, int y, int width, int height, VoxelWorld* world);

    /**
     * @brief Set the world reference
     * @param world New world pointer
     */
    void setWorld(VoxelWorld* world) { m_world = world; }

    /**
     * @brief Refresh the hierarchy from world data
     */
    void refresh();

    /**
     * @brief Get the currently selected node
     * @return Pointer to selected node or nullptr
     */
    HierarchyNode* getSelectedNode() const { return m_selectedNode; }

    /**
     * @brief Set selection callback
     * @param callback Callback to invoke when selection changes
     */
    void setSelectionCallback(SelectionCallback callback) { m_selectionCallback = callback; }

    /**
     * @brief Select all nodes
     */
    void selectAll();

    /**
     * @brief Deselect all nodes
     */
    void deselectAll();

    /**
     * @brief Add a new node to the hierarchy
     * @param name Name of the node
     * @param parent Parent node (nullptr for root)
     * @return Pointer to the created node
     */
    HierarchyNode* addNode(const std::string& name, HierarchyNode* parent = nullptr);

protected:
    // Override base class method
    bool handleMessage(UINT msg, WPARAM wParam, LPARAM lParam, LRESULT& result) override;

private:
    void buildHierarchyFromWorld();
    void addNodeToTree(HierarchyNode* node, HTREEITEM parentItem);
    void onTreeSelectionChanged();
    void showContextMenu(int x, int y);
    
    bool renameSelectedNode();
    bool duplicateSelectedNode();
    bool deleteSelectedNode();

private:
    VoxelWorld* m_world;
    std::unique_ptr<Win32TreeView> m_treeView;
    std::shared_ptr<HierarchyNode> m_rootNode;
    HierarchyNode* m_selectedNode;
    SelectionCallback m_selectionCallback;
    
    // Map tree items to hierarchy nodes
    std::map<HTREEITEM, HierarchyNode*> m_itemToNode;
    std::map<HierarchyNode*, HTREEITEM> m_nodeToItem;
    
    // Context menu
    HMENU m_contextMenu;
};

} // namespace fresh

#endif // _WIN32
