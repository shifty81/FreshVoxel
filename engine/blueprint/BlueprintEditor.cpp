#include "blueprint/BlueprintEditor.h"

#include <fstream>
#include <sstream>

namespace fresh
{

static std::string blueprintNodeTypeName(BlueprintNodeType type)
{
    switch (type) {
    case BlueprintNodeType::Event: return "Event";
    case BlueprintNodeType::Function: return "Function";
    case BlueprintNodeType::Branch: return "Branch";
    case BlueprintNodeType::Sequence: return "Sequence";
    case BlueprintNodeType::ForLoop: return "ForLoop";
    case BlueprintNodeType::Variable_Get: return "Variable_Get";
    case BlueprintNodeType::Variable_Set: return "Variable_Set";
    case BlueprintNodeType::Custom: return "Custom";
    }
    return "Custom";
}

static BlueprintNodeType blueprintNodeTypeFromName(const std::string& name)
{
    if (name == "Event") return BlueprintNodeType::Event;
    if (name == "Function") return BlueprintNodeType::Function;
    if (name == "Branch") return BlueprintNodeType::Branch;
    if (name == "Sequence") return BlueprintNodeType::Sequence;
    if (name == "ForLoop") return BlueprintNodeType::ForLoop;
    if (name == "Variable_Get") return BlueprintNodeType::Variable_Get;
    if (name == "Variable_Set") return BlueprintNodeType::Variable_Set;
    return BlueprintNodeType::Custom;
}

static std::string blueprintPinTypeName(BlueprintPinType type)
{
    switch (type) {
    case BlueprintPinType::Execution: return "Execution";
    case BlueprintPinType::Bool: return "Bool";
    case BlueprintPinType::Int: return "Int";
    case BlueprintPinType::Float: return "Float";
    case BlueprintPinType::String: return "String";
    case BlueprintPinType::Vector3: return "Vector3";
    case BlueprintPinType::Any: return "Any";
    }
    return "Any";
}

static BlueprintPinType blueprintPinTypeFromName(const std::string& name)
{
    if (name == "Execution") return BlueprintPinType::Execution;
    if (name == "Bool") return BlueprintPinType::Bool;
    if (name == "Int") return BlueprintPinType::Int;
    if (name == "Float") return BlueprintPinType::Float;
    if (name == "String") return BlueprintPinType::String;
    if (name == "Vector3") return BlueprintPinType::Vector3;
    return BlueprintPinType::Any;
}

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
    std::ifstream file(name);
    if (!file.is_open()) {
        return false;
    }

    std::string line;
    // Header: FRESH_BLUEPRINT v1
    if (!std::getline(file, line) || line != "FRESH_BLUEPRINT v1") {
        return false;
    }

    auto newGraph = std::make_unique<BlueprintGraph>();

    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string tag;
        iss >> tag;

        if (tag == "name") {
            std::string graphName;
            std::getline(iss >> std::ws, graphName);
            newGraph->setName(graphName);
        } else if (tag == "node") {
            std::string nodeName, typeName;
            float posX = 0.0f, posY = 0.0f;
            int inputCount = 0, outputCount = 0;
            iss >> std::ws;
            std::getline(iss, nodeName, '|');
            iss >> typeName >> posX >> posY >> inputCount >> outputCount;
            auto* node = newGraph->addNode(nodeName, blueprintNodeTypeFromName(typeName));
            node->setPosition(glm::vec2(posX, posY));

            for (int i = 0; i < inputCount; ++i) {
                if (!std::getline(file, line))
                    break;
                std::istringstream pinStream(line);
                std::string pinTag, pinName, pinTypeName;
                pinStream >> pinTag >> std::ws;
                std::getline(pinStream, pinName, '|');
                pinStream >> pinTypeName;
                if (pinTag == "input") {
                    node->addInput(pinName, blueprintPinTypeFromName(pinTypeName));
                }
            }
            for (int i = 0; i < outputCount; ++i) {
                if (!std::getline(file, line))
                    break;
                std::istringstream pinStream(line);
                std::string pinTag, pinName, pinTypeName;
                pinStream >> pinTag >> std::ws;
                std::getline(pinStream, pinName, '|');
                pinStream >> pinTypeName;
                if (pinTag == "output") {
                    node->addOutput(pinName, blueprintPinTypeFromName(pinTypeName));
                }
            }
        } else if (tag == "connection") {
            int fromNodeId = 0, fromPinId = 0, toNodeId = 0, toPinId = 0;
            iss >> fromNodeId >> fromPinId >> toNodeId >> toPinId;
            newGraph->connect(fromNodeId, fromPinId, toNodeId, toPinId);
        }
    }

    m_graph = std::move(newGraph);
    return true;
}

bool BlueprintEditor::saveGraph()
{
    if (!m_graph) {
        return false;
    }
    return saveGraphToFile(m_graph->getName());
}

bool BlueprintEditor::saveGraphToFile(const std::string& path)
{
    if (!m_graph) {
        return false;
    }

    std::ofstream file(path);
    if (!file.is_open()) {
        return false;
    }

    file << "FRESH_BLUEPRINT v1\n";
    file << "name " << m_graph->getName() << "\n";

    for (const auto& nodePtr : m_graph->getNodes()) {
        const auto* node = nodePtr.get();
        file << "node " << node->getName() << "|"
             << blueprintNodeTypeName(node->getType()) << " "
             << node->getPosition().x << " " << node->getPosition().y << " "
             << node->getInputs().size() << " " << node->getOutputs().size() << "\n";
        for (const auto& pin : node->getInputs()) {
            file << "input " << pin.name << "|" << blueprintPinTypeName(pin.type) << "\n";
        }
        for (const auto& pin : node->getOutputs()) {
            file << "output " << pin.name << "|" << blueprintPinTypeName(pin.type) << "\n";
        }
    }

    for (const auto& conn : m_graph->getConnections()) {
        file << "connection " << conn.fromNodeId << " " << conn.fromPinId
             << " " << conn.toNodeId << " " << conn.toPinId << "\n";
    }

    return true;
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
