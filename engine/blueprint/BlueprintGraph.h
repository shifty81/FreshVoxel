#pragma once

#include <memory>
#include <string>
#include <vector>

#include "blueprint/BlueprintNode.h"

namespace fresh
{

/**
 * @brief Represents a connection between two pins on different nodes
 */
struct BlueprintConnection
{
    int fromNodeId;
    int fromPinId;
    int toNodeId;
    int toPinId;
};

/**
 * @brief A graph of interconnected blueprint nodes
 */
class BlueprintGraph
{
public:
    BlueprintGraph() = default;
    ~BlueprintGraph() = default;

    /**
     * @brief Add a new node to the graph
     */
    BlueprintNode* addNode(const std::string& name, BlueprintNodeType type);

    /**
     * @brief Remove a node and its connections from the graph
     */
    bool removeNode(int id);

    /**
     * @brief Create a connection between two pins
     */
    bool connect(int fromNodeId, int fromPinId, int toNodeId, int toPinId);

    /**
     * @brief Remove a connection between two pins
     */
    bool disconnect(int fromNodeId, int fromPinId, int toNodeId, int toPinId);

    /**
     * @brief Find a node by its id
     */
    [[nodiscard]] BlueprintNode* getNode(int id) const;

    [[nodiscard]] const std::vector<std::unique_ptr<BlueprintNode>>& getNodes() const
    {
        return m_nodes;
    }
    [[nodiscard]] const std::vector<BlueprintConnection>& getConnections() const
    {
        return m_connections;
    }
    [[nodiscard]] const std::string& getName() const { return m_name; }

    void setName(const std::string& name);

    /**
     * @brief Remove all nodes and connections
     */
    void clear();

    BlueprintGraph(const BlueprintGraph&) = delete;
    BlueprintGraph& operator=(const BlueprintGraph&) = delete;
    BlueprintGraph(BlueprintGraph&&) = default;
    BlueprintGraph& operator=(BlueprintGraph&&) = default;

private:
    std::string m_name;
    std::vector<std::unique_ptr<BlueprintNode>> m_nodes;
    std::vector<BlueprintConnection> m_connections;
    int m_nextNodeId = 0;
};

} // namespace fresh
