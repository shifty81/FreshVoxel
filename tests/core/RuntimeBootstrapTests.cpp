#include <gtest/gtest.h>
#include "core/RuntimeBootstrap.h"

class RuntimeBootstrapTest : public ::testing::Test {
protected:
    fresh::RuntimeBootstrap bootstrap;
};

TEST_F(RuntimeBootstrapTest, DefaultState_NotInitialized) {
    EXPECT_FALSE(bootstrap.isInitialized());
}

TEST_F(RuntimeBootstrapTest, Initialize_SetsInitialized) {
    EXPECT_TRUE(bootstrap.initialize(fresh::RuntimeMode::Game));
    EXPECT_TRUE(bootstrap.isInitialized());
}

TEST_F(RuntimeBootstrapTest, Initialize_SetsMode) {
    bootstrap.initialize(fresh::RuntimeMode::Editor);
    EXPECT_EQ(bootstrap.mode(), fresh::RuntimeMode::Editor);
}

TEST_F(RuntimeBootstrapTest, DoubleInitialize_ReturnsTrue) {
    bootstrap.initialize(fresh::RuntimeMode::Game);
    EXPECT_TRUE(bootstrap.initialize(fresh::RuntimeMode::Game));
}

TEST_F(RuntimeBootstrapTest, AllModes_InitializeSuccessfully) {
    fresh::RuntimeBootstrap b1, b2, b3;
    EXPECT_TRUE(b1.initialize(fresh::RuntimeMode::Editor));
    EXPECT_TRUE(b2.initialize(fresh::RuntimeMode::Game));
    EXPECT_TRUE(b3.initialize(fresh::RuntimeMode::Server));
}
