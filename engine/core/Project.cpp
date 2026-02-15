#include "core/Project.h"
#include "core/Logger.h"
#include "voxel/VoxelWorld.h"
#include "generation/IWorldGenerator.h"

#include <fstream>
#include <filesystem>
#include <sstream>

namespace fs = std::filesystem;

namespace fresh
{

ProjectManager::ProjectManager()
    : m_projectOpen(false)
    , m_projectFilePath("")
{
}

ProjectManager::~ProjectManager()
{
    if (m_projectOpen) {
        closeProject();
    }
}

bool ProjectManager::createNewProject(const std::string& name, 
                                      const std::string& path,
                                      const std::string& templateType)
{
    LOG_INFO("Creating new project: " + name + " at " + path);

    // Validate inputs
    if (name.empty() || path.empty()) {
        LOG_ERROR("Project name and path cannot be empty");
        return false;
    }

    // Create project directory structure
    if (!createProjectDirectory(path)) {
        LOG_ERROR("Failed to create project directory: " + path);
        return false;
    }

    // Initialize project settings
    m_settings.name = name;
    m_settings.path = path;
    m_settings.version = "0.1.0";
    m_settings.templateType = templateType;
    m_settings.hasWorld = false;
    m_settings.worldGeneratorType = "";
    m_settings.worldSeed = 0;

    // Apply template
    if (!applyTemplate(templateType)) {
        LOG_WARNING("Failed to apply template: " + templateType + ", using blank project");
    }

    // Save project file
    m_projectFilePath = path + "/" + name + ".freshproj";
    if (!saveProjectFile()) {
        LOG_ERROR("Failed to save project file: " + m_projectFilePath);
        return false;
    }

    m_projectOpen = true;
    LOG_INFO("Project created successfully: " + name);
    return true;
}

bool ProjectManager::openProject(const std::string& projectFilePath)
{
    LOG_INFO("Opening project: " + projectFilePath);

    if (!fs::exists(projectFilePath)) {
        LOG_ERROR("Project file does not exist: " + projectFilePath);
        return false;
    }

    if (!loadProjectFile(projectFilePath)) {
        LOG_ERROR("Failed to load project file: " + projectFilePath);
        return false;
    }

    m_projectFilePath = projectFilePath;
    m_projectOpen = true;
    LOG_INFO("Project opened successfully: " + m_settings.name);
    return true;
}

bool ProjectManager::saveProject()
{
    if (!m_projectOpen) {
        LOG_ERROR("No project is currently open");
        return false;
    }

    return saveProjectFile();
}

void ProjectManager::closeProject()
{
    if (m_projectOpen) {
        LOG_INFO("Closing project: " + m_settings.name);
        m_projectOpen = false;
        m_projectFilePath = "";
        m_settings = ProjectSettings(); // Reset to default
    }
}

VoxelWorld* ProjectManager::createWorldInProject(const std::string& generatorType, int seed)
{
    if (!m_projectOpen) {
        LOG_ERROR("No project is open");
        return nullptr;
    }

    LOG_INFO("Creating world in project with generator: " + generatorType + ", seed: " + std::to_string(seed));

    // Update project settings
    m_settings.hasWorld = true;
    m_settings.worldGeneratorType = generatorType;
    m_settings.worldSeed = seed;

    // Save updated project settings
    saveProjectFile();

    // Note: World creation is handled by Engine, this just updates project metadata
    return nullptr; // Engine will create the actual world
}

void ProjectManager::removeWorldFromProject()
{
    if (!m_projectOpen) {
        LOG_ERROR("No project is open");
        return;
    }

    LOG_INFO("Removing world from project");
    m_settings.hasWorld = false;
    m_settings.worldGeneratorType = "";
    m_settings.worldSeed = 0;

    // Save updated project settings
    saveProjectFile();
}

std::vector<std::string> ProjectManager::getAvailableTemplates()
{
    return {
        "Blank",
        "3D Voxel Game",
        "2D Platformer",
        "2D Top-Down"
    };
}

std::string ProjectManager::getTemplateDescription(const std::string& templateType)
{
    if (templateType == "Blank") {
        return "Empty project with no pre-generated content. Build from scratch.";
    } else if (templateType == "3D Voxel Game") {
        return "Minecraft-style 3D voxel world with procedural terrain.";
    } else if (templateType == "2D Platformer") {
        return "Terraria-style side-scrolling platformer world.";
    } else if (templateType == "2D Top-Down") {
        return "Zelda-style top-down 2D world.";
    } else {
        return "Unknown template type.";
    }
}

bool ProjectManager::loadProjectFile(const std::string& projectFilePath)
{
    std::ifstream file(projectFilePath);
    if (!file.is_open()) {
        return false;
    }

    // Simple text-based format for now
    // Format:
    // NAME=value
    // PATH=value
    // VERSION=value
    // DESCRIPTION=value
    // TEMPLATE=value
    // HAS_WORLD=true/false
    // WORLD_GENERATOR=value
    // WORLD_SEED=value

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue; // Skip empty lines and comments

        size_t pos = line.find('=');
        if (pos == std::string::npos) continue;

        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);

