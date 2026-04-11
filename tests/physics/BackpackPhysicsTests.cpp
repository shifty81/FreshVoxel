#include <gtest/gtest.h>

#include "physics/BackpackPhysics.h"

using namespace fresh::physics;

class BackpackPhysicsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        bp = std::make_unique<BackpackPhysics>();
    }

    void TearDown() override
    {
        bp.reset();
    }

    BackpackItem makeItem(const std::string& name, const glm::vec3& restOffset = {0.0f, -0.5f, 0.0f})
    {
        BackpackItem item;
        item.name = name;
        item.anchorBone = "spine";
        item.restOffset = restOffset;
        return item;
    }

    std::unique_ptr<BackpackPhysics> bp;
};

// ============================================================================
// Construction / Empty State Tests
// ============================================================================

TEST_F(BackpackPhysicsTest, DefaultState_HasNoItems)
{
    EXPECT_EQ(bp->getItemCount(), 0);
}

TEST_F(BackpackPhysicsTest, DefaultGravity_IsDownward)
{
    glm::vec3 g = bp->getGravity();
    EXPECT_FLOAT_EQ(g.x, 0.0f);
    EXPECT_LT(g.y, 0.0f);
    EXPECT_FLOAT_EQ(g.z, 0.0f);
}

// ============================================================================
// addItem Tests
// ============================================================================

TEST_F(BackpackPhysicsTest, AddItem_IncreasesItemCount)
{
    bp->addItem(makeItem("bag"), {0.0f, 0.0f, 0.0f});
    EXPECT_EQ(bp->getItemCount(), 1);
}

TEST_F(BackpackPhysicsTest, AddItem_CreatesTwoParticles)
{
    bp->addItem(makeItem("bag"), {0.0f, 0.0f, 0.0f});
    const auto* particles = bp->getParticles("bag");
    ASSERT_NE(particles, nullptr);
    EXPECT_EQ(static_cast<int>(particles->size()), 2);
}

TEST_F(BackpackPhysicsTest, AddItem_AnchorParticleIsPinned)
{
    bp->addItem(makeItem("bag"), {0.0f, 0.0f, 0.0f});
    const auto* particles = bp->getParticles("bag");
    ASSERT_NE(particles, nullptr);
    EXPECT_TRUE((*particles)[0].pinned);
}

TEST_F(BackpackPhysicsTest, AddItem_BodyParticleIsNotPinned)
{
    bp->addItem(makeItem("bag"), {0.0f, 0.0f, 0.0f});
    const auto* particles = bp->getParticles("bag");
    ASSERT_NE(particles, nullptr);
    EXPECT_FALSE((*particles)[1].pinned);
}

TEST_F(BackpackPhysicsTest, AddItem_CreatesOneConstraint)
{
    bp->addItem(makeItem("bag"), {0.0f, 0.0f, 0.0f});
    const auto* constraints = bp->getConstraints("bag");
    ASSERT_NE(constraints, nullptr);
    EXPECT_EQ(static_cast<int>(constraints->size()), 1);
}

TEST_F(BackpackPhysicsTest, AddItem_AnchorPositionMatchesInput)
{
    glm::vec3 anchor{1.0f, 2.0f, 3.0f};
    bp->addItem(makeItem("bag"), anchor);
    EXPECT_EQ(bp->getAnchorPosition("bag"), anchor);
}

TEST_F(BackpackPhysicsTest, AddItem_BodyPositionIsAnchorPlusRestOffset)
{
    glm::vec3 anchor{1.0f, 2.0f, 3.0f};
    glm::vec3 restOffset{0.0f, -0.5f, 0.0f};
    bp->addItem(makeItem("bag", restOffset), anchor);

    glm::vec3 expectedBody = anchor + restOffset;
    EXPECT_NEAR(bp->getBodyPosition("bag").x, expectedBody.x, 1e-5f);
    EXPECT_NEAR(bp->getBodyPosition("bag").y, expectedBody.y, 1e-5f);
    EXPECT_NEAR(bp->getBodyPosition("bag").z, expectedBody.z, 1e-5f);
}

// ============================================================================
// removeItem Tests
// ============================================================================

TEST_F(BackpackPhysicsTest, RemoveItem_DecreasesItemCount)
{
    bp->addItem(makeItem("bag"), {0.0f, 0.0f, 0.0f});
    bp->removeItem("bag");
    EXPECT_EQ(bp->getItemCount(), 0);
}

