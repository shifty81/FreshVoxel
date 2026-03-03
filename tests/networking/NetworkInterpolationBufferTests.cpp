#include <gtest/gtest.h>
#include "networking/NetworkInterpolationBuffer.h"

class NetworkInterpolationBufferTest : public ::testing::Test {
protected:
    fresh::networking::NetworkInterpolationBuffer buffer;
};

TEST_F(NetworkInterpolationBufferTest, EmptyBuffer_ReturnsInvalid) {
    auto state = buffer.interpolate(1, 0.0f);
    EXPECT_FALSE(state.valid);
}

TEST_F(NetworkInterpolationBufferTest, SingleSnapshot_ReturnsPosition) {
    fresh::networking::EntitySnapshot snap{1, 10, 100.0f, 200.0f, 300.0f, 0, 0, 0, 45.0f};
    buffer.pushSnapshot(snap);

    auto state = buffer.interpolate(1, 10.0f);
    EXPECT_TRUE(state.valid);
    EXPECT_FLOAT_EQ(state.posX, 100.0f);
    EXPECT_FLOAT_EQ(state.posY, 200.0f);
    EXPECT_FLOAT_EQ(state.posZ, 300.0f);
}

TEST_F(NetworkInterpolationBufferTest, TwoSnapshots_Interpolates) {
    buffer.pushSnapshot({1, 10, 0.0f, 0.0f, 0.0f, 0, 0, 0, 0.0f});
    buffer.pushSnapshot({1, 20, 100.0f, 0.0f, 0.0f, 0, 0, 0, 0.0f});

    auto state = buffer.interpolate(1, 15.0f);
    EXPECT_TRUE(state.valid);
    EXPECT_NEAR(state.posX, 50.0f, 0.01f);
}

TEST_F(NetworkInterpolationBufferTest, Extrapolation_UsesVelocity) {
    buffer.pushSnapshot({1, 10, 0.0f, 0.0f, 0.0f, 10.0f, 0.0f, 0.0f, 0.0f});

    auto state = buffer.interpolate(1, 12.0f);
    EXPECT_TRUE(state.valid);
    EXPECT_NEAR(state.posX, 20.0f, 0.01f);
}

TEST_F(NetworkInterpolationBufferTest, BeyondMaxExtrapolation_ReturnsInvalid) {
    buffer.pushSnapshot({1, 10, 0.0f, 0.0f, 0.0f, 0, 0, 0, 0.0f});

    auto state = buffer.interpolate(1, 20.0f);
    EXPECT_FALSE(state.valid);
}

TEST_F(NetworkInterpolationBufferTest, RemoveEntity_ClearsData) {
    buffer.pushSnapshot({1, 10, 0.0f, 0.0f, 0.0f, 0, 0, 0, 0.0f});
    EXPECT_EQ(buffer.trackedEntityCount(), 1u);

    buffer.removeEntity(1);
    EXPECT_EQ(buffer.trackedEntityCount(), 0u);
}
