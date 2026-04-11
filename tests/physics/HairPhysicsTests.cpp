#include <gtest/gtest.h>

#include "physics/HairPhysics.h"

using namespace fresh::physics;

class HairPhysicsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        hair = std::make_unique<HairPhysics>();
    }

    void TearDown() override
    {
        hair.reset();
    }

    std::unique_ptr<HairPhysics> hair;
};

// ============================================================================
// Construction / Empty State Tests
// ============================================================================

TEST_F(HairPhysicsTest, DefaultState_HasNoStrands)
{
    EXPECT_EQ(hair->getStrandCount(), 0);
}

TEST_F(HairPhysicsTest, DefaultGravity_IsDownward)
{
    glm::vec3 g = hair->getGravity();
    EXPECT_FLOAT_EQ(g.x, 0.0f);
    EXPECT_LT(g.y, 0.0f);
    EXPECT_FLOAT_EQ(g.z, 0.0f);
}

// ============================================================================
// addStrand Tests
// ============================================================================

TEST_F(HairPhysicsTest, AddStrand_IncreasesStrandCount)
{
    hair->addStrand("strand1", {0.0f, 1.0f, 0.0f}, 5, 0.2f);
    EXPECT_EQ(hair->getStrandCount(), 1);
}

TEST_F(HairPhysicsTest, AddStrand_CreatesCorrectParticleCount)
{
    hair->addStrand("strand1", {0.0f, 1.0f, 0.0f}, 6, 0.2f);
    const HairStrand* strand = hair->getStrand("strand1");
    ASSERT_NE(strand, nullptr);
    EXPECT_EQ(static_cast<int>(strand->particles.size()), 6);
}

TEST_F(HairPhysicsTest, AddStrand_RootParticleIsPinned)
{
    hair->addStrand("strand1", {0.0f, 1.0f, 0.0f}, 4, 0.2f);
    const HairStrand* strand = hair->getStrand("strand1");
    ASSERT_NE(strand, nullptr);
    EXPECT_TRUE(strand->particles[0].pinned);
}

TEST_F(HairPhysicsTest, AddStrand_NonRootParticlesAreNotPinned)
{
    hair->addStrand("strand1", {0.0f, 1.0f, 0.0f}, 4, 0.2f);
    const HairStrand* strand = hair->getStrand("strand1");
    ASSERT_NE(strand, nullptr);
    for (int i = 1; i < static_cast<int>(strand->particles.size()); ++i)
        EXPECT_FALSE(strand->particles[i].pinned) << "Particle " << i << " should not be pinned";
}

TEST_F(HairPhysicsTest, AddStrand_RootPositionMatchesInput)
{
    glm::vec3 root{2.0f, 3.0f, 4.0f};
    hair->addStrand("strand1", root, 3, 0.5f);
    const HairStrand* strand = hair->getStrand("strand1");
    ASSERT_NE(strand, nullptr);
    EXPECT_NEAR(strand->particles[0].position.x, root.x, 1e-5f);
    EXPECT_NEAR(strand->particles[0].position.y, root.y, 1e-5f);
    EXPECT_NEAR(strand->particles[0].position.z, root.z, 1e-5f);
}

TEST_F(HairPhysicsTest, AddStrand_ParticlesChainDownward)
{
    hair->addStrand("strand1", {0.0f, 5.0f, 0.0f}, 3, 1.0f);
    const HairStrand* strand = hair->getStrand("strand1");
    ASSERT_NE(strand, nullptr);
    EXPECT_NEAR(strand->particles[1].position.y, 4.0f, 1e-5f);
    EXPECT_NEAR(strand->particles[2].position.y, 3.0f, 1e-5f);
}

TEST_F(HairPhysicsTest, AddStrand_MinParticleCountEnforced)
{
    // Requesting 0 particles should be clamped to 2
    hair->addStrand("strand1", {0.0f, 0.0f, 0.0f}, 0, 0.2f);
    const HairStrand* strand = hair->getStrand("strand1");
    ASSERT_NE(strand, nullptr);
    EXPECT_GE(static_cast<int>(strand->particles.size()), 2);
}

// ============================================================================
// removeStrand Tests
// ============================================================================

TEST_F(HairPhysicsTest, RemoveStrand_DecreasesStrandCount)
{
    hair->addStrand("strand1", {0.0f, 0.0f, 0.0f}, 4, 0.2f);
    hair->removeStrand("strand1");
    EXPECT_EQ(hair->getStrandCount(), 0);
}

