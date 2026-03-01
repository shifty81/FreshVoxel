#pragma once

#include <string>
#include <unordered_map>

#include "dialogue/DialogueGraph.h"

namespace fresh
{

/**
 * @brief Current state of the dialogue system
 */
enum class DialogueState
{
    Inactive,
    ShowingDialogue,
    ShowingChoices,
    WaitingForInput,
    ExecutingScript,
    Complete
};

/**
 * @brief Manages dialogue playback through a dialogue graph
 *
 * Condition nodes evaluate simple expressions of the form
 * "variable operator value" (e.g., "player_gold > 100").
 * Supported operators: ==, !=, >, <, >=, <=.
 * Variables are set via setVariable() before starting dialogue.
 */
class DialogueManager
{
public:
    DialogueManager() = default;
    ~DialogueManager() = default;

    /**
     * @brief Start a dialogue from the graph's Start node
     * @return true if the dialogue was started successfully
     */
    bool startDialogue(DialogueGraph* graph);

    /**
     * @brief Advance to the next node from a Dialogue node
     * @return true if advancement succeeded
     */
    bool advance();

    /**
     * @brief Select a choice by its id from the current Choice node
     * @return true if the choice was found and followed
     */
    bool selectChoice(int choiceId);

    [[nodiscard]] DialogueNode* getCurrentNode() const { return m_currentNode; }
    [[nodiscard]] DialogueState getState() const { return m_state; }
    [[nodiscard]] bool isActive() const { return m_state != DialogueState::Inactive && m_state != DialogueState::Complete; }

    /**
     * @brief Stop the current dialogue
     */
    void stopDialogue();

    /**
     * @brief Set a dialogue variable for condition evaluation
     */
    void setVariable(const std::string& name, int value);

    /**
     * @brief Get a dialogue variable value (returns 0 if not set)
     */
    int getVariable(const std::string& name) const;

    DialogueManager(const DialogueManager&) = delete;
    DialogueManager& operator=(const DialogueManager&) = delete;

private:
    /**
     * @brief Transition to a node by id, automatically passing through
     *        LuaScript and Condition nodes
     */
    bool transitionTo(int nodeId);

    /**
     * @brief Evaluate a condition expression (e.g., "player_gold > 100")
     * @return true if the condition is met
     */
    bool evaluateCondition(const std::string& condition) const;

    DialogueGraph* m_currentGraph = nullptr;
    DialogueNode* m_currentNode = nullptr;
    DialogueState m_state = DialogueState::Inactive;
    std::unordered_map<std::string, int> m_variables;
};

} // namespace fresh
