#include <gtest/gtest.h>

#include "character/RagdollSystem.h"
#include "character/HumanoidSkeleton.h"

using namespace fresh;

class RagdollSystemTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        skeleton = std::make_unique<HumanoidSkeleton>();
        skeleton->initialize();
        ragdoll = std::make_unique<RagdollSystem>();
    }

    void TearDown() override
    {
        ragdoll.reset();
        skeleton.reset();
    }

    std::unique_ptr<HumanoidSkeleton> skeleton;
    std::unique_ptr<RagdollSystem> ragdoll;
};

// ============================================================================
// Initialization Tests
// ============================================================================

TEST_F(RagdollSystemTest, DefaultState_IsNotActive)
{
    EXPECT_FALSE(ragdoll->isActive());
}

TEST_F(RagdollSystemTest, Initialize_CreatesBodiesFromSkeleton)
{
    ragdoll->initialize(*skeleton);

    // HumanoidSkeleton has 18 bones
    const auto& allBones = skeleton->getAllBones();
    EXPECT_EQ(static_cast<int>(ragdoll->getBodies().size()),
              static_cast<int>(allBones.size()));
}

TEST_F(RagdollSystemTest, Initialize_CreatesConstraintsFromParentChildRelationships)
{
    ragdoll->initialize(*skeleton);

    // Every non-root bone produces one constraint (parent-child edge)
    int expectedConstraints = static_cast<int>(ragdoll->getBodies().size()) - 1;
    EXPECT_EQ(static_cast<int>(ragdoll->getConstraints().size()), expectedConstraints);
}

// ============================================================================
// Activate / Deactivate Tests
// ============================================================================

TEST_F(RagdollSystemTest, Activate_SetsActiveState)
{
    ragdoll->initialize(*skeleton);
    ragdoll->activate(glm::vec3(0.0f));
    EXPECT_TRUE(ragdoll->isActive());
}

TEST_F(RagdollSystemTest, Deactivate_ClearsActiveState)
{
    ragdoll->initialize(*skeleton);
    ragdoll->activate(glm::vec3(0.0f));
    ragdoll->deactivate();
    EXPECT_FALSE(ragdoll->isActive());
}

TEST_F(RagdollSystemTest, Activate_PositionsRootBodyAtGivenPosition)
{
    ragdoll->initialize(*skeleton);
    glm::vec3 rootPos(5.0f, 10.0f, 15.0f);
    ragdoll->activate(rootPos);

    int rootIdx = ragdoll->getBodyIndex("root");
    ASSERT_GE(rootIdx, 0);
    glm::vec3 pos = ragdoll->getBodies()[rootIdx].position;
    EXPECT_FLOAT_EQ(pos.x, rootPos.x);
    EXPECT_FLOAT_EQ(pos.y, rootPos.y);
    EXPECT_FLOAT_EQ(pos.z, rootPos.z);
}

// ============================================================================
// Update Tests
// ============================================================================

TEST_F(RagdollSystemTest, Update_DoesNothingWhenNotActive)
{
    ragdoll->initialize(*skeleton);

    // Record positions before update
    auto bodiesBefore = ragdoll->getBodies();
    ragdoll->update(1.0f / 60.0f);
    const auto& bodiesAfter = ragdoll->getBodies();

    for (size_t i = 0; i < bodiesBefore.size(); ++i) {
        EXPECT_FLOAT_EQ(bodiesAfter[i].position.x, bodiesBefore[i].position.x);
        EXPECT_FLOAT_EQ(bodiesAfter[i].position.y, bodiesBefore[i].position.y);
        EXPECT_FLOAT_EQ(bodiesAfter[i].position.z, bodiesBefore[i].position.z);
    }
}

TEST_F(RagdollSystemTest, Update_MovesBodiesUnderGravityWhenActive)
{
    ragdoll->initialize(*skeleton);
    ragdoll->activate(glm::vec3(0.0f, 10.0f, 0.0f));

    // Record initial Y of a non-root body
    int spineIdx = ragdoll->getBodyIndex("spine");
    ASSERT_GE(spineIdx, 0);
    float initialY = ragdoll->getBodies()[spineIdx].position.y;

    ragdoll->update(1.0f / 60.0f);

    float afterY = ragdoll->getBodies()[spineIdx].position.y;
    EXPECT_LT(afterY, initialY);
}

// ============================================================================
// Body Lookup Tests
// ============================================================================

TEST_F(RagdollSystemTest, GetBodyIndex_FindsBodyByBoneName)
{
    ragdoll->initialize(*skeleton);
    EXPECT_GE(ragdoll->getBodyIndex("root"), 0);
    EXPECT_GE(ragdoll->getBodyIndex("head"), 0);
    EXPECT_GE(ragdoll->getBodyIndex("left_hand"), 0);
}

TEST_F(RagdollSystemTest, GetBodyIndex_ReturnsNegativeOneForUnknownName)
{
    ragdoll->initialize(*skeleton);
    EXPECT_EQ(ragdoll->getBodyIndex("NonExistentBone"), -1);
    EXPECT_EQ(ragdoll->getBodyIndex(""), -1);
}

