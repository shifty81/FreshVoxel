#include "character/Bone.h"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>

namespace fresh
{

Bone::Bone(const std::string& name, Bone* parent)
    : m_name(name)
    , m_parent(parent)
    , m_localPosition(0.0f)
    , m_localRotation(1.0f, 0.0f, 0.0f, 0.0f) // Identity quaternion
    , m_localScale(1.0f)
{
    if (m_parent) {
        m_parent->addChild(this);
    }
}

glm::vec3 Bone::getWorldPosition() const
{
    if (m_parent) {
        glm::vec4 worldPos = m_parent->getWorldTransform() * glm::vec4(m_localPosition, 1.0f);
        return glm::vec3(worldPos);
    }
    return m_localPosition;
}

glm::quat Bone::getWorldRotation() const
{
    if (m_parent) {
        return m_parent->getWorldRotation() * m_localRotation;
    }
    return m_localRotation;
}

glm::mat4 Bone::getLocalTransform() const
{
    glm::mat4 transform(1.0f);
    transform = glm::translate(transform, m_localPosition);
    transform = transform * glm::mat4_cast(m_localRotation);
    transform = glm::scale(transform, m_localScale);
    return transform;
}

glm::mat4 Bone::getWorldTransform() const
{
    if (m_parent) {
        return m_parent->getWorldTransform() * getLocalTransform();
    }
    return getLocalTransform();
}

void Bone::addChild(Bone* child)
{
    if (child && std::find(m_children.begin(), m_children.end(), child) == m_children.end()) {
        m_children.push_back(child);
    }
}

void Bone::removeChild(Bone* child)
{
    auto it = std::find(m_children.begin(), m_children.end(), child);
    if (it != m_children.end()) {
        m_children.erase(it);
    }
}

float Bone::getLength() const
{
    if (!m_children.empty()) {
        // Return distance to first child
        glm::vec3 childPos = m_children[0]->getLocalPosition();
        return glm::length(childPos);
    }
    // Default length for end bones
    return 1.0f;
}

} // namespace fresh
