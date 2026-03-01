#include "dialogue/DialogueGraph.h"

#include <algorithm>

namespace fresh
{

DialogueNode* DialogueGraph::addNode(DialogueNodeType type)
{
    auto node = std::make_unique<DialogueNode>(m_nextNodeId++, type);
    auto* ptr = node.get();
    m_nodes.push_back(std::move(node));
    return ptr;
}

bool DialogueGraph::removeNode(int id)
{
    auto it = std::find_if(m_nodes.begin(), m_nodes.end(),
                           [id](const auto& node) { return node->getId() == id; });
    if (it == m_nodes.end()) {
        return false;
    }

    if (m_startNodeId == id) {
        m_startNodeId = -1;
    }

    m_nodes.erase(it);
    return true;
}

DialogueNode* DialogueGraph::getNode(int id) const
{
    auto it = std::find_if(m_nodes.begin(), m_nodes.end(),
                           [id](const auto& node) { return node->getId() == id; });
    if (it != m_nodes.end()) {
        return it->get();
    }
    return nullptr;
}

DialogueNode* DialogueGraph::getStartNode() const
{
    if (m_startNodeId < 0) {
        return nullptr;
    }
    return getNode(m_startNodeId);
}

void DialogueGraph::setStartNode(int id)
{
    m_startNodeId = id;
}

void DialogueGraph::setName(const std::string& name)
{
    m_name = name;
}

void DialogueGraph::clear()
{
    m_nodes.clear();
    m_startNodeId = -1;
    m_nextNodeId = 0;
}

} // namespace fresh
