#include "character/RagdollSystem.h"

#include <cassert>
#include <cmath>

namespace fresh
{

void RagdollSystem::initialize(const HumanoidSkeleton& skeleton)
{
    m_bodies.clear();
    m_constraints.clear();
    m_active = false;

    const Bone* root = skeleton.getRootBone();
    if (!root)
        return;

    buildFromBone(root, -1);
}

void RagdollSystem::buildFromBone(const Bone* bone, int parentIndex)
{
    if (!bone)
        return;

    // Create a body for this bone
    RagdollBody body;
    body.boneName = bone->getName();
    body.position = bone->getWorldPosition();
    body.previousPosition = body.position;

    int currentIndex = static_cast<int>(m_bodies.size());
    m_bodies.push_back(body);

    // Create a constraint between parent and this bone
    if (parentIndex >= 0)
    {
        RagdollConstraint constraint;
        constraint.bodyA = parentIndex;
        constraint.bodyB = currentIndex;
        constraint.restLength = glm::length(
            m_bodies[currentIndex].position - m_bodies[parentIndex].position);
        m_constraints.push_back(constraint);
    }

    // Recurse into children
    for (const Bone* child : bone->getChildren())
    {
        buildFromBone(child, currentIndex);
    }
}

void RagdollSystem::update(float deltaTime)
{
    if (!m_active)
        return;

    applyForce(m_gravity);
    verletIntegrate(deltaTime);
    satisfyConstraints();

    // Clear accelerations
    for (auto& body : m_bodies)
    {
        body.acceleration = glm::vec3(0.0f);
    }
}

void RagdollSystem::activate(const glm::vec3& rootPosition)
{
    m_active = true;

    if (m_bodies.empty())
        return;

    // Translate all bodies so the root body sits at rootPosition
    glm::vec3 offset = rootPosition - m_bodies[0].position;
    for (auto& body : m_bodies)
    {
        body.position += offset;
        body.previousPosition = body.position;
    }
}

void RagdollSystem::deactivate()
{
    m_active = false;
}

bool RagdollSystem::isActive() const
{
    return m_active;
}

void RagdollSystem::applyForce(const glm::vec3& force)
{
    for (auto& body : m_bodies)
    {
        if (!body.pinned)
        {
            body.acceleration += force / body.mass;
        }
    }
}

void RagdollSystem::applyImpulse(int bodyIndex, const glm::vec3& impulse)
{
    assert(bodyIndex >= 0 && bodyIndex < static_cast<int>(m_bodies.size()));
    if (bodyIndex < 0 || bodyIndex >= static_cast<int>(m_bodies.size()))
        return;

    if (!m_bodies[bodyIndex].pinned)
    {
        // Verlet derives velocity from position delta, so offset position directly.
        // Impulse is momentum change, so divide by mass for velocity change.
        m_bodies[bodyIndex].position += impulse / m_bodies[bodyIndex].mass;
    }
}

void RagdollSystem::applyImpulseAtPoint(const glm::vec3& point, const glm::vec3& impulse,
                                         float radius)
{
    if (radius <= 0.0f)
        return;

    for (size_t i = 0; i < m_bodies.size(); ++i)
    {
        if (m_bodies[i].pinned)
            continue;

        float distance = glm::length(m_bodies[i].position - point);
        if (distance < radius)
        {
            float falloff = 1.0f - distance / radius;
            m_bodies[i].position += impulse * falloff / m_bodies[i].mass;
        }
    }
}

const std::vector<RagdollBody>& RagdollSystem::getBodies() const
{
    return m_bodies;
}

const std::vector<RagdollConstraint>& RagdollSystem::getConstraints() const
{
    return m_constraints;
}

int RagdollSystem::getBodyIndex(const std::string& boneName) const
{
    for (size_t i = 0; i < m_bodies.size(); ++i)
    {
        if (m_bodies[i].boneName == boneName)
            return static_cast<int>(i);
    }
    return -1;
}

glm::vec3 RagdollSystem::getBodyPosition(const std::string& boneName) const
{
    int index = getBodyIndex(boneName);
    if (index >= 0)
        return m_bodies[index].position;
    return glm::vec3(0.0f);
}

void RagdollSystem::setDamping(float d)
{
    m_damping = d;
}

float RagdollSystem::getDamping() const
{
    return m_damping;
}

void RagdollSystem::setConstraintIterations(int n)
{
    m_constraintIterations = n;
}

int RagdollSystem::getConstraintIterations() const
{
    return m_constraintIterations;
}

void RagdollSystem::setGravity(const glm::vec3& g)
{
    m_gravity = g;
}

glm::vec3 RagdollSystem::getGravity() const
{
    return m_gravity;
}

void RagdollSystem::verletIntegrate(float dt)
{
    for (auto& body : m_bodies)
    {
        if (body.pinned)
            continue;

        glm::vec3 currentPos = body.position;
        // Verlet integration: newPos = pos + (pos - prevPos) * (1 - damping) + accel * dt^2
        body.position = currentPos + (currentPos - body.previousPosition) * (1.0f - m_damping)
                        + body.acceleration * dt * dt;
        body.previousPosition = currentPos;
    }
}

void RagdollSystem::satisfyConstraints()
{
    for (int iteration = 0; iteration < m_constraintIterations; ++iteration)
    {
        for (auto& constraint : m_constraints)
        {
            RagdollBody& a = m_bodies[constraint.bodyA];
            RagdollBody& b = m_bodies[constraint.bodyB];

            glm::vec3 delta = b.position - a.position;
            float currentLength = glm::length(delta);
            if (currentLength < 1e-7f)
                continue;

            float difference = (currentLength - constraint.restLength) / currentLength;
            glm::vec3 correction = delta * 0.5f * difference;

            if (!a.pinned)
                a.position += correction;
            if (!b.pinned)
                b.position -= correction;
        }
    }
}

} // namespace fresh
