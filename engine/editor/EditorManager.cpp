#include "editor/EditorManager.h"

// Prevent Windows.h from defining min/max macros (must be before any Windows includes)
#ifdef _WIN32
    #ifndef NOMINMAX
    #define NOMINMAX
    #endif
#endif

#include <algorithm>
#include <climits>
#include <cmath>
#include <iostream>
#include "core/Logger.h"
#include "core/Project.h"
#include "ecs/EntityManager.h"
#include "gameplay/Player.h"
#include "gameplay/Camera.h"
#ifdef _WIN32
    #include "core/Win32Window.h"
    #include "ui/native/Win32Toolbar.h"
    #include "input/Win32InputManager.h"
    #include <windows.h>
    #include <filesystem>
#else
    #include "core/Window.h"
    #include "input/InputManager.h"
#endif
#include "editor/WorldEditor.h"
#include "editor/SelectionManager.h"
#include "editor/SelectionRenderer.h"
#include "editor/TransformGizmo.h"
#include "editor/FileDialogManager.h"
#include "editor/LayoutManager.h"
#include "editor/EditorSettingsDialog.h"
#include "editor/CameraController.h"
#include "serialization/WorldSerializer.h"
#include "devtools/DebugRenderer.h"
#include "renderer/RenderContext.h"
#include "ui/ConsolePanel.h"
#include "ui/ContentBrowserPanel.h"
#include "ui/EditorMenuBar.h"
#include "ui/EditorToolbar.h"
#include "ui/HotbarPanel.h"
// Removed: ImGuiContext.h (ImGui has been removed from this project)
#include "ui/InspectorPanel.h"
// Removed ImGui-based panels (replaced with Win32 native versions):
// #include "ui/MainMenuPanel.h"
// #include "ui/SettingsPanel.h"
// #include "ui/EngineConfigPanel.h"
#include "ui/SceneHierarchyPanel.h"
#include "ui/VoxelToolPalette.h"
#include "voxel/VoxelWorld.h"

#include "ui/VoxelToolPalette.h"
#include "voxel/VoxelWorld.h"

#include <glm/glm.hpp>

#ifdef _WIN32
    #include "ui/WindowsThemeManager.h"
    #include "ui/WindowsDialogManager.h"
    #include "ui/WindowsTaskbarManager.h"
    // Removed ImGui-based panel: #include "ui/WindowsCustomizationPanel.h"
    #include "ui/native/Win32InspectorPanel.h"
    #include "ui/native/Win32SceneHierarchyPanel.h"
    #include "ui/native/Win32ContentBrowserPanel.h"
    #include "ui/native/Win32ConsolePanel.h"
    #include "ui/native/Win32HUD.h"
    #include "ui/native/Win32HUDOverlay.h"
    #include "ui/native/Win32SettingsDialog.h"
    #include "ui/native/Win32TerraformingPanel.h"
    #include "ui/native/Win32ViewportPanel.h"
    #include "ui/native/Win32StatusBar.h"
    #include "ui/NativeMainMenu.h"
    #include "ui/native/Win32LLMAssistantPanel.h"
#endif

namespace fresh
{

#ifdef _WIN32
// Helper function to convert wide string to narrow string safely
static std::string toNarrowString(const std::wstring& wstr)
{
    if (wstr.empty()) return std::string();
    
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), nullptr, 0, nullptr, nullptr);
    if (size_needed <= 0) return std::string();
    
    std::string strTo(size_needed, 0);
    int result = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), &strTo[0], size_needed, nullptr, nullptr);
    if (result <= 0) return std::string();
    
    return strTo;
}
#endif

// Panel layout constants - Unreal Engine style layout
namespace {
    constexpr int PANEL_MARGIN = 0;            // No spacing between panels (seamless Unreal-style layout)
    constexpr int TOOLBAR_HEIGHT = 80;         // Height reserved for menu/toolbar
    
    // Left side - Asset/Content Browser panel (narrow vertical)
    constexpr int LEFT_PANEL_WIDTH = 220;      // Asset browser on left (narrow like Unreal)
    
    // Right side - Outliner + Inspector panels (stacked vertically)
    constexpr int RIGHT_PANEL_WIDTH = 350;     // World Outliner + Inspector on right
    constexpr float OUTLINER_HEIGHT_RATIO = 0.40f;  // Outliner takes 40% of right panel height
    constexpr float INSPECTOR_HEIGHT_RATIO = 0.60f; // Inspector takes 60% of right panel height
    
    // Bottom - Content Browser + Console (horizontal strip below viewport)
    constexpr int BOTTOM_PANEL_HEIGHT = 220;   // Content Browser + Console at bottom
    constexpr int CONTENT_BROWSER_WIDTH = 700; // Content Browser width in bottom panel
    
    // Minimum sizes
    constexpr int MIN_VIEWPORT_WIDTH = 400;
    constexpr int MIN_VIEWPORT_HEIGHT = 300;
    constexpr int MIN_CONSOLE_WIDTH = 300;  // Console needs reasonable width for command input/output
    
    /**
     * @brief Helper function to show and update a Win32 panel window
     * @param hwnd Window handle to show and update
     */
    inline void showPanelWindow(HWND hwnd) {
        if (hwnd) {
            ShowWindow(hwnd, SW_SHOW);
            UpdateWindow(hwnd);
        }
    }
} // anonymous namespace

EditorManager::EditorManager()
    : m_initialized(false),
      m_visible(false),
      m_window(nullptr),
      m_renderContext(nullptr),
      m_world(nullptr),
      m_worldEditor(nullptr),
      m_entityManager(nullptr),
      m_player(nullptr),
      m_projectManager(nullptr),
      m_showSceneHierarchy(true),
      m_showInspector(true),
      m_showContentBrowser(true),
      m_showConsole(true),
      m_showToolPalette(true),
      m_showLLMAssistant(false),
      m_showGrid(false)  // Grid display initially disabled
{
}

EditorManager::~EditorManager()
{
    shutdown();
}

