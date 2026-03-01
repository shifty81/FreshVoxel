#pragma once

#include <string>
#include <vector>

#include <glm/glm.hpp>

namespace fresh
{

/**
 * @brief Data type carried by a blueprint pin
 */
enum class BlueprintPinType { Execution, Bool, Int, Float, String, Vector3, Any };

/**
 * @brief Category of a blueprint node
 */
enum class BlueprintNodeType
{
    Event,
    Function,
    Branch,
    Sequence,
    ForLoop,
    Variable_Get,
    Variable_Set,
    Custom
};

/**
 * @brief A single input or output pin on a blueprint node
 */
struct BlueprintPin
{
    int id;
    std::string name;
    BlueprintPinType type;
    bool isOutput;
    int connectedPinId = -1;
};

/**
 * @brief Base class for all blueprint graph nodes
 */
class BlueprintNode
{
public:
    BlueprintNode(int id, std::string name, BlueprintNodeType type);
    virtual ~BlueprintNode() = default;

    /**
     * @brief Execute this node's logic
     * @return true if execution succeeded
     */
    virtual bool execute();

    [[nodiscard]] int getId() const { return m_id; }
    [[nodiscard]] const std::string& getName() const { return m_name; }
    [[nodiscard]] BlueprintNodeType getType() const { return m_type; }
    [[nodiscard]] const glm::vec2& getPosition() const { return m_position; }
    [[nodiscard]] const std::vector<BlueprintPin>& getInputs() const { return m_inputs; }
    [[nodiscard]] const std::vector<BlueprintPin>& getOutputs() const { return m_outputs; }

    void setPosition(glm::vec2 position);

    /**
     * @brief Add an input pin to this node
     */
    BlueprintPin& addInput(std::string name, BlueprintPinType type);

    /**
     * @brief Add an output pin to this node
     */
    BlueprintPin& addOutput(std::string name, BlueprintPinType type);

    BlueprintNode(const BlueprintNode&) = delete;
    BlueprintNode& operator=(const BlueprintNode&) = delete;
    BlueprintNode(BlueprintNode&&) = default;
    BlueprintNode& operator=(BlueprintNode&&) = default;

protected:
    int m_id;
    std::string m_name;
    BlueprintNodeType m_type;
    glm::vec2 m_position{0.0f, 0.0f};
    std::vector<BlueprintPin> m_inputs;
    std::vector<BlueprintPin> m_outputs;
    int m_nextPinId = 0;
};

} // namespace fresh
