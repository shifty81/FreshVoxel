#include "physics/ClothSimulation.h"

#include <cassert>
#include <cmath>

namespace fresh
{
namespace physics
{

ClothSimulation::ClothSimulation(int width, int height, float spacing)
    : m_width(width),
      m_height(height),
      m_spacing(spacing),
      m_damping(0.01f),
      m_constraintIterations(5)
{
    // Create particle grid in the XY plane
    m_particles.resize(static_cast<size_t>(width * height));
    m_initialPositions.resize(m_particles.size());

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            int index = getParticleIndex(x, y);
            glm::vec3 pos(static_cast<float>(x) * spacing, static_cast<float>(y) * spacing, 0.0f);
            m_particles[index].position = pos;
            m_particles[index].previousPosition = pos;
            m_initialPositions[index] = pos;
        }
    }

    // Create structural constraints (horizontal and vertical neighbors)
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            // Horizontal neighbor
            if (x < width - 1)
            {
                ClothConstraint c;
                c.particleA = getParticleIndex(x, y);
                c.particleB = getParticleIndex(x + 1, y);
                c.restLength = spacing;
                m_constraints.push_back(c);
            }
            // Vertical neighbor
            if (y < height - 1)
            {
                ClothConstraint c;
                c.particleA = getParticleIndex(x, y);
                c.particleB = getParticleIndex(x, y + 1);
                c.restLength = spacing;
                m_constraints.push_back(c);
            }
            // Diagonal neighbors (shear constraints)
            float diagonalLength = spacing * std::sqrt(2.0f);
            if (x < width - 1 && y < height - 1)
            {
                ClothConstraint c;
                c.particleA = getParticleIndex(x, y);
                c.particleB = getParticleIndex(x + 1, y + 1);
                c.restLength = diagonalLength;
                m_constraints.push_back(c);
            }
            if (x > 0 && y < height - 1)
            {
                ClothConstraint c;
                c.particleA = getParticleIndex(x, y);
                c.particleB = getParticleIndex(x - 1, y + 1);
                c.restLength = diagonalLength;
                m_constraints.push_back(c);
            }
        }
    }
}

void ClothSimulation::update(float deltaTime)
{
    verletIntegrate(deltaTime);
    satisfyConstraints();

    // Clear accelerations
    for (auto& particle : m_particles)
    {
        particle.acceleration = glm::vec3(0.0f);
    }
}

void ClothSimulation::applyForce(const glm::vec3& force)
{
    for (auto& particle : m_particles)
    {
        if (!particle.pinned)
        {
            particle.acceleration += force / particle.mass;
        }
    }
}

void ClothSimulation::pinParticle(int index)
{
    assert(index >= 0 && index < static_cast<int>(m_particles.size()));
    if (index < 0 || index >= static_cast<int>(m_particles.size()))
        return;
    m_particles[index].pinned = true;
}

void ClothSimulation::unpinParticle(int index)
{
    assert(index >= 0 && index < static_cast<int>(m_particles.size()));
    if (index < 0 || index >= static_cast<int>(m_particles.size()))
        return;
    m_particles[index].pinned = false;
}

void ClothSimulation::setParticlePosition(int index, const glm::vec3& pos)
{
    assert(index >= 0 && index < static_cast<int>(m_particles.size()));
    if (index < 0 || index >= static_cast<int>(m_particles.size()))
        return;
    m_particles[index].position = pos;
    m_particles[index].previousPosition = pos;
}

const std::vector<ClothParticle>& ClothSimulation::getParticles() const
{
    return m_particles;
}

const std::vector<ClothConstraint>& ClothSimulation::getConstraints() const
{
    return m_constraints;
}

int ClothSimulation::getWidth() const
{
    return m_width;
}

int ClothSimulation::getHeight() const
{
    return m_height;
}

int ClothSimulation::getParticleIndex(int x, int y) const
{
    assert(x >= 0 && x < m_width && y >= 0 && y < m_height);
    return y * m_width + x;
}

void ClothSimulation::setDamping(float d)
{
    m_damping = d;
}

float ClothSimulation::getDamping() const
{
    return m_damping;
}

void ClothSimulation::setConstraintIterations(int n)
{
    m_constraintIterations = n;
}

int ClothSimulation::getConstraintIterations() const
{
    return m_constraintIterations;
}

void ClothSimulation::reset()
{
    for (size_t i = 0; i < m_particles.size(); ++i)
    {
        m_particles[i].position = m_initialPositions[i];
        m_particles[i].previousPosition = m_initialPositions[i];
        m_particles[i].acceleration = glm::vec3(0.0f);
    }
}

void ClothSimulation::verletIntegrate(float dt)
{
    for (auto& particle : m_particles)
    {
        if (particle.pinned)
            continue;

        glm::vec3 currentPos = particle.position;
        // Verlet integration: newPos = pos + (pos - prevPos) * (1 - damping) + accel * dt^2
        particle.position = currentPos + (currentPos - particle.previousPosition) * (1.0f - m_damping)
                            + particle.acceleration * dt * dt;
        particle.previousPosition = currentPos;
    }
}

void ClothSimulation::satisfyConstraints()
{
    for (int iteration = 0; iteration < m_constraintIterations; ++iteration)
    {
        for (auto& constraint : m_constraints)
        {
            ClothParticle& a = m_particles[constraint.particleA];
            ClothParticle& b = m_particles[constraint.particleB];

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

} // namespace physics
} // namespace fresh
