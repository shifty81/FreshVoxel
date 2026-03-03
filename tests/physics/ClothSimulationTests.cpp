#include <gtest/gtest.h>

#include "physics/ClothSimulation.h"

using namespace fresh::physics;

class ClothSimulationTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        cloth = std::make_unique<ClothSimulation>(m_width, m_height, m_spacing);
    }

    void TearDown() override
    {
        cloth.reset();
    }

    static constexpr int m_width = 5;
    static constexpr int m_height = 4;
    static constexpr float m_spacing = 1.0f;

    std::unique_ptr<ClothSimulation> cloth;
};

// ============================================================================
// Construction Tests
// ============================================================================

TEST_F(ClothSimulationTest, Construction_CreatesCorrectParticleCount)
{
    EXPECT_EQ(static_cast<int>(cloth->getParticles().size()), m_width * m_height);
}

TEST_F(ClothSimulationTest, Construction_CreatesCorrectConstraintCount)
{
    // Structural: horizontal (width-1)*height + vertical width*(height-1)
    int structural = (m_width - 1) * m_height + m_width * (m_height - 1);
    // Shear: two diagonals per interior cell = 2 * (width-1) * (height-1)
    int shear = 2 * (m_width - 1) * (m_height - 1);
    int expected = structural + shear;
    EXPECT_EQ(static_cast<int>(cloth->getConstraints().size()), expected);
}

TEST_F(ClothSimulationTest, Construction_InitialPositionsAreGridLayout)
{
    const auto& particles = cloth->getParticles();
    for (int y = 0; y < m_height; ++y) {
        for (int x = 0; x < m_width; ++x) {
            int idx = cloth->getParticleIndex(x, y);
            const auto& pos = particles[idx].position;
            EXPECT_FLOAT_EQ(pos.x, static_cast<float>(x) * m_spacing);
            EXPECT_FLOAT_EQ(pos.y, static_cast<float>(y) * m_spacing);
            EXPECT_FLOAT_EQ(pos.z, 0.0f);
        }
    }
}

TEST_F(ClothSimulationTest, GetWidth_ReturnsCorrectValue)
{
    EXPECT_EQ(cloth->getWidth(), m_width);
}

TEST_F(ClothSimulationTest, GetHeight_ReturnsCorrectValue)
{
    EXPECT_EQ(cloth->getHeight(), m_height);
}

// ============================================================================
// Index Tests
// ============================================================================

TEST_F(ClothSimulationTest, GetParticleIndex_ReturnsRowMajorIndex)
{
    for (int y = 0; y < m_height; ++y) {
        for (int x = 0; x < m_width; ++x) {
            EXPECT_EQ(cloth->getParticleIndex(x, y), y * m_width + x);
        }
    }
}

// ============================================================================
// Pin / Unpin Tests
// ============================================================================

TEST_F(ClothSimulationTest, PinParticle_PreventMovementDuringUpdate)
{
    int idx = cloth->getParticleIndex(2, 2);
    cloth->pinParticle(idx);
    glm::vec3 before = cloth->getParticles()[idx].position;

    cloth->applyForce(glm::vec3(0.0f, -9.81f, 0.0f));
    cloth->update(1.0f / 60.0f);

    glm::vec3 after = cloth->getParticles()[idx].position;
    EXPECT_FLOAT_EQ(after.x, before.x);
    EXPECT_FLOAT_EQ(after.y, before.y);
    EXPECT_FLOAT_EQ(after.z, before.z);
}

TEST_F(ClothSimulationTest, UnpinParticle_AllowsMovementAfterUnpin)
{
    int idx = cloth->getParticleIndex(2, 2);
    cloth->pinParticle(idx);

    // Unpin and apply gravity
    cloth->unpinParticle(idx);
    cloth->applyForce(glm::vec3(0.0f, -9.81f, 0.0f));
    cloth->update(1.0f / 60.0f);

    glm::vec3 expected = glm::vec3(2.0f * m_spacing, 2.0f * m_spacing, 0.0f);
    glm::vec3 after = cloth->getParticles()[idx].position;
    // Particle should have moved from its initial position
    float dist = glm::length(after - expected);
    EXPECT_GT(dist, 0.0f);
}

// ============================================================================
// Position Tests
// ============================================================================

TEST_F(ClothSimulationTest, SetParticlePosition_MovesParticleAndResetsVelocity)
{
    int idx = cloth->getParticleIndex(0, 0);
    glm::vec3 newPos(10.0f, 20.0f, 30.0f);
    cloth->setParticlePosition(idx, newPos);

    const auto& p = cloth->getParticles()[idx];
    EXPECT_FLOAT_EQ(p.position.x, 10.0f);
    EXPECT_FLOAT_EQ(p.position.y, 20.0f);
    EXPECT_FLOAT_EQ(p.position.z, 30.0f);
    // Previous position should also be set to eliminate velocity
    EXPECT_FLOAT_EQ(p.previousPosition.x, 10.0f);
    EXPECT_FLOAT_EQ(p.previousPosition.y, 20.0f);
    EXPECT_FLOAT_EQ(p.previousPosition.z, 30.0f);
}

// ============================================================================
// Force / Update Tests
// ============================================================================