bool EditorManager::initialize(WindowType* window, IRenderContext* renderContext, VoxelWorld* world,
                               WorldEditor* worldEditor, InputManagerType* inputManager,
                               ecs::EntityManager* entityManager)
{
    (void)inputManager;  // Unused parameter
    if (m_initialized) {
        LOG_WARNING_C("EditorManager already initialized", "EditorManager");
        return true;
    }

    if (!window || !renderContext) {
        LOG_ERROR_C("Invalid parameters for EditorManager", "EditorManager");
        return false;
    }

    m_window = window;
    m_renderContext = renderContext;
    m_world = world;
    m_worldEditor = worldEditor;
    m_entityManager = entityManager;

#ifndef _WIN32
    // Initialize ImGui-based UI panels (non-Windows platforms)
    // These are not available on Windows which uses native Win32 UI instead
    
    // Initialize UI panels that require world/worldEditor
    // Only initialize these if world and worldEditor are available
    if (world && worldEditor) {
        m_sceneHierarchy = std::make_unique<SceneHierarchyPanel>();
        if (!m_sceneHierarchy->initialize(world)) {
            LOG_ERROR_C("Failed to initialize Scene Hierarchy panel", "EditorManager");
            return false;
        }

        m_inspector = std::make_unique<InspectorPanel>();
        if (!m_inspector->initialize(m_entityManager)) {
            LOG_ERROR_C("Failed to initialize Inspector panel", "EditorManager");
            return false;
        }

        m_menuBar = std::make_unique<EditorMenuBar>();
        if (!m_menuBar->initialize(world, worldEditor)) {
            LOG_ERROR_C("Failed to initialize Menu Bar", "EditorManager");
            return false;
        }

        // Link menu bar to panel visibility flags
        m_menuBar->setSceneHierarchyVisible(&m_showSceneHierarchy);
        m_menuBar->setInspectorVisible(&m_showInspector);
        m_menuBar->setContentBrowserVisible(&m_showContentBrowser);
        m_menuBar->setConsoleVisible(&m_showConsole);
        m_menuBar->setToolPaletteVisible(&m_showToolPalette);

        // Set settings callback to open settings panel
        // Removed: m_settingsPanel is commented out in header
        // m_menuBar->setSettingsCallback([this]() {
        //     if (m_settingsPanel) {
        //         m_settingsPanel->setVisible(true);
        //     }
        // });

        // Set engine configuration callback to open engine config panel
        // Removed: m_engineConfigPanel is commented out in header
        // m_menuBar->setEngineConfigCallback([this]() {
        //     if (m_engineConfigPanel) {
        //         m_engineConfigPanel->setVisible(true);
        //     }
        // });
        
        // Set editor settings callback to open editor settings dialog
        m_menuBar->setEditorSettingsCallback([this]() {
            if (m_editorSettingsDialog) {
                m_editorSettingsDialog->setVisible(true);
            }
        });

        // Set import assets callback to show import dialog in content browser
        m_menuBar->setImportAssetsCallback([this]() {
            if (m_contentBrowser) {
                m_contentBrowser->showImportDialog();
            }
        });

        // Set dialogue editor callback
        m_menuBar->setDialogueEditorCallback([this]() {
            launchDialogueEditor();
        });

        // Set select all/deselect all callbacks for scene hierarchy
        m_menuBar->setSelectAllCallback([this]() {
            if (m_sceneHierarchy) {
                m_sceneHierarchy->selectAll();
            }
        });

        m_menuBar->setDeselectAllCallback([this]() {
            if (m_sceneHierarchy) {
                m_sceneHierarchy->deselectAll();
            }
        });
        
        // Set up layout management callbacks
        m_menuBar->setLoadLayoutCallback([this](const std::string& name) {
            loadLayout(name);
        });
        
        m_menuBar->setSaveLayoutCallback([this]() {
            saveCurrentLayout();
        });
        
        m_menuBar->setResetLayoutCallback([this]() {
            resetLayout();
        });

        // Set undo/redo callbacks for terraforming
        m_menuBar->setUndoCallback([this, worldEditor]() {
            if (worldEditor && worldEditor->getTerraformingSystem()) {
                if (worldEditor->getTerraformingSystem()->undo()) {
                    LOG_INFO_C("Undo performed", "EditorManager");
                } else {
                    LOG_INFO_C("Nothing to undo", "EditorManager");
                }
            }
        });

        m_menuBar->setRedoCallback([this, worldEditor]() {
            if (worldEditor && worldEditor->getTerraformingSystem()) {
                if (worldEditor->getTerraformingSystem()->redo()) {
                    LOG_INFO_C("Redo performed", "EditorManager");
                } else {
                    LOG_INFO_C("Nothing to redo", "EditorManager");
                }
            }
        });

        // Set Cut/Copy/Paste/Delete callbacks for selection manager
        m_menuBar->setCutCallback([this, world]() {
            if (m_selectionManager && m_selectionManager->hasSelection()) {
                m_selectionManager->cutToClipboard(world);
                LOG_INFO_C("Cut selection to clipboard", "EditorManager");
            } else {
                LOG_INFO_C("No selection to cut", "EditorManager");
            }
        });

        m_menuBar->setCopyCallback([this, world]() {
            if (m_selectionManager && m_selectionManager->hasSelection()) {
                m_selectionManager->copyToClipboard(world);
                LOG_INFO_C("Copied selection to clipboard", "EditorManager");
            } else {
                LOG_INFO_C("No selection to copy", "EditorManager");
            }
        });

        m_menuBar->setPasteCallback([this, world]() {
            if (m_selectionManager && m_selectionManager->hasClipboardData()) {
                // Get paste position from camera/player if available
                glm::ivec3 pastePos(0, 64, 0);  // Default position
                
                if (m_player) {
                    // Use player's camera position as the paste location
                    const auto& camera = m_player->getCamera();
                    glm::vec3 cameraPos = camera.getPosition();
                    // Offset slightly forward from camera
                    glm::vec3 forward = camera.getFront();
                    glm::vec3 targetPos = cameraPos + forward * 5.0f;
                    pastePos = glm::ivec3(
                        static_cast<int>(std::floor(targetPos.x)),
                        static_cast<int>(std::floor(targetPos.y)),
                        static_cast<int>(std::floor(targetPos.z))
                    );
                }
                
                m_selectionManager->pasteFromClipboard(pastePos, world);
                LOG_INFO_C("Pasted clipboard content at " + 
                          std::to_string(pastePos.x) + ", " + 
                          std::to_string(pastePos.y) + ", " + 
                          std::to_string(pastePos.z), "EditorManager");
            } else {
                LOG_INFO_C("Clipboard is empty", "EditorManager");
            }
        });

        m_menuBar->setDeleteCallback([this, world]() {
            if (m_selectionManager && m_selectionManager->hasSelection()) {
                m_selectionManager->deleteSelected(world);
                LOG_INFO_C("Deleted selection", "EditorManager");
            } else {
                LOG_INFO_C("No selection to delete", "EditorManager");
            }
        });

        // Note: File dialog callbacks will be set up after WindowsDialogManager is initialized
        // See below in Windows-specific initialization section

        m_toolbar = std::make_unique<EditorToolbar>();
        if (!m_toolbar->initialize()) {
            LOG_ERROR_C("Failed to initialize Toolbar", "EditorManager");
            return false;
        }
        
        // Set up toolbar callbacks for transform gizmo mode switching
        m_toolbar->setToolCallback([this](EditorToolbar::Tool tool) {
            if (!m_transformGizmo) return;
            
            switch (tool) {
                case EditorToolbar::Tool::Move:
                    m_transformGizmo->setMode(TransformGizmo::Mode::Translate);
                    LOG_INFO_C("Toolbar: Transform Gizmo set to Translate mode", "EditorManager");
                    break;
                case EditorToolbar::Tool::Rotate:
                    m_transformGizmo->setMode(TransformGizmo::Mode::Rotate);
                    LOG_INFO_C("Toolbar: Transform Gizmo set to Rotate mode", "EditorManager");
                    break;
                case EditorToolbar::Tool::Scale:
                    m_transformGizmo->setMode(TransformGizmo::Mode::Scale);
                    LOG_INFO_C("Toolbar: Transform Gizmo set to Scale mode", "EditorManager");
                    break;
                default:
                    // Other tools don't affect the transform gizmo
                    break;
            }
        });

        m_contentBrowser = std::make_unique<ContentBrowserPanel>();
        if (!m_contentBrowser->initialize("assets")) {
            LOG_ERROR_C("Failed to initialize Content Browser panel", "EditorManager");
            return false;
        }

        m_console = std::make_unique<ConsolePanel>();
        if (!m_console->initialize()) {
            LOG_ERROR_C("Failed to initialize Console panel", "EditorManager");
            return false;
        }

        m_voxelTools = std::make_unique<VoxelToolPalette>();
        if (!m_voxelTools->initialize(worldEditor->getTerraformingSystem())) {
            LOG_ERROR_C("Failed to initialize Voxel Tool Palette", "EditorManager");
            return false;
        }

        // Initialize debug renderer for selection visualization
        m_debugRenderer = std::make_unique<devtools::DebugRenderer>();
        m_debugRenderer->setEnabled(true);
        LOG_INFO_C("Debug Renderer initialized", "EditorManager");

        // Initialize selection manager
        m_selectionManager = std::make_unique<SelectionManager>();
        LOG_INFO_C("Selection Manager initialized", "EditorManager");

        // Initialize selection renderer
        m_selectionRenderer = std::make_unique<SelectionRenderer>();
        if (!m_selectionRenderer->initialize(m_debugRenderer.get())) {
            LOG_ERROR_C("Failed to initialize Selection Renderer", "EditorManager");
            return false;
        }
        LOG_INFO_C("Selection Renderer initialized", "EditorManager");
        
        // Initialize transform gizmo
        m_transformGizmo = std::make_unique<TransformGizmo>();
        m_transformGizmo->setDebugRenderer(m_debugRenderer.get());
        LOG_INFO_C("Transform Gizmo initialized", "EditorManager");
        
        // Initialize world serializer
        m_worldSerializer = std::make_unique<WorldSerializer>();
        LOG_INFO_C("World Serializer initialized", "EditorManager");
        
        // Initialize layout manager
        m_layoutManager = std::make_unique<LayoutManager>();
        m_layoutManager->initialize("configs/layouts.ini");
        
        // Initialize editor settings dialog
        m_editorSettingsDialog = std::make_unique<EditorSettingsDialog>();
        m_editorSettingsDialog->initialize("configs/editor_settings.ini");
        LOG_INFO_C("Editor Settings Dialog initialized", "EditorManager");
        
        // Initialize camera controller
        m_cameraController = std::make_unique<CameraController>();
        // Camera controller will be fully initialized when player is set via setPlayer()
        LOG_INFO_C("Camera Controller created (waiting for player)", "EditorManager");
        
        // Load the current or default layout
        LayoutConfig currentLayout;
        if (m_layoutManager->loadLayout(m_layoutManager->getCurrentLayoutName(), currentLayout)) {
            m_showSceneHierarchy = currentLayout.showSceneHierarchy;
            m_showInspector = currentLayout.showInspector;
            m_showContentBrowser = currentLayout.showContentBrowser;
            m_showConsole = currentLayout.showConsole;
            m_showToolPalette = currentLayout.showToolPalette;
            LOG_INFO_C("Loaded layout: " + m_layoutManager->getCurrentLayoutName(), "EditorManager");
        }
    } else {
        LOG_INFO_C("World and WorldEditor not provided, deferring initialization of world-dependent panels", "EditorManager");
    }

    // Removed ImGui-based panels (replaced with Win32 native versions):
    // - MainMenuPanel -> replaced with Win32 native main menu
    // - SettingsPanel -> replaced with Win32SettingsDialog
    // - EngineConfigPanel -> replaced with Win32 property panels

    // Initialize hotbar panel (for play mode)
    m_hotbar = std::make_unique<HotbarPanel>();
    if (!m_hotbar->initialize()) {
        LOG_ERROR_C("Failed to initialize Hotbar Panel", "EditorManager");
        return false;
    }
    m_hotbar->setVisible(false); // Hidden by default (shown only in play mode)
#endif // _WIN32

#ifdef _WIN32
    // Initialize Windows-native integration features
    LOG_INFO_C("Initializing Windows-native integration features", "EditorManager");
    
    // Initialize Windows Theme Manager
    m_windowsThemeManager = std::make_unique<WindowsThemeManager>();
    if (m_windowsThemeManager->initialize()) {
        // Force dark theme for better readability regardless of Windows settings
        // This ensures consistent dark background with white text
        m_windowsThemeManager->setTheme(WindowsTheme::Dark);
        LOG_INFO_C("Windows Theme Manager initialized with Dark theme", "EditorManager");
    } else {
        LOG_WARNING_C("Failed to initialize Windows Theme Manager", "EditorManager");
    }
    
    // Initialize Windows Dialog Manager
    m_windowsDialogManager = std::make_unique<WindowsDialogManager>();
    void* nativeHandle = window->getNativeWindowHandle();
    if (nativeHandle && m_windowsDialogManager->initialize(nativeHandle)) {
        LOG_INFO_C("Windows Dialog Manager initialized", "EditorManager");
    } else {
        LOG_WARNING_C("Failed to initialize Windows Dialog Manager", "EditorManager");
    }
    
    // Initialize Windows Taskbar Manager
    m_windowsTaskbarManager = std::make_unique<WindowsTaskbarManager>();
    if (nativeHandle && m_windowsTaskbarManager->initialize(nativeHandle)) {
        LOG_INFO_C("Windows Taskbar Manager initialized", "EditorManager");
    } else {
        LOG_WARNING_C("Failed to initialize Windows Taskbar Manager", "EditorManager");
    }
    
    // Removed ImGui-based WindowsCustomizationPanel (replaced with native Win32 settings dialogs)
    
#ifdef FRESH_WIN32_UI
    // Initialize native Win32 UI panels (replaces ImGui panels)
    LOG_INFO_C("Initializing native Win32 UI panels", "EditorManager");
    
    // Get Win32 window handle
    Win32Window* win32Window = dynamic_cast<Win32Window*>(window);
    if (win32Window) {
        HWND hwnd = win32Window->getHandle();
        
        // Get window client area size for layout calculations
        RECT clientRect;
        GetClientRect(hwnd, &clientRect);
        int clientWidth = clientRect.right - clientRect.left;
        int clientHeight = clientRect.bottom - clientRect.top;
        
        LOG_INFO_C("Client area at initialization: " + std::to_string(clientWidth) + "x" + 
                   std::to_string(clientHeight), "EditorManager");
        
        // Get actual toolbar height for layout calculations
        // The toolbar is a child window that must be accounted for in panel positioning
        int actualToolbarHeight = getActualToolbarHeight();
        LOG_INFO_C("Actual toolbar height: " + std::to_string(actualToolbarHeight), "EditorManager");
        
        // Calculate panel positions - Unreal Engine style layout:
        // - Left: Asset browser (narrow vertical panel)
        // - Center: Viewport (large central area)
        // - Right: World Outliner (top) + Inspector (bottom) stacked vertically
        // - Bottom: Content Browser + Console (horizontal strip below viewport)
        // 
        // Window size requirements:
        // - Minimum recommended: 1280x720 for comfortable editing
        // - Absolute minimum: 800x600 (panels may overlap or become cramped)
        // - Below 800x600: Layout degrades - viewport/console enforced to MIN sizes
        // - Fixed-width panels (left 220px, right 350px) always maintain size
        // - Dynamic panels (viewport, console) adjust to fill available space
        
        // Left panel - Asset/Terraforming tools (narrow vertical, full height minus bottom panel)
        int leftPanelX = PANEL_MARGIN;
        int leftPanelY = actualToolbarHeight;
        int leftPanelHeight = clientHeight - actualToolbarHeight - BOTTOM_PANEL_HEIGHT - PANEL_MARGIN * 2;
        
        // Right panel - split vertically between Outliner (top) and Inspector (bottom)
        int rightPanelX = clientWidth - RIGHT_PANEL_WIDTH - PANEL_MARGIN;
        int rightPanelY = actualToolbarHeight;
        int rightPanelUsableHeight = clientHeight - actualToolbarHeight - BOTTOM_PANEL_HEIGHT - PANEL_MARGIN * 2;
        int outlinerHeight = static_cast<int>(rightPanelUsableHeight * OUTLINER_HEIGHT_RATIO);
        // Inspector height is derived rather than calculated separately to ensure heights sum exactly
        // to rightPanelUsableHeight (accounting for margin), avoiding pixel gaps from rounding errors
        int inspectorHeight = rightPanelUsableHeight - outlinerHeight - PANEL_MARGIN;
        
        // Bottom panel - Content Browser + Console (horizontal strip)
        int bottomPanelY = clientHeight - BOTTOM_PANEL_HEIGHT - PANEL_MARGIN;
        
        // Viewport - center area between left and right panels
        int viewportX = LEFT_PANEL_WIDTH + PANEL_MARGIN * 2;
        int viewportY = actualToolbarHeight;
        int viewportWidth = std::max(MIN_VIEWPORT_WIDTH, rightPanelX - viewportX - PANEL_MARGIN);
        int viewportHeight = std::max(MIN_VIEWPORT_HEIGHT, bottomPanelY - actualToolbarHeight - PANEL_MARGIN);
        
        // Create native Viewport Panel FIRST (central 3D view) - positioned in center between left and right panels
        // IMPORTANT: Create viewport first so it's at the bottom of the Z-order, allowing UI panels to be visible on top
        m_viewportPanel = std::make_unique<Win32ViewportPanel>();
        if (m_viewportPanel->create(hwnd, viewportX, viewportY, viewportWidth, viewportHeight)) {
            LOG_INFO_C("Native Win32 Viewport Panel created at (" + 
                      std::to_string(viewportX) + ", " + std::to_string(viewportY) + ") " +
                      "size " + std::to_string(viewportWidth) + "x" + std::to_string(viewportHeight), "EditorManager");
            // Explicitly show viewport panel from startup
            showPanelWindow(m_viewportPanel->getHandle());
        } else {
            LOG_ERROR_C("Failed to create Native Win32 Viewport Panel", "EditorManager");
        }
        
        // Create native Terraforming Panel (left side, where asset browser would be in Unreal)
        // This serves as our "tools" panel similar to Unreal's left panel
        if (worldEditor) {
            m_nativeTerraformingPanel = std::make_unique<Win32TerraformingPanel>();
            if (m_nativeTerraformingPanel->initialize(hwnd, worldEditor)) {
                // Position on the left side
                m_nativeTerraformingPanel->setPosition(leftPanelX, leftPanelY);
                m_nativeTerraformingPanel->setSize(LEFT_PANEL_WIDTH, leftPanelHeight);
                LOG_INFO_C("Native Win32 Terraforming Panel created (left side)", "EditorManager");
                // Explicitly show terraforming panel from startup
                showPanelWindow(m_nativeTerraformingPanel->getHandle());
            }
        }
        
        // Create native Scene Hierarchy panel (right side, top position - World Outliner)
        m_nativeSceneHierarchy = std::make_unique<Win32SceneHierarchyPanel>();
        if (m_nativeSceneHierarchy->create(hwnd, rightPanelX, rightPanelY, RIGHT_PANEL_WIDTH, outlinerHeight, world)) {
            // Set selection callback to update inspector
            m_nativeSceneHierarchy->setSelectionCallback([this](HierarchyNode* node) {
                if (m_nativeInspector) {
                    m_nativeInspector->setInspectedNode(node);
                }
            });
            LOG_INFO_C("Native Win32 Scene Hierarchy Panel created (right top)", "EditorManager");
            // Explicitly show scene hierarchy panel from startup
            showPanelWindow(m_nativeSceneHierarchy->getHandle());
        }
        
        // Create native Inspector panel (right side, bottom position - Details panel)
        int inspectorY = rightPanelY + outlinerHeight + PANEL_MARGIN;
        m_nativeInspector = std::make_unique<Win32InspectorPanel>();
        if (m_nativeInspector->create(hwnd, rightPanelX, inspectorY, RIGHT_PANEL_WIDTH, inspectorHeight, m_entityManager)) {
            LOG_INFO_C("Native Win32 Inspector Panel created (right bottom)", "EditorManager");
            // Explicitly show inspector panel from startup
            showPanelWindow(m_nativeInspector->getHandle());
        }
        
        // Create native Content Browser panel (bottom left)
        m_nativeContentBrowser = std::make_unique<Win32ContentBrowserPanel>();
        if (m_nativeContentBrowser->create(hwnd, PANEL_MARGIN, bottomPanelY, CONTENT_BROWSER_WIDTH, BOTTOM_PANEL_HEIGHT, "assets")) {
            LOG_INFO_C("Native Win32 Content Browser Panel created (bottom left)", "EditorManager");
            // Explicitly show content browser panel from startup
            showPanelWindow(m_nativeContentBrowser->getHandle());
        }
        
        // Create native Console panel (bottom right, next to content browser)
        int consoleX = PANEL_MARGIN + CONTENT_BROWSER_WIDTH + PANEL_MARGIN;
        int consoleWidth = std::max(MIN_CONSOLE_WIDTH, clientWidth - consoleX - PANEL_MARGIN);
        m_nativeConsole = std::make_unique<Win32ConsolePanel>();
        if (m_nativeConsole->create(hwnd, consoleX, bottomPanelY, consoleWidth, BOTTOM_PANEL_HEIGHT)) {
            LOG_INFO_C("Native Win32 Console Panel created (bottom right)", "EditorManager");
            
            // Set up command callback for console command execution
            m_nativeConsole->setCommandCallback([this](const std::string& command) {
                handleConsoleCommand(command);
            });
            
            // Explicitly show console panel from startup
            showPanelWindow(m_nativeConsole->getHandle());
        }
        
        // Create native LLM Assistant panel (shares bottom-right area, hidden by default)
        // The assistant panel overlays the console area when shown via toggle
        m_nativeLLMAssistant = std::make_unique<Win32LLMAssistantPanel>();
        if (m_nativeLLMAssistant->create(hwnd, consoleX, bottomPanelY, consoleWidth, BOTTOM_PANEL_HEIGHT)) {
            LOG_INFO_C("Native Win32 LLM Assistant Panel created", "EditorManager");
            m_nativeLLMAssistant->setVisible(false); // Hidden by default, toggled via menu
        }
        
        // Create native HUD (for play mode)
        m_nativeHUD = std::make_unique<Win32HUD>();
        if (m_nativeHUD->initialize(hwnd)) {
            m_nativeHUD->setVisible(false); // Hidden by default
            LOG_INFO_C("Native Win32 HUD created", "EditorManager");
            
            // Create overlay window for HUD rendering
            m_hudOverlay = std::make_unique<Win32HUDOverlay>();
            if (m_hudOverlay->initialize(hwnd, m_nativeHUD.get())) {
                LOG_INFO_C("Win32 HUD overlay window created", "EditorManager");
            } else {
                LOG_WARNING_C("Failed to create HUD overlay window", "EditorManager");
            }
        }
        
        // Create native Settings Dialog
        m_nativeSettingsDialog = std::make_unique<Win32SettingsDialog>();
        LOG_INFO_C("Native Win32 Settings Dialog created", "EditorManager");
        
        // Create native Status Bar at bottom of window
        m_statusBar = std::make_unique<Win32StatusBar>();
        if (m_statusBar->create(hwnd, 4)) {  // 4 panes: Status | Position | Selection | FPS
            m_statusBar->setStatusText(L"Ready");
            LOG_INFO_C("Native Win32 Status Bar created", "EditorManager");
            // Explicitly show status bar from startup
            showPanelWindow(m_statusBar->getHandle());
        } else {
            LOG_ERROR_C("Failed to create Native Win32 Status Bar", "EditorManager");
        }
        
        // Ensure proper Z-order: UI panels should be on top of the viewport
        // Since viewport was created first, explicitly bring UI panels to front
        ensurePanelsOnTop();
        
        // Force initial paint of all panels by invalidating their client areas
        // This ensures panels are visible immediately on startup
        if (m_nativeInspector && m_nativeInspector->getHandle()) {
            InvalidateRect(m_nativeInspector->getHandle(), nullptr, TRUE);
        }
        if (m_nativeSceneHierarchy && m_nativeSceneHierarchy->getHandle()) {
            InvalidateRect(m_nativeSceneHierarchy->getHandle(), nullptr, TRUE);
        }
        if (m_nativeContentBrowser && m_nativeContentBrowser->getHandle()) {
            InvalidateRect(m_nativeContentBrowser->getHandle(), nullptr, TRUE);
        }
        if (m_nativeConsole && m_nativeConsole->getHandle()) {
            InvalidateRect(m_nativeConsole->getHandle(), nullptr, TRUE);
        }
        if (m_viewportPanel && m_viewportPanel->getHandle()) {
            InvalidateRect(m_viewportPanel->getHandle(), nullptr, TRUE);
        }
        if (m_statusBar && m_statusBar->getHandle()) {
            InvalidateRect(m_statusBar->getHandle(), nullptr, TRUE);
        }
        if (m_nativeTerraformingPanel && m_nativeTerraformingPanel->getHandle()) {
            InvalidateRect(m_nativeTerraformingPanel->getHandle(), nullptr, TRUE);
        }
        
        // Also invalidate the main window to ensure proper initial layout
        InvalidateRect(hwnd, nullptr, TRUE);
        UpdateWindow(hwnd);
        
        LOG_INFO_C("All native Win32 UI panels initialized successfully", "EditorManager");
    } else {
        LOG_WARNING_C("Win32Window not available, native panels not created", "EditorManager");
    }
#endif // FRESH_WIN32_UI
    
#endif // _WIN32

    // Connect scene hierarchy to inspector
    // When a node is selected in the hierarchy, show it in the inspector
    // This would require adding a callback system, but for now we'll handle it in render()

    m_initialized = true;
    LOG_INFO_C("EditorManager initialized successfully", "EditorManager");
    LOG_INFO_C("All editor UI panels initialized", "EditorManager");
    return true;
}

