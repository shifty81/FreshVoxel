#include <gtest/gtest.h>

#include "character/HitReactionSystem.h"

#include <cmath>

using namespace fresh;

class HitReactionSystemTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        system = std::make_unique<HitReactionSystem>();
    }

    void TearDown() override
    {
        system.reset();
    }

    HitInfo createHit(const std::string& bone, float force, HitDirection dir) const
    {
        HitInfo hit;
        hit.hitPoint = glm::vec3(0.0f);
        hit.hitDirection = glm::vec3(0.0f, 0.0f, -1.0f);
        hit.force = force;
        hit.direction = dir;
        hit.targetBone = bone;
        return hit;
    }

    std::unique_ptr<HitReactionSystem> system;
};

// ============================================================================
// Default State Tests
// ============================================================================

TEST_F(HitReactionSystemTest, DefaultState_HasNoActiveReactions)
{
    EXPECT_EQ(system->getActiveReactionCount(), 0);
    EXPECT_FALSE(system->hasActiveReactions());
}

// ============================================================================
// processHit Tests
// ============================================================================

TEST_F(HitReactionSystemTest, ProcessHit_CreatesActiveReaction)
{
    HitInfo hit = createHit("Chest", 1.0f, HitDirection::Front);
    system->processHit(hit);

    EXPECT_EQ(system->getActiveReactionCount(), 1);
    ASSERT_FALSE(system->getReactions().empty());
    EXPECT_TRUE(system->getReactions()[0].active);
}

TEST_F(HitReactionSystemTest, GetActiveReactionCount_ReturnsCorrectCount)
{
    system->processHit(createHit("Chest", 1.0f, HitDirection::Front));
    system->processHit(createHit("Head", 1.0f, HitDirection::Left));
    system->processHit(createHit("Spine", 1.0f, HitDirection::Back));

    EXPECT_EQ(system->getActiveReactionCount(), 3);
}

TEST_F(HitReactionSystemTest, HasActiveReactions_TrueAfterProcessHit)
{
    system->processHit(createHit("Chest", 1.0f, HitDirection::Front));
    EXPECT_TRUE(system->hasActiveReactions());
}

// ============================================================================
// Update / Decay Tests
// ============================================================================

TEST_F(HitReactionSystemTest, Update_DecaysReactionIntensity)
{
    system->processHit(createHit("Chest", 1.0f, HitDirection::Front));
    float initialIntensity = system->getReactions()[0].intensity;

    system->update(0.1f);

    EXPECT_LT(system->getReactions()[0].intensity, initialIntensity);
}

TEST_F(HitReactionSystemTest, Reaction_BecomesInactiveWhenIntensityReachesZero)
{
    system->processHit(createHit("Chest", 1.0f, HitDirection::Front));

    // Update with a very large time step to fully decay
    system->update(100.0f);

    EXPECT_EQ(system->getActiveReactionCount(), 0);
}

TEST_F(HitReactionSystemTest, Update_RemovesExpiredReactions)
{
    system->processHit(createHit("Chest", 1.0f, HitDirection::Front));

    // Decay fully
    system->update(100.0f);

    EXPECT_FALSE(system->hasActiveReactions());
    EXPECT_EQ(system->getActiveReactionCount(), 0);
}

// ============================================================================
// Bone Reaction Offset Tests
// ============================================================================

TEST_F(HitReactionSystemTest, GetBoneReactionOffset_ReturnsOffsetForMatchingBone)
{
    system->processHit(createHit("Chest", 5.0f, HitDirection::Front));

    glm::vec3 offset = system->getBoneReactionOffset("Chest");
    float magnitude = glm::length(offset);
    EXPECT_GT(magnitude, 0.0f);
}

TEST_F(HitReactionSystemTest, GetBoneReactionOffset_ReturnsZeroForNonMatchingBone)
{
    system->processHit(createHit("Chest", 5.0f, HitDirection::Front));

    glm::vec3 offset = system->getBoneReactionOffset("LeftFoot");
    EXPECT_FLOAT_EQ(offset.x, 0.0f);
    EXPECT_FLOAT_EQ(offset.y, 0.0f);
    EXPECT_FLOAT_EQ(offset.z, 0.0f);
}

// ============================================================================
// Max Reactions Tests
// ============================================================================

TEST_F(HitReactionSystemTest, SetMaxReactions_GetMaxReactions_WorkCorrectly)
{
    system->setMaxReactions(4);
    EXPECT_EQ(system->getMaxReactions(), 4);

    system->setMaxReactions(16);
    EXPECT_EQ(system->getMaxReactions(), 16);
}

TEST_F(HitReactionSystemTest, MaxReactionsLimit_IsEnforced)
{
    system->setMaxReactions(3);

    system->processHit(createHit("Chest", 1.0f, HitDirection::Front));
    system->processHit(createHit("Head", 2.0f, HitDirection::Left));
    system->processHit(createHit("Spine", 3.0f, HitDirection::Back));
    system->processHit(createHit("LeftHand", 4.0f, HitDirection::Right));

    EXPECT_LE(system->getActiveReactionCount(), 3);
}

