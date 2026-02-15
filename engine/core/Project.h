#pragma once

#include <string>
#include <memory>
#include <vector>

namespace fresh
{

class VoxelWorld;
class IWorldGenerator;

/**
 * @brief Project Settings
 * 
 * Configuration and metadata for a Fresh Engine project.
 * Similar to Unreal Engine's .uproject file.
 */
struct ProjectSettings
{
    std::string name;                  // Project name
    std::string path;                  // Project directory path
    std::string version;               // Engine version
    std::string description;           // Project description
    std::string templateType;          // Template used (Blank, 3D Voxel, 2D Platformer, etc.)
    bool hasWorld;                     // Whether project contains a world
    std::string worldGeneratorType;    // Type of world generator if hasWorld is true
    int worldSeed;                     // World generation seed if hasWorld is true
    
    ProjectSettings()
        : name("UntitledProject")
        , path("")
        , version("0.1.0")
        , description("")
        , templateType("Blank")
        , hasWorld(false)
        , worldGeneratorType("")
        , worldSeed(0)
    {}
};

/**
 * @brief Project Manager
 * 
 * Manages project lifecycle in Unreal Engine 5 style.
 * A project is the top-level container for all game content:
 * - Assets (textures, models, sounds)
 * - Scenes/Levels (optional worlds)
 * - Scripts (Lua, C#)
 * - Configuration
 * 
 * Unlike the old system where you create a "world" immediately,
 * now you create a "project" which starts as a blank canvas.
 * Worlds are created within projects as needed.
 */
class ProjectManager
{
public:
    ProjectManager();
    ~ProjectManager();

    // Prevent copying
    ProjectManager(const ProjectManager&) = delete;
    ProjectManager& operator=(const ProjectManager&) = delete;

    /**
     * @brief Create a new blank project
     * @param name Project name
     * @param path Directory path where project will be created
     * @param templateType Template to use (Blank, 3D Voxel, 2D Platformer, etc.)
     * @return true if project was created successfully
     */
    bool createNewProject(const std::string& name, 
                         const std::string& path,
                         const std::string& templateType = "Blank");

    /**
     * @brief Open an existing project
     * @param projectFilePath Path to project file (.freshproj)
     * @return true if project was opened successfully
     */
    bool openProject(const std::string& projectFilePath);

    /**
     * @brief Save current project
     * @return true if project was saved successfully
     */
    bool saveProject();

    /**
     * @brief Close current project
     */
    void closeProject();

    /**
     * @brief Check if a project is currently open
     * @return true if project is open
     */
    bool isProjectOpen() const { return m_projectOpen; }

    /**
     * @brief Get current project settings
     * @return Reference to project settings
     */
    const ProjectSettings& getProjectSettings() const { return m_settings; }

    /**
     * @brief Get modifiable project settings
     * @return Reference to project settings
     */
    ProjectSettings& getProjectSettings() { return m_settings; }

    /**
     * @brief Get project directory path
     * @return Project directory path
     */
    std::string getProjectPath() const { return m_settings.path; }

    /**
     * @brief Get project name
     * @return Project name
     */
    std::string getProjectName() const { return m_settings.name; }

    /**
     * @brief Create a new world within the current project
     * @param generatorType Type of world generator to use
     * @param seed World generation seed
     * @return Pointer to created world or nullptr on failure
     */
    VoxelWorld* createWorldInProject(const std::string& generatorType, int seed);

    /**
     * @brief Remove world from current project
     */
    void removeWorldFromProject();

    /**
     * @brief Check if current project has a world
     * @return true if project contains a world
     */
    bool hasWorld() const { return m_settings.hasWorld; }

    /**
     * @brief Get list of available project templates
     * @return Vector of template names
     */
    static std::vector<std::string> getAvailableTemplates();

    /**
     * @brief Get description for a template
     * @param templateType Template name
     * @return Template description
     */
    static std::string getTemplateDescription(const std::string& templateType);

private:
    bool loadProjectFile(const std::string& projectFilePath);
    bool saveProjectFile();
    bool createProjectDirectory(const std::string& path);
    bool applyTemplate(const std::string& templateType);

    ProjectSettings m_settings;
    bool m_projectOpen;
    std::string m_projectFilePath;
};

} // namespace fresh
