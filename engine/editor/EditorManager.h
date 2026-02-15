#pragma once

#include <functional>
#include <memory>
#include <string>

// Include stub headers OUTSIDE namespace to avoid nested namespace issues
// These define type aliases for Windows builds: SceneHierarchyPanel, InspectorPanel, ContentBrowserPanel, ConsolePanel, EditorMenuBar
#ifdef _WIN32
#include "ui/SceneHierarchyPanel.h"
#include "ui/InspectorPanel.h"
#include "ui/ContentBrowserPanel.h"
#include "ui/ConsolePanel.h"
#include "ui/EditorMenuBar.h"
#include "ui/NativeMainMenu.h"  // For WorldCreationParams
#endif

namespace fresh
{

#ifdef _WIN32
class Win32Window;
class Win32InputManager;
using WindowType = Win32Window;
using InputManagerType = Win32InputManager;
#else
class Window;
class InputManager;
using WindowType = Window;
using InputManagerType = InputManager;
#endif

class IRenderContext;
class VoxelWorld;
class WorldEditor;
class EditorToolbar;
class VoxelToolPalette;
// Removed ImGui-based panels (replaced with Win32 native versions):
// class MainMenuPanel;
// class SettingsPanel;
// class EngineConfigPanel;
class HotbarPanel;
class SelectionManager;
class SelectionRenderer;
class WorldSerializer;
class TransformGizmo;
class LayoutManager;
class EditorSettingsDialog;
class CameraController;
class ProjectManager;

namespace ecs
{
class EntityManager;
}

namespace devtools
{
class DebugRenderer;
}

#ifdef _WIN32
class WindowsThemeManager;
class WindowsDialogManager;
class WindowsTaskbarManager;
// Removed ImGui-based panel: class WindowsCustomizationPanel;
class Win32InspectorPanel;
class Win32SceneHierarchyPanel;
class Win32ContentBrowserPanel;
class Win32ConsolePanel;
class Win32HUD;
class Win32HUDOverlay;
class Win32SettingsDialog;
class Win32TerraformingPanel;
class Win32MenuBar;
class Win32ViewportPanel;
class Win32StatusBar;
class Win32LLMAssistantPanel;
#else
// Forward declarations for non-Windows platforms
class EditorMenuBar;
class InspectorPanel;
class SceneHierarchyPanel;
class ContentBrowserPanel;
class ConsolePanel;
#endif

/**
 * @brief Comprehensive Editor Manager
 *
 * Manages all editor UI panels and coordinates the editor interface.
 * This is the main entry point for the integrated game engine editor.
 */
class EditorManager
{
public:
    EditorManager();
    ~EditorManager();

    // Prevent copying
    EditorManager(const EditorManager&) = delete;
    EditorManager& operator=(const EditorManager&) = delete;

    /**
     * @brief Initialize the editor manager
     * @param window Main window
     * @param renderContext Render context for ImGui
     * @param world Voxel world (optional, can be nullptr - world-dependent panels will be initialized later)
     * @param worldEditor World editor instance (optional, can be nullptr - world-dependent panels will be initialized later)
     * @param inputManager Input manager for settings
     * @param entityManager Entity manager for component editing (optional)
     * @return true if successful
     */
    bool initialize(WindowType* window, IRenderContext* renderContext, VoxelWorld* world = nullptr,
                    WorldEditor* worldEditor = nullptr, InputManagerType* inputManager = nullptr,
                    ecs::EntityManager* entityManager = nullptr);

    /**
     * @brief Begin a new editor frame
     * Call this at the start of each frame
     */
    void beginFrame();

    /**
     * @brief Update editor state
     * @param deltaTime Time since last frame
     */
    void update(float deltaTime);

    /**
     * @brief Render all editor UI
     */
    void render();

    /**
     * @brief End the editor frame and submit to GPU
     */
    void endFrame();

    /**
     * @brief Shutdown the editor and cleanup
     */
    void shutdown();

    /**
     * @brief Update world and world editor references
     * @param world New voxel world reference
     * @param worldEditor New world editor reference
     * @return true if successful
     */
    bool updateWorld(VoxelWorld* world, WorldEditor* worldEditor);

    /**
     * @brief Set player reference for camera-based operations
     * @param player Player instance
     */
    void setPlayer(class Player* player);

    /**
     * @brief Set project manager reference
     * @param projectManager Project manager instance
     */
    void setProjectManager(class ProjectManager* projectManager);

#ifdef _WIN32
    /**
     * @brief Handle window resize - updates layout of all panels including viewport
     * @param clientWidth New client area width
     * @param clientHeight New client area height
     */
    void onWindowResize(int clientWidth, int clientHeight);
#endif