void EditorManager::beginFrame()
{
    if (!m_initialized) {
        return;
    }
}

void EditorManager::update(float deltaTime)
{
    // Silence unreferenced parameter warning
    (void)deltaTime;
    
    if (!m_initialized) {
        return;
    }

#ifdef _WIN32
    // Update HUD and overlay visibility/position
    if (m_nativeHUD && m_hudOverlay && m_hudOverlay->isInitialized()) {
        bool hudVisible = m_nativeHUD->isVisible();
        
        // Show or hide overlay based on HUD visibility
        m_hudOverlay->setVisible(hudVisible);
        
        if (hudVisible && m_player) {
            // Update HUD with current player stats
            Win32HUD::HUDStats stats;
            stats.health = m_player->getHealth();
            stats.maxHealth = m_player->getMaxHealth();
            stats.stamina = m_player->getStamina();
            stats.maxStamina = m_player->getMaxStamina();
            
            // Get player position
            auto pos = m_player->getPosition();
            stats.posX = pos.x;
            stats.posY = pos.y;
            stats.posZ = pos.z;
            
            // FPS can be set from Engine
            // stats.fps is updated elsewhere
            
            m_nativeHUD->updateStats(stats);
            
            // Update overlay position and trigger repaint
            m_hudOverlay->updatePosition();
            m_hudOverlay->invalidate();
        }
    }
    
    // Update Status Bar with current editor state
    if (m_statusBar) {
        // Update position display
        if (m_player) {
            auto pos = m_player->getPosition();
            m_statusBar->setCursorPosition(pos.x, pos.y, pos.z);
        }
        
        // Update selection count
        if (m_selectionManager && m_selectionManager->hasSelection()) {
            size_t selectionSize = m_selectionManager->getSelectionSize();
            // Clamp to INT_MAX to prevent overflow
            int selectionCount = (selectionSize > static_cast<size_t>(INT_MAX)) ? 
                                INT_MAX : static_cast<int>(selectionSize);
            m_statusBar->setSelectionInfo(selectionCount, L"voxels");
        } else {
            m_statusBar->setSelectionInfo(0, L"voxels");
        }
        
        // Update current tool/mode information
        if (m_worldEditor && m_worldEditor->getTerraformingSystem()) {
            auto* terraform = m_worldEditor->getTerraformingSystem();
            std::wstring toolInfo = L"Tool: ";
            
            // Get current tool name
            switch(terraform->getCurrentTool()) {
                case TerraformTool::SingleBlock: toolInfo += L"Single Block"; break;
                case TerraformTool::Brush: toolInfo += L"Brush"; break;
                case TerraformTool::Sphere: toolInfo += L"Sphere"; break;
                case TerraformTool::FilledSphere: toolInfo += L"Filled Sphere"; break;
                case TerraformTool::Cube: toolInfo += L"Cube"; break;
                case TerraformTool::FilledCube: toolInfo += L"Filled Cube"; break;
                case TerraformTool::Line: toolInfo += L"Line"; break;
                case TerraformTool::Flatten: toolInfo += L"Flatten"; break;
                case TerraformTool::Smooth: toolInfo += L"Smooth"; break;
                case TerraformTool::Paint: toolInfo += L"Paint"; break;
                default: toolInfo += L"Unknown"; break;
            }
            
            // Add mode
            toolInfo += L" (";
            switch(terraform->getCurrentMode()) {
                case TerraformMode::Place: toolInfo += L"Place"; break;
                case TerraformMode::Remove: toolInfo += L"Remove"; break;
                case TerraformMode::Replace: toolInfo += L"Replace"; break;
                default: toolInfo += L"Unknown"; break;
            }
            toolInfo += L")";
            
            m_statusBar->setStatusText(toolInfo);
        } else {
            m_statusBar->setStatusText(L"Ready");
        }
    }
#endif
}

