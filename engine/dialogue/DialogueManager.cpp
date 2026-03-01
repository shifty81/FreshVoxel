#include "dialogue/DialogueManager.h"

#include <algorithm>
#include <sstream>

namespace fresh
{

void DialogueManager::setVariable(const std::string& name, int value)
{
    m_variables[name] = value;
}

int DialogueManager::getVariable(const std::string& name) const
{
    auto it = m_variables.find(name);
    if (it != m_variables.end()) {
        return it->second;
    }
    return 0;
}

bool DialogueManager::evaluateCondition(const std::string& condition) const
{
    // Parse "variable operator value" expressions
    // Supported operators: ==, !=, >, <, >=, <=
    std::istringstream iss(condition);
    std::string varName, op;
    int value = 0;

    if (!(iss >> varName >> op >> value)) {
        // Malformed expression — default to false
        return false;
    }

    int varValue = getVariable(varName);

    if (op == "==") return varValue == value;
    if (op == "!=") return varValue != value;
    if (op == ">")  return varValue > value;
    if (op == "<")  return varValue < value;
    if (op == ">=") return varValue >= value;
    if (op == "<=") return varValue <= value;

    // Unknown operator — default to false
    return false;
}

bool DialogueManager::startDialogue(DialogueGraph* graph)
{
    if (!graph) {
        return false;
    }

    DialogueNode* startNode = graph->getStartNode();
    if (!startNode || startNode->getType() != DialogueNodeType::Start) {
        return false;
    }

    m_currentGraph = graph;
    m_currentNode = startNode;
    m_state = DialogueState::Inactive;

    // Advance past the Start node to the first real node
    return transitionTo(startNode->getNextNodeId());
}

bool DialogueManager::advance()
{
    if (!m_currentNode || m_state == DialogueState::Inactive ||
        m_state == DialogueState::Complete) {
        return false;
    }

    if (m_currentNode->getType() != DialogueNodeType::Dialogue) {
        return false;
    }

    return transitionTo(m_currentNode->getNextNodeId());
}

bool DialogueManager::selectChoice(int choiceId)
{
    if (!m_currentNode || m_state != DialogueState::ShowingChoices) {
        return false;
    }

    if (m_currentNode->getType() != DialogueNodeType::Choice) {
        return false;
    }

    const auto& choices = m_currentNode->getChoices();
    auto it = std::find_if(choices.begin(), choices.end(),
                           [choiceId](const DialogueChoice& c) { return c.id == choiceId; });
    if (it == choices.end()) {
        return false;
    }

    return transitionTo(it->targetNodeId);
}

void DialogueManager::stopDialogue()
{
    m_currentGraph = nullptr;
    m_currentNode = nullptr;
    m_state = DialogueState::Inactive;
}

bool DialogueManager::transitionTo(int nodeId)
{
    if (!m_currentGraph || nodeId < 0) {
        m_state = DialogueState::Complete;
        m_currentNode = nullptr;
        return false;
    }

    DialogueNode* node = m_currentGraph->getNode(nodeId);
    if (!node) {
        m_state = DialogueState::Complete;
        m_currentNode = nullptr;
        return false;
    }

    m_currentNode = node;

    switch (node->getType()) {
    case DialogueNodeType::Dialogue:
        m_state = DialogueState::ShowingDialogue;
        return true;

    case DialogueNodeType::Choice:
        m_state = DialogueState::ShowingChoices;
        return true;

    case DialogueNodeType::End:
        m_state = DialogueState::Complete;
        return true;

    case DialogueNodeType::LuaScript:
        m_state = DialogueState::ExecutingScript;
        // Auto-advance past script nodes
        return transitionTo(node->getNextNodeId());

    case DialogueNodeType::Condition:
        if (evaluateCondition(node->getCondition())) {
            return transitionTo(node->getTrueBranchNodeId());
        } else {
            return transitionTo(node->getFalseBranchNodeId());
        }

    case DialogueNodeType::Start:
        // Should not encounter Start mid-graph; advance past it
        return transitionTo(node->getNextNodeId());
    }

    return false;
}

} // namespace fresh
