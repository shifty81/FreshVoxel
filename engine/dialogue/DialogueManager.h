#pragma once

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
 * @note Condition nodes currently always follow the true branch.
 *       Condition expression evaluation will be implemented when the
 *       Lua scripting integration is wired into the dialogue system.
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

    DialogueManager(const DialogueManager&) = delete;
    DialogueManager& operator=(const DialogueManager&) = delete;

private:
    /**
     * @brief Transition to a node by id, automatically passing through
     *        LuaScript and Condition nodes
     */
    bool transitionTo(int nodeId);

    DialogueGraph* m_currentGraph = nullptr;
    DialogueNode* m_currentNode = nullptr;
    DialogueState m_state = DialogueState::Inactive;
};

} // namespace fresh
