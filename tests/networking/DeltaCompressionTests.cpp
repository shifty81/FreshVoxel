#include <gtest/gtest.h>
#include "networking/DeltaCompression.h"

class DeltaCompressionTest : public ::testing::Test {
protected:
    fresh::networking::DeltaCompression dc{10};
};

TEST_F(DeltaCompressionTest, Quantize_RoundTrip) {
    float val = 123.456f;
    int32_t q = fresh::networking::DeltaCompression::quantizePosition(val);
    float dq = fresh::networking::DeltaCompression::dequantizePosition(q);
    EXPECT_NEAR(dq, val, 0.01f);
}

TEST_F(DeltaCompressionTest, FirstEncode_IsKeyframe) {
    std::vector<fresh::networking::EntitySnapshot> snapshots = {
        {1, 1, 10.0f, 20.0f, 30.0f, 0, 0, 0, 90.0f}
    };

    auto frame = dc.encode(snapshots);
    EXPECT_EQ(frame.entries.size(), 1u);
    EXPECT_EQ(frame.entries[0].frameType, fresh::networking::FrameType::Keyframe);
}

TEST_F(DeltaCompressionTest, SecondEncode_IsDelta) {
    dc.encode({{1, 1, 10.0f, 20.0f, 30.0f, 0, 0, 0, 90.0f}});

    auto frame2 = dc.encode({{1, 2, 11.0f, 21.0f, 31.0f, 0, 0, 0, 91.0f}});
    EXPECT_EQ(frame2.entries[0].frameType, fresh::networking::FrameType::Delta);
}

TEST_F(DeltaCompressionTest, EncodeDecodeRoundTrip) {
    std::vector<fresh::networking::EntitySnapshot> original = {
        {1, 1, 10.5f, 20.5f, 30.5f, 1.0f, 2.0f, 3.0f, 45.0f}
    };

    auto frame = dc.encode(original);

    fresh::networking::DeltaCompression decoder{10};
    auto decoded = decoder.decode(frame);

    ASSERT_EQ(decoded.size(), 1u);
    EXPECT_NEAR(decoded[0].posX, 10.5f, 0.02f);
    EXPECT_NEAR(decoded[0].posY, 20.5f, 0.02f);
    EXPECT_NEAR(decoded[0].posZ, 30.5f, 0.02f);
    EXPECT_NEAR(decoded[0].rotYaw, 45.0f, 0.2f);
}

TEST_F(DeltaCompressionTest, ForceKeyframe) {
    dc.encode({{1, 1, 0, 0, 0, 0, 0, 0, 0}});
    dc.forceKeyframe(1);
    auto frame = dc.encode({{1, 2, 0, 0, 0, 0, 0, 0, 0}});
    EXPECT_EQ(frame.entries[0].frameType, fresh::networking::FrameType::Keyframe);
}
