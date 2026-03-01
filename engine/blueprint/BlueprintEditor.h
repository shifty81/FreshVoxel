#pragma once

#include <memory>
#include <string>

#include "blueprint/BlueprintGraph.h"

namespace fresh
{

/**
 * @brief Visual editor for blueprint node graphs
 */
class BlueprintEditor
{
public:
    BlueprintEditor() = default;
    ~BlueprintEditor() = default;

    /**
     * @brief Initialize the editor
     */
    bool initialize();

    /**
     * @brief Shut down the editor and release resources
     */
    void shutdown();

    /**
     * @brief Update editor state
     */
    void update(float deltaTime);

    /**
     * @brief Render the editor UI
     */
    void render();

    /**
     * @brief Create a new empty graph
     */
    void newGraph(const std::string& name);

    /**
     * @brief Load a graph by name
     */
    bool loadGraph(const std::string& name);

    /**
     * @brief Save the current graph
     */
    bool saveGraph();

    /**
     * @brief Save the current graph to a specific file path
     */
    bool saveGraphToFile(const std::string& path);

    [[nodiscard]] BlueprintGraph* getGraph() const;

    void setVisible(bool visible);
    [[nodiscard]] bool isVisible() const { return m_visible; }
    void toggle();

    BlueprintEditor(const BlueprintEditor&) = delete;
    BlueprintEditor& operator=(const BlueprintEditor&) = delete;

private:
    std::unique_ptr<BlueprintGraph> m_graph;
    bool m_initialized = false;
    bool m_visible = false;
};

} // namespace fresh
