#pragma once

#include <functional>
#include <string>
#include <vector>

#include <Windows.h>

namespace fresh
{

/**
 * @brief 2D World style enumeration
 */
enum class WorldStyle2D {
    Platformer = 0,  // Side-scrolling (Terraria-like)
    TopDown = 1      // Overhead view (Zelda-like)
};

/**
 * @brief World size enumeration
 */
enum class WorldSize {
    Small = 0,    // 8 chunks radius
    Medium = 1,   // 16 chunks radius
    Large = 2,    // 24 chunks radius
    Huge = 3      // 32 chunks radius
};

/**
 * @brief Terrain type enumeration
 */
enum class TerrainType {
    Flat = 0,
    Hills = 1,
    Mountains = 2,
    Islands = 3
};

/**
 * @brief Biome type enumeration
 */
enum class BiomeType {
    Forest = 0,
    Desert = 1,
    Snow = 2,
    Jungle = 3,
    Mixed = 4
};

/**
 * @brief World creation parameters structure
 */
struct WorldCreationParams {
    std::string name;
    int seed;
    bool is3D;
    int gameStyle2D; // WorldStyle2D cast to int for backward compatibility
    WorldSize worldSize;
    TerrainType terrainType;
    BiomeType biomeType;

    WorldCreationParams()
        : name("New Project")
        , seed(0)
        , is3D(true)
        , gameStyle2D(0)
        , worldSize(WorldSize::Medium)
        , terrainType(TerrainType::Hills)
        , biomeType(BiomeType::Mixed)
    {}
};

/**
 * @brief Native Windows Main Menu using Win32 API
 *
 * This is a proof-of-concept replacement for MainMenuPanel that uses
 * pure Win32 API instead of ImGui. Features:
 * - Native Windows dialog
 * - Dark theme with Unreal Engine colors
 * - Fully opaque windows (no transparency issues)
 * - Native buttons, text boxes, and controls
 * - Modal dialogs for world creation
 */
class NativeMainMenu
{
public:
    NativeMainMenu();
    ~NativeMainMenu();

    /**
     * @brief Initialize the native main menu
     * @param parentWindow Parent window handle (can be nullptr for standalone)
     * @return true if successful
     */
    bool initialize(HWND parentWindow);

    /**
     * @brief Show the main menu (blocks until user makes selection)
     * @return true if user wants to proceed (create/load world)
     */
    bool show();

    /**
     * @brief Show the project creation dialog directly (for File → New Project menu)
     * @return true if user confirmed project creation
     */
    bool showCreateWorldDialog();

    /**
     * @brief Show the load project dialog directly (for File → Open Project menu)
     * @return true if user selected a project to load
     */
    bool showLoadWorldDialog();

    /**
     * @brief Check if user wants to create a new world
     */
    bool shouldCreateNewWorld() const
    {
        return m_createNewWorld;
    }

    /**
     * @brief Check if user wants to load existing world
     */
    bool shouldLoadWorld() const
    {
        return m_loadWorld;
    }

    /**
     * @brief Get the new world name
     */
    const std::wstring& getNewWorldName() const
    {
        return m_newWorldName;
    }

    /**
     * @brief Get the world seed
     */
    int getWorldSeed() const
    {
        return m_worldSeed;
    }

    /**
     * @brief Check if world should be 3D
     */
    bool isWorld3D() const
    {
        return m_isWorld3D;
    }

    /**
     * @brief Get the 2D world style
     */
    WorldStyle2D get2DWorldStyle() const
    {
        return m_world2DStyle;
    }

    /**
     * @brief Get the world size
     */
    WorldSize getWorldSize() const
    {
        return m_worldSize;
    }

    /**
     * @brief Get the terrain type
     */
    TerrainType getTerrainType() const
    {
        return m_terrainType;
    }

    /**
     * @brief Get the biome type
     */
    BiomeType getBiomeType() const
    {
        return m_biomeType;
    }

    /**
     * @brief Get the world to load
     */
    const std::wstring& getLoadWorldName() const
    {
        return m_loadWorldName;
    }

private:
    // Dialog procedure for main menu
    static INT_PTR CALLBACK mainMenuDialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    // Dialog procedure for world creation
    static INT_PTR CALLBACK createWorldDialogProc(HWND hwnd, UINT msg, WPARAM wParam,
                                                  LPARAM lParam);

    // Dialog procedure for world loading
    static INT_PTR CALLBACK loadWorldDialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    // Create the main menu dialog
    void createMainMenuDialog(HWND hwnd);

    // Create the world creation dialog
    void createWorldCreationDialog(HWND hwnd);

    // Create the world loading dialog
    void createWorldLoadingDialog(HWND hwnd);

    // Apply dark theme to window
    void applyDarkTheme(HWND hwnd);

    // Apply dark theme to control
    void applyDarkThemeToControl(HWND control);

    // Scan for saved worlds
    void scanWorldSaves();

private:
    HWND m_parentWindow;
    HWND m_mainDialog;

    // State
    bool m_createNewWorld;
    bool m_loadWorld;
    bool m_isWorld3D;
    WorldStyle2D m_world2DStyle;
    WorldSize m_worldSize;
    TerrainType m_terrainType;
    BiomeType m_biomeType;

    // World data
    std::wstring m_newWorldName;
    int m_worldSeed;
    std::wstring m_loadWorldName;
    std::vector<std::wstring> m_worldSaves;

    // Control IDs
    enum ControlIDs {
        ID_BTN_CREATE_WORLD = 1001,
        ID_BTN_LOAD_WORLD = 1002,
        ID_BTN_EXIT = 1003,
        ID_EDIT_WORLD_NAME = 1004,
        ID_EDIT_SEED = 1005,
        ID_RADIO_3D = 1006,
        ID_RADIO_2D = 1007,
        ID_BTN_CREATE = 1008,
        ID_BTN_CANCEL = 1009,
        ID_LIST_WORLDS = 1010,
        ID_BTN_LOAD = 1011,
        ID_STATIC_TITLE = 1012,
        ID_RADIO_2D_PLATFORMER = 1013,
        ID_RADIO_2D_TOPDOWN = 1014,
        ID_STATIC_2D_STYLE = 1015,
        ID_COMBO_WORLD_SIZE = 1016,
        ID_COMBO_TERRAIN_TYPE = 1017,
        ID_COMBO_BIOME_TYPE = 1018,
    };

    // Unreal Engine dark theme colors
    struct UnrealColors {
        static constexpr COLORREF WindowBg = RGB(32, 32, 32);        // #202020
        static constexpr COLORREF ButtonNormal = RGB(0, 122, 204);   // #007ACC
        static constexpr COLORREF ButtonHover = RGB(28, 151, 234);   // #1C97EA
        static constexpr COLORREF ButtonActive = RGB(0, 98, 163);    // #0062A3
        static constexpr COLORREF TextNormal = RGB(241, 241, 241);   // #F1F1F1
        static constexpr COLORREF TextDisabled = RGB(108, 108, 108); // #6C6C6C
        static constexpr COLORREF InputBg = RGB(51, 51, 51);         // #333333
    };
};

} // namespace fresh
