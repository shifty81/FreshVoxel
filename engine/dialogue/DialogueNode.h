#pragma once

#include <string>
#include <vector>

#include <glm/glm.hpp>

namespace fresh
{

/**
 * @brief Type of a dialogue graph node
 */
enum class DialogueNodeType { Start, Dialogue, Choice, Condition, LuaScript, End };

/**
 * @brief A single selectable choice within a Choice node
 */
struct DialogueChoice
{
    int id;
    std::string text;
    int targetNodeId = -1;
};

/**
 * @brief A node in a dialogue graph
 */
class DialogueNode
{
public:
    DialogueNode(int id, DialogueNodeType type);
    ~DialogueNode() = default;

    [[nodiscard]] int getId() const { return m_id; }
    [[nodiscard]] DialogueNodeType getType() const { return m_type; }
    [[nodiscard]] const std::string& getSpeakerName() const { return m_speakerName; }
    [[nodiscard]] const std::string& getText() const { return m_text; }
    [[nodiscard]] const glm::vec2& getPosition() const { return m_position; }
    [[nodiscard]] const std::vector<DialogueChoice>& getChoices() const { return m_choices; }
    [[nodiscard]] const std::string& getCondition() const { return m_condition; }
    [[nodiscard]] const std::string& getScript() const { return m_script; }
    [[nodiscard]] int getNextNodeId() const { return m_nextNodeId; }
    [[nodiscard]] int getTrueBranchNodeId() const { return m_trueBranchNodeId; }
    [[nodiscard]] int getFalseBranchNodeId() const { return m_falseBranchNodeId; }

    void setSpeakerName(const std::string& speakerName);
    void setText(const std::string& text);
    void setPosition(glm::vec2 position);
    void setCondition(const std::string& condition);
    void setScript(const std::string& script);
    void setNextNodeId(int nextNodeId);
    void setTrueBranchNodeId(int trueBranchNodeId);
    void setFalseBranchNodeId(int falseBranchNodeId);

    /**
     * @brief Add a choice to this Choice node
     * @return Reference to the newly added choice
     */
    DialogueChoice& addChoice(const std::string& text);

    DialogueNode(const DialogueNode&) = delete;
    DialogueNode& operator=(const DialogueNode&) = delete;
    DialogueNode(DialogueNode&&) = default;
    DialogueNode& operator=(DialogueNode&&) = default;

protected:
    int m_id;
    DialogueNodeType m_type;
    std::string m_speakerName;
    std::string m_text;
    glm::vec2 m_position{0.0f, 0.0f};
    std::vector<DialogueChoice> m_choices;
    std::string m_condition;
    std::string m_script;
    int m_nextNodeId = -1;
    int m_trueBranchNodeId = -1;
    int m_falseBranchNodeId = -1;
    int m_nextChoiceId = 0;
};

} // namespace fresh