TEST_F(HairPhysicsTest, RemoveStrand_NonExistent_NoEffect)
{
    hair->addStrand("strand1", {0.0f, 0.0f, 0.0f}, 4, 0.2f);
    hair->removeStrand("does_not_exist");
    EXPECT_EQ(hair->getStrandCount(), 1);
}

// ============================================================================
// setRootPosition Tests
// ============================================================================

TEST_F(HairPhysicsTest, SetRootPosition_MovesRootParticle)
{
    hair->addStrand("strand1", {0.0f, 0.0f, 0.0f}, 4, 0.2f);
    glm::vec3 newRoot{1.0f, 2.0f, 3.0f};
    hair->setRootPosition("strand1", newRoot);
    const HairStrand* strand = hair->getStrand("strand1");
    ASSERT_NE(strand, nullptr);
    EXPECT_NEAR(strand->particles[0].position.x, newRoot.x, 1e-5f);
    EXPECT_NEAR(strand->particles[0].position.y, newRoot.y, 1e-5f);
    EXPECT_NEAR(strand->particles[0].position.z, newRoot.z, 1e-5f);
}

// ============================================================================
// update / Gravity Tests
// ============================================================================

TEST_F(HairPhysicsTest, Update_GravityMovesTipParticleDown)
{
    hair->addStrand("strand1", {0.0f, 5.0f, 0.0f}, 4, 1.0f);
    const HairStrand* strand = hair->getStrand("strand1");
    ASSERT_NE(strand, nullptr);
    float tipYBefore = strand->particles.back().position.y;

    hair->update(0.016f);

    EXPECT_LT(strand->particles.back().position.y, tipYBefore);
}

TEST_F(HairPhysicsTest, Update_RootParticleStaysFixed)
{
    glm::vec3 root{0.0f, 5.0f, 0.0f};
    hair->addStrand("strand1", root, 4, 1.0f);

    hair->update(0.1f);

    const HairStrand* strand = hair->getStrand("strand1");
    ASSERT_NE(strand, nullptr);
    EXPECT_NEAR(strand->particles[0].position.x, root.x, 1e-5f);
    EXPECT_NEAR(strand->particles[0].position.y, root.y, 1e-5f);
    EXPECT_NEAR(strand->particles[0].position.z, root.z, 1e-5f);
}

// ============================================================================
// Damping / Iterations Tests
// ============================================================================

TEST_F(HairPhysicsTest, SetDamping_StoresValue)
{
    hair->setDamping(0.05f);
    EXPECT_FLOAT_EQ(hair->getDamping(), 0.05f);
}

TEST_F(HairPhysicsTest, SetConstraintIterations_StoresValue)
{
    hair->setConstraintIterations(8);
    EXPECT_EQ(hair->getConstraintIterations(), 8);
}

// ============================================================================
// reset Tests
// ============================================================================

TEST_F(HairPhysicsTest, Reset_RestoresChainLayout)
{
    glm::vec3 root{0.0f, 5.0f, 0.0f};
    hair->addStrand("strand1", root, 3, 1.0f);

    // Run several frames so particles move
    for (int i = 0; i < 10; ++i)
        hair->update(0.016f);

    hair->reset();

    const HairStrand* strand = hair->getStrand("strand1");
    ASSERT_NE(strand, nullptr);
    // After reset, tip should be back near root.y - (n-1)*segmentLength
    float expectedTipY = root.y - 2.0f * 1.0f;
    EXPECT_NEAR(strand->particles[2].position.y, expectedTipY, 1e-4f);
}

// ============================================================================
// Multiple Strands Tests
// ============================================================================

TEST_F(HairPhysicsTest, MultipleStrands_TrackIndependently)
{
    hair->addStrand("strand1", {0.0f, 5.0f, 0.0f}, 4, 0.3f);
    hair->addStrand("strand2", {1.0f, 5.0f, 0.0f}, 4, 0.3f);
    EXPECT_EQ(hair->getStrandCount(), 2);
}

// ============================================================================
// getStrand / getStrands Tests
// ============================================================================

TEST_F(HairPhysicsTest, GetStrand_UnknownName_ReturnsNull)
{
    EXPECT_EQ(hair->getStrand("ghost"), nullptr);
}

TEST_F(HairPhysicsTest, GetStrands_ReturnsAllStrands)
{
    hair->addStrand("s1", {0.0f, 0.0f, 0.0f}, 3, 0.2f);
    hair->addStrand("s2", {1.0f, 0.0f, 0.0f}, 3, 0.2f);
    EXPECT_EQ(static_cast<int>(hair->getStrands().size()), 2);
}
