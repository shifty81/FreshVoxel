#include "physics/HairPhysics.h"

#include <algorithm>
#include <cassert>
#include <cmath>

namespace fresh
{
namespace physics
{

void HairPhysics::addStrand(const std::string& strandName, const glm::vec3& rootPosition,
                             int particleCount, float segmentLength)
{
    if (particleCount < 2)
        particleCount = 2;

    HairStrand strand;
    strand.name = strandName;
    strand.segmentLength = segmentLength;
    strand.particles.resize(static_cast<size_t>(particleCount));

    // Build chain downward along -Y from root
    for (int i = 0; i < particleCount; ++i)
    {
        glm::vec3 pos = rootPosition - glm::vec3(0.0f, static_cast<float>(i) * segmentLength, 0.0f);
        strand.particles[i].position = pos;
        strand.particles[i].previousPosition = pos;
        strand.particles[i].pinned = (i == 0);
    }

    m_strands.push_back(std::move(strand));
}

void HairPhysics::removeStrand(const std::string& strandName)
{
    m_strands.erase(
        std::remove_if(m_strands.begin(), m_strands.end(),
                       [&strandName](const HairStrand& s) { return s.name == strandName; }),
        m_strands.end());
}

void HairPhysics::setRootPosition(const std::string& strandName, const glm::vec3& rootPosition)
{
    HairStrand* strand = findStrand(strandName);
    if (!strand || strand->particles.empty())
        return;

    // Teleport root without building velocity
    strand->particles[0].position = rootPosition;
    strand->particles[0].previousPosition = rootPosition;
}

void HairPhysics::update(float deltaTime)
{
    for (auto& strand : m_strands)
    {
        // Apply gravity to non-pinned particles
        for (auto& p : strand.particles)
        {
            if (!p.pinned)
                p.acceleration += m_gravity;
        }

        verletIntegrate(strand, deltaTime);
        satisfyConstraints(strand);

        // Clear accelerations
        for (auto& p : strand.particles)
            p.acceleration = glm::vec3(0.0f);
    }
}

void HairPhysics::applyForce(const glm::vec3& force)
{
    for (auto& strand : m_strands)
    {
        for (auto& p : strand.particles)
        {
            if (!p.pinned)
                p.acceleration += force / p.mass;
        }
    }
}

void HairPhysics::setGravity(const glm::vec3& gravity)
{
    m_gravity = gravity;
}

glm::vec3 HairPhysics::getGravity() const
{
    return m_gravity;
}

void HairPhysics::setDamping(float d)
{
    m_damping = d;
}

float HairPhysics::getDamping() const
{
    return m_damping;
}

void HairPhysics::setConstraintIterations(int n)
{
    m_constraintIterations = n;
}

int HairPhysics::getConstraintIterations() const
{
    return m_constraintIterations;
}

int HairPhysics::getStrandCount() const
{
    return static_cast<int>(m_strands.size());
}

const HairStrand* HairPhysics::getStrand(const std::string& strandName) const
{
    return findStrand(strandName);
}

const std::vector<HairStrand>& HairPhysics::getStrands() const
{
    return m_strands;
}

void HairPhysics::reset()
{
    for (auto& strand : m_strands)
    {
        if (strand.particles.empty())
            continue;

        const glm::vec3 root = strand.particles[0].position;
        for (int i = 0; i < static_cast<int>(strand.particles.size()); ++i)
        {
            glm::vec3 pos = root - glm::vec3(0.0f, static_cast<float>(i) * strand.segmentLength, 0.0f);
            strand.particles[i].position = pos;
            strand.particles[i].previousPosition = pos;
            strand.particles[i].acceleration = glm::vec3(0.0f);
        }
    }
}

// ── Private ───────────────────────────────────────────────────────────────────

void HairPhysics::verletIntegrate(HairStrand& strand, float dt)
{
    for (auto& p : strand.particles)
    {
        if (p.pinned)
            continue;

        glm::vec3 current = p.position;
        p.position = current + (current - p.previousPosition) * (1.0f - m_damping)
                     + p.acceleration * dt * dt;
        p.previousPosition = current;
    }
}

void HairPhysics::satisfyConstraints(HairStrand& strand)
{
    for (int iter = 0; iter < m_constraintIterations; ++iter)
    {
        // Chain constraints: each particle connected to the next
        for (int i = 0; i + 1 < static_cast<int>(strand.particles.size()); ++i)
        {
            HairParticle& a = strand.particles[i];
            HairParticle& b = strand.particles[i + 1];

            glm::vec3 delta = b.position - a.position;
            float len = glm::length(delta);
            if (len < 1e-7f)
                continue;

            float diff = (len - strand.segmentLength) / len;
            glm::vec3 correction = delta * 0.5f * diff;

            if (!a.pinned)
                a.position += correction;
            if (!b.pinned)
                b.position -= correction;
        }
    }
}

HairStrand* HairPhysics::findStrand(const std::string& name)
{
    for (auto& s : m_strands)
    {
        if (s.name == name)
            return &s;
    }
    return nullptr;
}

const HairStrand* HairPhysics::findStrand(const std::string& name) const
{
    for (const auto& s : m_strands)
    {
        if (s.name == name)
            return &s;
    }
    return nullptr;
}

} // namespace physics
} // namespace fresh
