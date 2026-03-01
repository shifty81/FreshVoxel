#pragma once

#include <memory>
#include <string>
#include <vector>

#include "dialogue/DialogueNode.h"

namespace fresh
{

/**
 * @brief A graph of interconnected dialogue nodes
 */
class DialogueGraph
{
public:
    DialogueGraph() = default;
    ~DialogueGraph() = default;

    /**
     * @brief Add a new node to the graph
     */
    DialogueNode* addNode(DialogueNodeType type);

    /**
     * @brief Remove a node from the graph
     */
    bool removeNode(int id);

    /**
     * @brief Find a node by its id
     */
    [[nodiscard]] DialogueNode* getNode(int id) const;

    /**
     * @brief Get the start node of the graph
     */
    [[nodiscard]] DialogueNode* getStartNode() const;

    /**
     * @brief Set the start node of the graph
     */
    void setStartNode(int id);

    [[nodiscard]] const std::vector<std::unique_ptr<DialogueNode>>& getNodes() const
    {
        return m_nodes;
    }
    [[nodiscard]] const std::string& getName() const { return m_name; }

    void setName(const std::string& name);

    /**
     * @brief Remove all nodes
     */
    void clear();

    DialogueGraph(const DialogueGraph&) = delete;
    DialogueGraph& operator=(const DialogueGraph&) = delete;
    DialogueGraph(DialogueGraph&&) = default;
    DialogueGraph& operator=(DialogueGraph&&) = default;

private:
    std::string m_name;
    std::vector<std::unique_ptr<DialogueNode>> m_nodes;
    int m_startNodeId = -1;
    int m_nextNodeId = 0;
};

} // namespace fresh