TEST_F(ClothSimulationTest, ApplyForce_GravityCausesDownwardMovement)
{
    // Record initial Y of an unpinned center particle
    int idx = cloth->getParticleIndex(2, 2);
    float initialY = cloth->getParticles()[idx].position.y;

    cloth->applyForce(glm::vec3(0.0f, -9.81f, 0.0f));
    cloth->update(1.0f / 60.0f);

    float afterY = cloth->getParticles()[idx].position.y;
    EXPECT_LT(afterY, initialY);
}

TEST_F(ClothSimulationTest, Constraints_MaintainApproximateRestLength)
{
    // Apply a large force and update several times
    cloth->applyForce(glm::vec3(0.0f, -9.81f, 0.0f));
    for (int i = 0; i < 10; ++i) {
        cloth->update(1.0f / 60.0f);
    }

    const auto& particles = cloth->getParticles();
    const auto& constraints = cloth->getConstraints();
    for (const auto& c : constraints) {
        float dist = glm::length(particles[c.particleA].position - particles[c.particleB].position);
        // Constraint solver should keep lengths within a reasonable tolerance
        EXPECT_NEAR(dist, c.restLength, c.restLength * 0.5f);
    }
}

// ============================================================================
// Damping / Iterations Tests
// ============================================================================

TEST_F(ClothSimulationTest, SetDamping_GetDamping_WorkCorrectly)
{
    cloth->setDamping(0.5f);
    EXPECT_FLOAT_EQ(cloth->getDamping(), 0.5f);

    cloth->setDamping(0.0f);
    EXPECT_FLOAT_EQ(cloth->getDamping(), 0.0f);

    cloth->setDamping(1.0f);
    EXPECT_FLOAT_EQ(cloth->getDamping(), 1.0f);
}

TEST_F(ClothSimulationTest, SetConstraintIterations_GetConstraintIterations_WorkCorrectly)
{
    cloth->setConstraintIterations(16);
    EXPECT_EQ(cloth->getConstraintIterations(), 16);

    cloth->setConstraintIterations(1);
    EXPECT_EQ(cloth->getConstraintIterations(), 1);
}

// ============================================================================
// Reset Tests
// ============================================================================

TEST_F(ClothSimulationTest, Reset_RestoresInitialPositions)
{
    // Move particles by applying force and updating
    cloth->applyForce(glm::vec3(0.0f, -9.81f, 0.0f));
    cloth->update(1.0f / 60.0f);

    cloth->reset();

    const auto& particles = cloth->getParticles();
    for (int y = 0; y < m_height; ++y) {
        for (int x = 0; x < m_width; ++x) {
            int idx = cloth->getParticleIndex(x, y);
            EXPECT_FLOAT_EQ(particles[idx].position.x, static_cast<float>(x) * m_spacing);
            EXPECT_FLOAT_EQ(particles[idx].position.y, static_cast<float>(y) * m_spacing);
            EXPECT_FLOAT_EQ(particles[idx].position.z, 0.0f);
        }
    }
}

TEST_F(ClothSimulationTest, Reset_ZerosVelocities)
{
    // Apply force and update to give particles velocity
    cloth->applyForce(glm::vec3(0.0f, -9.81f, 0.0f));
    cloth->update(1.0f / 60.0f);

    cloth->reset();

    // After reset, position and previousPosition should match (zero velocity)
    const auto& particles = cloth->getParticles();
    for (const auto& p : particles) {
        EXPECT_FLOAT_EQ(p.position.x, p.previousPosition.x);
        EXPECT_FLOAT_EQ(p.position.y, p.previousPosition.y);
        EXPECT_FLOAT_EQ(p.position.z, p.previousPosition.z);
    }
}

// ============================================================================
// Multi-step Simulation Tests
// ============================================================================

TEST_F(ClothSimulationTest, MultipleUpdates_ProduceContinuousMovementUnderGravity)
{
    int idx = cloth->getParticleIndex(2, 2);

    float prevY = cloth->getParticles()[idx].position.y;
    for (int i = 0; i < 5; ++i) {
        cloth->applyForce(glm::vec3(0.0f, -9.81f, 0.0f));
        cloth->update(1.0f / 60.0f);
        float currentY = cloth->getParticles()[idx].position.y;
        EXPECT_LT(currentY, prevY);
        prevY = currentY;
    }
}

TEST_F(ClothSimulationTest, PinnedCorners_GravityProducesHangingBehavior)
{
    // Pin top row corners
    int topLeft = cloth->getParticleIndex(0, m_height - 1);
    int topRight = cloth->getParticleIndex(m_width - 1, m_height - 1);
    cloth->pinParticle(topLeft);
    cloth->pinParticle(topRight);

    float pinnedY = cloth->getParticles()[topLeft].position.y;

    // Simulate many steps under gravity
    for (int i = 0; i < 60; ++i) {
        cloth->applyForce(glm::vec3(0.0f, -9.81f, 0.0f));
        cloth->update(1.0f / 60.0f);
    }

    // Pinned corners should not move
    EXPECT_FLOAT_EQ(cloth->getParticles()[topLeft].position.y, pinnedY);
    EXPECT_FLOAT_EQ(cloth->getParticles()[topRight].position.y, pinnedY);

    // Bottom-center particle should hang below the pinned row
    int bottomCenter = cloth->getParticleIndex(m_width / 2, 0);
    EXPECT_LT(cloth->getParticles()[bottomCenter].position.y, pinnedY);
}
