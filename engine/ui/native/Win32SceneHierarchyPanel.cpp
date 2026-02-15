#ifdef _WIN32

#include "ui/native/Win32SceneHierarchyPanel.h"
#include "ui/native/UnrealStyleTheme.h"
#include "ui/SceneHierarchyPanel.h"
#include "voxel/VoxelWorld.h"
#include "core/Logger.h"
#include <windowsx.h>
#include <functional>

namespace fresh
{

// Context menu command IDs
constexpr int CMD_RENAME = 2001;
constexpr int CMD_DUPLICATE = 2002;
constexpr int CMD_DELETE = 2003;
constexpr int CMD_ADD_OBJECT = 2004;

Win32SceneHierarchyPanel::Win32SceneHierarchyPanel()
    : Win32Panel(),
      m_world(nullptr),
      m_treeView(nullptr),
      m_rootNode(nullptr),
      m_selectedNode(nullptr),
      m_selectionCallback(nullptr),
      m_contextMenu(nullptr)
{
}

Win32SceneHierarchyPanel::~Win32SceneHierarchyPanel()
{
    if (m_contextMenu) {
        DestroyMenu(m_contextMenu);
    }
}

bool Win32SceneHierarchyPanel::create(HWND parent, int x, int y, int width, int height, VoxelWorld* world)
{
    m_world = world;
    
    if (!Win32Panel::create(parent, x, y, width, height, L"Scene Hierarchy")) {
        return false;
    }
    
    // Create tree view control
    m_treeView = std::make_unique<Win32TreeView>();
    RECT clientRect;
    GetClientRect(m_hwnd, &clientRect);
    
    if (!m_treeView->create(m_hwnd, 0, 0, clientRect.right, clientRect.bottom)) {
        LOG_ERROR_C("Failed to create TreeView control", "Win32SceneHierarchyPanel");
        return false;
    }
    
    // Create context menu
    m_contextMenu = CreatePopupMenu();
    AppendMenuW(m_contextMenu, MF_STRING, CMD_ADD_OBJECT, L"Add Object");
    AppendMenuW(m_contextMenu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(m_contextMenu, MF_STRING, CMD_RENAME, L"Rename");
    AppendMenuW(m_contextMenu, MF_STRING, CMD_DUPLICATE, L"Duplicate");
    AppendMenuW(m_contextMenu, MF_STRING, CMD_DELETE, L"Delete");
    
    // Build initial hierarchy
    refresh();
    
    LOG_INFO_C("Win32SceneHierarchyPanel created", "Win32SceneHierarchyPanel");
    return true;
}

bool Win32SceneHierarchyPanel::initialize(VoxelWorld* world)
{
    // For standalone initialization without parent window
    // Store the world reference for later use when create() is called
    m_world = world;
    LOG_INFO_C("Scene Hierarchy panel initialized (stub)", "Win32SceneHierarchyPanel");
    return true;
}

void Win32SceneHierarchyPanel::refresh()
{
    if (!m_treeView) {
        return;
    }
    
    // Clear existing tree
    m_treeView->clear();
    m_itemToNode.clear();
    m_nodeToItem.clear();
    m_selectedNode = nullptr;
    
    // Build hierarchy from world
    buildHierarchyFromWorld();
}

void Win32SceneHierarchyPanel::buildHierarchyFromWorld()
{
    // Create root node
    m_rootNode = std::make_shared<HierarchyNode>("World");
    
    if (!m_world) {
        // Add placeholder items
        HTREEITEM rootItem = m_treeView->addRootItem("World", m_rootNode.get());
        m_itemToNode[rootItem] = m_rootNode.get();
        m_nodeToItem[m_rootNode.get()] = rootItem;
        return;
    }
    
    // Add root item to tree
    HTREEITEM rootItem = m_treeView->addRootItem("World", m_rootNode.get());
    m_itemToNode[rootItem] = m_rootNode.get();
    m_nodeToItem[m_rootNode.get()] = rootItem;
    
    // Add chunks as children
    // Note: In a real implementation, we would iterate through world chunks
    // For now, add sample hierarchy
    
    // Entities node
    auto entitiesNode = std::make_shared<HierarchyNode>("Entities");
    m_rootNode->children.push_back(entitiesNode);
    addNodeToTree(entitiesNode.get(), rootItem);
    
    // Terrain node
    auto terrainNode = std::make_shared<HierarchyNode>("Terrain");
    m_rootNode->children.push_back(terrainNode);
    addNodeToTree(terrainNode.get(), rootItem);
    
    // Chunks under terrain (sample)
    for (int i = 0; i < 5; ++i) {
        std::string chunkName = "Chunk_" + std::to_string(i);
        auto chunkNode = std::make_shared<HierarchyNode>(chunkName);
        terrainNode->children.push_back(chunkNode);
        
        HTREEITEM terrainItem = m_nodeToItem[terrainNode.get()];
        addNodeToTree(chunkNode.get(), terrainItem);
    }
    
    // Expand root by default
    m_treeView->expandItem(rootItem);
}

void Win32SceneHierarchyPanel::addNodeToTree(HierarchyNode* node, HTREEITEM parentItem)
{
    if (!node || !m_treeView) {
        return;
    }
    
    HTREEITEM item = m_treeView->addChildItem(parentItem, node->name, node);
    m_itemToNode[item] = node;
    m_nodeToItem[node] = item;
    
    // Recursively add children
    for (auto& child : node->children) {
        addNodeToTree(child.get(), item);
    }
}

void Win32SceneHierarchyPanel::onTreeSelectionChanged()
{
    if (!m_treeView) {
        return;
    }
    
    HTREEITEM selectedItem = m_treeView->getSelectedItem();
    
    if (selectedItem && m_itemToNode.find(selectedItem) != m_itemToNode.end()) {
        m_selectedNode = m_itemToNode[selectedItem];
        
        // Mark node as selected
        if (m_selectedNode) {
            m_selectedNode->selected = true;
        }
        
        // Invoke callback
        if (m_selectionCallback) {
            m_selectionCallback(m_selectedNode);
        }
        
        LOG_INFO_C("Node selected: " + (m_selectedNode ? m_selectedNode->name : "nullptr"), "Win32SceneHierarchyPanel");
    } else {
        m_selectedNode = nullptr;
        
        if (m_selectionCallback) {
            m_selectionCallback(nullptr);
        }
    }
}

void Win32SceneHierarchyPanel::showContextMenu(int x, int y)
{
    if (!m_contextMenu) {
        return;
    }
    
    // Enable/disable menu items based on selection
    UINT enableFlags = m_selectedNode ? MF_ENABLED : MF_GRAYED;
    EnableMenuItem(m_contextMenu, CMD_RENAME, MF_BYCOMMAND | enableFlags);
    EnableMenuItem(m_contextMenu, CMD_DUPLICATE, MF_BYCOMMAND | enableFlags);
    EnableMenuItem(m_contextMenu, CMD_DELETE, MF_BYCOMMAND | enableFlags);
    
    // Show menu - x,y are already in screen coordinates from WM_CONTEXTMENU
    // Do NOT call ClientToScreen here as that would double the offset
    TrackPopupMenu(m_contextMenu, TPM_RIGHTBUTTON, x, y, 0, m_hwnd, nullptr);
}

bool Win32SceneHierarchyPanel::renameSelectedNode()
{
    if (!m_selectedNode) {
        return false;
    }
    
    // Create a simple input dialog for rename
    wchar_t buffer[256] = {0};
    std::wstring currentName(m_selectedNode->name.begin(), m_selectedNode->name.end());
    wcsncpy_s(buffer, 256, currentName.c_str(), _TRUNCATE);
    
    // Create a simple dialog
    HWND hDialog = CreateWindowExW(
        WS_EX_DLGMODALFRAME | WS_EX_TOPMOST,
        L"STATIC",
        L"Rename Node",
        WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, 350, 150,
        m_hwnd, nullptr, GetModuleHandle(nullptr), nullptr
    );
    
    if (!hDialog) {
        LOG_ERROR_C("Failed to create rename dialog", "Win32SceneHierarchyPanel");
        return false;
    }
    
    // Apply theme
    UnrealStyleTheme::ApplyToWindow(hDialog);
    
    // Create label
    HWND hLabel = CreateWindowExW(
        0, L"STATIC", L"New name:",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        20, 20, 300, 20,
        hDialog, nullptr, GetModuleHandle(nullptr), nullptr
    );
    
    // Create edit control
    HWND hEdit = CreateWindowExW(
        WS_EX_CLIENTEDGE, L"EDIT", buffer,
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
        20, 45, 300, 25,
        hDialog, (HMENU)1001, GetModuleHandle(nullptr), nullptr
    );
    
    // Create OK button
    HWND hOK = CreateWindowExW(
        0, L"BUTTON", L"OK",
        WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
        140, 85, 80, 30,
        hDialog, (HMENU)IDOK, GetModuleHandle(nullptr), nullptr
    );
    
    // Create Cancel button
    HWND hCancel = CreateWindowExW(
        0, L"BUTTON", L"Cancel",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        230, 85, 80, 30,
        hDialog, (HMENU)IDCANCEL, GetModuleHandle(nullptr), nullptr
    );
    
    // Set font for controls
    HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    SendMessageW(hLabel, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessageW(hEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessageW(hOK, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessageW(hCancel, WM_SETFONT, (WPARAM)hFont, TRUE);
    
    // Focus the edit control and select all text
    SetFocus(hEdit);
    SendMessageW(hEdit, EM_SETSEL, 0, -1);
    
    // Center dialog relative to parent
    RECT parentRect, dialogRect;
    GetWindowRect(m_hwnd, &parentRect);
    GetWindowRect(hDialog, &dialogRect);
    int x = parentRect.left + ((parentRect.right - parentRect.left) - (dialogRect.right - dialogRect.left)) / 2;
    int y = parentRect.top + ((parentRect.bottom - parentRect.top) - (dialogRect.bottom - dialogRect.top)) / 2;
    SetWindowPos(hDialog, nullptr, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
    
    // Simple message loop for modal dialog
    MSG msg;
    bool dialogResult = false;
    bool dialogActive = true;
    
    while (dialogActive && GetMessageW(&msg, nullptr, 0, 0)) {
        if (msg.hwnd == hDialog || IsChild(hDialog, msg.hwnd)) {
            if (msg.message == WM_COMMAND) {
                int cmdId = LOWORD(msg.wParam);
                if (cmdId == IDOK) {
                    // Get new name from edit control
                    wchar_t newName[256] = {0};
                    GetWindowTextW(hEdit, newName, 256);
                    
                    if (wcslen(newName) > 0) {
                        // Convert to narrow string
                        char narrowName[256];
                        WideCharToMultiByte(CP_UTF8, 0, newName, -1, narrowName, 256, nullptr, nullptr);
                        
                        // Update node name
                        m_selectedNode->name = narrowName;
                        
                        // Update tree view item
                        auto it = m_nodeToItem.find(m_selectedNode);
                        if (it != m_nodeToItem.end()) {
                            m_treeView->setItemText(it->second, narrowName);
                        }
                        
                        dialogResult = true;
                        LOG_INFO_C("Node renamed to: " + m_selectedNode->name, "Win32SceneHierarchyPanel");
                    }
                    
                    dialogActive = false;
                } else if (cmdId == IDCANCEL) {
                    dialogActive = false;
                }
            } else if (msg.message == WM_CLOSE) {
                dialogActive = false;
            }
        }
        
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    
    DestroyWindow(hDialog);
    return dialogResult;
}

bool Win32SceneHierarchyPanel::duplicateSelectedNode()
{
    if (!m_selectedNode) {
        return false;
    }
    
    // Create a copy of the selected node
    auto newNode = std::make_shared<HierarchyNode>(m_selectedNode->name + " (Copy)");
    newNode->type = m_selectedNode->type;
    newNode->data = m_selectedNode->data;
    
    // Find the parent of the selected node
    HierarchyNode* parent = nullptr;
    if (m_rootNode) {
        // Search for parent in the tree
        std::function<HierarchyNode*(HierarchyNode*, HierarchyNode*)> findParent;
        findParent = [&](HierarchyNode* current, HierarchyNode* target) -> HierarchyNode* {
            for (const auto& child : current->children) {
                if (child.get() == target) {
                    return current;
                }
                HierarchyNode* found = findParent(child.get(), target);
                if (found) {
                    return found;
                }
            }
            return nullptr;
        };
        
        parent = findParent(m_rootNode.get(), m_selectedNode);
    }
    
    // Add the new node to the parent (or root if no parent found)
    if (!parent) {
        parent = m_rootNode.get();
    }
    
    if (parent) {
        parent->children.push_back(newNode);
        
        // Add to tree view
        HTREEITEM parentItem = m_nodeToItem[parent];
        addNodeToTree(newNode.get(), parentItem);
        
        // Expand parent to show the new node
        if (m_treeView) {
            m_treeView->expandItem(parentItem);
        }
        
        LOG_INFO_C("Node duplicated: " + m_selectedNode->name + " -> " + newNode->name, "Win32SceneHierarchyPanel");
        return true;
    }
    
    return false;
}

bool Win32SceneHierarchyPanel::deleteSelectedNode()
{
    if (!m_selectedNode) {
        return false;
    }
    
    // Don't allow deleting the root node
    if (m_selectedNode == m_rootNode.get()) {
        LOG_WARNING_C("Cannot delete root node", "Win32SceneHierarchyPanel");
        return false;
    }
    
    // Confirm deletion with user
    int result = MessageBoxW(
        m_hwnd,
        (L"Are you sure you want to delete '" + 
         std::wstring(m_selectedNode->name.begin(), m_selectedNode->name.end()) + 
         L"'?").c_str(),
        L"Confirm Delete",
        MB_YESNO | MB_ICONQUESTION
    );
    
    if (result != IDYES) {
        return false;
    }
    
    // Find the parent and remove the node
    bool deleted = false;
    if (m_rootNode) {
        std::function<bool(HierarchyNode*)> removeNode;
        removeNode = [&](HierarchyNode* current) -> bool {
            auto& children = current->children;
            for (auto it = children.begin(); it != children.end(); ++it) {
                if (it->get() == m_selectedNode) {
                    // Remove from tree view first
                    auto itemIt = m_nodeToItem.find(m_selectedNode);
                    if (itemIt != m_nodeToItem.end()) {
                        if (m_treeView) {
                            m_treeView->removeItem(itemIt->second);
                        }
                        m_itemToNode.erase(itemIt->second);
                        m_nodeToItem.erase(itemIt);
                    }
                    
                    // Remove from hierarchy
                    children.erase(it);
                    deleted = true;
                    return true;
                }
                
                // Recursively search in children
                if (removeNode(it->get())) {
                    return true;
                }
            }
            return false;
        };
        
        deleted = removeNode(m_rootNode.get());
    }
    
    if (deleted) {
        m_selectedNode = nullptr;
        LOG_INFO_C("Node deleted successfully", "Win32SceneHierarchyPanel");
    } else {
        LOG_WARNING_C("Failed to delete node", "Win32SceneHierarchyPanel");
    }
    
    return deleted;
}

void Win32SceneHierarchyPanel::selectAll()
{
    // Select all nodes in tree
    // Note: Win32 TreeView doesn't support multi-selection by default
    LOG_INFO_C("Select all requested", "Win32SceneHierarchyPanel");
}

void Win32SceneHierarchyPanel::deselectAll()
{
    if (m_treeView) {
        m_treeView->setSelectedItem(nullptr);
        m_selectedNode = nullptr;
    }
}

HierarchyNode* Win32SceneHierarchyPanel::addNode(const std::string& name, HierarchyNode* parent)
{
    auto newNode = std::make_shared<HierarchyNode>(name);
    
    if (!parent) {
        parent = m_rootNode.get();
    }
    
    if (parent) {
        parent->children.push_back(newNode);
        
        // Add to tree view
        HTREEITEM parentItem = m_nodeToItem[parent];
        addNodeToTree(newNode.get(), parentItem);
        
        // Expand parent
        if (m_treeView) {
            m_treeView->expandItem(parentItem);
        }
    }
    
    return newNode.get();
}

bool Win32SceneHierarchyPanel::handleMessage(UINT msg, WPARAM wParam, LPARAM lParam, LRESULT& result)
{
    switch (msg) {
        case WM_NOTIFY: {
            NMHDR* nmhdr = (NMHDR*)lParam;
            if (nmhdr->code == TVN_SELCHANGED) {
                onTreeSelectionChanged();
            }
            result = 0;
            return true;
        }
        
        case WM_CONTEXTMENU: {
            int xPos = GET_X_LPARAM(lParam);
            int yPos = GET_Y_LPARAM(lParam);
            showContextMenu(xPos, yPos);
            result = 0;
            return true;
        }
        
        case WM_COMMAND: {
            int cmdId = LOWORD(wParam);
            
            switch (cmdId) {
                case CMD_RENAME:
                    renameSelectedNode();
                    break;
                case CMD_DUPLICATE:
                    duplicateSelectedNode();
                    break;
                case CMD_DELETE:
                    deleteSelectedNode();
                    break;
                case CMD_ADD_OBJECT:
                    addNode("New Object", m_selectedNode);
                    break;
            }
            result = 0;
            return true;
        }
        
        case WM_SIZE: {
            // Resize tree view to fill panel
            if (m_treeView) {
                RECT clientRect;
                GetClientRect(m_hwnd, &clientRect);
                // Tree view is accessed via its window handle
                // We would need to expose the HWND from Win32TreeView
                // For now, assume it's handled
            }
            result = 0;
            return true;
        }
    }
    
    return false; // Let base class handle it
}

} // namespace fresh

#endif // _WIN32