    /**
     * @brief Check if editor is initialized
     * @return true if initialized
     */
    bool isInitialized() const
    {
        return m_initialized;
    }

    /**
     * @brief Toggle editor visibility
     */
    void toggle()
    {
        m_visible = !m_visible;
    }

    /**
     * @brief Set editor visibility
     * @param visible true to show, false to hide
     */
    void setVisible(bool visible);

    /**
     * @brief Check if editor is visible
     * @return true if visible
     */
    bool isVisible() const
    {
        return m_visible;
    }

    /**
     * @brief Check if editor wants to capture mouse
     * @return true if editor is using the mouse
     */
    bool wantCaptureMouse() const;

    /**
     * @brief Check if editor wants to capture keyboard
     * @return true if editor is using the keyboard
     */
    bool wantCaptureKeyboard() const;

    /**
     * @brief Set callback for world creation
     * @param callback Function to call when a new world should be created (worldName, seed, is3D, gameStyle2D)
     */
    void setWorldCreationCallback(std::function<void(const WorldCreationParams&)> callback)
    {
        m_worldCreationCallback = callback;
    }

    /**
     * @brief Set callback for world loading
     * @param callback Function to call when a world should be loaded (worldName)
     */
    void setWorldLoadCallback(std::function<void(const std::string&)> callback)
    {
        m_worldLoadCallback = callback;
    }

    /**
     * @brief Set callback for entering play mode
     * @param callback Function to call when entering play mode
     */
    void setEnterPlayModeCallback(std::function<void()> callback)
    {
        m_enterPlayModeCallback = callback;
    }

    /**
     * @brief Set callback for exiting play mode
     * @param callback Function to call when exiting play mode
     */
    void setExitPlayModeCallback(std::function<void()> callback)
    {
        m_exitPlayModeCallback = callback;
    }

    // Removed ImGui-based panel getters (replaced with Win32 native versions):
    // MainMenuPanel* getMainMenuPanel() const
    // SettingsPanel* getSettingsPanel() const  
    // EngineConfigPanel* getEngineConfigPanel() const
    // WindowsCustomizationPanel* getWindowsCustomizationPanel() const

    /**
     * @brief Get the scene hierarchy panel
     * @return Scene hierarchy panel pointer
     */
    SceneHierarchyPanel* getSceneHierarchy() const
    {
        return m_sceneHierarchy.get();
    }

    /**
     * @brief Get the toolbar
     * @return Toolbar pointer
     */
    EditorToolbar* getToolbar() const
    {
        return m_toolbar.get();
    }

    /**
     * @brief Get the hotbar panel
     * @return Hotbar panel pointer
     */
    HotbarPanel* getHotbar() const
    {
        return m_hotbar.get();
    }

#ifdef _WIN32
    /**
     * @brief Get the native Win32 HUD
     * @return Win32 HUD pointer
     */
    Win32HUD* getHUD() const
    {
        return m_nativeHUD.get();
    }
#endif

    /**
     * @brief Get the voxel tool palette
     * @return Voxel tool palette pointer
     */
    VoxelToolPalette* getVoxelToolPalette() const
    {
        return m_voxelTools.get();
    }

    /**
     * @brief Get the selection manager
     * @return Selection manager pointer
     */
    SelectionManager* getSelectionManager() const
    {
        return m_selectionManager.get();
    }

    /**
     * @brief Get the selection renderer
     * @return Selection renderer pointer
     */
    SelectionRenderer* getSelectionRenderer() const
    {
        return m_selectionRenderer.get();
    }

    /**
     * @brief Get the transform gizmo
     * @return Transform gizmo pointer
     */
    TransformGizmo* getTransformGizmo() const
    {
        return m_transformGizmo.get();
    }

#ifdef _WIN32
    
    /**
     * @brief Get the native Win32 terraforming panel
     * @return Terraforming panel pointer (nullptr if not on Windows or panel not created)
     */
    Win32TerraformingPanel* getTerraformingPanel() const
    {
        return m_nativeTerraformingPanel.get();
    }
    
    /**
     * @brief Get the viewport panel
     * @return Viewport panel pointer (nullptr if not on Windows or panel not created)
     */
    Win32ViewportPanel* getViewportPanel() const
    {
        return m_viewportPanel.get();
    }
#endif

    // ========== Editor Operations (for native menu integration) ==========
    
    /**
     * @brief Perform undo operation
     */
    void undo();
    
    /**
     * @brief Perform redo operation
     */
    void redo();
    
    /**
     * @brief Cut selection to clipboard
     */
    void cut();
    
    /**
     * @brief Copy selection to clipboard
     */
    void copy();
    
