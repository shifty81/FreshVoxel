#pragma once

#include <string>
#include <vector>

namespace fresh
{

/**
 * @brief Provides project-specific context for the LLM assistant
 * 
 * Generates system prompts that give the LLM deep knowledge about the
 * Fresh Voxel Engine, including its architecture, Lua scripting API,
 * editor tools, voxel types, and game building workflows.
 * 
 * This enables the LLM to provide contextual assistance when developers
 * are building games with the engine.
 */
class ProjectContext
{
public:
    /**
     * @brief Context topic categories for focused assistance
     */
    enum class Topic {
        General,        ///< Overall engine knowledge
        LuaScripting,   ///< Lua scripting API and examples
        VoxelBuilding,  ///< Voxel types, world building, terrain
        EditorTools,    ///< Editor tools and workflows
        NPCAndAI,       ///< NPC creation and AI behaviors
        GameDesign      ///< Game design patterns and best practices
    };
    
    ProjectContext();
    ~ProjectContext() = default;
    
    /**
     * @brief Generate the full system prompt for the LLM
     * @param topic Optional topic focus for more relevant responses
     * @return System prompt string with project context
     */
    std::string generateSystemPrompt(Topic topic = Topic::General) const;
    
    /**
     * @brief Get the core engine description
     * @return Engine description for the system prompt
     */
    static std::string getEngineDescription();
    
    /**
     * @brief Get Lua scripting API reference
     * @return Lua API reference for the system prompt
     */
    static std::string getLuaScriptingContext();
    
    /**
     * @brief Get voxel types and building reference
     * @return Voxel building reference for the system prompt
     */
    static std::string getVoxelBuildingContext();
    
    /**
     * @brief Get editor tools reference
     * @return Editor tools reference for the system prompt
     */
    static std::string getEditorToolsContext();
    
    /**
     * @brief Get NPC and AI system reference
     * @return NPC/AI reference for the system prompt
     */
    static std::string getNPCAndAIContext();
    
    /**
     * @brief Get available topic names for display
     * @return Vector of topic name strings
     */
    static std::vector<std::string> getTopicNames();
    
    /**
     * @brief Convert a topic enum to display name
     * @param topic The topic to convert
     * @return Display name string
     */
    static std::string topicToString(Topic topic);
};

} // namespace fresh