// ============================================================================
// Global Intensity Tests
// ============================================================================

TEST_F(HitReactionSystemTest, SetGlobalIntensity_GetGlobalIntensity_WorkCorrectly)
{
    system->setGlobalIntensity(0.5f);
    EXPECT_FLOAT_EQ(system->getGlobalIntensity(), 0.5f);

    system->setGlobalIntensity(2.0f);
    EXPECT_FLOAT_EQ(system->getGlobalIntensity(), 2.0f);
}

// ============================================================================
// Clear Tests
// ============================================================================

TEST_F(HitReactionSystemTest, Clear_RemovesAllReactions)
{
    system->processHit(createHit("Chest", 1.0f, HitDirection::Front));
    system->processHit(createHit("Head", 1.0f, HitDirection::Back));
    system->processHit(createHit("Spine", 1.0f, HitDirection::Left));

    system->clear();

    EXPECT_EQ(system->getActiveReactionCount(), 0);
    EXPECT_FALSE(system->hasActiveReactions());
}

// ============================================================================
// computeHitDirection Tests
// ============================================================================

TEST_F(HitReactionSystemTest, ComputeHitDirection_FrontHit)
{
    glm::vec3 forward(0.0f, 0.0f, -1.0f);
    glm::vec3 hitDir(0.0f, 0.0f, -1.0f); // Hit direction aligned with forward
    EXPECT_EQ(HitReactionSystem::computeHitDirection(forward, hitDir), HitDirection::Front);
}

TEST_F(HitReactionSystemTest, ComputeHitDirection_BackHit)
{
    glm::vec3 forward(0.0f, 0.0f, -1.0f);
    glm::vec3 hitDir(0.0f, 0.0f, 1.0f); // Hit direction opposite to forward
    EXPECT_EQ(HitReactionSystem::computeHitDirection(forward, hitDir), HitDirection::Back);
}

TEST_F(HitReactionSystemTest, ComputeHitDirection_LeftHit)
{
    glm::vec3 forward(0.0f, 0.0f, -1.0f);
    glm::vec3 hitDir(1.0f, 0.0f, 0.0f); // Hit direction along +X (left of character)
    EXPECT_EQ(HitReactionSystem::computeHitDirection(forward, hitDir), HitDirection::Left);
}

TEST_F(HitReactionSystemTest, ComputeHitDirection_RightHit)
{
    glm::vec3 forward(0.0f, 0.0f, -1.0f);
    glm::vec3 hitDir(-1.0f, 0.0f, 0.0f); // Hit direction along -X (right of character)
    EXPECT_EQ(HitReactionSystem::computeHitDirection(forward, hitDir), HitDirection::Right);
}

TEST_F(HitReactionSystemTest, ComputeHitDirection_AboveHit)
{
    glm::vec3 forward(0.0f, 0.0f, -1.0f);
    glm::vec3 hitDir(0.0f, -1.0f, 0.0f); // Hit direction along -Y (downward)
    EXPECT_EQ(HitReactionSystem::computeHitDirection(forward, hitDir), HitDirection::Above);
}

TEST_F(HitReactionSystemTest, ComputeHitDirection_BelowHit)
{
    glm::vec3 forward(0.0f, 0.0f, -1.0f);
    glm::vec3 hitDir(0.0f, 1.0f, 0.0f); // Hit direction along +Y (upward)
    EXPECT_EQ(HitReactionSystem::computeHitDirection(forward, hitDir), HitDirection::Below);
}

// ============================================================================
// Hit Response Behavior Tests
// ============================================================================

TEST_F(HitReactionSystemTest, FrontHit_ProducesBackwardPitch)
{
    HitInfo hit = createHit("Chest", 5.0f, HitDirection::Front);
    system->processHit(hit);

    glm::vec3 offset = system->getBoneReactionOffset("Chest");
    // Front hit should produce positive X rotation (backward pitch)
    EXPECT_GT(offset.x, 0.0f);
}

TEST_F(HitReactionSystemTest, HigherForce_ProducesLargerRotationOffset)
{
    HitInfo weakHit = createHit("Chest", 1.0f, HitDirection::Front);
    HitInfo strongHit = createHit("Chest", 10.0f, HitDirection::Front);

    system->processHit(weakHit);
    glm::vec3 weakOffset = system->getBoneReactionOffset("Chest");
    float weakMagnitude = glm::length(weakOffset);

    system->clear();

    system->processHit(strongHit);
    glm::vec3 strongOffset = system->getBoneReactionOffset("Chest");
    float strongMagnitude = glm::length(strongOffset);

    EXPECT_GT(strongMagnitude, weakMagnitude);
}
