#ifdef _WIN32

#include "ui/native/Win32ContentBrowserPanel.h"
#include "ui/native/UnrealStyleTheme.h"
#include "core/Logger.h"
#include <windowsx.h>
#include <filesystem>
#include <shlobj.h>
#include <commdlg.h>

namespace fresh
{

// Context menu command IDs
constexpr int CMD_IMPORT = 3001;
constexpr int CMD_COPY = 3002;
constexpr int CMD_PASTE = 3003;
constexpr int CMD_DELETE_FILE = 3004;
constexpr int CMD_RENAME_FILE = 3005;
constexpr int CMD_OPEN_LOCATION = 3006;

// Toolbar button IDs
constexpr int BTN_BACK = 3100;
constexpr int BTN_UP = 3101;
constexpr int BTN_REFRESH = 3102;
constexpr int BTN_VIEW_MODE = 3103;

Win32ContentBrowserPanel::Win32ContentBrowserPanel()
    : Win32Panel(),
      m_listView(nullptr),
      m_rootPath(""),
      m_currentPath(""),
      m_toolbar(nullptr),
      m_pathEdit(nullptr),
      m_searchEdit(nullptr),
      m_contextMenu(nullptr),
      m_itemSelectedCallback(nullptr),
      m_itemActivatedCallback(nullptr),
      m_historyIndex(-1),
      m_viewMode(0)  // Start with details view
{
}

Win32ContentBrowserPanel::~Win32ContentBrowserPanel()
{
    if (m_contextMenu) {
        DestroyMenu(m_contextMenu);
    }
}

bool Win32ContentBrowserPanel::create(HWND parent, int x, int y, int width, int height, const std::string& rootPath)
{
    m_rootPath = rootPath;
    m_currentPath = rootPath;
    
    if (!Win32Panel::create(parent, x, y, width, height, L"Content Browser")) {
        return false;
    }
    
    // Create toolbar
    createToolbar();
    
    // Create path bar
    m_pathEdit = CreateWindowExW(
        WS_EX_CLIENTEDGE, L"EDIT", L"",
        WS_CHILD | WS_VISIBLE | ES_LEFT | ES_AUTOHSCROLL | ES_READONLY,
        10, TOOLBAR_HEIGHT + 5, width - 20, PATH_BAR_HEIGHT - 10,
        m_hwnd, nullptr, GetModuleHandle(nullptr), nullptr
    );
    UnrealStyleTheme::ApplyToWindow(m_pathEdit);
    SendMessageW(m_pathEdit, WM_SETFONT, (WPARAM)UnrealStyleTheme::GetFont(), TRUE);
    
    // Create list view control
    m_listView = std::make_unique<Win32ListView>();
    int listY = TOOLBAR_HEIGHT + PATH_BAR_HEIGHT;
    int listHeight = height - listY;
    
    if (!m_listView->create(m_hwnd, 0, listY, width, listHeight)) {
        LOG_ERROR_C("Failed to create ListView control", "Win32ContentBrowserPanel");
        return false;
    }
    
    // Add columns to list view
    m_listView->addColumn("Name", 200);
    m_listView->addColumn("Type", 100);
    m_listView->addColumn("Size", 100);
    m_listView->addColumn("Modified", 150);
    
    // Create context menu
    m_contextMenu = CreatePopupMenu();
    AppendMenuW(m_contextMenu, MF_STRING, CMD_IMPORT, L"Import Assets...");
    AppendMenuW(m_contextMenu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(m_contextMenu, MF_STRING, CMD_COPY, L"Copy");
    AppendMenuW(m_contextMenu, MF_STRING, CMD_PASTE, L"Paste");
    AppendMenuW(m_contextMenu, MF_STRING, CMD_DELETE_FILE, L"Delete");
    AppendMenuW(m_contextMenu, MF_STRING, CMD_RENAME_FILE, L"Rename");
    AppendMenuW(m_contextMenu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(m_contextMenu, MF_STRING, CMD_OPEN_LOCATION, L"Show in Explorer");
    
    // Populate initial content
    refresh();
    
    LOG_INFO_C("Win32ContentBrowserPanel created", "Win32ContentBrowserPanel");
    return true;
}

bool Win32ContentBrowserPanel::initialize(const std::string& rootPath)
{
    // For standalone initialization without parent window
    // Store the root path for later use when create() is called
    m_rootPath = rootPath;
    m_currentPath = rootPath;
    LOG_INFO_C("Content Browser panel initialized (stub)", "Win32ContentBrowserPanel");
    return true;
}

void Win32ContentBrowserPanel::createToolbar()
{
    RECT clientRect;
    GetClientRect(m_hwnd, &clientRect);
    
    m_toolbar = CreateWindowExW(
        0, TOOLBARCLASSNAMEW, nullptr,
        WS_CHILD | WS_VISIBLE | TBSTYLE_FLAT | CCS_NODIVIDER,
        0, 0, clientRect.right, TOOLBAR_HEIGHT,
        m_hwnd, nullptr, GetModuleHandle(nullptr), nullptr
    );
    
    SendMessageW(m_toolbar, TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0);
    
    // Add toolbar buttons
    TBBUTTON buttons[4] = {};
    
    // Back button
    buttons[0].iBitmap = 0;
    buttons[0].idCommand = BTN_BACK;
    buttons[0].fsState = TBSTATE_ENABLED;
    buttons[0].fsStyle = BTNS_BUTTON;
    buttons[0].iString = (INT_PTR)L"Back";
    
    // Up button
    buttons[1].iBitmap = 1;
    buttons[1].idCommand = BTN_UP;
    buttons[1].fsState = TBSTATE_ENABLED;
    buttons[1].fsStyle = BTNS_BUTTON;
    buttons[1].iString = (INT_PTR)L"Up";
    
    // Refresh button
    buttons[2].iBitmap = 2;
    buttons[2].idCommand = BTN_REFRESH;
    buttons[2].fsState = TBSTATE_ENABLED;
    buttons[2].fsStyle = BTNS_BUTTON;
    buttons[2].iString = (INT_PTR)L"Refresh";
    
    // View mode button
    buttons[3].iBitmap = 3;
    buttons[3].idCommand = BTN_VIEW_MODE;
    buttons[3].fsState = TBSTATE_ENABLED;
    buttons[3].fsStyle = BTNS_BUTTON;
    buttons[3].iString = (INT_PTR)L"View";
    
    SendMessageW(m_toolbar, TB_ADDBUTTONSW, 4, (LPARAM)buttons);
    
    UnrealStyleTheme::ApplyToWindow(m_toolbar);
}

void Win32ContentBrowserPanel::refresh()
{
    if (!m_listView) {
        return;
    }
    
    // Clear existing items
    m_listView->clear();
    m_items.clear();
    
    // Update path edit
    std::wstring wpath(m_currentPath.begin(), m_currentPath.end());
    SetWindowTextW(m_pathEdit, wpath.c_str());
    
    // Populate from current directory
    populateListView();
}

void Win32ContentBrowserPanel::populateListView()
{
    namespace fs = std::filesystem;
    
    try {
        if (!fs::exists(m_currentPath) || !fs::is_directory(m_currentPath)) {
            LOG_WARNING_C("Directory does not exist: " + m_currentPath, "Win32ContentBrowserPanel");
            return;
        }
        
        // Iterate through directory
        for (const auto& entry : fs::directory_iterator(m_currentPath)) {
            std::string filename = entry.path().filename().string();
            std::string fullPath = entry.path().string();
            bool isDirectory = entry.is_directory();
            
            addFileItem(filename, fullPath, isDirectory);
        }
        
        LOG_INFO_C("Populated content browser with items from: " + m_currentPath, "Win32ContentBrowserPanel");
        
    } catch (const fs::filesystem_error& e) {
        LOG_ERROR_C("Filesystem error: " + std::string(e.what()), "Win32ContentBrowserPanel");
    }
}

void Win32ContentBrowserPanel::addFileItem(const std::string& filename, const std::string& fullPath, bool isDirectory)
{
    if (!m_listView) {
        return;
    }
    
    // Store item data
    ItemData item;
    item.name = filename;
    item.fullPath = fullPath;
    item.isDirectory = isDirectory;
    m_items.push_back(item);
    
    // Add to list view
    int index = m_listView->addItem(filename);
    
    // Type column
    std::string type = isDirectory ? "Folder" : "File";
    m_listView->setItemText(index, 1, type);
    
    // Size column (for files)
    if (!isDirectory) {
        try {
            namespace fs = std::filesystem;
            auto size = fs::file_size(fullPath);
            std::string sizeStr = std::to_string(size) + " bytes";
            m_listView->setItemText(index, 2, sizeStr);
        } catch (...) {
            m_listView->setItemText(index, 2, "Unknown");
        }
    }
    
    // Modified column
    m_listView->setItemText(index, 3, "Recently");
}

void Win32ContentBrowserPanel::navigateTo(const std::string& path)
{
    // Add to history if navigating to a new path
    if (path != m_currentPath) {
        // Remove any forward history if we're not at the end
        if (m_historyIndex < (int)m_navigationHistory.size() - 1) {
            m_navigationHistory.erase(
                m_navigationHistory.begin() + m_historyIndex + 1,
                m_navigationHistory.end()
            );
        }
        
        // Add new path to history
        m_navigationHistory.push_back(path);
        m_historyIndex = (int)m_navigationHistory.size() - 1;
    }
    
    m_currentPath = path;
    refresh();
}

void Win32ContentBrowserPanel::onItemSelected(int index)
{
    if (index < 0 || index >= (int)m_items.size()) {
        return;
    }
    
    const ItemData& item = m_items[index];
    
    LOG_INFO_C("Item selected: " + item.name, "Win32ContentBrowserPanel");
    
    if (m_itemSelectedCallback) {
        m_itemSelectedCallback(item.fullPath);
    }
}

void Win32ContentBrowserPanel::onItemActivated(int index)
{
    if (index < 0 || index >= (int)m_items.size()) {
        return;
    }
    
    const ItemData& item = m_items[index];
    
    if (item.isDirectory) {
        // Navigate into directory
        navigateTo(item.fullPath);
    } else {
        // Open file (invoke callback)
        LOG_INFO_C("Item activated: " + item.name, "Win32ContentBrowserPanel");
        
        if (m_itemActivatedCallback) {
            m_itemActivatedCallback(item.fullPath);
        }
    }
}

void Win32ContentBrowserPanel::showContextMenu(int x, int y)
{
    if (!m_contextMenu) {
        return;
    }
    
    // Show menu - x,y are already in screen coordinates from WM_CONTEXTMENU
    // Do NOT call ClientToScreen here as that would double the offset
    TrackPopupMenu(m_contextMenu, TPM_RIGHTBUTTON, x, y, 0, m_hwnd, nullptr);
}

void Win32ContentBrowserPanel::showImportDialog()
{
    LOG_INFO_C("Import dialog requested", "Win32ContentBrowserPanel");
    
    // Use native Windows file dialog to select files to import
    OPENFILENAMEW ofn = {};
    wchar_t fileNameBuffer[32768] = {0};  // Large buffer for multiple files
    
    ofn.lStructSize = sizeof(OPENFILENAMEW);
    ofn.hwndOwner = m_hwnd;
    ofn.lpstrFile = fileNameBuffer;
    ofn.nMaxFile = 32768;
    ofn.lpstrTitle = L"Import Assets";
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_ALLOWMULTISELECT;
    
    // Set up file filters for common asset types
    const wchar_t* filters = 
        L"All Asset Files\0*.png;*.jpg;*.jpeg;*.bmp;*.tga;*.obj;*.fbx;*.dae;*.wav;*.ogg;*.mp3;*.lua;*.txt\0"
        L"Image Files\0*.png;*.jpg;*.jpeg;*.bmp;*.tga\0"
        L"3D Model Files\0*.obj;*.fbx;*.dae\0"
        L"Audio Files\0*.wav;*.ogg;*.mp3\0"
        L"Script Files\0*.lua;*.txt\0"
        L"All Files\0*.*\0\0";
    ofn.lpstrFilter = filters;
    ofn.nFilterIndex = 1;
    
    if (GetOpenFileNameW(&ofn)) {
        // Parse selected files
        std::vector<std::wstring> selectedFiles;
        const wchar_t* p = fileNameBuffer;
        std::wstring directory(p);
        p += directory.length() + 1;
        
        if (*p == 0) {
            // Single file selected
            selectedFiles.push_back(directory);
        } else {
            // Multiple files selected
            while (*p) {
                std::wstring filename(p);
                std::wstring fullPath = directory + L"\\" + filename;
                selectedFiles.push_back(fullPath);
                p += filename.length() + 1;
            }
        }
        
        // Copy files to current directory
        namespace fs = std::filesystem;
        int successCount = 0;
        int failCount = 0;
        
        for (const auto& sourceFile : selectedFiles) {
            try {
                fs::path source(sourceFile);
                fs::path dest = fs::path(m_currentPath) / source.filename();
                
                // Check if file already exists
                if (fs::exists(dest)) {
                    int result = MessageBoxW(
                        m_hwnd,
                        (L"File '" + source.filename().wstring() + L"' already exists. Overwrite?").c_str(),
                        L"File Exists",
                        MB_YESNO | MB_ICONQUESTION
                    );
                    
                    if (result != IDYES) {
                        continue;
                    }
                }
                
                // Copy the file
                fs::copy(source, dest, fs::copy_options::overwrite_existing);
                successCount++;
                
            } catch (const fs::filesystem_error& e) {
                LOG_ERROR_C("Failed to import file: " + std::string(e.what()), "Win32ContentBrowserPanel");
                failCount++;
            }
        }
        
        // Refresh the view to show imported files
        refresh();
        
        // Show result message
        std::wstring message = L"Imported " + std::to_wstring(successCount) + L" file(s)";
        if (failCount > 0) {
            message += L"\nFailed to import " + std::to_wstring(failCount) + L" file(s)";
        }
        
        MessageBoxW(m_hwnd, message.c_str(), L"Import Complete", 
                   failCount > 0 ? MB_ICONWARNING : MB_ICONINFORMATION);
        
        LOG_INFO_C("Import completed: " + std::to_string(successCount) + " success, " + 
                   std::to_string(failCount) + " failed", "Win32ContentBrowserPanel");
    }
}

void Win32ContentBrowserPanel::onBackClicked()
{
    // Navigate back in history
    if (m_historyIndex > 0) {
        m_historyIndex--;
        m_currentPath = m_navigationHistory[m_historyIndex];
        refresh();
        LOG_INFO_C("Navigated back to: " + m_currentPath, "Win32ContentBrowserPanel");
    } else {
        LOG_INFO_C("No previous location in history", "Win32ContentBrowserPanel");
    }
}

void Win32ContentBrowserPanel::onUpClicked()
{
    // Navigate to parent directory
    namespace fs = std::filesystem;
    
    try {
        fs::path current(m_currentPath);
        if (current.has_parent_path()) {
            fs::path parent = current.parent_path();
            navigateTo(parent.string());
        }
    } catch (const fs::filesystem_error& e) {
        LOG_ERROR_C("Error navigating up: " + std::string(e.what()), "Win32ContentBrowserPanel");
    }
}

void Win32ContentBrowserPanel::onRefreshClicked()
{
    refresh();
}

void Win32ContentBrowserPanel::onViewModeChanged()
{
    if (!m_listView) {
        return;
    }
    
    // Cycle through view modes: Details -> List -> Icon -> SmallIcon -> Details
    m_viewMode = (m_viewMode + 1) % 4;
    
    Win32ListView::ViewMode mode;
    std::string modeName;
    
    switch (m_viewMode) {
        case 0:
            mode = Win32ListView::ViewMode::Details;
            modeName = "Details";
            break;
        case 1:
            mode = Win32ListView::ViewMode::List;
            modeName = "List";
            break;
        case 2:
            mode = Win32ListView::ViewMode::Icon;
            modeName = "Large Icons";
            break;
        case 3:
            mode = Win32ListView::ViewMode::SmallIcon;
            modeName = "Small Icons";
            break;
        default:
            mode = Win32ListView::ViewMode::Details;
            modeName = "Details";
            break;
    }
    
    m_listView->setViewMode(mode);
    LOG_INFO_C("View mode changed to: " + modeName, "Win32ContentBrowserPanel");
}

bool Win32ContentBrowserPanel::handleMessage(UINT msg, WPARAM wParam, LPARAM lParam, LRESULT& result)
{
    switch (msg) {
        case WM_NOTIFY: {
            NMHDR* nmhdr = (NMHDR*)lParam;
            if (nmhdr->code == LVN_ITEMCHANGED) {
                NMLISTVIEW* nmlv = (NMLISTVIEW*)lParam;
                if (nmlv->uNewState & LVIS_SELECTED) {
                    onItemSelected(nmlv->iItem);
                }
            } else if (nmhdr->code == NM_DBLCLK) {
                NMITEMACTIVATE* nmia = (NMITEMACTIVATE*)lParam;
                onItemActivated(nmia->iItem);
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
                case CMD_IMPORT:
                    showImportDialog();
                    break;
                case CMD_COPY:
                    LOG_INFO_C("Copy requested", "Win32ContentBrowserPanel");
                    break;
                case CMD_PASTE:
                    LOG_INFO_C("Paste requested", "Win32ContentBrowserPanel");
                    break;
                case CMD_DELETE_FILE:
                    LOG_INFO_C("Delete requested", "Win32ContentBrowserPanel");
                    break;
                case CMD_RENAME_FILE:
                    LOG_INFO_C("Rename requested", "Win32ContentBrowserPanel");
                    break;
                case CMD_OPEN_LOCATION:
                    LOG_INFO_C("Open location requested", "Win32ContentBrowserPanel");
                    break;
                case BTN_BACK:
                    onBackClicked();
                    break;
                case BTN_UP:
                    onUpClicked();
                    break;
                case BTN_REFRESH:
                    onRefreshClicked();
                    break;
                case BTN_VIEW_MODE:
                    onViewModeChanged();
                    break;
            }
            result = 0;
            return true;
        }
    }
    
    return false; // Let base class handle it
}

} // namespace fresh

#endif // _WIN32