    /**
     * @brief Paste from clipboard
     */
    void paste();
    
    /**
     * @brief Delete selected items
     */
    void deleteSelection();
    
    /**
     * @brief Select all items
     */
    void selectAll();
    
    /**
     * @brief Deselect all items
     */
    void deselectAll();
    
    /**
     * @brief Save current world
     */
    void saveWorld();
    
    /**
     * @brief Show save world dialog
     */
    void saveWorldAs();
    
    /**
     * @brief Show load world dialog
     */
    void loadWorld();
    
    /**
     * @brief Show new world dialog
     */
    void newWorld();
    
    /**
     * @brief Create new project
     */
    void newProject();
    
    /**
     * @brief Open existing project
     */
    void openProject();
    
    /**
     * @brief Toggle scene hierarchy panel visibility
     */
    void toggleSceneHierarchy();
    
    /**
     * @brief Toggle inspector panel visibility
     */
    void toggleInspector();
    
    /**
     * @brief Toggle content browser panel visibility
     */
    void toggleContentBrowser();
    
    /**
     * @brief Toggle console panel visibility
     */
    void toggleConsole();
    
    /**
     * @brief Toggle tool palette panel visibility
     */
    void toggleToolPalette();
    
    /**
     * @brief Toggle LLM assistant panel visibility
     */
    void toggleLLMAssistant();
    
#ifdef _WIN32
    /**
     * @brief Get the LLM assistant panel
     * @return LLM assistant panel pointer (nullptr if not on Windows or panel not created)
     */
    Win32LLMAssistantPanel* getLLMAssistantPanel() const
    {
        return m_nativeLLMAssistant.get();
    }
#endif
    
    /**
     * @brief Enter play mode (delegates to Engine via callback)
     */
    void enterPlayMode();
    
    /**
     * @brief Exit play mode (delegates to Engine via callback)
     */
    void exitPlayMode();
    
    /**
     * @brief Show settings panel
     */
    void showSettings();
    
    /**
     * @brief Show engine configuration panel
     */
    void showEngineConfig();
    
    /**
     * @brief Show editor settings dialog
     */
    void showEditorSettings();
    
    /**
     * @brief Show import assets dialog
     */
    void showImportAssets();
    
    /**
     * @brief Launch dialogue editor
     * Opens the dialogue editor tool for creating and editing game dialogues
     */
    void launchDialogueEditor();
    
    /**
     * @brief Load a workspace layout
     * @param name Layout name ("Default", "Minimal", "Debugging", or custom name)
     */
    void loadLayout(const std::string& name);
    
    /**
     * @brief Save the current workspace layout
     * @param name Layout name (if empty, saves to current layout)
     */
    void saveCurrentLayout(const std::string& name = "");
    
    /**
     * @brief Reset to default layout
     */
    void resetLayout();
    
    /**
     * @brief Get the layout manager
     * @return Layout manager pointer
     */
    LayoutManager* getLayoutManager() const
    {
        return m_layoutManager.get();
    }
    
    /**
     * @brief Get the editor settings dialog
     * @return Editor settings dialog pointer
     */
    EditorSettingsDialog* getEditorSettingsDialog() const
    {
        return m_editorSettingsDialog.get();
    }
    
    /**
     * @brief Get the camera controller
     * @return Camera controller pointer
     */
    CameraController* getCameraController() const
    {
        return m_cameraController.get();
    }
    
    // ========== Camera Control Operations ==========
    
    /**
     * @brief Set camera view mode
     * @param mode View mode name ("Perspective", "Top", "Front", "Side", etc.)
     */
    void setCameraViewMode(const std::string& mode);
    
    /**
     * @brief Focus camera on selection (F key)
     */
    void focusOnSelection();
    
    /**
     * @brief Frame selection in view
     */
    void frameSelection();
    
    /**
     * @brief Update FPS display in status bar
     * @param fps Current frames per second
     */
    void setFPS(float fps);
    
    /**
     * @brief Update memory usage display in status bar
     * @param usedMB Memory used in megabytes
     * @param totalMB Total available memory in megabytes
     */
    void setMemoryUsage(float usedMB, float totalMB);

#ifdef _WIN32
    /**
     * @brief Refresh all panel windows to ensure they are visible
     * 
     * Invalidates and updates all panel windows to trigger WM_PAINT messages,
     * ensuring they are properly displayed on screen. Should be called after:
     * - Initial panel creation and positioning
     * - Layout changes that might affect visibility
     */
    void refreshAllPanels();
    
