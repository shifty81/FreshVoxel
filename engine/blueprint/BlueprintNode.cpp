#include "blueprint/BlueprintNode.h"

#include <utility>

namespace fresh
{

BlueprintNode::BlueprintNode(int id, std::string name, BlueprintNodeType type)
    : m_id(id), m_name(std::move(name)), m_type(type)
{
}

bool BlueprintNode::execute()
{
    return true;
}

void BlueprintNode::setPosition(glm::vec2 position)
{
    m_position = position;
}

BlueprintPin& BlueprintNode::addInput(std::string name, BlueprintPinType type)
{
    m_inputs.push_back({m_nextPinId++, std::move(name), type, false});
    return m_inputs.back();
}

BlueprintPin& BlueprintNode::addOutput(std::string name, BlueprintPinType type)
{
    m_outputs.push_back({m_nextPinId++, std::move(name), type, true});
    return m_outputs.back();
}

} // namespace fresh
