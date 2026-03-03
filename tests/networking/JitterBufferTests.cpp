#include <gtest/gtest.h>
#include "networking/JitterBuffer.h"

class JitterBufferTest : public ::testing::Test {
protected:
    fresh::networking::JitterBuffer jb{0.1f, 64, false};
};

TEST_F(JitterBufferTest, EmptyBuffer_FlushReturnsEmpty) {
    auto result = jb.flush(1.0f);
    EXPECT_TRUE(result.empty());
}

TEST_F(JitterBufferTest, PushAndFlush_ReleasesAfterDelay) {
    jb.push(1, 0.0f, {1, 2, 3});
    auto early = jb.flush(0.05f);
    EXPECT_TRUE(early.empty());

    auto ready = jb.flush(0.15f);
    EXPECT_EQ(ready.size(), 1u);
    EXPECT_EQ(ready[0].tick, 1u);
}

TEST_F(JitterBufferTest, LatePackets_Dropped) {
    jb.push(5, 0.0f, {});
    jb.flush(0.2f);

    jb.push(3, 0.3f, {});
    EXPECT_EQ(jb.totalDropped(), 1u);
}

TEST_F(JitterBufferTest, Reset_ClearsState) {
    jb.push(1, 0.0f, {});
    jb.reset();
    EXPECT_EQ(jb.bufferedCount(), 0u);
    EXPECT_EQ(jb.totalPushed(), 0u);
    EXPECT_EQ(jb.totalDropped(), 0u);
}

TEST_F(JitterBufferTest, TickOrdering_MaintainedOnFlush) {
    jb.push(3, 0.0f, {});
    jb.push(1, 0.01f, {});
    jb.push(2, 0.02f, {});

    auto ready = jb.flush(0.2f);
    ASSERT_EQ(ready.size(), 3u);
    EXPECT_EQ(ready[0].tick, 1u);
    EXPECT_EQ(ready[1].tick, 2u);
    EXPECT_EQ(ready[2].tick, 3u);
}