TEST_F(RagdollSystemTest, GetBodyPosition_ReturnsBodyPositionByName)
{
    ragdoll->initialize(*skeleton);
    ragdoll->activate(glm::vec3(5.0f, 10.0f, 15.0f));

    glm::vec3 rootPos = ragdoll->getBodyPosition("root");
    EXPECT_FLOAT_EQ(rootPos.x, 5.0f);
    EXPECT_FLOAT_EQ(rootPos.y, 10.0f);
    EXPECT_FLOAT_EQ(rootPos.z, 15.0f);
}

// ============================================================================
// Impulse Tests
// ============================================================================

TEST_F(RagdollSystemTest, ApplyImpulse_MovesSpecificBody)
{
    ragdoll->initialize(*skeleton);
    ragdoll->activate(glm::vec3(0.0f));

    int headIdx = ragdoll->getBodyIndex("head");
    ASSERT_GE(headIdx, 0);
    glm::vec3 before = ragdoll->getBodies()[headIdx].position;

    ragdoll->applyImpulse(headIdx, glm::vec3(10.0f, 0.0f, 0.0f));
    ragdoll->update(1.0f / 60.0f);

    glm::vec3 after = ragdoll->getBodies()[headIdx].position;
    EXPECT_GT(after.x, before.x);
}

TEST_F(RagdollSystemTest, ApplyImpulseAtPoint_AffectsBodiesWithinRadius)
{
    ragdoll->initialize(*skeleton);
    ragdoll->activate(glm::vec3(0.0f));

    // Get a body position to use as the impulse point
    int headIdx = ragdoll->getBodyIndex("head");
    ASSERT_GE(headIdx, 0);
    glm::vec3 headPos = ragdoll->getBodies()[headIdx].position;
    glm::vec3 before = headPos;

    ragdoll->applyImpulseAtPoint(headPos, glm::vec3(10.0f, 0.0f, 0.0f), 5.0f);
    ragdoll->update(1.0f / 60.0f);

    glm::vec3 after = ragdoll->getBodies()[headIdx].position;
    EXPECT_GT(after.x, before.x);
}

TEST_F(RagdollSystemTest, ApplyImpulseAtPoint_DoesNotAffectBodiesOutsideRadius)
{
    ragdoll->initialize(*skeleton);
    ragdoll->activate(glm::vec3(0.0f));

    // Use a point far from any body with a tiny radius
    glm::vec3 farPoint(1000.0f, 1000.0f, 1000.0f);
    auto bodiesBefore = ragdoll->getBodies();

    ragdoll->applyImpulseAtPoint(farPoint, glm::vec3(10.0f, 0.0f, 0.0f), 0.01f);

    // Bodies should not have moved from the impulse (gravity may still apply on update)
    const auto& bodiesAfter = ragdoll->getBodies();
    for (size_t i = 0; i < bodiesBefore.size(); ++i) {
        EXPECT_FLOAT_EQ(bodiesAfter[i].position.x, bodiesBefore[i].position.x);
        EXPECT_FLOAT_EQ(bodiesAfter[i].position.y, bodiesBefore[i].position.y);
        EXPECT_FLOAT_EQ(bodiesAfter[i].position.z, bodiesBefore[i].position.z);
    }
}

// ============================================================================
// Damping / Gravity / Iterations Tests
// ============================================================================

TEST_F(RagdollSystemTest, SetDamping_GetDamping_WorkCorrectly)
{
    ragdoll->setDamping(0.1f);
    EXPECT_FLOAT_EQ(ragdoll->getDamping(), 0.1f);

    ragdoll->setDamping(0.5f);
    EXPECT_FLOAT_EQ(ragdoll->getDamping(), 0.5f);
}

TEST_F(RagdollSystemTest, SetGravity_GetGravity_WorkCorrectly)
{
    glm::vec3 g(0.0f, -20.0f, 0.0f);
    ragdoll->setGravity(g);
    glm::vec3 result = ragdoll->getGravity();
    EXPECT_FLOAT_EQ(result.x, g.x);
    EXPECT_FLOAT_EQ(result.y, g.y);
    EXPECT_FLOAT_EQ(result.z, g.z);
}

// ============================================================================
// Constraint Maintenance Test
// ============================================================================

TEST_F(RagdollSystemTest, Constraints_MaintainRestLengthDuringSimulation)
{
    ragdoll->initialize(*skeleton);
    ragdoll->activate(glm::vec3(0.0f, 10.0f, 0.0f));

    // Run several simulation steps
    for (int i = 0; i < 30; ++i) {
        ragdoll->update(1.0f / 60.0f);
    }

    const auto& bodies = ragdoll->getBodies();
    const auto& constraints = ragdoll->getConstraints();
    for (const auto& c : constraints) {
        float dist = glm::length(bodies[c.bodyA].position - bodies[c.bodyB].position);
        EXPECT_NEAR(dist, c.restLength, c.restLength * 0.5f);
    }
}
