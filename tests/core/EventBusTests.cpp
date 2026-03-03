#include <gtest/gtest.h>
#include "core/EventBus.h"

class EventBusTest : public ::testing::Test {
protected:
    fresh::EventBus bus;
};

TEST_F(EventBusTest, SubscribeAndPublish_ReceivesEvent) {
    int received = 0;
    bus.subscribe("TEST", [&](const fresh::Event& e) {
        received++;
        EXPECT_EQ(e.type, "TEST");
    });

    bus.publish({"TEST"});
    EXPECT_EQ(received, 1);
}

TEST_F(EventBusTest, Unsubscribe_StopsReceiving) {
    int received = 0;
    auto id = bus.subscribe("TEST", [&](const fresh::Event&) { received++; });

    bus.publish({"TEST"});
    EXPECT_EQ(received, 1);

    bus.unsubscribe(id);
    bus.publish({"TEST"});
    EXPECT_EQ(received, 1);
}

TEST_F(EventBusTest, WildcardSubscription_ReceivesAllEvents) {
    int received = 0;
    bus.subscribe("*", [&](const fresh::Event&) { received++; });

    bus.publish({"EVENT_A"});
    bus.publish({"EVENT_B"});
    EXPECT_EQ(received, 2);
}

TEST_F(EventBusTest, EnqueueAndFlush_DefersDelivery) {
    int received = 0;
    bus.subscribe("DEFERRED", [&](const fresh::Event&) { received++; });

    bus.enqueue({"DEFERRED"});
    EXPECT_EQ(received, 0);
    EXPECT_EQ(bus.queueSize(), 1u);

    bus.flush();
    EXPECT_EQ(received, 1);
    EXPECT_EQ(bus.queueSize(), 0u);
}

TEST_F(EventBusTest, SubscriptionCount_TracksCorrectly) {
    EXPECT_EQ(bus.subscriptionCount(), 0u);

    auto id1 = bus.subscribe("A", [](const fresh::Event&) {});
    auto id2 = bus.subscribe("B", [](const fresh::Event&) {});
    EXPECT_EQ(bus.subscriptionCount(), 2u);

    bus.unsubscribe(id1);
    EXPECT_EQ(bus.subscriptionCount(), 1u);

    bus.unsubscribe(id2);
    EXPECT_EQ(bus.subscriptionCount(), 0u);
}

TEST_F(EventBusTest, Reset_ClearsEverything) {
    bus.subscribe("A", [](const fresh::Event&) {});
    bus.enqueue({"A"});

    bus.reset();
    EXPECT_EQ(bus.subscriptionCount(), 0u);
    EXPECT_EQ(bus.queueSize(), 0u);
    EXPECT_EQ(bus.totalPublished(), 0u);
}

TEST_F(EventBusTest, EventParameters_PassedCorrectly) {
    int64_t receivedInt = 0;
    double receivedFloat = 0.0;
    uint32_t receivedSender = 0;

    bus.subscribe("DATA", [&](const fresh::Event& e) {
        receivedInt = e.intParam;
        receivedFloat = e.floatParam;
        receivedSender = e.senderId;
    });

    fresh::Event evt;
    evt.type = "DATA";
    evt.senderId = 42;
    evt.intParam = 100;
    evt.floatParam = 3.14;
    bus.publish(evt);

    EXPECT_EQ(receivedSender, 42u);
    EXPECT_EQ(receivedInt, 100);
    EXPECT_DOUBLE_EQ(receivedFloat, 3.14);
}

TEST_F(EventBusTest, NullCallback_ReturnsZero) {
    auto id = bus.subscribe("TEST", nullptr);
    EXPECT_EQ(id, 0u);
    EXPECT_EQ(bus.subscriptionCount(), 0u);
}
