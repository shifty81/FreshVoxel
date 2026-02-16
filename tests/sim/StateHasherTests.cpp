#include <gtest/gtest.h>
#include "sim/StateHasher.h"

class StateHasherTest : public ::testing::Test {
protected:
    fresh::StateHasher hasher;

    void SetUp() override {
        hasher.reset(42);
    }
};

TEST_F(StateHasherTest, InitialState) {
    EXPECT_NE(hasher.currentHash(), 0u);
    EXPECT_EQ(hasher.currentTick(), 0u);
    EXPECT_TRUE(hasher.history().empty());
}

TEST_F(StateHasherTest, AdvanceTickUpdatesHash) {
    uint64_t hashBefore = hasher.currentHash();
    std::vector<uint8_t> state = {1, 2, 3};
    std::vector<uint8_t> inputs = {4, 5};
    hasher.advanceTick(1, state, inputs);

    EXPECT_NE(hasher.currentHash(), hashBefore);
    EXPECT_EQ(hasher.currentTick(), 1u);
    EXPECT_EQ(hasher.history().size(), 1u);
}

TEST_F(StateHasherTest, DeterministicHash) {
    std::vector<uint8_t> state = {10, 20, 30};
    std::vector<uint8_t> inputs = {1};

    fresh::StateHasher hasher2;
    hasher2.reset(42);

    hasher.advanceTick(1, state, inputs);
    hasher2.advanceTick(1, state, inputs);

    EXPECT_EQ(hasher.currentHash(), hasher2.currentHash());
}

TEST_F(StateHasherTest, DifferentSeedsDifferentHashes) {
    fresh::StateHasher hasher2;
    hasher2.reset(99);

    std::vector<uint8_t> state = {1};
    std::vector<uint8_t> inputs = {2};

    hasher.advanceTick(1, state, inputs);
    hasher2.advanceTick(1, state, inputs);

    EXPECT_NE(hasher.currentHash(), hasher2.currentHash());
}

TEST_F(StateHasherTest, FindDivergenceNoDivergence) {
    fresh::StateHasher hasher2;
    hasher2.reset(42);

    std::vector<uint8_t> state = {1, 2};
    std::vector<uint8_t> inputs = {3};

    hasher.advanceTick(1, state, inputs);
    hasher2.advanceTick(1, state, inputs);

    EXPECT_EQ(hasher.findDivergence(hasher2), -1);
}

TEST_F(StateHasherTest, FindDivergenceDetectsDifference) {
    fresh::StateHasher hasher2;
    hasher2.reset(42);

    std::vector<uint8_t> state1 = {1, 2};
    std::vector<uint8_t> inputs1 = {3};

    // Both advance tick 1 the same
    hasher.advanceTick(1, state1, inputs1);
    hasher2.advanceTick(1, state1, inputs1);

    // Tick 2 diverges
    std::vector<uint8_t> stateA = {4, 5};
    std::vector<uint8_t> stateB = {4, 6}; // different!

    hasher.advanceTick(2, stateA, inputs1);
    hasher2.advanceTick(2, stateB, inputs1);

    EXPECT_EQ(hasher.findDivergence(hasher2), 2);
}

TEST_F(StateHasherTest, ResetClearsHistory) {
    std::vector<uint8_t> state = {1};
    std::vector<uint8_t> inputs = {2};
    hasher.advanceTick(1, state, inputs);
    EXPECT_EQ(hasher.history().size(), 1u);

    hasher.reset(0);
    EXPECT_TRUE(hasher.history().empty());
    EXPECT_EQ(hasher.currentTick(), 0u);
}

TEST_F(StateHasherTest, HashCombineStatic) {
    uint8_t data[] = {0xAA, 0xBB};
    uint64_t h = fresh::StateHasher::hashCombine(0, data, 2);
    EXPECT_NE(h, 0u);

    // Same data, same result
    uint64_t h2 = fresh::StateHasher::hashCombine(0, data, 2);
    EXPECT_EQ(h, h2);
}
