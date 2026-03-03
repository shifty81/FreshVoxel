#include <gtest/gtest.h>
#include "networking/LagCompensation.h"

class LagCompensationTest : public ::testing::Test {
protected:
    fresh::networking::LagCompensation lc{64, 40};
};

TEST_F(LagCompensationTest, StoreAndRetrieve) {
    lc.storeSnapshot({1, 10, 100.0f, 200.0f, 300.0f, 0, 0, 0, 45.0f});

    auto state = lc.getStateAtTick(1, 10.0f);
    EXPECT_TRUE(state.valid);
    EXPECT_FLOAT_EQ(state.posX, 100.0f);
    EXPECT_FLOAT_EQ(state.posY, 200.0f);
}

TEST_F(LagCompensationTest, Interpolation_BetweenSnapshots) {
    lc.storeSnapshot({1, 10, 0.0f, 0.0f, 0.0f, 0, 0, 0, 0.0f});
    lc.storeSnapshot({1, 20, 100.0f, 0.0f, 0.0f, 0, 0, 0, 0.0f});

    auto state = lc.getStateAtTick(1, 15.0f);
    EXPECT_TRUE(state.valid);
    EXPECT_NEAR(state.posX, 50.0f, 0.01f);
}

TEST_F(LagCompensationTest, HitTest_HitsWithinRadius) {
    lc.storeSnapshot({1, 10, 10.0f, 0.0f, 0.0f, 0, 0, 0, 0.0f});

    auto result = lc.hitTest(1, 8.0f, 0.0f, 0.0f, 10, 5.0f);
    EXPECT_TRUE(result.hit);
    EXPECT_NEAR(result.distance, 2.0f, 0.01f);
}

TEST_F(LagCompensationTest, HitTest_MissesOutsideRadius) {
    lc.storeSnapshot({1, 10, 100.0f, 0.0f, 0.0f, 0, 0, 0, 0.0f});

    auto result = lc.hitTest(1, 0.0f, 0.0f, 0.0f, 10, 5.0f);
    EXPECT_FALSE(result.hit);
}

TEST_F(LagCompensationTest, MaxRewindTick_ClampedCorrectly) {
    for (uint32_t i = 1; i <= 100; ++i) {
        lc.storeSnapshot({1, i, static_cast<float>(i), 0, 0, 0, 0, 0, 0});
    }

    auto result = lc.hitTest(1, 0, 0, 0, 10, 1000.0f);
    EXPECT_GE(result.rewindTick, 100u - 40u);
}

TEST_F(LagCompensationTest, RemoveEntity) {
    lc.storeSnapshot({1, 10, 0, 0, 0, 0, 0, 0, 0});
    EXPECT_EQ(lc.trackedEntityCount(), 1u);
    lc.removeEntity(1);
    EXPECT_EQ(lc.trackedEntityCount(), 0u);
}
