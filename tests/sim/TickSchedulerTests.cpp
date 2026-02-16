#include <gtest/gtest.h>
#include "sim/TickScheduler.h"

class TickSchedulerTest : public ::testing::Test {
protected:
    fresh::TickScheduler scheduler;
};

TEST_F(TickSchedulerTest, DefaultTickRate) {
    EXPECT_EQ(scheduler.tickRate(), 30u);
}

TEST_F(TickSchedulerTest, SetTickRate) {
    scheduler.setTickRate(60);
    EXPECT_EQ(scheduler.tickRate(), 60u);
}

TEST_F(TickSchedulerTest, ZeroTickRateClampedToOne) {
    scheduler.setTickRate(0);
    EXPECT_EQ(scheduler.tickRate(), 1u);
}

TEST_F(TickSchedulerTest, FixedDeltaTime) {
    scheduler.setTickRate(60);
    EXPECT_FLOAT_EQ(scheduler.fixedDeltaTime(), 1.0f / 60.0f);
}

TEST_F(TickSchedulerTest, TickIncrements) {
    scheduler.setFramePacing(false); // disable sleep for fast testing
    EXPECT_EQ(scheduler.currentTick(), 0u);
    scheduler.tick([](float) {});
    EXPECT_EQ(scheduler.currentTick(), 1u);
    scheduler.tick([](float) {});
    EXPECT_EQ(scheduler.currentTick(), 2u);
}

TEST_F(TickSchedulerTest, TickCallbackReceivesDeltaTime) {
    scheduler.setTickRate(30);
    scheduler.setFramePacing(false);
    float receivedDt = 0.0f;
    scheduler.tick([&](float dt) { receivedDt = dt; });
    EXPECT_FLOAT_EQ(receivedDt, 1.0f / 30.0f);
}

TEST_F(TickSchedulerTest, FramePacingDefault) {
    EXPECT_TRUE(scheduler.framePacingEnabled());
}

TEST_F(TickSchedulerTest, SetFramePacing) {
    scheduler.setFramePacing(false);
    EXPECT_FALSE(scheduler.framePacingEnabled());
    scheduler.setFramePacing(true);
    EXPECT_TRUE(scheduler.framePacingEnabled());
}
