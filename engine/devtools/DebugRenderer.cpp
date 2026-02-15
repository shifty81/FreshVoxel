#include "devtools/DebugRenderer.h"

#include <algorithm>

namespace fresh
{
namespace devtools
{

DebugRenderer::DebugRenderer() : enabled(true) {}

DebugRenderer::~DebugRenderer() {}

void DebugRenderer::drawLine(const glm::vec3& start, const glm::vec3& end, const glm::vec4& color,
                             float duration)
{
    DebugDrawCommand cmd;
    cmd.type = DebugShapeType::Line;
    cmd.position = start;
    cmd.size = end;
    cmd.color = color;
    cmd.duration = duration;
    commands.push_back(cmd);
}

void DebugRenderer::drawBox(const glm::vec3& center, const glm::vec3& size, const glm::vec4& color,
                            float duration)
{
    DebugDrawCommand cmd;
    cmd.type = DebugShapeType::Box;
    cmd.position = center;
    cmd.size = size;
    cmd.color = color;
    cmd.duration = duration;
    commands.push_back(cmd);
}

void DebugRenderer::drawSphere(const glm::vec3& center, float radius, const glm::vec4& color,
                               float duration)
{
    DebugDrawCommand cmd;
    cmd.type = DebugShapeType::Sphere;
    cmd.position = center;
    cmd.size = glm::vec3(radius);
    cmd.color = color;
    cmd.duration = duration;
    commands.push_back(cmd);
}

void DebugRenderer::drawArrow(const glm::vec3& start, const glm::vec3& end, const glm::vec4& color,
                              float duration)
{
    DebugDrawCommand cmd;
    cmd.type = DebugShapeType::Arrow;
    cmd.position = start;
    cmd.size = end;
    cmd.color = color;
    cmd.duration = duration;
    commands.push_back(cmd);
}

void DebugRenderer::drawText(const glm::vec3& position, const std::string& text,
                             const glm::vec4& color, float duration)
{
    DebugDrawCommand cmd;
    cmd.type = DebugShapeType::Text;
    cmd.position = position;
    cmd.text = text;
    cmd.color = color;
    cmd.duration = duration;
    commands.push_back(cmd);
}

void DebugRenderer::update(float deltaTime)
{
    if (!enabled) {
        return;
    }

    removeExpiredCommands(deltaTime);
}

void DebugRenderer::render()
{
    if (!enabled) {
        return;
    }

    // Render all debug commands
    // This would integrate with the actual rendering system
    for (const auto& cmd : commands) {
        (void)cmd; // Suppress unused warning - implementation pending
        // Render based on cmd.type
        // Implementation would depend on the graphics API
    }
}

void DebugRenderer::clear()
{
    commands.clear();
}

void DebugRenderer::removeExpiredCommands(float deltaTime)
{
    // Remove one-frame commands and expired persistent commands
    commands.erase(std::remove_if(commands.begin(), commands.end(),
                                  [deltaTime](DebugDrawCommand& cmd) {
                                      if (cmd.duration > 0.0f) {
                                          cmd.duration -= deltaTime;
                                          return cmd.duration <= 0.0f;
                                      }
                                      return cmd.duration == 0.0f; // Remove one-frame commands
                                  }),
                   commands.end());
}

} // namespace devtools
} // namespace fresh