TEST_F(BackpackPhysicsTest, RemoveItem_NonExistent_NoEffect)
{
    bp->addItem(makeItem("bag"), {0.0f, 0.0f, 0.0f});
    bp->removeItem("does_not_exist");
    EXPECT_EQ(bp->getItemCount(), 1);
}

// ============================================================================
// setAnchorPosition Tests
// ============================================================================

TEST_F(BackpackPhysicsTest, SetAnchorPosition_MovesAnchorParticle)
{
    bp->addItem(makeItem("bag"), {0.0f, 0.0f, 0.0f});
    glm::vec3 newAnchor{5.0f, 6.0f, 7.0f};
    bp->setAnchorPosition("bag", newAnchor);
    EXPECT_EQ(bp->getAnchorPosition("bag"), newAnchor);
}

TEST_F(BackpackPhysicsTest, SetAnchorPosition_DoesNotMoveBodyParticle)
{
    glm::vec3 startAnchor{0.0f, 0.0f, 0.0f};
    bp->addItem(makeItem("bag", {0.0f, -0.5f, 0.0f}), startAnchor);
    glm::vec3 bodyBefore = bp->getBodyPosition("bag");

    bp->setAnchorPosition("bag", {10.0f, 10.0f, 10.0f});
    // Body position should not change (anchor teleport only)
    EXPECT_EQ(bp->getBodyPosition("bag"), bodyBefore);
}

// ============================================================================
// update / Gravity Tests
// ============================================================================

TEST_F(BackpackPhysicsTest, Update_GravityMovesBodyParticleDown)
{
    bp->addItem(makeItem("bag", {0.0f, -0.5f, 0.0f}), {0.0f, 0.0f, 0.0f});
    glm::vec3 bodyBefore = bp->getBodyPosition("bag");

    bp->update(0.016f); // ~1 frame at 60fps

    // Body should move downward
    EXPECT_LT(bp->getBodyPosition("bag").y, bodyBefore.y);
}

TEST_F(BackpackPhysicsTest, Update_AnchorParticleStaysFixed)
{
    glm::vec3 anchor{0.0f, 5.0f, 0.0f};
    bp->addItem(makeItem("bag"), anchor);
    glm::vec3 anchorBefore = bp->getAnchorPosition("bag");

    bp->update(0.1f);

    EXPECT_EQ(bp->getAnchorPosition("bag"), anchorBefore);
}

// ============================================================================
// Damping / Iterations Tests
// ============================================================================

TEST_F(BackpackPhysicsTest, SetDamping_StoresValue)
{
    bp->setDamping(0.1f);
    EXPECT_FLOAT_EQ(bp->getDamping(), 0.1f);
}

TEST_F(BackpackPhysicsTest, SetConstraintIterations_StoresValue)
{
    bp->setConstraintIterations(10);
    EXPECT_EQ(bp->getConstraintIterations(), 10);
}

// ============================================================================
// reset Tests
// ============================================================================

TEST_F(BackpackPhysicsTest, Reset_ReturnsBodyToRestPosition)
{
    glm::vec3 anchor{0.0f, 0.0f, 0.0f};
    glm::vec3 restOffset{0.0f, -0.5f, 0.0f};
    bp->addItem(makeItem("bag", restOffset), anchor);

    // Simulate several frames so body moves
    for (int i = 0; i < 10; ++i)
        bp->update(0.016f);

    bp->reset();

    glm::vec3 expectedBody = anchor + restOffset;
    EXPECT_NEAR(bp->getBodyPosition("bag").y, expectedBody.y, 1e-4f);
}

// ============================================================================
// Multiple Items Tests
// ============================================================================

TEST_F(BackpackPhysicsTest, MultipleItems_TrackIndependently)
{
    bp->addItem(makeItem("bagA", {0.0f, -0.5f, 0.0f}), {0.0f, 5.0f, 0.0f});
    bp->addItem(makeItem("bagB", {0.0f, -1.0f, 0.0f}), {2.0f, 5.0f, 0.0f});
    EXPECT_EQ(bp->getItemCount(), 2);
    EXPECT_NE(bp->getBodyPosition("bagA"), bp->getBodyPosition("bagB"));
}

// ============================================================================
// Unknown Item Queries
// ============================================================================

TEST_F(BackpackPhysicsTest, GetParticles_UnknownItem_ReturnsNull)
{
    EXPECT_EQ(bp->getParticles("ghost"), nullptr);
}

TEST_F(BackpackPhysicsTest, GetBodyPosition_UnknownItem_ReturnsZero)
{
    EXPECT_EQ(bp->getBodyPosition("ghost"), glm::vec3(0.0f));
}