void EditorManager::render()
{
    if (!m_initialized || !m_visible) {
        return;
    }

    // Render panels based on visibility flags
#ifndef FRESH_WIN32_UI
    // Use panels when native Win32 UI is not available
    if (m_showSceneHierarchy && m_sceneHierarchy) {
        m_sceneHierarchy->render();

        // Connect to inspector
        if (m_inspector) {
            auto* selectedNode = m_sceneHierarchy->getSelectedNode();
            m_inspector->setInspectedNode(selectedNode);
        }
    }

    if (m_showInspector && m_inspector) {
        m_inspector->render();
    }

    if (m_showContentBrowser && m_contentBrowser) {
        m_contentBrowser->render();
    }

    if (m_showConsole && m_console) {
        m_console->render();
    }
#else
    // When using native Win32 UI, native panels are always visible and managed by Win32
    // No need to render them here - they are native windows
    // The native panels were created during initialization and handle their own rendering
#endif // !FRESH_WIN32_UI

    if (m_showToolPalette && m_voxelTools) {
        m_voxelTools->render();
    }

    // Removed: m_mainMenuPanel is commented out in header
    // if (m_mainMenuPanel) {
    //     m_mainMenuPanel->render();
    // }

    // Removed: m_settingsPanel is commented out in header
    // if (m_settingsPanel) {
    //     m_settingsPanel->render();
    // }

    // Removed: m_engineConfigPanel is commented out in header
    // if (m_engineConfigPanel) {
    //     m_engineConfigPanel->render();
    // }
    
    // Render editor settings dialog if visible
    if (m_editorSettingsDialog) {
        m_editorSettingsDialog->render();
    }

#ifndef FRESH_WIN32_UI
    // Render hotbar (shown in play mode)
    // On Windows with FRESH_WIN32_UI, use native Win32HUD instead
    if (m_hotbar) {
        m_hotbar->render();
    }
#endif // !FRESH_WIN32_UI

    // Render selection visualization (3D rendering, not UI)
    if (m_selectionRenderer && m_selectionManager) {
        m_selectionRenderer->render(m_selectionManager.get());
    }

    // Update and render debug visualization
    if (m_debugRenderer) {
        m_debugRenderer->render();
    }

#ifdef _WIN32
    // Removed: m_windowsCustomizationPanel is commented out in header
    // if (m_windowsCustomizationPanel) {
    //     m_windowsCustomizationPanel->render();
    // }
#endif
}

void EditorManager::endFrame()
{
    if (!m_initialized) {
        return;
    }
}

void EditorManager::shutdown()
{
    if (!m_initialized) {
        return;
    }

    // Shutdown in reverse order
    m_hotbar.reset();
    // Removed: m_settingsPanel is commented out in header
    // m_settingsPanel.reset();
    // Removed: m_mainMenuPanel is commented out in header
    // m_mainMenuPanel.reset();
    m_voxelTools.reset();
    m_console.reset();
    m_contentBrowser.reset();
    m_toolbar.reset();
    m_menuBar.reset();
    m_inspector.reset();
    m_sceneHierarchy.reset();

#ifdef _WIN32
    // Shutdown Windows-native features
    // Removed: m_windowsCustomizationPanel is commented out in header
    // if (m_windowsCustomizationPanel) {
    //     m_windowsCustomizationPanel.reset();
    // }
    if (m_windowsTaskbarManager) {
        m_windowsTaskbarManager->shutdown();
        m_windowsTaskbarManager.reset();
    }
    if (m_windowsDialogManager) {
        m_windowsDialogManager->shutdown();
        m_windowsDialogManager.reset();
    }
    if (m_windowsThemeManager) {
        m_windowsThemeManager->shutdown();
        m_windowsThemeManager.reset();
    }
#endif

    // Clear all pointer references to avoid dangling pointers
    m_window = nullptr;
    m_renderContext = nullptr;
    m_world = nullptr;
    m_worldEditor = nullptr;

    m_initialized = false;
    m_visible = false; // Reset visibility flag on shutdown
    LOG_INFO_C("EditorManager shutdown complete", "EditorManager");
}

void EditorManager::setPlayer(Player* player)
{
    m_player = player;
    
    // Initialize camera controller when player is set
    if (m_cameraController && player) {
        m_cameraController->initialize(player);
        LOG_INFO_C("Camera controller initialized with player", "EditorManager");
    }
}

void EditorManager::setProjectManager(ProjectManager* projectManager)
{
    m_projectManager = projectManager;
    LOG_INFO_C("Project manager reference set in EditorManager", "EditorManager");
}

bool EditorManager::updateWorld(VoxelWorld* world, WorldEditor* worldEditor)
{
    if (!m_initialized) {
        LOG_ERROR_C("EditorManager not initialized, cannot update world", "EditorManager");
        return false;
    }

    if (!world || !worldEditor) {
        LOG_ERROR_C("Invalid world or worldEditor parameters", "EditorManager");
        return false;
    }

    // Windows native UI - update world reference
    m_world = world;
    m_worldEditor = worldEditor;
    
#ifdef _WIN32
    // Create world-dependent panels if they don't exist yet
    // This handles the case where EditorManager was initialized without a world
    Win32Window* win32Window = dynamic_cast<Win32Window*>(m_window);
    if (win32Window) {
        HWND hwnd = win32Window->getHandle();
        
        // Create Terraforming Panel if it doesn't exist
        if (!m_nativeTerraformingPanel && worldEditor) {
            // Get window client area size for layout calculations
            RECT clientRect;
            GetClientRect(hwnd, &clientRect);
            int clientHeight = clientRect.bottom - clientRect.top;
            
            // Get actual toolbar height
            int actualToolbarHeight = getActualToolbarHeight();
            
            // Calculate panel position (same as in initialize)
            int leftPanelX = PANEL_MARGIN;
            int leftPanelY = actualToolbarHeight;
            int leftPanelHeight = clientHeight - actualToolbarHeight - BOTTOM_PANEL_HEIGHT - PANEL_MARGIN * 2;
            
            // Create native Terraforming Panel
            m_nativeTerraformingPanel = std::make_unique<Win32TerraformingPanel>();
            if (m_nativeTerraformingPanel->initialize(hwnd, worldEditor)) {
                m_nativeTerraformingPanel->setPosition(leftPanelX, leftPanelY);
                m_nativeTerraformingPanel->setSize(LEFT_PANEL_WIDTH, leftPanelHeight);
                LOG_INFO_C("Native Win32 Terraforming Panel created after world update", "EditorManager");
                
                // Ensure panel is visible and painted
                HWND panelHwnd = m_nativeTerraformingPanel->getHandle();
                ShowWindow(panelHwnd, SW_SHOW);
                InvalidateRect(panelHwnd, nullptr, TRUE);
                UpdateWindow(panelHwnd);
            } else {
                LOG_ERROR_C("Failed to create Terraforming Panel after world update", "EditorManager");
            }
        }
        
        // Update Scene Hierarchy with new world data and refresh it
        if (m_nativeSceneHierarchy && m_showSceneHierarchy) {
            // Update the world reference and refresh the tree view
            m_nativeSceneHierarchy->setWorld(world);
            m_nativeSceneHierarchy->refresh();
            
            // Ensure panel is visible and updated
            HWND hierarchyHwnd = m_nativeSceneHierarchy->getHandle();
            if (hierarchyHwnd) {
                ShowWindow(hierarchyHwnd, SW_SHOW);
                InvalidateRect(hierarchyHwnd, nullptr, TRUE);
                UpdateWindow(hierarchyHwnd);
            }
            LOG_INFO_C("Scene Hierarchy updated and refreshed with new world", "EditorManager");
            std::cout << "✓ Scene Hierarchy populated with world data" << std::endl;
        } else {
            if (!m_nativeSceneHierarchy) {
                LOG_WARNING_C("Scene Hierarchy panel not created - cannot update with world", "EditorManager");
                std::cerr << "WARNING: Scene Hierarchy panel is missing!" << std::endl;
            }
            if (!m_showSceneHierarchy) {
                LOG_INFO_C("Scene Hierarchy is hidden - skipping update", "EditorManager");
            }
        }
        
        // Ensure all other panels are visible (they should have been created during initialize)
        // but explicitly show them based on visibility flags
        if (m_nativeInspector && m_showInspector) {
            HWND inspectorHwnd = m_nativeInspector->getHandle();
            if (inspectorHwnd) {
                ShowWindow(inspectorHwnd, SW_SHOW);
                InvalidateRect(inspectorHwnd, nullptr, TRUE);
                UpdateWindow(inspectorHwnd);
            }
        }
        
        if (m_nativeContentBrowser && m_showContentBrowser) {
            HWND contentHwnd = m_nativeContentBrowser->getHandle();
            if (contentHwnd) {
                ShowWindow(contentHwnd, SW_SHOW);
                InvalidateRect(contentHwnd, nullptr, TRUE);
                UpdateWindow(contentHwnd);
            }
        }
        
        if (m_nativeConsole && m_showConsole) {
            HWND consoleHwnd = m_nativeConsole->getHandle();
            if (consoleHwnd) {
                ShowWindow(consoleHwnd, SW_SHOW);
                InvalidateRect(consoleHwnd, nullptr, TRUE);
                UpdateWindow(consoleHwnd);
            }
        }
        
        // Viewport and Status Bar should always be visible (no visibility flags for these)
        if (m_viewportPanel) {
            HWND viewportHwnd = m_viewportPanel->getHandle();
            if (viewportHwnd) {
                ShowWindow(viewportHwnd, SW_SHOW);
                InvalidateRect(viewportHwnd, nullptr, TRUE);
                UpdateWindow(viewportHwnd);
            }
        }
        
        if (m_statusBar) {
            HWND statusHwnd = m_statusBar->getHandle();
            if (statusHwnd) {
                ShowWindow(statusHwnd, SW_SHOW);
                InvalidateRect(statusHwnd, nullptr, TRUE);
                UpdateWindow(statusHwnd);
            }
        }
        
        // Ensure proper Z-order: UI panels should be on top of the viewport
        ensurePanelsOnTop();
        
        // Force main window to redraw all children
        InvalidateRect(hwnd, nullptr, TRUE);
        UpdateWindow(hwnd);
        
        LOG_INFO_C("All editor panels ensured visible and brought to front after world update", "EditorManager");
    }
#endif
    
    LOG_INFO_C("EditorManager updated with new world (Windows native UI)", "EditorManager");
    return true;
}

bool EditorManager::wantCaptureMouse() const
{
#ifdef _WIN32
    // Check if the mouse cursor is over any editor UI panel (not the viewport).
    // If the cursor is over a UI panel, the editor wants to capture mouse input
    // so that clicks do not leak through to the viewport/scene.
    if (!m_visible) {
        return false;
    }

    POINT pt;
    if (!GetCursorPos(&pt)) {
        return false;
    }

    // If the mouse is inside the viewport, the editor does NOT want to capture it
    // — let the scene handle it instead.
    if (m_viewportPanel && m_viewportPanel->isMouseInViewport(pt.x, pt.y)) {
        return false;
    }

    // Check each native UI panel; if the cursor is over any of them, capture.
    auto isCursorOverPanel = [&pt](HWND hwnd) -> bool {
        if (!hwnd || !IsWindowVisible(hwnd)) {
            return false;
        }
        RECT rc;
        if (GetWindowRect(hwnd, &rc) && PtInRect(&rc, pt)) {
            return true;
        }
        return false;
    };

    if (m_nativeInspector && isCursorOverPanel(m_nativeInspector->getHandle())) return true;
    if (m_nativeSceneHierarchy && isCursorOverPanel(m_nativeSceneHierarchy->getHandle())) return true;
    if (m_nativeContentBrowser && isCursorOverPanel(m_nativeContentBrowser->getHandle())) return true;
    if (m_nativeConsole && isCursorOverPanel(m_nativeConsole->getHandle())) return true;
    if (m_nativeTerraformingPanel && isCursorOverPanel(m_nativeTerraformingPanel->getHandle())) return true;
    if (m_statusBar && isCursorOverPanel(m_statusBar->getHandle())) return true;

    return false;
#else
    // For non-Windows platforms, return false to allow normal game interaction
    return false;
#endif
}

bool EditorManager::wantCaptureKeyboard() const
{
    // For Windows native UI, check if any UI element is capturing keyboard
    // For now, return false to allow normal game interaction
    return false;
}

// ========== Editor Operations Implementation ==========

void EditorManager::undo()
{
    if (m_worldEditor && m_worldEditor->getTerraformingSystem()) {
        if (m_worldEditor->getTerraformingSystem()->undo()) {
            LOG_INFO_C("Undo performed", "EditorManager");
        } else {
            LOG_INFO_C("Nothing to undo", "EditorManager");
        }
    }
}

void EditorManager::redo()
{
    if (m_worldEditor && m_worldEditor->getTerraformingSystem()) {
        if (m_worldEditor->getTerraformingSystem()->redo()) {
            LOG_INFO_C("Redo performed", "EditorManager");
        } else {
            LOG_INFO_C("Nothing to redo", "EditorManager");
        }
    }
}

