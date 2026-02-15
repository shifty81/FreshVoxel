#include "ui/MainMenu.h"

#include <cstring>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

namespace fresh
{

MainMenu::MainMenu()
    : m_state(MenuState::MainMenu),
      m_createNewWorld(false),
      m_loadWorld(false),
      m_worldSeed(0),
      m_selectedWorldIndex(0),
      m_selectedMainMenuItem(0)
{
    std::memset(m_worldNameBuffer, 0, sizeof(m_worldNameBuffer));
    std::memset(m_seedBuffer, 0, sizeof(m_seedBuffer));

    // Default world name
#ifdef _WIN32
    // Use strncpy_s on Windows to avoid C4996 warning
    strncpy_s(m_worldNameBuffer, sizeof(m_worldNameBuffer), "New Project", _TRUNCATE);
#else
    std::strncpy(m_worldNameBuffer, "New Project", sizeof(m_worldNameBuffer) - 1);
#endif
    m_worldNameBuffer[sizeof(m_worldNameBuffer) - 1] = '\0'; // Ensure null termination

    // Random seed by default
    m_worldSeed = static_cast<int>(std::time(nullptr));
    std::snprintf(m_seedBuffer, sizeof(m_seedBuffer), "%d", m_worldSeed);
}

MainMenu::~MainMenu() {}

bool MainMenu::initialize()
{
    std::cout << "Initializing Main Menu..." << std::endl;
    scanWorldSaves();
    return true;
}

void MainMenu::update(float deltaTime)
{
    (void)deltaTime; // Parameter reserved for future use
    // Menu state updates would happen here
    // For now, this is handled in render()
}

void MainMenu::render()
{
    switch (m_state) {
    case MenuState::MainMenu:
        renderMainMenu();
        break;
    case MenuState::NewWorld:
        renderNewWorldMenu();
        break;
    case MenuState::LoadWorld:
        renderLoadWorldMenu();
        break;
    case MenuState::Settings:
        renderSettingsMenu();
        break;
    default:
        break;
    }
}

void MainMenu::renderMainMenu()
{
    // Console-based menu for pre-initialization phase
    // This menu runs BEFORE graphics/window initialization, so cannot use mouse input
    // All in-game features and editor support full mouse interaction once initialized
    // 
    // Future enhancement: Move menu to post-initialization for graphical mouse-enabled UI

    std::cout << "\n";
    std::cout << "========================================\n";
    std::cout << "       FRESH VOXEL ENGINE\n";
    std::cout << "========================================\n";
    std::cout << "\n";
    std::cout << "  1. Create New Project\n";
    std::cout << "  2. Load Existing Project\n";
    std::cout << "  3. Settings\n";
    std::cout << "  4. Exit\n";
    std::cout << "\n";
    std::cout << "Select option (1-4): ";

    int choice;
    std::cin >> choice;

    switch (choice) {
    case 1:
        m_state = MenuState::NewWorld;
        break;
    case 2:
        m_state = MenuState::LoadWorld;
        scanWorldSaves();
        break;
    case 3:
        m_state = MenuState::Settings;
        break;
    case 4:
        std::cout << "Exiting...\n";
        exit(0);
        break;
    default:
        std::cout << "Invalid choice. Please try again.\n";
        break;
    }
}

void MainMenu::renderNewWorldMenu()
{
    std::cout << "\n";
    std::cout << "========================================\n";
    std::cout << "        CREATE NEW WORLD\n";
    std::cout << "========================================\n";
    std::cout << "\n";

    std::cout << "Enter world name: ";
    std::cin.ignore();
    std::cin.getline(m_worldNameBuffer, sizeof(m_worldNameBuffer));

    std::cout << "Enter world seed (or press Enter for random): ";
    std::cin.getline(m_seedBuffer, sizeof(m_seedBuffer));

    if (std::strlen(m_seedBuffer) > 0) {
        m_worldSeed = std::atoi(m_seedBuffer);
    } else {
        m_worldSeed = static_cast<int>(std::time(nullptr));
    }

    m_newWorldName = std::string(m_worldNameBuffer);

    std::cout << "\n";
    std::cout << "Creating world '" << m_newWorldName << "' with seed " << m_worldSeed << "...\n";
    std::cout << "\n";

    m_createNewWorld = true;
    m_state = MenuState::InGame;
}

void MainMenu::renderLoadWorldMenu()
{
    std::cout << "\n";
    std::cout << "========================================\n";
    std::cout << "        LOAD EXISTING WORLD\n";
    std::cout << "========================================\n";
    std::cout << "\n";

    if (m_worldSaves.empty()) {
        std::cout << "No saved worlds found.\n";
        std::cout << "\n";
        std::cout << "Press Enter to return to main menu...";
        std::cin.ignore();
        std::cin.get();
        m_state = MenuState::MainMenu;
        return;
    }

    std::cout << "Available worlds:\n";
    for (size_t i = 0; i < m_worldSaves.size(); ++i) {
        std::cout << "  " << (i + 1) << ". " << m_worldSaves[i] << "\n";
    }
    std::cout << "  " << (m_worldSaves.size() + 1) << ". Back to main menu\n";
    std::cout << "\n";
    std::cout << "Select world to load: ";

    int choice;
    std::cin >> choice;

    if (choice > 0 && choice <= static_cast<int>(m_worldSaves.size())) {
        m_loadWorldName = m_worldSaves[choice - 1];
        std::cout << "\n";
        std::cout << "Loading world '" << m_loadWorldName << "'...\n";
        std::cout << "\n";

        m_loadWorld = true;
        m_state = MenuState::InGame;
    } else if (choice == static_cast<int>(m_worldSaves.size() + 1)) {
        m_state = MenuState::MainMenu;
    } else {
        std::cout << "Invalid choice. Please try again.\n";
    }
}

void MainMenu::renderSettingsMenu()
{
    std::cout << "\n";
    std::cout << "========================================\n";
    std::cout << "           SETTINGS\n";
    std::cout << "========================================\n";
    std::cout << "\n";
    std::cout << "  1. Render Distance: 8 chunks\n";
    std::cout << "  2. Graphics Quality: High\n";
    std::cout << "  3. Back to Main Menu\n";
    std::cout << "\n";
    std::cout << "Select option: ";

    int choice;
    std::cin >> choice;

    if (choice == 3) {
        m_state = MenuState::MainMenu;
    } else {
        std::cout << "Settings changes not yet implemented.\n";
    }
}

void MainMenu::clearFlags()
{
    m_createNewWorld = false;
    m_loadWorld = false;
}

void MainMenu::scanWorldSaves()
{
    m_worldSaves.clear();

    std::string savePath = "saves";

    try {
        // Create saves directory if it doesn't exist
        if (!fs::exists(savePath)) {
            fs::create_directories(savePath);
            std::cout << "Created saves directory: " << savePath << std::endl;
            return;
        }

        // Scan for .world files
        for (const auto& entry : fs::directory_iterator(savePath)) {
            if (entry.is_regular_file() && entry.path().extension() == ".world") {
                m_worldSaves.push_back(entry.path().stem().string());
            }
        }

        std::cout << "Found " << m_worldSaves.size() << " saved worlds" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error scanning world saves: " << e.what() << std::endl;
    }
}

} // namespace fresh