        if (key == "NAME") {
            m_settings.name = value;
        } else if (key == "PATH") {
            m_settings.path = value;
        } else if (key == "VERSION") {
            m_settings.version = value;
        } else if (key == "DESCRIPTION") {
            m_settings.description = value;
        } else if (key == "TEMPLATE") {
            m_settings.templateType = value;
        } else if (key == "HAS_WORLD") {
            m_settings.hasWorld = (value == "true" || value == "1");
        } else if (key == "WORLD_GENERATOR") {
            m_settings.worldGeneratorType = value;
        } else if (key == "WORLD_SEED") {
            m_settings.worldSeed = std::stoi(value);
        }
    }

    file.close();
    return true;
}

bool ProjectManager::saveProjectFile()
{
    std::ofstream file(m_projectFilePath);
    if (!file.is_open()) {
        return false;
    }

    file << "# Fresh Engine Project File\n";
    file << "# Generated by Fresh Voxel Engine\n\n";
    file << "NAME=" << m_settings.name << "\n";
    file << "PATH=" << m_settings.path << "\n";
    file << "VERSION=" << m_settings.version << "\n";
    file << "DESCRIPTION=" << m_settings.description << "\n";
    file << "TEMPLATE=" << m_settings.templateType << "\n";
    file << "HAS_WORLD=" << (m_settings.hasWorld ? "true" : "false") << "\n";
    file << "WORLD_GENERATOR=" << m_settings.worldGeneratorType << "\n";
    file << "WORLD_SEED=" << m_settings.worldSeed << "\n";

    file.close();
    return true;
}

bool ProjectManager::createProjectDirectory(const std::string& path)
{
    try {
        // Create main project directory
        fs::create_directories(path);

        // Create standard subdirectories (Unreal Engine style)
        fs::create_directories(path + "/Content");       // Assets, textures, models
        fs::create_directories(path + "/Content/Maps");  // Scenes/Levels
        fs::create_directories(path + "/Content/Assets"); // Imported assets
        fs::create_directories(path + "/Content/Scripts"); // Lua scripts
        fs::create_directories(path + "/Config");        // Configuration files
        fs::create_directories(path + "/Saved");         // Save games, logs

        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to create project directory: " + std::string(e.what()));
        return false;
    }
}

bool ProjectManager::applyTemplate(const std::string& templateType)
{
    // Template application sets up initial project state
    // For now, templates don't create worlds immediately
    // They just configure project settings for future world creation

    if (templateType == "Blank") {
        // Nothing to do for blank template
        m_settings.description = "Blank project";
        return true;
    } else if (templateType == "3D Voxel Game") {
        m_settings.description = "3D Voxel Game Project";
        // Don't create world yet, just set preference
        return true;
    } else if (templateType == "2D Platformer") {
        m_settings.description = "2D Platformer Project";
        return true;
    } else if (templateType == "2D Top-Down") {
        m_settings.description = "2D Top-Down Project";
        return true;
    }

    return false;
}

} // namespace fresh
