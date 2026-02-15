#pragma once
#include <string>
#include <vector>

#include <glm/glm.hpp>

namespace fresh
{
namespace devtools
{

/**
 * @brief Debug shape types
 */
enum class DebugShapeType { Line, Box, Sphere, Arrow, Text };

/**
 * @brief Debug draw command
 */
struct DebugDrawCommand {
    DebugShapeType type;
    glm::vec3 position;
    glm::vec3 size;
    glm::vec4 color;
    std::string text;
    float duration; // 0 = one frame, > 0 = persist for duration

    DebugDrawCommand()
        : type(DebugShapeType::Line), position(0.0f), size(0.0f), color(1.0f), duration(0.0f)
    {
    }
};

/**
 * @brief Debug visualization for game objects and physics
 */
class DebugRenderer
{
public:
    DebugRenderer();
    ~DebugRenderer();

    // Draw primitives
    void drawLine(const glm::vec3& start, const glm::vec3& end,
                  const glm::vec4& color = glm::vec4(1.0f), float duration = 0.0f);
    void drawBox(const glm::vec3& center, const glm::vec3& size,
                 const glm::vec4& color = glm::vec4(1.0f), float duration = 0.0f);
    void drawSphere(const glm::vec3& center, float radius, const glm::vec4& color = glm::vec4(1.0f),
                    float duration = 0.0f);
    void drawArrow(const glm::vec3& start, const glm::vec3& end,
                   const glm::vec4& color = glm::vec4(1.0f), float duration = 0.0f);
    void drawText(const glm::vec3& position, const std::string& text,
                  const glm::vec4& color = glm::vec4(1.0f), float duration = 0.0f);

    // Update (remove expired commands)
    void update(float deltaTime);

    // Render all debug commands
    void render();

    // Clear all commands
    void clear();

    // Enable/disable
    void setEnabled(bool isEnabled)
    {
        this->enabled = isEnabled;
    }
    bool isEnabled() const
    {
        return enabled;
    }

private:
    std::vector<DebugDrawCommand> commands;
    bool enabled;

    void removeExpiredCommands(float deltaTime);
};

} // namespace devtools
} // namespace fresh
