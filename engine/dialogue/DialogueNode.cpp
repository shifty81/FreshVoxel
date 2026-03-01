#include "dialogue/DialogueNode.h"

#include <utility>

namespace fresh
{

DialogueNode::DialogueNode(int id, DialogueNodeType type)
    : m_id(id), m_type(type)
{
}

void DialogueNode::setSpeakerName(const std::string& speakerName)
{
    m_speakerName = speakerName;
}

void DialogueNode::setText(const std::string& text)
{
    m_text = text;
}

void DialogueNode::setPosition(glm::vec2 position)
{
    m_position = position;
}

void DialogueNode::setCondition(const std::string& condition)
{
    m_condition = condition;
}

void DialogueNode::setScript(const std::string& script)
{
    m_script = script;
}

void DialogueNode::setNextNodeId(int nextNodeId)
{
    m_nextNodeId = nextNodeId;
}

void DialogueNode::setTrueBranchNodeId(int trueBranchNodeId)
{
    m_trueBranchNodeId = trueBranchNodeId;
}

void DialogueNode::setFalseBranchNodeId(int falseBranchNodeId)
{
    m_falseBranchNodeId = falseBranchNodeId;
}

DialogueChoice& DialogueNode::addChoice(const std::string& text)
{
    m_choices.push_back({m_nextChoiceId++, text});
    return m_choices.back();
}

} // namespace fresh
