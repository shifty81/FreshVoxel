#include <gtest/gtest.h>
#include "networking/NetworkQualityMonitor.h"

class NetworkQualityMonitorTest : public ::testing::Test {
protected:
    fresh::networking::NetworkQualityMonitor monitor;
};

TEST_F(NetworkQualityMonitorTest, InitialValues) {
    EXPECT_FLOAT_EQ(monitor.getSmoothedRTT(), 0.0f);
    EXPECT_FLOAT_EQ(monitor.getJitter(), 0.0f);
    EXPECT_FLOAT_EQ(monitor.getPacketLossRate(), 0.0f);
}

TEST_F(NetworkQualityMonitorTest, RTT_FirstSample) {
    monitor.recordRTT(0.1f);
    EXPECT_FLOAT_EQ(monitor.getSmoothedRTT(), 0.1f);
}

TEST_F(NetworkQualityMonitorTest, RTT_EWMA) {
    monitor.recordRTT(0.1f);
    monitor.recordRTT(0.2f);
    EXPECT_GT(monitor.getSmoothedRTT(), 0.1f);
    EXPECT_LT(monitor.getSmoothedRTT(), 0.2f);
}

TEST_F(NetworkQualityMonitorTest, PacketLoss_DetectedFromGaps) {
    monitor.recordPacketArrival(1);
    monitor.recordPacketArrival(2);
    monitor.recordPacketArrival(5);
    EXPECT_GT(monitor.getPacketLossRate(), 0.0f);
}

TEST_F(NetworkQualityMonitorTest, NoLoss_ZeroRate) {
    for (uint32_t i = 0; i < 10; ++i) {
        monitor.recordPacketArrival(i);
    }
    EXPECT_FLOAT_EQ(monitor.getPacketLossRate(), 0.0f);
}

TEST_F(NetworkQualityMonitorTest, AdaptiveInterpolation_InRange) {
    monitor.recordRTT(0.05f);
    float interp = monitor.getAdaptiveInterpolationTime();
    EXPECT_GE(interp, fresh::networking::NetworkQualityMonitor::kMinInterpTime);
    EXPECT_LE(interp, fresh::networking::NetworkQualityMonitor::kMaxInterpTime);
}

TEST_F(NetworkQualityMonitorTest, Reset_ClearsState) {
    monitor.recordRTT(0.1f);
    monitor.recordPacketArrival(1);
    monitor.reset();
    EXPECT_FLOAT_EQ(monitor.getSmoothedRTT(), 0.0f);
    EXPECT_FLOAT_EQ(monitor.getPacketLossRate(), 0.0f);
}

TEST_F(NetworkQualityMonitorTest, NegativeRTT_Ignored) {
    monitor.recordRTT(-0.1f);
    EXPECT_FLOAT_EQ(monitor.getSmoothedRTT(), 0.0f);
}