void EditorManager::cut()
{
    if (m_selectionManager && m_selectionManager->hasSelection()) {
        m_selectionManager->cutToClipboard(m_world);
        LOG_INFO_C("Cut selection to clipboard", "EditorManager");
    } else {
        LOG_INFO_C("No selection to cut", "EditorManager");
    }
}

void EditorManager::copy()
{
    if (m_selectionManager && m_selectionManager->hasSelection()) {
        m_selectionManager->copyToClipboard(m_world);
        LOG_INFO_C("Copied selection to clipboard", "EditorManager");
    } else {
        LOG_INFO_C("No selection to copy", "EditorManager");
    }
}

void EditorManager::paste()
{
    if (m_selectionManager && m_selectionManager->hasClipboardData()) {
        // Get paste position from camera/player if available
        glm::ivec3 pastePos(0, 64, 0);  // Default position
        
        if (m_player) {
            // Use player's camera position as the paste location
            const auto& camera = m_player->getCamera();
            glm::vec3 cameraPos = camera.getPosition();
            // Offset slightly forward from camera
            glm::vec3 forward = camera.getFront();
            glm::vec3 targetPos = cameraPos + forward * 5.0f;
            pastePos = glm::ivec3(
                static_cast<int>(std::floor(targetPos.x)),
                static_cast<int>(std::floor(targetPos.y)),
                static_cast<int>(std::floor(targetPos.z))
            );
        }
        
        m_selectionManager->pasteFromClipboard(pastePos, m_world);
        LOG_INFO_C("Pasted clipboard content at " + 
                  std::to_string(pastePos.x) + ", " + 
                  std::to_string(pastePos.y) + ", " + 
                  std::to_string(pastePos.z), "EditorManager");
    } else {
        LOG_INFO_C("Clipboard is empty", "EditorManager");
    }
}

void EditorManager::deleteSelection()
{
    if (m_selectionManager && m_selectionManager->hasSelection()) {
        m_selectionManager->deleteSelected(m_world);
        LOG_INFO_C("Deleted selection", "EditorManager");
    } else {
        LOG_INFO_C("No selection to delete", "EditorManager");
    }
}

void EditorManager::selectAll()
{
    if (m_sceneHierarchy) {
        m_sceneHierarchy->selectAll();
        LOG_INFO_C("Selected all", "EditorManager");
    }
#ifdef _WIN32
    else if (m_nativeSceneHierarchy) {
        m_nativeSceneHierarchy->selectAll();
        LOG_INFO_C("Selected all (native)", "EditorManager");
    }
#endif
}

void EditorManager::deselectAll()
{
    if (m_sceneHierarchy) {
        m_sceneHierarchy->deselectAll();
        LOG_INFO_C("Deselected all", "EditorManager");
    }
#ifdef _WIN32
    else if (m_nativeSceneHierarchy) {
        m_nativeSceneHierarchy->deselectAll();
        LOG_INFO_C("Deselected all (native)", "EditorManager");
    }
#endif
}

void EditorManager::saveWorld()
{
#ifdef _WIN32
    if (m_worldSerializer && m_world) {
        if (!m_currentWorldPath.empty()) {
            // Save to current path
            if (m_worldSerializer->saveWorld(m_world, m_currentWorldPath)) {
                LOG_INFO_C("World saved successfully to: " + m_currentWorldPath, "EditorManager");
                if (m_windowsDialogManager) {
                    m_windowsDialogManager->showMessageBox(
                        "Save World",
                        "World saved successfully!",
                        MessageBoxButtons::OK,
                        MessageBoxIcon::Information
                    );
                }
            } else {
                LOG_ERROR_C("Failed to save world to: " + m_currentWorldPath, "EditorManager");
                if (m_windowsDialogManager) {
                    m_windowsDialogManager->showMessageBox(
                        "Save World",
                        "Failed to save world!",
                        MessageBoxButtons::OK,
                        MessageBoxIcon::Error
                    );
                }
            }
        } else {
            // No current path, show save as dialog
            saveWorldAs();
        }
    }
#else
    LOG_INFO_C("Save World not implemented on this platform", "EditorManager");
#endif
}

void EditorManager::saveWorldAs()
{
#ifdef _WIN32
    if (m_windowsDialogManager && m_windowsDialogManager->isInitialized() && m_worldSerializer && m_world) {
        LOG_INFO_C("Opening Save World As dialog", "EditorManager");
        
        std::vector<FileFilter> filters = {
            {"Fresh World Files", "*.world"},
            {"All Files", "*.*"}
        };
        
        auto selectedFile = m_windowsDialogManager->showSaveFileDialog(
            "Save World As", "world", filters);
        
        if (!selectedFile.empty()) {
            LOG_INFO_C("User selected save path: " + selectedFile, "EditorManager");
            
            if (m_worldSerializer->saveWorld(m_world, selectedFile)) {
                m_currentWorldPath = selectedFile;
                LOG_INFO_C("World saved successfully to: " + selectedFile, "EditorManager");
                
                m_windowsDialogManager->showMessageBox(
                    "Save World",
                    "World saved successfully!",
                    MessageBoxButtons::OK,
                    MessageBoxIcon::Information
                );
            } else {
                LOG_ERROR_C("Failed to save world to: " + selectedFile, "EditorManager");
                
                m_windowsDialogManager->showMessageBox(
                    "Save World",
                    "Failed to save world!",
                    MessageBoxButtons::OK,
                    MessageBoxIcon::Error
                );
            }
        } else {
            LOG_INFO_C("Save cancelled by user", "EditorManager");
        }
    }
#else
    // Use cross-platform file dialog manager
    if (FileDialogManager::isAvailable() && m_worldSerializer && m_world) {
        LOG_INFO_C("Opening Save World As dialog", "EditorManager");
        
        std::vector<FileDialogManager::Filter> filters = {
            {"Fresh World Files", "world"},
            {"All Files", "*"}
        };
        
        auto selectedFile = FileDialogManager::saveFile(filters, "", "NewWorld.world");
        
        if (!selectedFile.empty()) {
            LOG_INFO_C("User selected save path: " + selectedFile, "EditorManager");
            
            if (m_worldSerializer->saveWorld(m_world, selectedFile)) {
                m_currentWorldPath = selectedFile;
                LOG_INFO_C("World saved successfully to: " + selectedFile, "EditorManager");
            } else {
                LOG_ERROR_C("Failed to save world to: " + selectedFile, "EditorManager");
            }
        } else {
            LOG_INFO_C("Save cancelled by user", "EditorManager");
        }
    } else {
        LOG_INFO_C("Save World As not available (file dialogs not available)", "EditorManager");
    }
#endif
}

void EditorManager::loadWorld()
{
#ifdef _WIN32
    if (m_windowsDialogManager && m_windowsDialogManager->isInitialized() && m_worldSerializer && m_world) {
        LOG_INFO_C("Opening Load World dialog", "EditorManager");
        
        std::vector<FileFilter> filters = {
            {"Fresh World Files", "*.world"},
            {"All Files", "*.*"}
        };
        
        auto selectedFiles = m_windowsDialogManager->showOpenFileDialog(
            "Open World", filters, false);
        
        if (!selectedFiles.empty()) {
            const std::string& worldPath = selectedFiles[0];
            LOG_INFO_C("User selected world file: " + worldPath, "EditorManager");
            
            // Clear existing world by unloading all chunks
            auto& chunks = m_world->getChunks();
            std::vector<ChunkPos> chunksToUnload;
            for (const auto& [pos, chunk] : chunks) {
                chunksToUnload.push_back(pos);
            }
            for (const auto& pos : chunksToUnload) {
                m_world->unloadChunk(pos);
            }
            
            // Load the world
            if (m_worldSerializer->loadWorld(m_world, worldPath)) {
                m_currentWorldPath = worldPath;
                LOG_INFO_C("World loaded successfully from: " + worldPath, "EditorManager");
                
                m_windowsDialogManager->showMessageBox(
                    "Load World",
                    "World loaded successfully!",
                    MessageBoxButtons::OK,
                    MessageBoxIcon::Information
                );
            } else {
                LOG_ERROR_C("Failed to load world from: " + worldPath, "EditorManager");
                
                m_windowsDialogManager->showMessageBox(
                    "Load World",
                    "Failed to load world!",
                    MessageBoxButtons::OK,
                    MessageBoxIcon::Error
                );
            }
        } else {
            LOG_INFO_C("Load cancelled by user", "EditorManager");
        }
    }
#else
    // Use cross-platform file dialog manager
    if (FileDialogManager::isAvailable() && m_worldSerializer && m_world) {
        LOG_INFO_C("Opening Load World dialog", "EditorManager");
        
        std::vector<FileDialogManager::Filter> filters = {
            {"Fresh World Files", "world"},
            {"All Files", "*"}
        };
        
        auto selectedFile = FileDialogManager::openFile(filters);
        
        if (!selectedFile.empty()) {
            LOG_INFO_C("User selected world file: " + selectedFile, "EditorManager");
            
            // Clear existing world by unloading all chunks
            auto& chunks = m_world->getChunks();
            std::vector<ChunkPos> chunksToUnload;
            for (const auto& [pos, chunk] : chunks) {
                chunksToUnload.push_back(pos);
            }
            for (const auto& pos : chunksToUnload) {
                m_world->unloadChunk(pos);
            }
            
            // Load the world
            if (m_worldSerializer->loadWorld(m_world, selectedFile)) {
                m_currentWorldPath = selectedFile;
                LOG_INFO_C("World loaded successfully from: " + selectedFile, "EditorManager");
            } else {
                LOG_ERROR_C("Failed to load world from: " + selectedFile, "EditorManager");
            }
        } else {
            LOG_INFO_C("Load cancelled by user", "EditorManager");
        }
    } else {
        LOG_INFO_C("Load World not available (file dialogs not available)", "EditorManager");
    }
#endif
}

void EditorManager::newWorld()
{
#ifdef _WIN32
    if (m_windowsDialogManager) {
        // Show confirmation dialog if there's unsaved work
        auto result = m_windowsDialogManager->showMessageBox(
            "New Scene",
            "Create a new scene? Any unsaved changes will be lost.",
            MessageBoxButtons::YesNo,
            MessageBoxIcon::Question
        );
        
        if (result == MessageBoxResult::Yes) {
            // Use NativeMainMenu to show world creation dialog directly
            LOG_INFO_C("Showing native Win32 world creation dialog", "EditorManager");
            
            // Create and show the world creation dialog directly
            NativeMainMenu mainMenu;
            HWND parentHwnd = m_window ? m_window->getHandle() : nullptr;
            
            if (mainMenu.initialize(parentHwnd)) {
                // Show the create world dialog directly (skips main menu)
                if (mainMenu.showCreateWorldDialog()) {
                    // Get world parameters from dialog and create WorldCreationParams struct
                    WorldCreationParams params;
                    std::wstring wWorldName = mainMenu.getNewWorldName();
                    params.name = toNarrowString(wWorldName);
                    params.seed = mainMenu.getWorldSeed();
                    params.is3D = mainMenu.isWorld3D();
                    params.gameStyle2D = static_cast<int>(mainMenu.get2DWorldStyle());
                    params.worldSize = mainMenu.getWorldSize();
                    params.terrainType = mainMenu.getTerrainType();
                    params.biomeType = mainMenu.getBiomeType();
                    
                    LOG_INFO_C("Scene creation confirmed: " + params.name + 
                               " (seed=" + std::to_string(params.seed) + 
                               ", 3D=" + std::to_string(params.is3D) +
                               ", 2D style=" + std::to_string(params.gameStyle2D) +
                               ", size=" + std::to_string(static_cast<int>(params.worldSize)) +
                               ", terrain=" + std::to_string(static_cast<int>(params.terrainType)) +
                               ", biome=" + std::to_string(static_cast<int>(params.biomeType)) + ")", "EditorManager");
                    
                    // Call the callback to notify Engine to create the scene/world
                    if (m_worldCreationCallback) {
                        m_worldCreationCallback(params);
                    } else {
                        LOG_WARNING_C("Scene creation callback not set!", "EditorManager");
                        m_windowsDialogManager->showMessageBox(
                            "Scene Creation Error",
                            "Cannot create scene: callback not configured.",
                            MessageBoxButtons::OK,
                            MessageBoxIcon::Error
                        );
                    }
                } else {
                    LOG_INFO_C("Scene creation cancelled by user", "EditorManager");
                }
            } else {
                LOG_ERROR_C("Failed to initialize native scene creation dialog", "EditorManager");
                m_windowsDialogManager->showMessageBox(
                    "Scene Creation Error",
                    "Failed to initialize scene creation dialog.",
                    MessageBoxButtons::OK,
                    MessageBoxIcon::Error
                );
            }
        } else {
            LOG_INFO_C("New scene cancelled by user", "EditorManager");
        }
    }
#else
    LOG_INFO_C("New Project not implemented on this platform", "EditorManager");
#endif
}