    /**
     * @brief Ensure all UI panels are on top of the viewport in Z-order
     * 
     * This function brings all UI panels to the front using SetWindowPos with HWND_TOP,
     * ensuring they remain visible above the viewport window. Should be called after:
     * - Initial panel creation
     * - World updates (when new panels might be created)
     * - Window resizing (to maintain proper Z-order)
     */
    void ensurePanelsOnTop();
#endif

private:
#ifdef _WIN32
    /**
     * @brief Handle console command execution
     * @param command The command string entered by the user
     */
    void handleConsoleCommand(const std::string& command);
    
    /**
     * @brief Get the actual height of the Win32 toolbar
     * 
     * Queries the toolbar's actual height dynamically. If the toolbar is not available,
     * returns the TOOLBAR_HEIGHT constant as a fallback.
     * 
     * @return Actual toolbar height in pixels
     */
    int getActualToolbarHeight() const;
#endif

    bool m_initialized;
    bool m_visible;

    // UI Panels
    std::unique_ptr<SceneHierarchyPanel> m_sceneHierarchy;
    std::unique_ptr<InspectorPanel> m_inspector;
    std::unique_ptr<EditorMenuBar> m_menuBar;
    std::unique_ptr<EditorToolbar> m_toolbar;
    std::unique_ptr<ContentBrowserPanel> m_contentBrowser;
    std::unique_ptr<ConsolePanel> m_console;
    std::unique_ptr<VoxelToolPalette> m_voxelTools;
    // Removed ImGui-based panels (replaced with Win32 native versions):
    // std::unique_ptr<MainMenuPanel> m_mainMenuPanel;
    // std::unique_ptr<SettingsPanel> m_settingsPanel;
    // std::unique_ptr<EngineConfigPanel> m_engineConfigPanel;
    std::unique_ptr<HotbarPanel> m_hotbar;
    std::unique_ptr<SelectionManager> m_selectionManager;
    std::unique_ptr<SelectionRenderer> m_selectionRenderer;
    std::unique_ptr<TransformGizmo> m_transformGizmo;
    std::unique_ptr<devtools::DebugRenderer> m_debugRenderer;
    std::unique_ptr<WorldSerializer> m_worldSerializer;
    std::unique_ptr<LayoutManager> m_layoutManager;
    std::unique_ptr<EditorSettingsDialog> m_editorSettingsDialog;
    std::unique_ptr<CameraController> m_cameraController;

#ifdef _WIN32
    // Windows-native integration managers
    std::unique_ptr<WindowsThemeManager> m_windowsThemeManager;
    std::unique_ptr<WindowsDialogManager> m_windowsDialogManager;
    std::unique_ptr<WindowsTaskbarManager> m_windowsTaskbarManager;
    // Removed ImGui-based panel: std::unique_ptr<WindowsCustomizationPanel> m_windowsCustomizationPanel;
    
    // Native Win32 UI panels (used when FRESH_WIN32_UI is defined)
    std::unique_ptr<Win32InspectorPanel> m_nativeInspector;
    std::unique_ptr<Win32SceneHierarchyPanel> m_nativeSceneHierarchy;
    std::unique_ptr<Win32ContentBrowserPanel> m_nativeContentBrowser;
    std::unique_ptr<Win32ConsolePanel> m_nativeConsole;
    std::unique_ptr<Win32HUD> m_nativeHUD;
    std::unique_ptr<Win32HUDOverlay> m_hudOverlay;
    std::unique_ptr<Win32SettingsDialog> m_nativeSettingsDialog;
    std::unique_ptr<Win32TerraformingPanel> m_nativeTerraformingPanel;
    std::unique_ptr<Win32ViewportPanel> m_viewportPanel;
    std::unique_ptr<Win32StatusBar> m_statusBar;
    std::unique_ptr<Win32LLMAssistantPanel> m_nativeLLMAssistant;
#endif

    // References
    WindowType* m_window;
    IRenderContext* m_renderContext;
    VoxelWorld* m_world;
    WorldEditor* m_worldEditor;
    ecs::EntityManager* m_entityManager;
    class Player* m_player;
    class ProjectManager* m_projectManager;

    // Panel visibility flags
    bool m_showSceneHierarchy;
    bool m_showInspector;
    bool m_showContentBrowser;
    bool m_showConsole;
    bool m_showToolPalette;
    bool m_showLLMAssistant;
    bool m_showGrid;  // Grid display toggle
    
    // World file path
    std::string m_currentWorldPath;

    // Callbacks for world operations (set by Engine)
    std::function<void(const WorldCreationParams&)> m_worldCreationCallback;
    std::function<void(const std::string&)> m_worldLoadCallback;
    std::function<void()> m_enterPlayModeCallback;
    std::function<void()> m_exitPlayModeCallback;
};

} // namespace fresh
