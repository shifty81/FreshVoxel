#include <gtest/gtest.h>

#include "character/EnvironmentalInteraction.h"

using namespace fresh;

class EnvironmentalInteractionTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        ei = std::make_unique<EnvironmentalInteraction>();
    }

    void TearDown() override
    {
        ei.reset();
    }

    EnvironmentQuery grounded()
    {
        EnvironmentQuery q;
        q.isGrounded = true;
        return q;
    }

    EnvironmentQuery airborne()
    {
        EnvironmentQuery q;
        q.isGrounded = false;
        return q;
    }

    EnvironmentQuery inWater()
    {
        EnvironmentQuery q;
        q.isInWater = true;
        return q;
    }

    EnvironmentQuery climbing()
    {
        EnvironmentQuery q;
        q.hasClimbableSurface = true;
        q.isClimbInput = true;
        return q;
    }

    EnvironmentQuery ducking()
    {
        EnvironmentQuery q;
        q.isGrounded = true;
        q.isDuckInput = true;
        return q;
    }

    std::unique_ptr<EnvironmentalInteraction> ei;
};

// ============================================================================
// Default State Tests
// ============================================================================

TEST_F(EnvironmentalInteractionTest, DefaultState_IsGrounded)
{
    // System starts in Grounded by default (before any update)
    EXPECT_EQ(ei->getState(), EnvironmentState::Grounded);
}

TEST_F(EnvironmentalInteractionTest, Default_IsNotClimbing)
{
    EXPECT_FALSE(ei->isClimbing());
}

TEST_F(EnvironmentalInteractionTest, Default_IsNotSwimming)
{
    EXPECT_FALSE(ei->isSwimming());
}

TEST_F(EnvironmentalInteractionTest, Default_IsNotDucking)
{
    EXPECT_FALSE(ei->isDucking());
}

TEST_F(EnvironmentalInteractionTest, Default_HeightMultiplierIsOne)
{
    EXPECT_FLOAT_EQ(ei->getHeightMultiplier(), 1.0f);
}

TEST_F(EnvironmentalInteractionTest, Default_BuoyancyIsZero)
{
    EXPECT_FLOAT_EQ(ei->getBuoyancy(), 0.0f);
}

// ============================================================================
// State Transition Tests
// ============================================================================

TEST_F(EnvironmentalInteractionTest, UpdateInWater_TransitionsToSwimming)
{
    ei->update(inWater(), 0.016f);
    EXPECT_EQ(ei->getState(), EnvironmentState::Swimming);
    EXPECT_TRUE(ei->isSwimming());
}

TEST_F(EnvironmentalInteractionTest, UpdateClimbing_TransitionsToClimbing)
{
    ei->update(climbing(), 0.016f);
    EXPECT_EQ(ei->getState(), EnvironmentState::Climbing);
    EXPECT_TRUE(ei->isClimbing());
}

TEST_F(EnvironmentalInteractionTest, UpdateDucking_TransitionsToDucking)
{
    ei->update(ducking(), 0.016f);
    EXPECT_EQ(ei->getState(), EnvironmentState::Ducking);
    EXPECT_TRUE(ei->isDucking());
}

TEST_F(EnvironmentalInteractionTest, UpdateGrounded_TransitionsToGrounded)
{
    ei->update(grounded(), 0.016f);
    EXPECT_EQ(ei->getState(), EnvironmentState::Grounded);
}

TEST_F(EnvironmentalInteractionTest, UpdateAirborne_TransitionsToAirborne)
{
    ei->update(airborne(), 0.016f);
    EXPECT_EQ(ei->getState(), EnvironmentState::Airborne);
}

TEST_F(EnvironmentalInteractionTest, WaterPriority_OverridesClimbing)
{
    // Water has higher priority than climbing
    EnvironmentQuery q = climbing();
    q.isInWater = true;
    ei->update(q, 0.016f);
    EXPECT_EQ(ei->getState(), EnvironmentState::Swimming);
}

TEST_F(EnvironmentalInteractionTest, ClimbingPriority_OverridesDucking)
{
    // Climbing has higher priority than ducking
    EnvironmentQuery q = climbing();
    q.isGrounded = true;
    q.isDuckInput = true;
    ei->update(q, 0.016f);
    EXPECT_EQ(ei->getState(), EnvironmentState::Climbing);
}

// ============================================================================
// isGrounded Tests
// ============================================================================

TEST_F(EnvironmentalInteractionTest, IsGrounded_TrueWhenGroundedState)
{
    ei->update(grounded(), 0.016f);
    EXPECT_TRUE(ei->isGrounded());
}

TEST_F(EnvironmentalInteractionTest, IsGrounded_TrueWhenDucking)
{
    ei->update(ducking(), 0.016f);
    EXPECT_TRUE(ei->isGrounded());
}

