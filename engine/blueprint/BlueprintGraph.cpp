#include "blueprint/BlueprintGraph.h"

#include <algorithm>

namespace fresh
{

BlueprintNode* BlueprintGraph::addNode(const std::string& name, BlueprintNodeType type)
{
    auto node = std::make_unique<BlueprintNode>(m_nextNodeId++, name, type);
    auto* ptr = node.get();
    m_nodes.push_back(std::move(node));
    return ptr;
}

bool BlueprintGraph::removeNode(int id)
{
    auto it = std::find_if(m_nodes.begin(), m_nodes.end(),
                           [id](const auto& node) { return node->getId() == id; });
    if (it == m_nodes.end()) {
        return false;
    }

    // Remove all connections involving this node
    std::erase_if(m_connections, [id](const BlueprintConnection& conn) {
        return conn.fromNodeId == id || conn.toNodeId == id;
    });

    m_nodes.erase(it);
    return true;
}

bool BlueprintGraph::connect(int fromNodeId, int fromPinId, int toNodeId, int toPinId)
{
    // Verify both nodes exist
    if (!getNode(fromNodeId) || !getNode(toNodeId)) {
        return false;
    }

    // Check for duplicate connection
    auto it = std::find_if(
        m_connections.begin(), m_connections.end(),
        [&](const BlueprintConnection& conn) {
            return conn.fromNodeId == fromNodeId && conn.fromPinId == fromPinId &&
                   conn.toNodeId == toNodeId && conn.toPinId == toPinId;
        });
    if (it != m_connections.end()) {
        return false;
    }

    m_connections.push_back({fromNodeId, fromPinId, toNodeId, toPinId});
    return true;
}

bool BlueprintGraph::disconnect(int fromNodeId, int fromPinId, int toNodeId, int toPinId)
{
    auto it = std::find_if(
        m_connections.begin(), m_connections.end(),
        [&](const BlueprintConnection& conn) {
            return conn.fromNodeId == fromNodeId && conn.fromPinId == fromPinId &&
                   conn.toNodeId == toNodeId && conn.toPinId == toPinId;
        });
    if (it == m_connections.end()) {
        return false;
    }

    m_connections.erase(it);
    return true;
}

BlueprintNode* BlueprintGraph::getNode(int id) const
{
    auto it = std::find_if(m_nodes.begin(), m_nodes.end(),
                           [id](const auto& node) { return node->getId() == id; });
    if (it != m_nodes.end()) {
        return it->get();
    }
    return nullptr;
}

void BlueprintGraph::setName(const std::string& name)
{
    m_name = name;
}

void BlueprintGraph::clear()
{
    m_nodes.clear();
    m_connections.clear();
    m_nextNodeId = 0;
}

} // namespace fresh