void EditorManager::newProject()
{
#ifdef _WIN32
    if (!m_windowsDialogManager || !m_projectManager) {
        LOG_ERROR_C("Cannot create project: required components not initialized", "EditorManager");
        return;
    }
    
    // Show confirmation if there's an open project
    if (m_projectManager->isProjectOpen()) {
        auto result = m_windowsDialogManager->showMessageBox(
            "New Project",
            "Close current project? Any unsaved changes will be lost.",
            MessageBoxButtons::YesNo,
            MessageBoxIcon::Question
        );
        
        if (result != MessageBoxResult::Yes) {
            LOG_INFO_C("New project cancelled by user", "EditorManager");
            return;
        }
    }
    
    // Use FileDialogManager to select a folder for the new project
    LOG_INFO_C("Creating new project - selecting folder", "EditorManager");
    
    // Check if file dialogs are available
    if (!FileDialogManager::isAvailable()) {
        LOG_ERROR_C("File dialogs not available - NFD not compiled in", "EditorManager");
        m_windowsDialogManager->showMessageBox(
            "Feature Not Available",
            "File dialogs are not available. Please rebuild with Native File Dialog Extended (NFD) support.",
            MessageBoxButtons::OK,
            MessageBoxIcon::Error
        );
        return;
    }
    
    // Select folder where project will be created
    std::string selectedFolder = FileDialogManager::pickFolder("./Projects");
    
    if (selectedFolder.empty()) {
        LOG_INFO_C("New project cancelled - no folder selected", "EditorManager");
        return;
    }
    
    // Extract project name from the selected folder path
    std::string projectName = "UntitledProject";
    try {
        std::filesystem::path folderPath(selectedFolder);
        if (!folderPath.filename().empty()) {
            projectName = folderPath.filename().string();
        }
    } catch (const std::filesystem::filesystem_error& e) {
        LOG_WARNING_C("Failed to extract project name from path (filesystem error), using default: " + 
                      std::string(e.what()), "EditorManager");
    } catch (const std::exception& e) {
        LOG_WARNING_C("Failed to extract project name from path (unexpected error), using default: " + 
                      std::string(e.what()), "EditorManager");
    }
    
    // TODO: In the future, show a dialog to get project name and template type
    std::string projectPath = selectedFolder;
    std::string templateType = "Blank";
    
    LOG_INFO_C("Creating project '" + projectName + "' at: " + projectPath, "EditorManager");
    
    if (m_projectManager->createNewProject(projectName, projectPath, templateType)) {
        LOG_INFO_C("Project created successfully: " + projectName, "EditorManager");
        m_windowsDialogManager->showMessageBox(
            "Project Created",
            "New project created successfully: " + projectName,
            MessageBoxButtons::OK,
            MessageBoxIcon::Information
        );
    } else {
        LOG_ERROR_C("Failed to create project: " + projectName, "EditorManager");
        m_windowsDialogManager->showMessageBox(
            "Project Creation Error",
            "Failed to create project. Check logs for details.",
            MessageBoxButtons::OK,
            MessageBoxIcon::Error
        );
    }
#else
    LOG_INFO_C("New Project not implemented on this platform", "EditorManager");
#endif
}

void EditorManager::openProject()
{
#ifdef _WIN32
    if (!m_windowsDialogManager || !m_projectManager) {
        LOG_ERROR_C("Cannot open project: required components not initialized", "EditorManager");
        return;
    }
    
    // Show confirmation if there's an open project
    if (m_projectManager->isProjectOpen()) {
        auto result = m_windowsDialogManager->showMessageBox(
            "Open Project",
            "Close current project? Any unsaved changes will be lost.",
            MessageBoxButtons::YesNo,
            MessageBoxIcon::Question
        );
        
        if (result != MessageBoxResult::Yes) {
            LOG_INFO_C("Open project cancelled by user", "EditorManager");
            return;
        }
    }
    
    // Use FileDialogManager to select .freshproj file
    LOG_INFO_C("Opening project - selecting project file", "EditorManager");
    
    // Check if file dialogs are available
    if (!FileDialogManager::isAvailable()) {
        LOG_ERROR_C("File dialogs not available - NFD not compiled in", "EditorManager");
        m_windowsDialogManager->showMessageBox(
            "Feature Not Available",
            "File dialogs are not available. Please rebuild with Native File Dialog Extended (NFD) support.",
            MessageBoxButtons::OK,
            MessageBoxIcon::Error
        );
        return;
    }
    
    // Define filter for .freshproj files
    std::vector<FileDialogManager::Filter> filters = {
        { "Fresh Project Files", "freshproj" },
        { "All Files", "*" }
    };
    
    // Open file dialog to select project file
    std::string projectPath = FileDialogManager::openFile(filters, "./Projects");
    
    if (projectPath.empty()) {
        LOG_INFO_C("Open project cancelled - no file selected", "EditorManager");
        return;
    }
    
    LOG_INFO_C("Opening project file: " + projectPath, "EditorManager");
    
    if (m_projectManager->openProject(projectPath)) {
        LOG_INFO_C("Project opened successfully: " + m_projectManager->getProjectName(), "EditorManager");
        m_windowsDialogManager->showMessageBox(
            "Project Opened",
            "Project opened successfully: " + m_projectManager->getProjectName(),
            MessageBoxButtons::OK,
            MessageBoxIcon::Information
        );
    } else {
        LOG_ERROR_C("Failed to open project: " + projectPath, "EditorManager");
        m_windowsDialogManager->showMessageBox(
            "Project Open Error",
            "Failed to open project. File may not exist or is corrupted.",
            MessageBoxButtons::OK,
            MessageBoxIcon::Error
        );
    }
#else
    LOG_INFO_C("Open Project not implemented on this platform", "EditorManager");
#endif
}

void EditorManager::toggleSceneHierarchy()
{
    m_showSceneHierarchy = !m_showSceneHierarchy;
    LOG_INFO_C("Scene Hierarchy toggled: " + std::string(m_showSceneHierarchy ? "shown" : "hidden"), "EditorManager");
    
#ifdef FRESH_WIN32_UI
    // Toggle native Win32 panel visibility
    if (m_nativeSceneHierarchy) {
        m_nativeSceneHierarchy->setVisible(m_showSceneHierarchy);
    }
#endif
}

void EditorManager::toggleInspector()
{
    m_showInspector = !m_showInspector;
    LOG_INFO_C("Inspector toggled: " + std::string(m_showInspector ? "shown" : "hidden"), "EditorManager");
    
#ifdef FRESH_WIN32_UI
    // Toggle native Win32 panel visibility
    if (m_nativeInspector) {
        m_nativeInspector->setVisible(m_showInspector);
    }
#endif
}

void EditorManager::toggleContentBrowser()
{
    m_showContentBrowser = !m_showContentBrowser;
    LOG_INFO_C("Content Browser toggled: " + std::string(m_showContentBrowser ? "shown" : "hidden"), "EditorManager");
    
#ifdef FRESH_WIN32_UI
    // Toggle native Win32 panel visibility
    if (m_nativeContentBrowser) {
        LOG_INFO_C("Calling setVisible(" + std::string(m_showContentBrowser ? "true" : "false") + ") on Content Browser", "EditorManager");
        m_nativeContentBrowser->setVisible(m_showContentBrowser);
    } else {
        LOG_WARNING_C("m_nativeContentBrowser is null, cannot toggle visibility", "EditorManager");
    }
#else
    LOG_WARNING_C("FRESH_WIN32_UI not defined, Content Browser toggle has no effect", "EditorManager");
#endif
}

void EditorManager::toggleConsole()
{
    m_showConsole = !m_showConsole;
    LOG_INFO_C("Console toggled: " + std::string(m_showConsole ? "shown" : "hidden"), "EditorManager");
    
#ifdef FRESH_WIN32_UI
    // Toggle native Win32 panel visibility
    if (m_nativeConsole) {
        m_nativeConsole->setVisible(m_showConsole);
    }
#endif
}

void EditorManager::toggleToolPalette()
{
    m_showToolPalette = !m_showToolPalette;
    LOG_INFO_C("Tool Palette toggled: " + std::string(m_showToolPalette ? "shown" : "hidden"), "EditorManager");
}

void EditorManager::toggleLLMAssistant()
{
    m_showLLMAssistant = !m_showLLMAssistant;
    LOG_INFO_C("LLM Assistant toggled: " + std::string(m_showLLMAssistant ? "shown" : "hidden"), "EditorManager");
    
#ifdef FRESH_WIN32_UI
    if (m_nativeLLMAssistant) {
        m_nativeLLMAssistant->setVisible(m_showLLMAssistant);
    }
#endif
}

void EditorManager::setVisible(bool visible)
{
    m_visible = visible;
    
#ifdef _WIN32
    // Show or hide all Win32 native panels
    // These are the editor UI panels that should be visible in editor mode
    // and hidden in play mode
    // 
    // IMPORTANT: Respect individual panel visibility flags (m_showSceneHierarchy, etc.)
    // Only show a panel if both the editor is visible AND the panel's individual flag is true
    
    if (m_nativeSceneHierarchy) {
        m_nativeSceneHierarchy->setVisible(visible && m_showSceneHierarchy);
    }
    
    if (m_nativeInspector) {
        m_nativeInspector->setVisible(visible && m_showInspector);
    }
    
    if (m_nativeContentBrowser) {
        m_nativeContentBrowser->setVisible(visible && m_showContentBrowser);
    }
    
    if (m_nativeConsole) {
        m_nativeConsole->setVisible(visible && m_showConsole);
    }
    
    if (m_nativeLLMAssistant) {
        m_nativeLLMAssistant->setVisible(visible && m_showLLMAssistant);
    }
    
    if (m_nativeTerraformingPanel) {
        // Terraforming panel doesn't have a visibility flag, so just use the editor visibility
        m_nativeTerraformingPanel->setVisible(visible);
    }
    
    if (m_statusBar) {
        // Status bar should always be visible when editor is visible
        m_statusBar->setVisible(visible);
    }
    
    // CRITICAL FIX: Viewport panel should ALWAYS be visible (it shows the 3D world)
    // The viewport is visible in both editor mode and play mode
    // In editor mode, it's a panel among other panels showing the world
    // In play mode, it continues showing the world while other UI panels are hidden
    if (m_viewportPanel) {
        m_viewportPanel->setVisible(true);  // Always visible regardless of editor visibility
        LOG_INFO_C("Viewport panel kept visible (always visible for rendering)", "EditorManager");
    }
    
    // CRITICAL FIX: Ensure proper Z-order after visibility changes
    // This prevents viewport from appearing on top of UI panels
    ensurePanelsOnTop();
    
    LOG_INFO_C(visible ? "Editor panels shown" : "Editor panels hidden", "EditorManager");
#endif
}

void EditorManager::enterPlayMode()
{
    if (m_enterPlayModeCallback) {
        m_enterPlayModeCallback();
    } else {
        LOG_WARNING_C("Enter play mode callback not set", "EditorManager");
    }
}

void EditorManager::exitPlayMode()
{
    if (m_exitPlayModeCallback) {
        m_exitPlayModeCallback();
    } else {
        LOG_WARNING_C("Exit play mode callback not set", "EditorManager");
    }
}

void EditorManager::showSettings()
{
#ifdef _WIN32
    if (m_nativeSettingsDialog && m_window) {
        Win32SettingsDialog::Settings currentSettings;
        // Populate from editor settings if available
        if (m_editorSettingsDialog) {
            const auto& editorSettings = m_editorSettingsDialog->getSettings();
            currentSettings.vsync = editorSettings.vsync;
            currentSettings.fpsLimit = editorSettings.targetFPS;
            currentSettings.fov = 75.0f;
            currentSettings.mouseSensitivity = 0.5f;
        }
        HWND hwnd = static_cast<HWND>(m_window->getNativeWindowHandle());
        m_nativeSettingsDialog->show(hwnd, currentSettings);
        LOG_INFO_C("Settings dialog shown", "EditorManager");
    }
#endif
}

