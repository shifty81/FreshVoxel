#include <gtest/gtest.h>
#include "sim/AlertStack.h"

class AlertStackTest : public ::testing::Test {
protected:
    fresh::sim::AlertStack stack{4};
};

TEST_F(AlertStackTest, Push_IncreasesCount) {
    stack.push("Test alert", "System");
    EXPECT_EQ(stack.activeCount(), 1u);
}

TEST_F(AlertStackTest, Tick_ExpiresAlerts) {
    stack.push("Expires", "System", fresh::sim::AlertPriority::Medium, 1.0f);
    stack.tick(1.5f);
    EXPECT_EQ(stack.activeCount(), 0u);
}

TEST_F(AlertStackTest, Dismiss_RemovesAlert) {
    uint32_t id = stack.push("Dismiss me", "System");
    EXPECT_TRUE(stack.dismiss(id));
    EXPECT_EQ(stack.activeCount(), 0u);
}

TEST_F(AlertStackTest, DismissCategory_RemovesAll) {
    stack.push("Alert 1", "Combat");
    stack.push("Alert 2", "Combat");
    stack.push("Alert 3", "System");

    size_t removed = stack.dismissCategory("Combat");
    EXPECT_EQ(removed, 2u);
    EXPECT_EQ(stack.activeCount(), 1u);
}

TEST_F(AlertStackTest, ActiveSortedByPriority) {
    stack.push("Low", "System", fresh::sim::AlertPriority::Low);
    stack.push("Critical", "System", fresh::sim::AlertPriority::Critical);
    stack.push("Medium", "System", fresh::sim::AlertPriority::Medium);

    auto active = stack.active();
    ASSERT_EQ(active.size(), 3u);
    EXPECT_EQ(active[0].priority, fresh::sim::AlertPriority::Critical);
    EXPECT_EQ(active[1].priority, fresh::sim::AlertPriority::Medium);
    EXPECT_EQ(active[2].priority, fresh::sim::AlertPriority::Low);
}

TEST_F(AlertStackTest, MaxAlerts_EvictsLowestPriority) {
    stack.push("High 1", "System", fresh::sim::AlertPriority::High);
    stack.push("High 2", "System", fresh::sim::AlertPriority::High);
    stack.push("High 3", "System", fresh::sim::AlertPriority::High);
    stack.push("High 4", "System", fresh::sim::AlertPriority::High);

    stack.push("Critical", "System", fresh::sim::AlertPriority::Critical);
    EXPECT_LE(stack.activeCount(), 4u);
}

TEST_F(AlertStackTest, Clear_RemovesAll) {
    stack.push("A", "System");
    stack.push("B", "System");
    stack.clear();
    EXPECT_EQ(stack.activeCount(), 0u);
}

TEST_F(AlertStackTest, ActiveInCategory_FiltersCorrectly) {
    stack.push("Combat 1", "Combat");
    stack.push("System 1", "System");
    stack.push("Combat 2", "Combat");

    auto combat = stack.activeInCategory("Combat");
    EXPECT_EQ(combat.size(), 2u);
}