TEST_F(EnvironmentalInteractionTest, IsGrounded_FalseWhenAirborne)
{
    ei->update(airborne(), 0.016f);
    EXPECT_FALSE(ei->isGrounded());
}

// ============================================================================
// getModifiedVelocity Tests
// ============================================================================

TEST_F(EnvironmentalInteractionTest, GroundedVelocity_Unmodified)
{
    ei->update(grounded(), 0.016f);
    glm::vec3 input{2.0f, 0.0f, 3.0f};
    EXPECT_EQ(ei->getModifiedVelocity(input), input);
}

TEST_F(EnvironmentalInteractionTest, SwimmingVelocity_ScaledBySwimSpeed)
{
    ei->update(inWater(), 0.016f);
    float swimSpeed = ei->getSwimSpeed();
    glm::vec3 input{2.0f, 1.0f, 3.0f};
    glm::vec3 expected = input * swimSpeed;
    glm::vec3 result = ei->getModifiedVelocity(input);
    EXPECT_NEAR(result.x, expected.x, 1e-5f);
    EXPECT_NEAR(result.y, expected.y, 1e-5f);
    EXPECT_NEAR(result.z, expected.z, 1e-5f);
}

TEST_F(EnvironmentalInteractionTest, ClimbingVelocity_OnlyVertical)
{
    ei->update(climbing(), 0.016f);
    glm::vec3 input{2.0f, 1.0f, 3.0f};
    glm::vec3 result = ei->getModifiedVelocity(input);
    // Horizontal components should be zeroed
    EXPECT_FLOAT_EQ(result.x, 0.0f);
    EXPECT_FLOAT_EQ(result.z, 0.0f);
    // Vertical scaled by climb speed
    float climbSpeed = ei->getClimbSpeed();
    EXPECT_NEAR(result.y, input.y * climbSpeed, 1e-5f);
}

TEST_F(EnvironmentalInteractionTest, DuckingVelocity_HorizontalReduced)
{
    ei->update(ducking(), 0.016f);
    float mult = ei->getDuckSpeedMultiplier();
    glm::vec3 input{2.0f, 0.0f, 3.0f};
    glm::vec3 result = ei->getModifiedVelocity(input);
    EXPECT_NEAR(result.x, input.x * mult, 1e-5f);
    EXPECT_NEAR(result.z, input.z * mult, 1e-5f);
}

// ============================================================================
// Height Multiplier / Buoyancy Tests
// ============================================================================

TEST_F(EnvironmentalInteractionTest, DuckingHeightMultiplier_IsLessThanOne)
{
    ei->update(ducking(), 0.016f);
    EXPECT_LT(ei->getHeightMultiplier(), 1.0f);
}

TEST_F(EnvironmentalInteractionTest, GroundedHeightMultiplier_IsOne)
{
    ei->update(grounded(), 0.016f);
    EXPECT_FLOAT_EQ(ei->getHeightMultiplier(), 1.0f);
}

TEST_F(EnvironmentalInteractionTest, SwimmingBuoyancy_IsPositive)
{
    ei->update(inWater(), 0.016f);
    EXPECT_GT(ei->getBuoyancy(), 0.0f);
}

TEST_F(EnvironmentalInteractionTest, GroundedBuoyancy_IsZero)
{
    ei->update(grounded(), 0.016f);
    EXPECT_FLOAT_EQ(ei->getBuoyancy(), 0.0f);
}

// ============================================================================
// Configuration Tests
// ============================================================================

TEST_F(EnvironmentalInteractionTest, SetClimbSpeed_StoresValue)
{
    ei->setClimbSpeed(5.0f);
    EXPECT_FLOAT_EQ(ei->getClimbSpeed(), 5.0f);
}

TEST_F(EnvironmentalInteractionTest, SetSwimSpeed_StoresValue)
{
    ei->setSwimSpeed(0.8f);
    EXPECT_FLOAT_EQ(ei->getSwimSpeed(), 0.8f);
}

TEST_F(EnvironmentalInteractionTest, SetDuckSpeedMultiplier_StoresValue)
{
    ei->setDuckSpeedMultiplier(0.3f);
    EXPECT_FLOAT_EQ(ei->getDuckSpeedMultiplier(), 0.3f);
}

TEST_F(EnvironmentalInteractionTest, SetDuckHeightMultiplier_StoresValue)
{
    ei->setDuckHeightMultiplier(0.6f);
    EXPECT_FLOAT_EQ(ei->getDuckHeightMultiplier(), 0.6f);
}

TEST_F(EnvironmentalInteractionTest, SetBuoyancy_ReflectedWhenSwimming)
{
    ei->setBuoyancy(5.0f);
    ei->update(inWater(), 0.016f);
    EXPECT_FLOAT_EQ(ei->getBuoyancy(), 5.0f);
}