void EditorManager::showEngineConfig()
{
    // Removed: m_engineConfigPanel is commented out in header
    // if (m_engineConfigPanel) {
    //     m_engineConfigPanel->setVisible(true);
    //     LOG_INFO_C("Engine configuration panel shown", "EditorManager");
    // }
}

void EditorManager::showEditorSettings()
{
    if (m_editorSettingsDialog) {
        m_editorSettingsDialog->setVisible(true);
        LOG_INFO_C("Editor settings dialog shown", "EditorManager");
    }
}

void EditorManager::showImportAssets()
{
    if (m_contentBrowser) {
        m_contentBrowser->showImportDialog();
        LOG_INFO_C("Import assets dialog shown", "EditorManager");
    }
#ifdef _WIN32
    else if (m_nativeContentBrowser) {
        m_nativeContentBrowser->showImportDialog();
        LOG_INFO_C("Import assets dialog shown (native)", "EditorManager");
    }
#endif
}

void EditorManager::launchDialogueEditor()
{
#ifdef _WIN32
    LOG_INFO_C("Launching dialogue editor", "EditorManager");
    
    // Path to the dialogue editor executable
    std::string editorPath = "dotnet/DialogueEditor/bin/Release/net9.0-windows/DialogueEditor.exe";
    
    // Check if the editor exists
    if (!std::filesystem::exists(editorPath)) {
        // Try debug build
        editorPath = "dotnet/DialogueEditor/bin/Debug/net9.0-windows/DialogueEditor.exe";
        
        if (!std::filesystem::exists(editorPath)) {
            LOG_ERROR_C("Dialogue editor executable not found. Please build the DialogueEditor project.", "EditorManager");
            
            return;
        }
    }
    
    // Launch the dialogue editor as a separate process
    STARTUPINFOA si = {};
    PROCESS_INFORMATION pi = {};
    si.cb = sizeof(si);
    
    // Create the process
    if (CreateProcessA(
        editorPath.c_str(),     // Application path
        nullptr,                // Command line
        nullptr,                // Process handle not inheritable
        nullptr,                // Thread handle not inheritable
        FALSE,                  // Set handle inheritance to FALSE
        0,                      // No creation flags
        nullptr,                // Use parent's environment block
        nullptr,                // Use parent's starting directory
        &si,                    // Pointer to STARTUPINFO structure
        &pi))                   // Pointer to PROCESS_INFORMATION structure
    {
        LOG_INFO_C("Dialogue editor launched successfully", "EditorManager");
        
        // Close process and thread handles as we don't need them
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
    else
    {
        DWORD error = GetLastError();
        LOG_ERROR_C("Failed to launch dialogue editor. Error code: " + std::to_string(error), "EditorManager");
    }
#else
    LOG_WARNING_C("Dialogue editor is only available on Windows", "EditorManager");
#endif
}

void EditorManager::loadLayout(const std::string& name)
{
    if (!m_layoutManager) {
        LOG_WARNING_C("Layout manager not initialized", "EditorManager");
        return;
    }
    
    LayoutConfig config;
    if (m_layoutManager->loadLayout(name, config)) {
        // Apply the layout configuration
        m_showSceneHierarchy = config.showSceneHierarchy;
        m_showInspector = config.showInspector;
        m_showContentBrowser = config.showContentBrowser;
        m_showConsole = config.showConsole;
        m_showToolPalette = config.showToolPalette;
        
        LOG_INFO_C("Layout applied: " + name, "EditorManager");
    } else {
        LOG_ERROR_C("Failed to load layout: " + name, "EditorManager");
    }
}

void EditorManager::saveCurrentLayout(const std::string& name)
{
    if (!m_layoutManager) {
        LOG_WARNING_C("Layout manager not initialized", "EditorManager");
        return;
    }
    
    // Get the current panel visibility states
    LayoutConfig config;
    config.showSceneHierarchy = m_showSceneHierarchy;
    config.showInspector = m_showInspector;
    config.showContentBrowser = m_showContentBrowser;
    config.showConsole = m_showConsole;
    config.showToolPalette = m_showToolPalette;
    
    // Use provided name or current layout name
    std::string layoutName = name.empty() ? m_layoutManager->getCurrentLayoutName() : name;
    
    if (m_layoutManager->saveLayout(layoutName, config)) {
        LOG_INFO_C("Layout saved: " + layoutName, "EditorManager");
    } else {
        LOG_ERROR_C("Failed to save layout: " + layoutName, "EditorManager");
    }
}

void EditorManager::resetLayout()
{
    loadLayout("Default");
}

void EditorManager::setCameraViewMode(const std::string& mode)
{
    if (!m_cameraController) {
        LOG_WARNING_C("Camera controller not initialized", "EditorManager");
        return;
    }
    
    if (!m_player) {
        LOG_WARNING_C("Player not set, cannot initialize camera controller", "EditorManager");
        return;
    }
    
    // Initialize camera controller with player if not already done
    if (!m_player) {
        m_cameraController->initialize(m_player);
    }
    
    // Map string to view mode
    CameraController::ViewMode viewMode;
    if (mode == "Perspective") {
        viewMode = CameraController::ViewMode::Perspective;
    } else if (mode == "Top") {
        viewMode = CameraController::ViewMode::Top;
    } else if (mode == "Front") {
        viewMode = CameraController::ViewMode::Front;
    } else if (mode == "Side" || mode == "Right") {
        viewMode = CameraController::ViewMode::Side;
    } else if (mode == "Bottom") {
        viewMode = CameraController::ViewMode::Bottom;
    } else if (mode == "Back") {
        viewMode = CameraController::ViewMode::Back;
    } else if (mode == "Left") {
        viewMode = CameraController::ViewMode::Left;
    } else {
        LOG_WARNING_C("Unknown camera view mode: " + mode, "EditorManager");
        return;
    }
    
    m_cameraController->setViewMode(viewMode);
}

void EditorManager::focusOnSelection()
{
    if (!m_cameraController) {
        LOG_WARNING_C("Camera controller not initialized", "EditorManager");
        return;
    }
    
    if (!m_player) {
        LOG_WARNING_C("Player not set", "EditorManager");
        return;
    }
    
    // Initialize camera controller with player if not already done
    if (!m_player) {
        m_cameraController->initialize(m_player);
    }
    
    if (!m_selectionManager || !m_selectionManager->hasSelection()) {
        LOG_INFO_C("No selection to focus on", "EditorManager");
        return;
    }
    
    // Get selection center
    const auto& selectedBlocks = m_selectionManager->getSelectedVoxels();
    if (selectedBlocks.empty()) {
        LOG_INFO_C("Selection is empty", "EditorManager");
        return;
    }
    
    // Calculate center of selected blocks
    glm::vec3 center(0.0f);
    for (const auto& block : selectedBlocks) {
        center += glm::vec3(block.x, block.y, block.z);
    }
    center /= static_cast<float>(selectedBlocks.size());
    
    m_cameraController->focusOn(center);
    LOG_INFO_C("Focused camera on selection center", "EditorManager");
}

void EditorManager::frameSelection()
{
    if (!m_cameraController) {
        LOG_WARNING_C("Camera controller not initialized", "EditorManager");
        return;
    }
    
    if (!m_player) {
        LOG_WARNING_C("Player not set", "EditorManager");
        return;
    }
    
    // Initialize camera controller with player if not already done
    if (!m_player) {
        m_cameraController->initialize(m_player);
    }
    
    if (!m_selectionManager || !m_selectionManager->hasSelection()) {
        LOG_INFO_C("No selection to frame", "EditorManager");
        return;
    }
    
    // Get selection bounds
    const auto& selectedBlocks = m_selectionManager->getSelectedVoxels();
    if (selectedBlocks.empty()) {
        LOG_INFO_C("Selection is empty", "EditorManager");
        return;
    }
    
    // Calculate bounding box of selected blocks
    glm::vec3 boundsMin(std::numeric_limits<float>::max());
    glm::vec3 boundsMax(std::numeric_limits<float>::lowest());
    
    for (const auto& block : selectedBlocks) {
        glm::vec3 pos(block.x, block.y, block.z);
        boundsMin = glm::min(boundsMin, pos);
        boundsMax = glm::max(boundsMax, pos);
    }
    
    m_cameraController->frameBox(boundsMin, boundsMax);
    LOG_INFO_C("Framed selection in view", "EditorManager");
}

void EditorManager::setFPS(float fps)
{
#ifdef _WIN32
    if (m_statusBar) {
        m_statusBar->setFPS(fps);
    }
    
    // Also update HUD if visible and in play mode
    if (m_nativeHUD && m_nativeHUD->isVisible()) {
        // Get current stats
        Win32HUD::HUDStats stats;
        if (m_player) {
            stats.health = m_player->getHealth();
            stats.maxHealth = m_player->getMaxHealth();
            stats.stamina = m_player->getStamina();
            stats.maxStamina = m_player->getMaxStamina();
            auto pos = m_player->getPosition();
            stats.posX = pos.x;
            stats.posY = pos.y;
            stats.posZ = pos.z;
        }
        stats.fps = static_cast<int>(fps);
        m_nativeHUD->updateStats(stats);
    }
#else
    (void)fps;  // Unused parameter on non-Windows platforms
#endif
}

void EditorManager::setMemoryUsage(float usedMB, float totalMB)
{
#ifdef _WIN32
    if (m_statusBar) {
        m_statusBar->setMemoryUsage(usedMB, totalMB);
    }
#else
    (void)usedMB;   // Unused parameter on non-Windows platforms
    (void)totalMB;  // Unused parameter on non-Windows platforms
#endif
}

#ifdef _WIN32
void EditorManager::onWindowResize(int clientWidth, int clientHeight)
{
    // Update panel layout when window is resized - Unreal Engine style layout
    // This ensures panels maintain proper positions and sizes
    
    if (!m_window) {
        return;
    }
    
    Win32Window* win32Window = dynamic_cast<Win32Window*>(m_window);
    if (!win32Window) {
        return;
    }
    
    HWND hwnd = win32Window->getHandle();
    if (!hwnd) {
        return;
    }
    
    // Recalculate panel positions based on new window size
    // Layout: Left (tools) | Center (viewport) | Right (outliner + inspector) with Bottom (content + console)
    
    // Get actual toolbar height dynamically
    int actualToolbarHeight = getActualToolbarHeight();
    
    // Left panel - Terraforming tools (narrow vertical, full height minus bottom panel)
    int leftPanelX = PANEL_MARGIN;
    int leftPanelY = actualToolbarHeight;
    int leftPanelHeight = clientHeight - actualToolbarHeight - BOTTOM_PANEL_HEIGHT - PANEL_MARGIN * 2;
    
    if (m_nativeTerraformingPanel) {
        m_nativeTerraformingPanel->setPosition(leftPanelX, leftPanelY);
        m_nativeTerraformingPanel->setSize(LEFT_PANEL_WIDTH, leftPanelHeight);
    }
    
    // Right panel - split vertically between Outliner (top) and Inspector (bottom)
    int rightPanelX = clientWidth - RIGHT_PANEL_WIDTH - PANEL_MARGIN;
    int rightPanelY = actualToolbarHeight;
    int rightPanelUsableHeight = clientHeight - actualToolbarHeight - BOTTOM_PANEL_HEIGHT - PANEL_MARGIN * 2;
    int outlinerHeight = static_cast<int>(rightPanelUsableHeight * OUTLINER_HEIGHT_RATIO);
    // Inspector height is derived rather than calculated separately to ensure heights sum exactly
    // to rightPanelUsableHeight (accounting for margin), avoiding pixel gaps from rounding errors
    int inspectorHeight = rightPanelUsableHeight - outlinerHeight - PANEL_MARGIN;
    
    if (m_nativeSceneHierarchy) {
        m_nativeSceneHierarchy->setPosition(rightPanelX, rightPanelY);
        m_nativeSceneHierarchy->setSize(RIGHT_PANEL_WIDTH, outlinerHeight);
    }
    
    int inspectorY = rightPanelY + outlinerHeight + PANEL_MARGIN;
    if (m_nativeInspector) {
        m_nativeInspector->setPosition(rightPanelX, inspectorY);
        m_nativeInspector->setSize(RIGHT_PANEL_WIDTH, inspectorHeight);
    }
    
    // Bottom panel - Content Browser + Console (horizontal strip)
    int bottomPanelY = clientHeight - BOTTOM_PANEL_HEIGHT - PANEL_MARGIN;
    
    if (m_nativeContentBrowser) {
        m_nativeContentBrowser->setPosition(PANEL_MARGIN, bottomPanelY);
        m_nativeContentBrowser->setSize(CONTENT_BROWSER_WIDTH, BOTTOM_PANEL_HEIGHT);
    }
    
    int consoleX = PANEL_MARGIN + CONTENT_BROWSER_WIDTH + PANEL_MARGIN;
    int consoleWidth = std::max(MIN_CONSOLE_WIDTH, clientWidth - consoleX - PANEL_MARGIN);
    if (m_nativeConsole) {
        m_nativeConsole->setPosition(consoleX, bottomPanelY);
        m_nativeConsole->setSize(consoleWidth, BOTTOM_PANEL_HEIGHT);
    }
    
    // LLM Assistant panel shares the same area as console (toggled)
    if (m_nativeLLMAssistant) {
        m_nativeLLMAssistant->setPosition(consoleX, bottomPanelY);
        m_nativeLLMAssistant->setSize(consoleWidth, BOTTOM_PANEL_HEIGHT);
    }
    
    // Viewport - center area between left and right panels, above bottom panel
    int viewportX = LEFT_PANEL_WIDTH + PANEL_MARGIN * 2;
    int viewportY = actualToolbarHeight;
    int viewportWidth = std::max(MIN_VIEWPORT_WIDTH, rightPanelX - viewportX - PANEL_MARGIN);
    int viewportHeight = std::max(MIN_VIEWPORT_HEIGHT, bottomPanelY - actualToolbarHeight - PANEL_MARGIN);
    
    if (m_viewportPanel) {
        m_viewportPanel->setPosition(viewportX, viewportY);
        m_viewportPanel->setSize(viewportWidth, viewportHeight);
    }
    
    // Update status bar layout - critical for proper repositioning during maximize/restore
    if (m_statusBar) {
        m_statusBar->updateLayout();
    }
    
    // Ensure proper Z-order after resize
    ensurePanelsOnTop();
    
    // Refresh all panels to ensure they're properly painted after resize
    // This is especially important during maximize/restore operations
    refreshAllPanels();
}

void EditorManager::ensurePanelsOnTop()
{
#ifdef _WIN32
    // Ensure proper Z-order within parent window's child windows:
    // All child windows should be visible, with viewport at back and UI panels in front
    
    // Strategy: Order child windows from back to front
    // 1. First, ensure viewport is at the back of sibling child windows
    // 2. Then place all UI panels in front
    
    // CRITICAL FIX: For child windows, we need to arrange them relative to each other
    // We'll place viewport at the bottom of the child window stack, then UI panels on top
    
    // Start by getting the first UI panel handle to use as reference
    HWND firstPanelHwnd = nullptr;
    if (m_nativeTerraformingPanel && m_nativeTerraformingPanel->getHandle()) {
        firstPanelHwnd = m_nativeTerraformingPanel->getHandle();
    } else if (m_nativeSceneHierarchy && m_nativeSceneHierarchy->getHandle()) {
        firstPanelHwnd = m_nativeSceneHierarchy->getHandle();
    }
    
    // Place viewport behind the first panel (or at bottom if no panels)
    if (m_viewportPanel && m_viewportPanel->getHandle()) {
        if (firstPanelHwnd) {
            // Place viewport behind first panel - this makes it at the back
            SetWindowPos(m_viewportPanel->getHandle(), firstPanelHwnd, 0, 0, 0, 0, 
                        SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
        } else {
            // No panels yet, just ensure it's visible
            SetWindowPos(m_viewportPanel->getHandle(), HWND_TOP, 0, 0, 0, 0, 
                        SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
        }
    }
    
    // Then bring all UI panels to the front to ensure they're visible above the viewport
    if (m_nativeTerraformingPanel && m_nativeTerraformingPanel->getHandle()) {
        SetWindowPos(m_nativeTerraformingPanel->getHandle(), HWND_TOP, 0, 0, 0, 0, 
                    SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    }
    if (m_nativeSceneHierarchy && m_nativeSceneHierarchy->getHandle()) {
        SetWindowPos(m_nativeSceneHierarchy->getHandle(), HWND_TOP, 0, 0, 0, 0, 
                    SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    }
    if (m_nativeInspector && m_nativeInspector->getHandle()) {
        SetWindowPos(m_nativeInspector->getHandle(), HWND_TOP, 0, 0, 0, 0, 
                    SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    }
    if (m_nativeContentBrowser && m_nativeContentBrowser->getHandle()) {
        SetWindowPos(m_nativeContentBrowser->getHandle(), HWND_TOP, 0, 0, 0, 0, 
                    SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    }
    if (m_nativeConsole && m_nativeConsole->getHandle()) {
        SetWindowPos(m_nativeConsole->getHandle(), HWND_TOP, 0, 0, 0, 0, 
                    SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    }
    if (m_nativeLLMAssistant && m_nativeLLMAssistant->getHandle()) {
        SetWindowPos(m_nativeLLMAssistant->getHandle(), HWND_TOP, 0, 0, 0, 0, 
                    SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    }
    if (m_statusBar && m_statusBar->getHandle()) {
        SetWindowPos(m_statusBar->getHandle(), HWND_TOP, 0, 0, 0, 0, 
                    SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    }
#endif
}

int EditorManager::getActualToolbarHeight() const
{
#ifdef _WIN32
    Win32Window* win32Window = dynamic_cast<Win32Window*>(m_window);
    if (win32Window) {
        auto* toolbar = win32Window->getToolbar();
        if (toolbar) {
            int height = toolbar->getHeight();
            if (height > 0) {
                return height;
            }
        }
    }
#endif
    // Fallback to constant if toolbar not available or has zero height
    return TOOLBAR_HEIGHT;
}

void EditorManager::refreshAllPanels()
{
#ifdef _WIN32
    // Invalidate and update all panel windows to ensure they are visible
    // This triggers WM_PAINT messages for all panels, forcing them to redraw
    
    if (m_viewportPanel && m_viewportPanel->getHandle()) {
        InvalidateRect(m_viewportPanel->getHandle(), nullptr, TRUE);
        UpdateWindow(m_viewportPanel->getHandle());
    }
    
    if (m_nativeTerraformingPanel && m_nativeTerraformingPanel->getHandle()) {
        InvalidateRect(m_nativeTerraformingPanel->getHandle(), nullptr, TRUE);
        UpdateWindow(m_nativeTerraformingPanel->getHandle());
    }
    
    if (m_nativeSceneHierarchy && m_nativeSceneHierarchy->getHandle()) {
        InvalidateRect(m_nativeSceneHierarchy->getHandle(), nullptr, TRUE);
        UpdateWindow(m_nativeSceneHierarchy->getHandle());
    }
    
    if (m_nativeInspector && m_nativeInspector->getHandle()) {
        InvalidateRect(m_nativeInspector->getHandle(), nullptr, TRUE);
        UpdateWindow(m_nativeInspector->getHandle());
    }
    
    if (m_nativeContentBrowser && m_nativeContentBrowser->getHandle()) {
        InvalidateRect(m_nativeContentBrowser->getHandle(), nullptr, TRUE);
        UpdateWindow(m_nativeContentBrowser->getHandle());
    }
    
    if (m_nativeConsole && m_nativeConsole->getHandle()) {
        InvalidateRect(m_nativeConsole->getHandle(), nullptr, TRUE);
        UpdateWindow(m_nativeConsole->getHandle());
    }
    
    if (m_nativeLLMAssistant && m_nativeLLMAssistant->getHandle()) {
        InvalidateRect(m_nativeLLMAssistant->getHandle(), nullptr, TRUE);
        UpdateWindow(m_nativeLLMAssistant->getHandle());
    }
    
    if (m_statusBar && m_statusBar->getHandle()) {
        InvalidateRect(m_statusBar->getHandle(), nullptr, TRUE);
        UpdateWindow(m_statusBar->getHandle());
    }
    
    // Also refresh the main window to ensure proper painting
    Win32Window* win32Window = dynamic_cast<Win32Window*>(m_window);
    if (win32Window) {
        HWND hwnd = win32Window->getHandle();
        if (hwnd) {
            InvalidateRect(hwnd, nullptr, TRUE);
            UpdateWindow(hwnd);
        }
    }
    
    LOG_INFO_C("All panels refreshed", "EditorManager");
#endif
}

void EditorManager::handleConsoleCommand(const std::string& command)
{
    // Trim whitespace
    std::string cmd = command;
    cmd.erase(0, cmd.find_first_not_of(" \t\r\n"));
    cmd.erase(cmd.find_last_not_of(" \t\r\n") + 1);
    
    if (cmd.empty()) {
        return;
    }
    
    LOG_INFO_C("Console command: " + cmd, "EditorManager");
    
    // Parse command
    std::string cmdName;
    std::string cmdArgs;
    size_t spacePos = cmd.find(' ');
    if (spacePos != std::string::npos) {
        cmdName = cmd.substr(0, spacePos);
        cmdArgs = cmd.substr(spacePos + 1);
    } else {
        cmdName = cmd;
    }
    
    // Convert to lowercase for case-insensitive comparison
    std::transform(cmdName.begin(), cmdName.end(), cmdName.begin(), ::tolower);
    
    // Handle commands
    if (cmdName == "help") {
        LOG_INFO_C("Available commands:", "Console");
        LOG_INFO_C("  help - Show this help message", "Console");
        LOG_INFO_C("  clear - Clear the console", "Console");
        LOG_INFO_C("  grid [on|off] - Toggle grid display", "Console");
        LOG_INFO_C("  play - Enter play mode", "Console");
        LOG_INFO_C("  stop - Exit play mode", "Console");
        LOG_INFO_C("  save - Save current world", "Console");
        LOG_INFO_C("  load - Load a world", "Console");
        LOG_INFO_C("  new - Create a new world", "Console");
        LOG_INFO_C("  quit - Exit the editor", "Console");
    }
    else if (cmdName == "clear") {
        if (m_nativeConsole) {
            m_nativeConsole->clear();
            LOG_INFO_C("Console cleared", "Console");
        }
    }
    else if (cmdName == "grid") {
        if (!cmdArgs.empty()) {
            std::transform(cmdArgs.begin(), cmdArgs.end(), cmdArgs.begin(), ::tolower);
            if (cmdArgs == "on") {
                m_showGrid = true;
                LOG_INFO_C("Grid display enabled", "Console");
            } else if (cmdArgs == "off") {
                m_showGrid = false;
                LOG_INFO_C("Grid display disabled", "Console");
            } else {
                LOG_WARNING_C("Usage: grid [on|off]", "Console");
            }
        } else {
            m_showGrid = !m_showGrid;
            LOG_INFO_C(m_showGrid ? "Grid display enabled" : "Grid display disabled", "Console");
        }
    }
    else if (cmdName == "play") {
        enterPlayMode();
        LOG_INFO_C("Entering play mode", "Console");
    }
    else if (cmdName == "stop") {
        exitPlayMode();
        LOG_INFO_C("Exiting play mode", "Console");
    }
    else if (cmdName == "save") {
        saveWorld();
        LOG_INFO_C("Save world command executed", "Console");
    }
    else if (cmdName == "load") {
        loadWorld();
        LOG_INFO_C("Load world command executed", "Console");
    }
    else if (cmdName == "new") {
        newWorld();
        LOG_INFO_C("New world command executed", "Console");
    }
    else if (cmdName == "quit" || cmdName == "exit") {
        LOG_INFO_C("Quit command received", "Console");
        // Request application exit
        #ifdef _WIN32
        if (m_window) {
            PostMessageW(m_window->getHandle(), WM_CLOSE, 0, 0);
        }
        #endif
    }
    else {
        LOG_WARNING_C("Unknown command: " + cmdName + " (type 'help' for available commands)", "Console");
    }
}
#endif

} // namespace fresh
