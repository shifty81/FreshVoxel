#include "blueprint/BlueprintEditor.h"

namespace fresh
{

bool BlueprintEditor::initialize()
{
    if (m_initialized) {
        return true;
    }

    m_graph = std::make_unique<BlueprintGraph>();
    m_graph->setName("Default");
    m_initialized = true;
    return true;
}

void BlueprintEditor::shutdown()
{
    m_graph.reset();
    m_initialized = false;
    m_visible = false;
}

void BlueprintEditor::update(float deltaTime)
{
    (void)deltaTime;
}

void BlueprintEditor::render()
{
    if (!m_visible || !m_initialized) {
        return;
    }
}

void BlueprintEditor::newGraph(const std::string& name)
{
    m_graph = std::make_unique<BlueprintGraph>();
    m_graph->setName(name);
}

bool BlueprintEditor::loadGraph(const std::string& name)
{
    (void)name;
    return false;
}

bool BlueprintEditor::saveGraph()
{
    if (!m_graph) {
        return false;
    }
    return false;
}

BlueprintGraph* BlueprintEditor::getGraph() const
{
    return m_graph.get();
}

void BlueprintEditor::setVisible(bool visible)
{
    m_visible = visible;
}

void BlueprintEditor::toggle()
{
    m_visible = !m_visible;
}

} // namespace fresh
