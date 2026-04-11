#include "physics/BackpackPhysics.h"

#include <algorithm>
#include <cassert>
#include <cmath>

namespace fresh
{
namespace physics
{

void BackpackPhysics::addItem(const BackpackItem& itemDef, const glm::vec3& anchorPosition)
{
    BackpackItem item = itemDef;

    glm::vec3 bodyPosition = anchorPosition + item.restOffset;

    // Particle 0 — anchor (pinned, follows character bone)
    BackpackParticle anchor;
    anchor.position = anchorPosition;
    anchor.previousPosition = anchorPosition;
    anchor.pinned = true;

    // Particle 1 — body (free to swing)
    BackpackParticle body;
    body.position = bodyPosition;
    body.previousPosition = bodyPosition;
    body.pinned = false;

    item.particles.clear();
    item.particles.push_back(anchor);
    item.particles.push_back(body);

    // One constraint linking anchor to body
    BackpackConstraint c;
    c.particleA = 0;
    c.particleB = 1;
    c.restLength = glm::length(item.restOffset);
    if (c.restLength < 1e-7f)
        c.restLength = 0.5f; // Fallback rest length

    item.constraints.clear();
    item.constraints.push_back(c);

    m_items.push_back(std::move(item));
}

void BackpackPhysics::removeItem(const std::string& name)
{
    m_items.erase(
        std::remove_if(m_items.begin(), m_items.end(),
                       [&name](const BackpackItem& i) { return i.name == name; }),
        m_items.end());
}

void BackpackPhysics::setAnchorPosition(const std::string& name, const glm::vec3& anchorPosition)
{
    BackpackItem* item = findItem(name);
    if (!item || item->particles.empty())
        return;

    // Teleport anchor particle without building velocity
    item->particles[0].position = anchorPosition;
    item->particles[0].previousPosition = anchorPosition;
}

void BackpackPhysics::update(float deltaTime)
{
    for (auto& item : m_items)
    {
        // Apply gravity to non-pinned particles
        for (auto& p : item.particles)
        {
            if (!p.pinned)
                p.acceleration += m_gravity;
        }

        verletIntegrate(item, deltaTime);
        satisfyConstraints(item);

        // Clear accumulated accelerations
        for (auto& p : item.particles)
            p.acceleration = glm::vec3(0.0f);
    }
}

void BackpackPhysics::applyForce(const glm::vec3& force)
{
    for (auto& item : m_items)
    {
        for (auto& p : item.particles)
        {
            if (!p.pinned)
                p.acceleration += force / p.mass;
        }
    }
}

void BackpackPhysics::setGravity(const glm::vec3& gravity)
{
    m_gravity = gravity;
}

glm::vec3 BackpackPhysics::getGravity() const
{
    return m_gravity;
}

void BackpackPhysics::setDamping(float d)
{
    m_damping = d;
}

float BackpackPhysics::getDamping() const
{
    return m_damping;
}

void BackpackPhysics::setConstraintIterations(int n)
{
    m_constraintIterations = n;
}

int BackpackPhysics::getConstraintIterations() const
{
    return m_constraintIterations;
}

int BackpackPhysics::getItemCount() const
{
    return static_cast<int>(m_items.size());
}

glm::vec3 BackpackPhysics::getAnchorPosition(const std::string& name) const
{
    const BackpackItem* item = findItem(name);
    if (!item || item->particles.empty())
        return glm::vec3(0.0f);
    return item->particles[0].position;
}

glm::vec3 BackpackPhysics::getBodyPosition(const std::string& name) const
{
    const BackpackItem* item = findItem(name);
    if (!item || item->particles.size() < 2)
        return glm::vec3(0.0f);
    return item->particles[1].position;
}

const std::vector<BackpackParticle>* BackpackPhysics::getParticles(const std::string& name) const
{
    const BackpackItem* item = findItem(name);
    if (!item)
        return nullptr;
    return &item->particles;
}

const std::vector<BackpackConstraint>* BackpackPhysics::getConstraints(
    const std::string& name) const
{
    const BackpackItem* item = findItem(name);
    if (!item)
        return nullptr;
    return &item->constraints;
}

void BackpackPhysics::reset()
{
    for (auto& item : m_items)
    {
        if (item.particles.size() < 2)
            continue;

        const glm::vec3 anchorPos = item.particles[0].position;
        const glm::vec3 bodyPos = anchorPos + item.restOffset;

        item.particles[0].position = anchorPos;
        item.particles[0].previousPosition = anchorPos;
        item.particles[0].acceleration = glm::vec3(0.0f);

        item.particles[1].position = bodyPos;
        item.particles[1].previousPosition = bodyPos;
        item.particles[1].acceleration = glm::vec3(0.0f);
    }
}

// ── Private ───────────────────────────────────────────────────────────────────

void BackpackPhysics::verletIntegrate(BackpackItem& item, float dt)
{
    for (auto& p : item.particles)
    {
        if (p.pinned)
            continue;

        glm::vec3 current = p.position;
        p.position = current + (current - p.previousPosition) * (1.0f - m_damping)
                     + p.acceleration * dt * dt;
        p.previousPosition = current;
    }
}

void BackpackPhysics::satisfyConstraints(BackpackItem& item)
{
    for (int iter = 0; iter < m_constraintIterations; ++iter)
    {
        for (auto& c : item.constraints)
        {
            BackpackParticle& a = item.particles[c.particleA];
            BackpackParticle& b = item.particles[c.particleB];

            glm::vec3 delta = b.position - a.position;
            float len = glm::length(delta);
            if (len < 1e-7f)
                continue;

            float diff = (len - c.restLength) / len;
            glm::vec3 correction = delta * 0.5f * diff;

            if (!a.pinned)
                a.position += correction;
            if (!b.pinned)
                b.position -= correction;
        }
    }
}

BackpackItem* BackpackPhysics::findItem(const std::string& name)
{
    for (auto& item : m_items)
    {
        if (item.name == name)
            return &item;
    }
    return nullptr;
}

const BackpackItem* BackpackPhysics::findItem(const std::string& name) const
{
    for (const auto& item : m_items)
    {
        if (item.name == name)
            return &item;
    }
    return nullptr;
}

} // namespace physics
} // namespace fresh
